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

#include "TransferFunction.h"

namespace ospray {
  namespace cpp_renderer {

    struct LinearTransferFunction : public TransferFunction
    {
      LinearTransferFunction() = default;
      virtual ~LinearTransferFunction() = default;

      virtual void commit() override;

      virtual std::string toString() const override;

      // TransferFunction interface //

      virtual vec3f color(float value) const override;
      virtual vec3f integratedColor(float value1,
                                    float value2) const override;

      virtual float opacity(float value) const override;
      virtual float integratedOpacity(float value1,
                                      float value2) const override;

      virtual float maxOpacity(const vec2f &range) const override;
      virtual vec2f minMaxOpacity(const vec2f &range) const override;

      // Data members //

      std::vector<vec3f> colorValues;
      std::vector<float> opacityValues;

      bool preIntegrationEnabled {false};
    };

  } // ::ospray::cpp_renderer
} // ::ospray

