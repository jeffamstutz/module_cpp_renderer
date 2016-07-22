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
#include "RaycastRenderer.h"
#include "util.h"

namespace ospray {
  namespace cpp_renderer {

    std::string RaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::RaycastRenderer";
    }

    void RaycastRenderer::renderSample(void */*perFrameData*/,
                                       ScreenSample &screenSample) const
    {
      auto &ray = screenSample.ray;
      auto &rgb   = screenSample.rgb;
      auto &z     = screenSample.z;
      auto &alpha = screenSample.alpha;

      traceRay(ray);

      if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
        const float c = 0.2f + 0.8f * abs(dot(normalize(ray.Ng), ray.dir));
        rgb.x = c;
        rgb.y = c;
        rgb.z = c;
        z     = ray.t;
        alpha = 1.f;
      }
    }

    OSP_REGISTER_RENDERER(RaycastRenderer, cpp_raycast)

    extern "C" void ospray_init_module_cpp()
    {
      printf("Loaded plugin 'cpp' ...\n");
    }

  }// namespace cpp_renderer
}// namespace ospray
