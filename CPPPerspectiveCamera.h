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

#include "CPPCamera.h"

namespace ospray {

  struct CPPPerspectiveCamera : public CPPCamera {
    std::string toString() const override;
    void commit() override;

    void getRay(const CameraSample &sample, Ray &ray) const override;

  public:
    // ------------------------------------------------------------------
    // the parameters we 'parsed' from our parameters
    // ------------------------------------------------------------------
    float fovy;
    float aspect;
    float apertureRadius;
    float focusDistance;

    // ------------------------------------------------------------------
    // the parameters we used to put in the ispc-side structure
    // ------------------------------------------------------------------
    ospcommon::vec3f dir_00; //!< direction of ray with screenSample=(0,0); scaled to focusDistance
    ospcommon::vec3f dir_du; //!< delta of ray direction between two pixels in X; scaled to focusDistance
    ospcommon::vec3f dir_dv; //!< delta of ray direction between two pixels in Y; scaled to focusDistance
    ospcommon::vec2f imageStart; //!< where the viewable tile begins [0..1]
    ospcommon::vec2f imageEnd; //!< where the viewable tile ends [0..1]
    float scaledAperture; //!< radius of aperture, divided by horizontal image plane size
  };


}// namespace ospray
