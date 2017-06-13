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

// ospray_cpp
#include "simd.h"
// ospray
#include "ospray/common/OSPCommon.h"
// embree
#include "embree2/rtcore.h"

#include "Stream.h"

namespace ospray {
  namespace cpp_renderer {

    struct RTCORE_ALIGN(simd::width * 4) RayN {
      /* ray input data */
      simd::vec3f org;
      simd::vec3f dir;
      simd::vfloat t0 {inf};
      simd::vfloat t {0.f};
      simd::vfloat time;
      simd::vint mask {0XFFFFFFFF};

      /* hit data */
      simd::vec3f Ng;

      simd::vfloat u;
      simd::vfloat v;

      simd::vint geomID {RTC_INVALID_GEOMETRY_ID};
      simd::vint primID {RTC_INVALID_GEOMETRY_ID};
      simd::vint instID {RTC_INVALID_GEOMETRY_ID};

      // Helper functions //

      inline simd::vmaski hitSomething() const;
    };

    using RayNStream = SimdStream<RayN>;

    // Inlined member definitions /////////////////////////////////////////////

    inline simd::vmaski RayN::hitSomething() const
    {
      return geomID != static_cast<int>(RTC_INVALID_GEOMETRY_ID);
    }

    // Inlined helper functions ///////////////////////////////////////////////

    /*! \brief helper function for querying if an individual ray is active */
    inline simd::vmaskf rayIsActive(const RayN& ray)
    {
      return ray.t0 <= ray.t;
    }

    /*! \brief helper function for querying if an individual ray is active */
    inline simd::vmaskf rayIsActive(const RayNStream &rays, int i)
    {
      return rayIsActive(rays[i]);
    }

    /*! \brief helper function for disabling individual rays in a stream */
    inline void disableRay(RayN &ray)
    {
      auto t0 = ray.t0;
      auto t  = ray.t;
      auto active = rayIsActive(ray);
#if USE_PSIMD
      t0 = psimd::select(active, t0, t);
      t  = psimd::select(active, t, t0);
#else
      t0 = simd::select(active, t0, t);
      t  = simd::select(active, t, t0);
#endif
    }

    /*! \brief helper function for disabling individual rays in a stream */
    inline void disableRay(RayNStream &rays, int i)
    {
      disableRay(rays[i]);
    }

    inline void resetRay(RayN &ray)
    {
      disableRay(ray);
      ray.geomID = RTC_INVALID_GEOMETRY_ID;
    }

    inline void resetRay(RayNStream &rays, int i)
    {
      resetRay(rays[i]);
    }

  }// ::ospray::cpp_renderer
} // ::ospray
