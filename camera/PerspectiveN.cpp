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
#include "PerspectiveN.h"
// embree
#include "embree2/rtcore.h"
#include "embree2/rtcore_ray.h"
#include "embree2/rtcore_geometry.h"

namespace ospray {
  namespace cpp_renderer {

    std::string PerspectiveCameraN::toString() const
    {
      return "ospray::PerspectiveCameraN";
    }

    void PerspectiveCameraN::commit()
    {
      cpp_renderer::CameraN::commit();

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

    void PerspectiveCameraN::getRay(const CameraSampleN &sample,
                                    RayN &ray) const
    {
      simd::vec3f org = simd::vec3f{pos};

      auto dir_00 = simd::vec3f{this->dir_00};
      auto dir_du = simd::vec3f{this->dir_du};
      auto dir_dv = simd::vec3f{this->dir_dv};

      auto imageStart_x = simd::vfloat{imageStart.x};
      auto imageStart_y = simd::vfloat{imageStart.y};
      auto imageEnd_x   = simd::vfloat{imageEnd.x};
      auto imageEnd_y   = simd::vfloat{imageEnd.y};

      simd::vec3f dir = dir_00
        + ((imageStart_x * dir_du)
           + (sample.screen.x * (dir_du * (imageEnd_x - imageStart_x))))
        + ((imageStart_y * dir_dv)
           + (sample.screen.y * (dir_dv * (imageEnd_y - imageStart_y))));

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
#if USE_PSIMD
      ray.t   = vfloat(inf);
#else
      ray.t   = inf;
#endif
    }

    OSP_REGISTER_CAMERA(PerspectiveCameraN, cpp_perspective_simd);
    OSP_REGISTER_CAMERA(PerspectiveCameraN, cpp_perspective_stream_simd);

  }// namespace cpp_renderer
}// namespace ospray
