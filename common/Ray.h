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

namespace ospray {
  namespace cpp_renderer {
    /*! \brief ospray *scalar* ray class w/ correct alignment for embree  */
    struct RTCORE_ALIGN(16) Ray {
      /* ray input data */
      vec3fa org;
      vec3fa dir;
      float t0;
      float t;
      float time;
      int32 mask;

      /* hit data */
      vec3fa Ng;

      float u;
      float v;

      int geomID{RTC_INVALID_GEOMETRY_ID};
      int primID{RTC_INVALID_GEOMETRY_ID};
      int instID{RTC_INVALID_GEOMETRY_ID};
    };
  }// ::ospray::cpp_renderer
} // ::ospray
