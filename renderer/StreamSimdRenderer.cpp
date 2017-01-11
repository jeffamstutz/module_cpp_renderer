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
#include "StreamSimdRenderer.h"
#include "../util.h"

namespace ospray {
  namespace cpp_renderer {

    std::string StreamSimdRenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamRenderer";
    }

    void StreamSimdRenderer::renderTile(void *perFrameData,
                                        Tile &tile,
                                        size_t jobID) const
    {
      const float spp_inv = 1.f / spp;

      const int fbw = currentFB->size.x;
      const int fbh = currentFB->size.y;

      const auto startSampleID = ospcommon::max(tile.accumID, 0)*spp;

      static std::uniform_real_distribution<float> distribution {0.f, 1.f};

      constexpr int STREAM_ITERATIONS =
                      (RENDERTILE_PIXELS_PER_JOB / simd::width) /
                      SIMD_STREAM_SIZE;

      for (auto j = 0; j < STREAM_ITERATIONS; ++j) {

        ScreenSampleNStream screenSamples;
        CameraSampleNStream cameraSamples;
        const auto begin = (jobID * RENDERTILE_PIXELS_PER_JOB / simd::width)
                           + (j * SIMD_STREAM_SIZE);
        const auto end   = begin + SIMD_STREAM_SIZE;

        mask_stream masks;

        for (auto i = begin; i < end; ++i) {
          const int streamID = i - begin;

          auto &sampleID = screenSamples.sampleID[streamID];
          auto tile_x = simd::load<simd::vint>(&z_order.xs[i]);
          auto tile_y = simd::load<simd::vint>(&z_order.ys[i]);
          sampleID.x = tile.region.lower.x + tile_x;
          sampleID.y = tile.region.lower.y + tile_y;
          auto &tileOffset = screenSamples.tileOffset[streamID];
          tileOffset = -1;
          resetRay(screenSamples.rays, streamID);

          auto &active = masks[streamID];
          active = (sampleID.x < fbw) && (sampleID.y < fbh);

          if (simd::none(active))
            continue;

          tileOffset = tile_x + (tile_y * TILE_SIZE);
          float tMax = inf;

          // NOTE(jda) - This assumes spp = 1
          auto pixel_du = simd::randUniformDist();
          auto pixel_dv = simd::randUniformDist();
          sampleID.z = startSampleID;

          auto &cameraSample = cameraSamples[streamID];
          cameraSample.screen.x =
              (simd::cast<simd::vfloat>(sampleID.x) + pixel_du)
               * rcp(float(fbw));
          cameraSample.screen.y =
              (simd::cast<simd::vfloat>(sampleID.y) + pixel_dv)
              * rcp(float(fbh));

#if 0
          cameraSample.lens.x = simd::randUniformDist();
          cameraSample.lens.y = simd::randUniformDist();
#endif

          auto &ray = screenSamples.rays[streamID];
          currentCameraN->getRay(cameraSample, ray);
          ray.t = tMax;
        }

        renderStream(masks, perFrameData, screenSamples);

        auto writeTile = [&](const simd::vmaski &active,
                             ScreenSampleNRef sample)
        {
          sample.rgb *= simd::vfloat{spp_inv};

          const auto &tileOffset = sample.tileOffset;
#if 0 // NOTE(jda) - adding the active mask seems to mask out ALL lanes...
          simd::store(sample.rgb.x, (float*)tile.r, tileOffset, active);
          simd::store(sample.rgb.y, (float*)tile.g, tileOffset, active);
          simd::store(sample.rgb.z, (float*)tile.b, tileOffset, active);
          simd::store(sample.alpha, (float*)tile.a, tileOffset, active);
          simd::store(sample.z    , (float*)tile.z, tileOffset, active);
#else
          UNUSED(active);
          simd::store(sample.rgb.x, (float*)tile.r, tileOffset);
          simd::store(sample.rgb.y, (float*)tile.g, tileOffset);
          simd::store(sample.rgb.z, (float*)tile.b, tileOffset);
          simd::store(sample.alpha, (float*)tile.a, tileOffset);
          simd::store(sample.z    , (float*)tile.z, tileOffset);
#endif
        };

        for_each_sample(screenSamples, writeTile, sampleEnabledN);
      }
    }

    void StreamSimdRenderer::renderSample(simd::vmaski active,
                                          void *perFrameData,
                                          ScreenSampleN &sample) const
    {
      UNUSED(perFrameData, sample);
      throw std::runtime_error("Type Mismatch: calling renderSample() in a"
                               " stream renderer...");
    }

  }// namespace cpp_renderer
}// namespace ospray
