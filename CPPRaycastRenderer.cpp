// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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
#include "CPPRaycastRenderer.h"

namespace ospray {
  namespace cpp_renderer {

    std::string RaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::RaycastRenderer";
    }

    void RaycastRenderer::commit()
    {
      Renderer::commit();
      currentCamera = dynamic_cast<Camera*>(getParamObject("camera"));
      assert(currentCamera);
    }

    void *RaycastRenderer::beginFrame(FrameBuffer *fb)
    {
      currentFB = fb;
      return nullptr;
    }

    void RaycastRenderer::renderTile(void */*perFrameData*/,
                                        Tile &tile,
                                        size_t /*jobID*/) const
    {
      float pixel_du = .5f;
      float pixel_dv = .5f;

      for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
          ScreenSample screenSample;
          screenSample.sampleID.x = tile.region.lower.x + x;
          screenSample.sampleID.y = tile.region.lower.y + y;
          screenSample.sampleID.z = 0;

          CameraSample cameraSample;
          cameraSample.screen.x = (screenSample.sampleID.x + pixel_du) *
                                  rcp(float(currentFB->size.x));
          cameraSample.screen.y = (screenSample.sampleID.y + pixel_dv) *
                                  rcp(float(currentFB->size.y));

          auto &ray = screenSample.ray;
          currentCamera->getRay(cameraSample, ray);
          traceRay(ray);

          auto &rgb   = screenSample.rgb;
          auto &z     = screenSample.z;
          auto &alpha = screenSample.alpha;

          if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
            const float c = 0.2f + 0.8f * abs(dot(normalize(ray.Ng), ray.dir));
            rgb.x = c;
            rgb.y = c;
            rgb.z = c;
            z     = ray.t;
            alpha = 1.f;
          }

          int i = y * TILE_SIZE + x;
          tile.r[i] = rgb.x;
          tile.g[i] = rgb.y;
          tile.b[i] = rgb.z;
          tile.a[i] = alpha;
          tile.z[i] = z;
        }
      }
    }

    void RaycastRenderer::endFrame(void */*perFrameData*/,
                                      const int32 /*fbChannelFlags*/)
    {
      // NOTE(jda) - override to *not* run default behavior
    }

    OSP_REGISTER_RENDERER(RaycastRenderer, cpp_raycast)

    extern "C" void ospray_init_module_cpp()
    {
      printf("Loaded plugin 'cpp' ...\n");
    }

  }// namespace cpp_renderer
}// namespace ospray
