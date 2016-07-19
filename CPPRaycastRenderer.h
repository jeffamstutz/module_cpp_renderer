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

#include "render/Renderer.h"

namespace ospray {
  namespace cpp_renderer {

    struct CPPRaycastRenderer : public Renderer
    {
      std::string toString() const override;

      void *beginFrame(FrameBuffer *fb) override;

      void renderTile(void *perFrameData,
                      Tile &tile,
                      size_t jobID) const override;

      void endFrame(void *perFrameData, const int32 fbChannelFlags) override;
    };

  }// namespace cpp_renderer
}// namespace ospray
