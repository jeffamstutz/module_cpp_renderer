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

#pragma once

#include "Light.h"

namespace ospray {
  namespace cpp_renderer {

    class OSPRAY_SDK_INTERFACE AmbientLight : public cpp_renderer::Light
    {
      public:

        std::string toString() const override;
        void commit() override;

        Light_SampleRes sample(const DifferentialGeometry &dg,
                               const vec2f &s) override;

        Light_EvalRes eval(const DifferentialGeometry &dg,
                           const vec3f &dir,
                           float maxDist) override;

      private:

        vec3f color {1.f};     //!< RGB color of the light
        float intensity {1.f}; //!< Amount of light emitted
        vec3f radiance {1.f};
    };

  }
}
