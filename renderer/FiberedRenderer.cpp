// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

// ospray
#include "FiberedRenderer.h"
#include "../util.h"
#include "../common/concurrent_async.h"
#include "../common/concurrent_for.h"

#include "ospcommon/tasking/parallel_for.h"
#include "ospray/render/LoadBalancer.h"

#include <random>

static thread_local std::minstd_rand generator;

namespace ospray {
  namespace cpp_renderer {

    std::string FiberedRenderer::toString() const
    {
      return "ospray::cpp_renderer::Renderer";
    }

    void FiberedRenderer::commit()
    {
      ospray::Renderer::commit();
      currentCamera = dynamic_cast<Camera*>(getParamObject("camera"));
      bgColor       = getParam3f("bgColor", vec3f(1.f));
      precomputeZOrder();
    }

    void *FiberedRenderer::beginFrame(FrameBuffer *fb)
    {
      currentFB = fb;
      fb->beginFrame();

      if (currentCamera == nullptr) {
        throw std::runtime_error("You are using a C++ only renderer without"
                                 " using a C++ only camera!");
      }

      return nullptr;
    }

    void FiberedRenderer::renderTile(void *perFrameData,
                                     Tile &tile,
                                     size_t jobID) const
    {
      const float spp_inv = 1.f / spp;

      const auto begin = jobID * RENDERTILE_PIXELS_PER_JOB;
      const auto end   = begin + RENDERTILE_PIXELS_PER_JOB;
      const auto startSampleID = ospcommon::max(tile.accumID, 0)*spp;

      static std::uniform_real_distribution<float> distribution {0.f, 1.f};

      std::vector<Ray*> ray_bundle;
      ray_bundle.reserve(RENDERTILE_PIXELS_PER_JOB);

      auto fiber = concurrent_async([&]() {
        while (true) {
          boost::this_fiber::yield();

          if (ray_bundle.empty())
            return;

          RTCIntersectContext ctx{RTC_INTERSECT_INCOHERENT, nullptr};
#if 0
          rtcIntersect1Mp(model->embreeSceneHandle, &ctx,
                          (RTCRay**)ray_bundle.data(), ray_bundle.size());
#else
          return;
#endif
        }
      });

      //////////////////////////////////
      //TODO: don't hijack perFrameData!
      //////////////////////////////////
      perFrameData = &ray_bundle;

      concurrent_for(RENDERTILE_PIXELS_PER_JOB, [&](int fiberID) {
        const int i = fiberID + begin;

        ScreenSample screenSample;
        screenSample.sampleID.x = tile.region.lower.x + z_order.xs[i];
        screenSample.sampleID.y = tile.region.lower.y + z_order.ys[i];
        screenSample.sampleID.z = startSampleID;

        auto &sampleID = screenSample.sampleID;

        if ((sampleID.x >= currentFB->size.x) ||
            (sampleID.y >= currentFB->size.y))
          return;

        float tMax = inf;
#if 0
        // set ray t value for early ray termination if we have a maximum depth
        // texture
        if (self->maxDepthTexture) {
          // always sample center of pixel
          vec2f depthTexCoord;
          depthTexCoord.x = (screenSample.sampleID.x + 0.5f) * fb->rcpSize.x;
          depthTexCoord.y = (screenSample.sampleID.y + 0.5f) * fb->rcpSize.y;

          tMax = min(get1f(self->maxDepthTexture, depthTexCoord), infinity);
        }
#endif

        for (int s = 0; s < spp; s++) {
          float pixel_du = distribution(generator);
          float pixel_dv = distribution(generator);
          screenSample.sampleID.z = startSampleID+s;

          CameraSample cameraSample;
          cameraSample.screen.x = (screenSample.sampleID.x + pixel_du) *
                                  rcp(float(currentFB->size.x));
          cameraSample.screen.y = (screenSample.sampleID.y + pixel_dv) *
                                  rcp(float(currentFB->size.y));

          cameraSample.lens.x = distribution(generator);
          cameraSample.lens.y = distribution(generator);

          auto &ray = screenSample.ray;
          currentCamera->getRay(cameraSample, ray);
          ray.t = tMax;

          renderSample(perFrameData, screenSample);

          auto &rgb   = screenSample.rgb;
          auto &z     = screenSample.z;
          auto &alpha = screenSample.alpha;

          rgb *= spp_inv;

          const auto pixel = z_order.xs[i] + (z_order.ys[i] * TILE_SIZE);
          tile.r[pixel] = rgb.x;
          tile.g[pixel] = rgb.y;
          tile.b[pixel] = rgb.z;
          tile.a[pixel] = alpha;
          tile.z[pixel] = z;
        }
      });

      fiber.join();
    }

    void FiberedRenderer::endFrame(void *perFrameData,
                                   const int32 fbChannelFlags)
    {
      UNUSED(perFrameData, fbChannelFlags);
      // NOTE(jda) - override to *not* run default behavior
    }

  }// namespace cpp_renderer
}// namespace ospray
