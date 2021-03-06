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

// ospray
#include "ospray/common/OSPCommon.h"
// embree
#include "embree2/rtcore.h"

#include "Stream.h"

namespace ospray {
  namespace cpp_renderer {

    /*! \brief ospray *scalar* ray class w/ correct alignment for embree  */
    struct RTCORE_ALIGN(16) Ray {
      /* ray input data */
      vec3fa org;
      vec3fa dir;
      float t0{inf};
      float t{0.f};
      float time;
      int32 mask{0xFFFFFFFF};

      /* hit data */
      vec3fa Ng;

      float u;
      float v;

      int geomID{RTC_INVALID_GEOMETRY_ID};
      int primID{RTC_INVALID_GEOMETRY_ID};
      int instID{RTC_INVALID_GEOMETRY_ID};

      // Helper functions //

      inline bool hitSomething() const;
    };

    using RayStream = Stream<Ray>;

    // Inlined member definitions /////////////////////////////////////////////

    inline bool Ray::hitSomething() const
    {
      return geomID != static_cast<int>(RTC_INVALID_GEOMETRY_ID);
    }

    // Inlined helper functions ///////////////////////////////////////////////

    /*! \brief helper function for querying if an individual ray is active */
    inline bool rayIsActive(const Ray& ray)
    {
      return ray.t0 <= ray.t;
    }

    /*! \brief helper function for querying if an individual ray is active */
    inline bool rayIsActive(const RayStream &rays, int i)
    {
      return rayIsActive(rays[i]);
    }

    /*! \brief helper function for disabling individual rays in a stream */
    inline void disableRay(Ray &ray)
    {
      if (rayIsActive(ray)) std::swap(ray.t0, ray.t);
    }

    /*! \brief helper function for disabling individual rays in a stream */
    inline void disableRay(RayStream &rays, int i)
    {
      disableRay(rays[i]);
    }

    inline void resetRay(Ray &ray)
    {
      disableRay(ray);
      ray.geomID = RTC_INVALID_GEOMETRY_ID;
    }

    inline void resetRay(RayStream &rays, int i)
    {
      resetRay(rays[i]);
    }

    inline std::pair<float, float> intersectBox(const Ray &ray,
                                                const box3f &box)
    {
      const vec3f mins = (box.lower - ray.org) * rcp(ray.dir);
      const vec3f maxs = (box.upper - ray.org) * rcp(ray.dir);

      return {reduce_max(vec4f{ospcommon::min(mins, maxs), ray.t0}),
              reduce_min(vec4f{ospcommon::max(mins, maxs), ray.t})};
    }

  }// ::ospray::cpp_renderer
} // ::ospray
