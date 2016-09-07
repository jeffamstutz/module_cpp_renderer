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

#include "DirectionalLight.h"
#include "../math/sampling.h"

#define COS_ANGLE_MAX 0.99999988f

namespace ospray {
  namespace cpp_renderer {

    std::string DirectionalLight::toString() const
    {
      return "ospray::cpp_renderer::DirectionalLight";
    }

    void DirectionalLight::commit()
    {
      direction = getParam3f("direction", vec3f(0.f, 0.f, 1.f));
      color     = getParam3f("color", vec3f(1.f));
      intensity = getParam1f("intensity", 1.f);
      angularDiameter = getParam1f("angularDiameter", 0.f);

      const vec3f radiance = color * intensity;
      direction = -normalize(direction);

      angularDiameter = clamp(angularDiameter, 0.f, 180.f);
      const float cosAngle = ospcommon::cos(deg2rad(0.5f*angularDiameter));
    }

    Light_SampleRes DirectionalLight::sample(const DifferentialGeometry &dg,
                                             const vec2f &s)
    {
      Light_SampleRes res;

      res.dir  = frame.vz;
      res.dist = inf;
      res.pdf  = pdf;

      if (cosAngle < COS_ANGLE_MAX)
        res.dir = frame * uniformSampleCone(cosAngle, s);

      res.weight = radiance; // *pdf/pdf cancel

      return res;
    }

    Light_EvalRes DirectionalLight::eval(const DifferentialGeometry &dg,
                                         const vec3f &dir,
                                         float maxDist)
    {
      Light_EvalRes res;
      res.radiance = vec3f(0.f);

      if (static_cast<float>(inf) <= maxDist
          && cosAngle < COS_ANGLE_MAX
          && dot(frame.vz, dir) > cosAngle) {
        res.radiance = radiance * pdf;
        res.pdf      = pdf;
      }

      return res;
    }

    OSP_REGISTER_LIGHT(DirectionalLight, cpp_DirectionalLight);
    OSP_REGISTER_LIGHT(DirectionalLight, cpp_DistantLight);
    OSP_REGISTER_LIGHT(DirectionalLight, cpp_distant);
    OSP_REGISTER_LIGHT(DirectionalLight, cpp_directional);

  }// namespace cpp_renderer
}// namespace ospray
