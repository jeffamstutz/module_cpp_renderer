// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
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

#include "transferFunction/TransferFunction.h"

namespace ospray {
  namespace cpp_renderer {

    struct TransferFunction : public ::ospray::TransferFunction
    {
      TransferFunction() = default;
      virtual ~TransferFunction() = default;
      virtual void commit() override;
      virtual std::string toString() const override;

      // Sampling interface //

      virtual vec3f color(float value) const = 0;
      virtual vec3f integratedColor(float value1, float value2) const = 0;

      virtual float opacity(float value) const = 0;
      virtual float integratedOpacity(float value1, float value2) const = 0;

      virtual float maxOpacity(const vec2f &range) const = 0;
      virtual vec2f minMaxOpacity(const vec2f &range) const = 0;

      // Data members //

      vec2f valueRange {0.f, 1.f};
    };

  } // ::ospray::cpp_renderer
} // ::ospray
