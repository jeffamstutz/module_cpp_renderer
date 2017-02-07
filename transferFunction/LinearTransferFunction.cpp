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

#include "common/Data.h"
#include "common/OSPCommon.h"
#include "LinearTransferFunction.h"

namespace ospray {
  namespace cpp_renderer {

    void LinearTransferFunction::commit()
    {
      // Retrieve the color and opacity values.
      auto *colorData   = getParamData("colors", nullptr);
      auto *opacityData = getParamData("opacities", nullptr);

      colorValues.clear();
      opacityValues.clear();

      if (colorData) {
        colorValues.resize(colorData->numItems);
        memcpy(colorValues.data(), colorData->data, colorData->numBytes);
      }

      if (opacityData) {
        opacityValues.resize(opacityData->numItems);
        memcpy(opacityValues.data(), opacityData->data, opacityData->numBytes);
      }

      preIntegrationEnabled = getParam1i("preIntegration", 0);

      TransferFunction::commit();
    }

    std::string LinearTransferFunction::toString() const
    {
      return "ospray::cpp_renderer::LinearTransferFunction";
    }

    vec3f LinearTransferFunction::color(float value) const
    {
      if (isnan(value))
        return vec3f{0.0f};

      if (colorValues.empty())
        return vec3f{1.0f};

      const int numColors = static_cast<int>(colorValues.size());

      if (value <= valueRange.x)
        return colorValues[0];

      if (value >= valueRange.y)
        return colorValues[numColors - 1];

      // Map the value into the range [0.0, 1.0].
      value = (value - valueRange.x)
              / (valueRange.y - valueRange.x)
              * (numColors - 1.0f);

      // Compute the color index and fractional offset.
      int   index     = ospcommon::floor(value);
      float remainder = value - index;

      // The interpolated color.
      return ((1.0f - remainder) * colorValues[index]
             + remainder * colorValues[ospcommon::min(index+1, numColors-1)]);
    }

    vec3f LinearTransferFunction::integratedColor(float value1,
                                                  float value2) const
    {
      NOT_IMPLEMENTED
    }

    float LinearTransferFunction::opacity(float value) const
    {
      if (isnan(value))
        return 0.0f;

      if (opacityValues.empty())
        return 1.0f;

      const int numOpacities = static_cast<int>(opacityValues.size());

      if (value <= valueRange.x)
        return opacityValues[0];

      if (value >= valueRange.y)
        return opacityValues[numOpacities - 1];

      // Map the value into the range [0.0, numValues).
      const float remapped = (value - valueRange.x)
                             / (valueRange.y - valueRange.x)
                             * (numOpacities - 1.0f);

      // Compute the opacity index and fractional offset.
      int index = ospcommon::floor(remapped);
      float remainder = remapped - index;

      // The interpolated opacity.
      return (1.0f - remainder) * opacityValues[index]
             + remainder * opacityValues[ospcommon::min(index+1, numOpacities-1)];
    }

    float LinearTransferFunction::integratedOpacity(float value1,
                                                    float value2) const
    {
      NOT_IMPLEMENTED
    }

    float LinearTransferFunction::maxOpacity(const vec2f &range) const
    {
      NOT_IMPLEMENTED
    }

    vec2f LinearTransferFunction::minMaxOpacity(const vec2f &range) const
    {
      NOT_IMPLEMENTED
    }

    // A piecewise linear transfer function.
    OSP_REGISTER_TRANSFER_FUNCTION(LinearTransferFunction, cpp_piecewise_linear);
    OSP_REGISTER_TRANSFER_FUNCTION(LinearTransferFunction, cpp_tf);

  } // ::ospray::cpp_renderer
} // ::ospray

