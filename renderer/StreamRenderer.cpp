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
#include "StreamRenderer.h"
#include "../util.h"

#include <random>

static thread_local std::minstd_rand generator;

namespace ospray {
  namespace cpp_renderer {

    std::string StreamRenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamRenderer";
    }

    void StreamRenderer::renderTile(void *perFrameData,
                                    Tile &tile,
                                    size_t jobID) const
    {
      const float spp_inv = 1.f / spp;

      const int fbw = currentFB->size.x;
      const int fbh = currentFB->size.y;

      const auto startSampleID = max(tile.accumID, 0)*spp;

      static std::uniform_real_distribution<float> distribution {0.f, 1.f};

      constexpr int STREAM_ITERATIONS = RENDERTILE_PIXELS_PER_JOB / STREAM_SIZE;

      for (auto j = 0; j < STREAM_ITERATIONS; ++j) {

        ScreenSampleStream screenSamples;
        CameraSampleStream cameraSamples;
        const auto begin = jobID * RENDERTILE_PIXELS_PER_JOB + j * STREAM_SIZE;
        const auto end   = begin + STREAM_SIZE;

        for (auto i = begin; i < end; ++i) {
          const int streamID = i - begin;

          auto &sampleID = screenSamples.sampleID[streamID];
          sampleID.x = tile.region.lower.x + z_order.xs[i];
          sampleID.y = tile.region.lower.y + z_order.ys[i];
          auto &tileOffset = screenSamples.tileOffset[streamID];
          tileOffset = -1;
          resetRay(screenSamples.rays, streamID);

          if ((sampleID.x >= fbw) || (sampleID.y >= fbh))
            continue;

          tileOffset = z_order.xs[i] + (z_order.ys[i] * TILE_SIZE);
          float tMax = inf;

          // NOTE(jda) - This assumes spp = 1
          float pixel_du = distribution(generator);
          float pixel_dv = distribution(generator);
          sampleID.z = startSampleID;

          CameraSample &cameraSample = cameraSamples[streamID];
          cameraSample.screen.x = (sampleID.x + pixel_du) * rcp(float(fbw));
          cameraSample.screen.y = (sampleID.y + pixel_dv) * rcp(float(fbh));

          cameraSample.lens.x = distribution(generator);
          cameraSample.lens.y = distribution(generator);

          auto &ray = screenSamples.rays[streamID];
          currentCamera->getRay(cameraSample, ray);
          ray.t = tMax;
        }

        renderStream(perFrameData, screenSamples);

        auto writeTile = [&](ScreenSampleRef sample)
        {
          sample.rgb *= spp_inv;

          const auto tileOffset = sample.tileOffset;
          tile.r[tileOffset] = sample.rgb.x;
          tile.g[tileOffset] = sample.rgb.y;
          tile.b[tileOffset] = sample.rgb.z;
          tile.a[tileOffset] = sample.alpha;
          tile.z[tileOffset] = sample.z;
        };

        for_each_sample(screenSamples, writeTile, sampleEnabled);
      }
    }

    void StreamRenderer::renderSample(void *perFrameData,
                                      ScreenSample &sample) const
    {
      UNUSED(perFrameData, sample);
      throw std::runtime_error("Type Mismatch: calling renderSample() in a"
                               " stream renderer...");
    }

  }// namespace cpp_renderer
}// namespace ospray
