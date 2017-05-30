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

#define USE_FIBERED_RENDERER 1

#if USE_FIBERED_RENDERER
#  include "../FiberedRenderer.h"
#else
#  include "../Renderer.h"
#endif

namespace ospray {
  namespace cpp_renderer {

#if USE_FIBERED_RENDERER
    struct SimpleAORenderer : public ospray::cpp_renderer::FiberedRenderer
#else
    struct SimpleAORenderer : public ospray::cpp_renderer::Renderer
#endif
    {
      std::string toString() const override;
      void commit() override;

      void renderSample(void *perFrameData,
                        ScreenSample &sample) const override;

      ospray::Material *createMaterial(const char *type) override;

    private:

#if USE_FIBERED_RENDERER
      void shade_ao(ScreenSample &sample, void* pfd) const;
#else
      void shade_ao(ScreenSample &sample) const;
#endif

      int   samplesPerFrame{1};
      float aoRayLength{1e20f};
    };

  }// namespace cpp_renderer
}// namespace ospray
