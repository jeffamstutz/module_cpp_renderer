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
#include "Raycast.h"
#include "../util.h"

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

      traceRay(ray);

      if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
        const float c = 0.2f + 0.8f * abs(dot(normalize(ray.Ng), ray.dir));
#if 1
        screenSample.rgb = vec3f{c};
#else
        screenSample.rgb = c * make_random_color(ray.primID);
#endif
        screenSample.z     = ray.t;
        screenSample.alpha = 1.f;
      } else {
        screenSample.rgb = bgColor;
      }
    }

    OSP_REGISTER_RENDERER(RaycastRenderer, cpp_raycast)

    extern "C" void ospray_init_module_cpp()
    {
      printf("Loaded plugin 'cpp' ...\n");
    }

  }// namespace cpp_renderer
}// namespace ospray
