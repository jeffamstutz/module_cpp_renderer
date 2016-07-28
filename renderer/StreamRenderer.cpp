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

#define FULL_TILE 1 // NOTE(jda) - enable to call renderStream()

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

      const auto begin = jobID * RENDERTILE_PIXELS_PER_JOB;
      const auto end   = begin + RENDERTILE_PIXELS_PER_JOB;
      const auto startSampleID = max(tile.accumID, 0)*spp;

      static std::uniform_real_distribution<float> distribution {0.f, 1.f};

      ScreenSampleStream screenSamples;
      CameraSampleStream cameraSamples;

      for (auto i = begin; i < end; ++i) {
        const int streamID = i - begin;

        ScreenSample &screenSample = screenSamples[streamID];
        screenSample.sampleID.x = tile.region.lower.x + z_order.xs[i];
        screenSample.sampleID.y = tile.region.lower.y + z_order.ys[i];
        screenSample.sampleID.z = startSampleID;

        auto &sampleID = screenSample.sampleID;
        if ((sampleID.x >= fbw) || (sampleID.y >= fbh))
          continue;

        screenSample.tileOffset = z_order.xs[i] + (z_order.ys[i] * TILE_SIZE);

        float tMax = inf;

        // NOTE(jda) - This assumes spp = 1
        float pixel_du = distribution(generator);
        float pixel_dv = distribution(generator);
        screenSample.sampleID.z = startSampleID;

        CameraSample &cameraSample = cameraSamples[streamID];
        cameraSample.screen.x = (screenSample.sampleID.x + pixel_du) *
                                rcp(float(fbw));
        cameraSample.screen.y = (screenSample.sampleID.y + pixel_dv) *
                                rcp(float(fbh));

        cameraSample.lens.x = distribution(generator);
        cameraSample.lens.y = distribution(generator);

        auto &ray = screenSample.ray;
        currentCamera->getRay(cameraSample, ray);
        ray.t = tMax;
      }

      renderStream(perFrameData, screenSamples);

      for (auto &screenSample : screenSamples) {
        if (screenSample.tileOffset < 0)
          continue;

        auto &rgb   = screenSample.rgb;
        auto &z     = screenSample.z;
        auto &alpha = screenSample.alpha;

        rgb *= spp_inv;

        const auto &pixel = screenSample.tileOffset;
        tile.r[pixel] = rgb.x;
        tile.g[pixel] = rgb.y;
        tile.b[pixel] = rgb.z;
        tile.a[pixel] = alpha;
        tile.z[pixel] = z;
      }
    }

  }// namespace cpp_renderer
}// namespace ospray
