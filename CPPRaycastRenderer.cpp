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

#include "CPPRaycastRenderer.h"

namespace ospray {
  namespace cpp_renderer {

    std::string CPPRaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::CPPRaycastRenderer";
    }

    void *CPPRaycastRenderer::beginFrame(FrameBuffer *fb)
    {
      currentFB = fb;
      return nullptr;
    }

    void CPPRaycastRenderer::renderTile(void *perFrameData,
                                        Tile &tile,
                                        size_t jobID) const
    {
      //TODO: implement

      for (int i = 0; i < TILE_SIZE*TILE_SIZE; ++i) {
        tile.r[i] = 1.f;
        tile.g[i] = 0.f;
        tile.b[i] = 0.f;
        tile.a[i] = 1.f;
        tile.z[i] = 1.f;
      }
    }

    void CPPRaycastRenderer::endFrame(void *perFrameData,
                                      const int32 fbChannelFlags)
    {
      //TODO: implement
    }

    OSP_REGISTER_RENDERER(CPPRaycastRenderer, cpp_raycast)

    extern "C" void ospray_init_module_cpp()
    {
      printf("Loaded plugin 'cpp' ...\n");
    }

  }// namespace cpp_renderer
}// namespace ospray
