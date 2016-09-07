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

#pragma once

#include "../Renderer.h"
#include "../../lights/Light.h"

namespace ospray {
  namespace cpp_renderer {

    struct SciVisRenderer : public ospray::cpp_renderer::Renderer
    {
      std::string toString() const override;
      void commit() override;

      void renderSample(void *perFrameData,
                        ScreenSample &sample) const override;

      ospray::Material *createMaterial(const char *type) override;

    private:

      // Helper types //

      struct SciVisShadingInfo
      {
        float d  {1.f};
        float Ns {0.f};
        vec3f Kd {0.f};
        vec3f Ks {0.f};
      };

      // Shading functions //
      void shade_materials(vec3f &color,
                           const DifferentialGeometry &dg,
                           SciVisShadingInfo &info) const;

      void shade_ao(vec3f &color,
                    const DifferentialGeometry &dg,
                    const SciVisShadingInfo &info,
                    float &alpha,
                    const Ray &ray) const;

      void shade_lights(vec3f &color,
                        const DifferentialGeometry &dg,
                        const SciVisShadingInfo &info,
                        const Ray &ray,
                        int path_depth) const;

      // Data //

      bool  shadowsEnabled {true};
      bool  singleSidedLighting {true};
      int   samplesPerFrame {1};
      float aoRayLength {1e20f};
      float aoWeight {0.25f};
      int   maxDepth {10};

      std::vector<cpp_renderer::Light*> lights;
    };

  }// namespace cpp_renderer
}// namespace ospray
