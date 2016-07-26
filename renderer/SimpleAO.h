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

// embree
#include "embree2/rtcore.h"

#include "../camera/Camera.h"
#include "Renderer.h"

namespace ospray {
  namespace cpp_renderer {

    struct SimpleAORenderer : public ospray::cpp_renderer::Renderer
    {
      std::string toString() const override;
      void commit() override;

      void renderSample(void *perFrameData,
                        ScreenSample &sample) const override;

      ospray::Material *createMaterial(const char *type) override;

    private:

      void shade_ao(vec3f &color,
                    float &alpha,
                    const int accumID,
                    const Ray &ray,
                    const int32 pixel_x,
                    const int32 pixel_y,
                    const float rot_x,
                    const float rot_y) const;

      vec3f getRandomDir(const vec3f biNorm0,
                         const vec3f biNorm1,
                         const vec3f gNormal,
                         const float rot_x,
                         const float rot_y,
                         const float epsilon) const;

      int   samplesPerFrame{1};
      float aoRayLength{1e20f};
    };

  }// namespace cpp_renderer
}// namespace ospray
