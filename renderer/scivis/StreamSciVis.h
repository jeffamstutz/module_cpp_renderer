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

#include "../StreamRenderer.h"
#include "../../lights/Light.h"
#include "SciVisShadingInfo.h"

namespace ospray {
  namespace cpp_renderer {

    using RGBStream = Stream<vec3f>;
    using ShadingStream = Stream<SciVisShadingInfo>;

    struct StreamSciVisRenderer : public ospray::cpp_renderer::StreamRenderer
    {
      std::string toString() const override;
      void commit() override;

      void renderStream(void *perFrameData,
                        ScreenSampleStream &stream) const override;

      ospray::Material *createMaterial(const char *type) override;

    private:

      // Shading functions //

      ShadingStream
      computeShadingInfo(const DGStream &dg) const;

      RGBStream shade_ao(ScreenSampleStream &stream,
                         const DGStream &dgs,
                         const ShadingStream &ss,
                         const RayStream &rays) const;

      RGBStream shade_lights(const DGStream &dgs,
                             const ShadingStream &ss,
                             const RayStream &rays,
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
