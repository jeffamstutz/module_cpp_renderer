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

// ospray
#include "Volume.h"
#if 0
#include "transferFunction/TransferFunction.h"
#endif

namespace ospray {
  namespace cpp_renderer {

    std::string Volume::toString() const
    {
      return("ospray::cpp_renderer::Volume");
    }

    void Volume::commit()
    {
      // Set the gradient shading flag for the renderer.
      gradientShadingEnabled  = getParam1i("gradientShadingEnabled", 0);
      preIntegrationEnabled   = getParam1i("preIntegration", 0);
      singleShadingEnabled    = getParam1i("singleShade", 1);
      adaptiveSamplingEnabled = getParam1i("adaptiveSampling", 1);
      adaptiveScalar          = getParam1f("adaptiveScalar", 15.0f);
      adaptiveMaxSamplingRate = getParam1f("adaptiveMaxSamplingRate", 0.7f);
      adaptiveBacktrack       = getParam1f("adaptiveBacktrack", 0.03f);

      // Set the recommended sampling rate for ray casting based renderers.
      samplingRate = getParam1f("samplingRate", 1.0f);

      specular = getParam3f("specular", vec3f(0.3f));

      // Set the volume clipping box (empty by default for no clipping).
      volumeClippingBox =
          box3f(getParam3f("volumeClippingBoxLower", vec3f(0.f)),
                getParam3f("volumeClippingBoxUpper", vec3f(0.f)));
#if 0
      // Set the transfer function.
      TransferFunction *transferFunction =
          (TransferFunction *) getParamObject("transferFunction", NULL);
      exitOnCondition(transferFunction == NULL, "no transfer function specified");
      ispc::Volume_setTransferFunction(ispcEquivalent, transferFunction->getIE());
#endif
    }

  } // ::ospray::cpp_renderer
} // ::ospray

