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

    /*! a DirectionalLight is a singular light which is infinitely distant and
     *  thus projects parallel rays of light across the entire scene */
    class OSPRAY_SDK_INTERFACE DirectionalLight : public cpp_renderer::Light
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

        vec3f direction {0.f, 0.f, 1.f};//!< Direction of the emitted rays
        vec3f color {1.f};              //!< RGB color of the emitted light
        float intensity {1.f};          //!< Amount of light emitted
        float angularDiameter {0.f};    //!< Apparent size of the distant
                                        //   light, in degree (e.g. 0.53 for
                                        //   the sun)
        linear3f frame;//!< coordinate frame, with vz == direction *towards*
                       //   the light source
        vec3f radiance;//!< RGB color and intensity of light
        float cosAngle;//!< Angular limit of the cone light in an easier to
                       //   use form: cosine of the half angle in radians
        float pdf;     //!< Probability to sample a direction to the light
    };

  }// namespace cpp_renderer
}// namespace ospray
