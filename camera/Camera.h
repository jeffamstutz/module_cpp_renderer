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

#include "camera/Camera.h"
#include "../common/Ray.h"

#include <array>

namespace ospray {
  namespace cpp_renderer {

    /*! \brief Specifies the input parameters (time, screen, and lens
        samples) required for 'initRay' to generate a primary ray */
    struct CameraSample
    {
      ospcommon::vec2f screen; /*!< normalized screen sample, from [0..1] */
      ospcommon::vec2f lens;
  #if 0// currently unused from ispc version of CameraSample
      float time;
  #endif
    };

    using CameraSampleStream = std::array<CameraSample,
                                          RENDERTILE_PIXELS_PER_JOB>;

    // NOTE(jda) - This injects C++ interface function(s) without needing to
    //             change OSPRay itself. Once the design of CPP* classes are
    //             satisfactory, these interfaces can be collapsed into their
    //             ospray::* equivalent.
    struct Camera : public ospray::Camera
    {
      virtual void getRay(const CameraSample &cameraSample, Ray &ray) const = 0;
    };

  }// namespace cpp_renderer
}// namespace ospray
