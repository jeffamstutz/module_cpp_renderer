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

#include "AmbientLight.h"
#include "ospcommon/LinearSpace.h"

#include "../math/sampling.h"

namespace ospray {
  namespace cpp_renderer {

    void AmbientLight::commit()
    {
      color     = getParam3f("color", vec3f(1.f));
      intensity = getParam1f("intensity", 1.f);

      radiance = color * intensity;
    }

    Light_SampleRes AmbientLight::sample(const DifferentialGeometry &dg,
                                         const vec2f &s) const
    {
      Light_SampleRes res;

      const vec3f localDir = cosineSampleHemisphere(s);
      res.dir = frame(dg.Ns) * localDir;
      res.pdf = cosineSampleHemispherePDF(localDir);
      res.dist = inf;
      res.weight = radiance * rcp(res.pdf);

      return res;
    }

    Light_EvalRes AmbientLight::eval(const DifferentialGeometry &dg,
                                     const vec3f &dir,
                                     float maxDist) const
    {
      Light_EvalRes res;

      res.radiance = static_cast<float>(inf) <= maxDist ? radiance : vec3f(0.f);
      res.pdf = cosineSampleHemispherePDF(ospcommon::max(dot(dg.Ns, dir), 0.f));

      return res;
    }

    vec3f AmbientLight::getRadiance() const
    {
      return radiance;
    }

    std::string cpp_renderer::AmbientLight::toString() const
    {
      return "ospray::cpp_renderer::AmbientLight";
    }

    OSP_REGISTER_LIGHT(AmbientLight, cpp_AmbientLight);
    OSP_REGISTER_LIGHT(AmbientLight, cpp_ambient);

  }// namespace cpp_renderer
}// namespace ospray
