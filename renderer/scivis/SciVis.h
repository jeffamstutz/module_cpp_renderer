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

#define USE_FIBERED_RENDERER 1

#if USE_FIBERED_RENDERER
#  include "../FiberedRenderer.h"
#else
#  include "../Renderer.h"
#endif

#include "../../lights/Light.h"
#include "SciVisShadingInfo.h"

namespace ospray {
  namespace cpp_renderer {

#if USE_FIBERED_RENDERER
    struct SciVisRenderer : public ospray::cpp_renderer::FiberedRenderer
#else
    struct SciVisRenderer : public ospray::cpp_renderer::Renderer
#endif
    {
      std::string toString() const override;
      void commit() override;

      void renderSample(void *perFrameData,
                        ScreenSample &sample) const override;

      ospray::Material *createMaterial(const char *type) override;

    private:

      // Shading functions //
      SciVisShadingInfo
      computeShadingInfo(const DifferentialGeometry &dg) const;

      vec3f shade_ao(const DifferentialGeometry &dg,
                     const SciVisShadingInfo &info,
                     const Ray &ray,
                     void *perFrameData) const;

      vec3f shade_lights(const DifferentialGeometry &dg,
                         const SciVisShadingInfo &info,
                         const Ray &ray,
                         int path_depth,
                         void *perFrameData) const;

      // Data //

      bool  shadowsEnabled {true};
      bool  singleSidedLighting {true};
      int   samplesPerFrame {1};
      float aoDistance {1e20f};
      vec3f aoColor {0.f};
      int   maxDepth {10};

      std::vector<cpp_renderer::Light*> lights;
    };

  }// namespace cpp_renderer
}// namespace ospray
