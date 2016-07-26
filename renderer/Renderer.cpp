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
#include "Renderer.h"
#include "../util.h"
#include "common/tasking/parallel_for.h"

namespace ospray {
  namespace cpp_renderer {

    std::string Renderer::toString() const
    {
      return "ospray::cpp_renderer::Renderer";
    }

    void Renderer::commit()
    {
      ospray::Renderer::commit();
      precomputeZOrder();
      currentCamera = dynamic_cast<Camera*>(getParamObject("camera"));
      assert(currentCamera);
    }

    void *Renderer::beginFrame(FrameBuffer *fb)
    {
      currentFB = fb;
      fb->beginFrame();
      return nullptr;
    }

    void Renderer::renderTile(void *perFrameData,
                              Tile &tile,
                              size_t jobID) const
    {
      float pixel_du = .5f;
      float pixel_dv = .5f;

      const float spp_inv = 1.f / spp;

      const auto begin = jobID * RENDERTILE_PIXELS_PER_JOB;
      const auto end   = begin + RENDERTILE_PIXELS_PER_JOB;
      const auto startSampleID = max(tile.accumID, 0)*spp;

      for (auto i = begin; i < end; ++i) {
        ScreenSample screenSample;
        screenSample.sampleID.x = tile.region.lower.x + z_order.xs[i];
        screenSample.sampleID.y = tile.region.lower.y + z_order.ys[i];
        screenSample.sampleID.z = startSampleID;

        auto &sampleID = screenSample.sampleID;

        if ((sampleID.x >= currentFB->size.x) ||
            (sampleID.y >= currentFB->size.y))
          continue;

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

        for (uint32 s = 0; s < spp; s++) {
          pixel_du = precomputedHalton2(startSampleID+s);
          pixel_dv = precomputedHalton3(startSampleID+s);
          screenSample.sampleID.z = startSampleID+s;

          CameraSample cameraSample;
          cameraSample.screen.x = (screenSample.sampleID.x + pixel_du) *
                                  rcp(float(currentFB->size.x));
          cameraSample.screen.y = (screenSample.sampleID.y + pixel_dv) *
                                  rcp(float(currentFB->size.y));

          // TODO: fix correlations / better RNG
          cameraSample.lens.x = precomputedHalton3(startSampleID+s);
          cameraSample.lens.y = precomputedHalton5(startSampleID+s);

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
      }
    }

    void Renderer::endFrame(void */*perFrameData*/,
                            const int32 /*fbChannelFlags*/)
    {
      // NOTE(jda) - override to *not* run default behavior
    }

  }// namespace cpp_renderer
}// namespace ospray
