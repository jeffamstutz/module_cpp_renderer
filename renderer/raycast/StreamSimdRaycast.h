// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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

#include "../StreamSimdRenderer.h"

namespace ospray {
  namespace cpp_renderer {

    struct StreamSimdRaycastRenderer :
        public ospray::cpp_renderer::StreamSimdRenderer
    {
      std::string toString() const override;

      void renderStream(const mask_stream &active,
                        void *perFrameData,
                        ScreenSampleNStream &stream) const override;

      ospray::Material *createMaterial(const char *type) override;
    };

  }// namespace cpp_renderer
}// namespace ospray