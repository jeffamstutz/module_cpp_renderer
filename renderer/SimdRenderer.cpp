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
#include "SimdRenderer.h"
#include "../util.h"

namespace ospray {
  namespace cpp_renderer {

    // SimdRenderer definitions ///////////////////////////////////////////////

    std::string SimdRenderer::toString() const
    {
      return "ospray::cpp_renderer::SimdRenderer";
    }

    void SimdRenderer::commit()
    {
      ospray::cpp_renderer::Renderer::commit();
      currentCameraN = dynamic_cast<CameraN*>(getParamObject("camera"));
    }

    void *SimdRenderer::beginFrame(FrameBuffer *fb)
    {
      currentFB = fb;
      fb->beginFrame();

      if (currentCameraN == nullptr) {
        throw std::runtime_error("You are using a C++ simd renderer without"
                                 " using a C++ simd camera!");
      }

      return nullptr;
    }

    void SimdRenderer::renderTile(void *perFrameData,
                                  Tile &tile,
                                  size_t jobID) const
    {
      const float spp_inv = 1.f / spp;

      const auto begin = jobID * RENDERTILE_PIXELS_PER_JOB;
      const auto end   = begin + RENDERTILE_PIXELS_PER_JOB;
      const auto startSampleID = max(tile.accumID, 0)*spp;

      for (auto i = begin; i < end; i += simd::width) {
        ScreenSampleN screenSample;

        // NOTE(jda) - THIS MIGHT BE BROKEN...
        screenSample.sampleID.x = tile.region.lower.x +
                                  simd::load<simd::vint>(&z_order.xs[i]);
        screenSample.sampleID.y = tile.region.lower.y +
                                  simd::load<simd::vint>(&z_order.ys[i]);
        screenSample.sampleID.z = startSampleID;

        const auto &sampleID = screenSample.sampleID;

        auto active = (sampleID.x < simd::vint{currentFB->size.x}) ||
                      (sampleID.y < simd::vint{currentFB->size.y});

        if (!simd::any(active))
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

        for (int s = 0; s < spp; s++) {
          auto du = simd::randUniformDist();
          auto dv = simd::randUniformDist();
          screenSample.sampleID.z = startSampleID + s;

          CameraSampleN cameraSample;

          du += simd::cast<simd::vfloat>(screenSample.sampleID.x);
          dv += simd::cast<simd::vfloat>(screenSample.sampleID.y);
          cameraSample.screen.x = du * (1.f / currentFB->size.x);
          cameraSample.screen.y = dv * (1.f / currentFB->size.y);

          cameraSample.lens.x = simd::randUniformDist();
          cameraSample.lens.y = simd::randUniformDist();

          auto &ray = screenSample.ray;
          currentCameraN->getRay(cameraSample, ray);
          ray.t = tMax;

          renderSample(perFrameData, screenSample);

          auto &rgb   = screenSample.rgb;
          auto &z     = screenSample.z;
          auto &alpha = screenSample.alpha;

          rgb *= simd::vfloat{spp_inv};

          simd::foreach_active(active, [&](int j) {
            const auto pixel = z_order.xs[i+j] + (z_order.ys[i+j] * TILE_SIZE);
            tile.r[pixel] = rgb.x[j];
            tile.g[pixel] = rgb.y[j];
            tile.b[pixel] = rgb.z[j];
            tile.a[pixel] = alpha[j];
            tile.z[pixel] = z[j];
          });
        }
      }
    }

    void SimdRenderer::renderSample(void *perFrameData,
                                    ScreenSample &sample) const
    {
      UNUSED(perFrameData, sample);
      throw std::runtime_error("Type Mismatch: calling wrong renderSample() fcn"
                               " simd renderer...");
    }

  }// namespace cpp_renderer
}// namespace ospray
