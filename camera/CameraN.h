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

// cpp_renderer
#include "../common/RayN.h"
// ospray
#include "camera/Camera.h"

namespace ospray {
  namespace cpp_renderer {

    /*! \brief Specifies the input parameters (time, screen, and lens
        samples) required for 'initRay' to generate a primary ray */
    struct OSPRAY_ALIGN(64) CameraSampleN
    {
      simd::vec2f screen; /*!< normalized screen sample, from [0..1] */
      simd::vec2f lens;
  #if 0// currently unused from ispc version of CameraSample
      float time;
  #endif
    };

    using CameraSampleNStream = Stream<CameraSampleN>;

    // NOTE(jda) - This injects C++ interface function(s) without needing to
    //             change OSPRay itself. Once the design of CPP* classes are
    //             satisfactory, these interfaces can be collapsed into their
    //             ospray::* equivalent.
    struct CameraN : public ospray::Camera
    {
      virtual void getRay(const CameraSampleN &cameraSample,
                          RayN &ray) const = 0;
      virtual void commit() override;
    };

    // Inlined members ////////////////////////////////////////////////////////

    inline void CameraN::commit() {
      // "parse" the general expected parameters
      pos      = getParam3f("pos", vec3f(0.f));
      dir      = getParam3f("dir", vec3f(0.f, 0.f, 1.f));
      up       = getParam3f("up", vec3f(0.f, 1.f, 0.f));
      nearClip = getParam1f("near_clip", getParam1f("nearClip", 1e-6f));

      imageStart = getParam2f("image_start", getParam2f("imageStart", vec2f(0.f)));
      imageEnd   = getParam2f("image_end", getParam2f("imageEnd", vec2f(1.f)));
      clamp(imageStart, vec2f(0.f), vec2f(1.f));
      clamp(imageEnd, imageStart, vec2f(1.f));
    }

  }// namespace cpp_renderer
}// namespace ospray
