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

#include "lights/Light.h"
#include "../common/DifferentialGeometry.h"

namespace ospray {
  namespace cpp_renderer {

    struct Light_SampleRes
    {
      vec3f weight;//!< radiance that arrives at the given point divided by pdf
      vec3f dir;   //!< direction towards the light source
      float dist;  //!< largest valid t_far value for a shadow ray
      float pdf;   //!< probability density that this sample was taken
    };

    struct Light_EvalRes
    {
      vec3f radiance;//!< radiance that arrives at the given point (not
                     //   weighted by pdf)
      float pdf;     //!< probability density that the direction would have
                     //   been sampled
    };

    struct OSPRAY_SDK_INTERFACE Light : public ::ospray::Light
    {
      virtual std::string toString() const override;
      virtual Light_SampleRes sample(const DifferentialGeometry &dg,
                                     const vec2f &s) = 0;
      virtual Light_EvalRes eval(const DifferentialGeometry &dg,
                                 const vec3f &dir,
                                 float maxDist) = 0;
    };

  }// namespace cpp_renderer
}// namespace ospray
