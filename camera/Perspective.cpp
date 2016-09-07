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

// ospray
#include "Perspective.h"
// embree
#include "embree2/rtcore.h"
#include "embree2/rtcore_ray.h"
#include "embree2/rtcore_geometry.h"

namespace ospray {
  namespace cpp_renderer {

    std::string PerspectiveCamera::toString() const
    {
      return "ospray::PerspectiveCamera";
    }

    void PerspectiveCamera::commit()
    {
      cpp_renderer::Camera::commit();

      // ------------------------------------------------------------------
      // first, "parse" the additional expected parameters
      // ------------------------------------------------------------------
      fovy = getParamf("fovy", 60.f);
      aspect = getParamf("aspect", 1.f);
      apertureRadius = getParamf("apertureRadius", 0.f);
      focusDistance = getParamf("focusDistance", 1.f);

      // ------------------------------------------------------------------
      // now, update the local precomputed values
      // ------------------------------------------------------------------
      dir    = normalize(dir);
      dir_du = normalize(cross(dir, up));
      dir_dv = cross(dir_du, dir);

      float imgPlane_size_y = 2.f*tanf(fovy/2.f*M_PI/180.);
      float imgPlane_size_x = imgPlane_size_y * aspect;

      dir_du *= imgPlane_size_x;
      dir_dv *= imgPlane_size_y;

      dir_00 = dir - .5f * dir_du - .5f * dir_dv;

#if 0
      float scaledAperture = 0.f;
      // prescale to focal plane
      if (apertureRadius > 0.f) {
        dir_du *= focusDistance;
        dir_dv *= focusDistance;
        dir_00 *= focusDistance;
        scaledAperture = apertureRadius / imgPlane_size_x;
      }
#endif
    }

    void PerspectiveCamera::getRay(const CameraSample &sample,
                                   Ray &ray) const
    {
      vec3f org = pos;
      vec3f dir = dir_00
        + ((imageStart.x * dir_du)
           + (sample.screen.x * (dir_du * (imageEnd.x - imageStart.x))))
        + ((imageStart.y * dir_dv)
           + (sample.screen.y * (dir_dv * (imageEnd.y - imageStart.y))));

  #if 0
      if (self->super.doesDOF) {
        const vec3f llp = uniformSampleDisk(self->scaledAperture, sample.lens);
        // transform local lens point to focal plane (dir_XX are prescaled in this case)
        const vec3f lp = (llp.x * self->dir_du)
             + ((llp.y * self->aspect) * self->dir_dv);
        org = org + lp;
        dir = dir - lp;
      }
  #endif

      ray.org = org;
      ray.dir = normalize(dir);
      ray.t0  = nearClip;
      ray.t   = inf;
    }

    OSP_REGISTER_CAMERA(PerspectiveCamera, cpp_perspective);

  }// namespace cpp_renderer
}// namespace ospray
