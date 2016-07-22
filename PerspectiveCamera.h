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

#include "Camera.h"

namespace ospray {
  namespace cpp_renderer {

    struct PerspectiveCamera : public ospray::cpp_renderer::Camera {
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
      vec3f dir_00;
      vec3f dir_du;
      vec3f dir_dv;
      vec2f imageStart;
      vec2f imageEnd;
      float scaledAperture;
    };

  }// namespace cpp_renderer
}// namespace ospray
