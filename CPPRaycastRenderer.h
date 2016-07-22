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

// ospray
#include "render/Renderer.h"
// embree
#include "embree2/rtcore.h"

#include "CPPCamera.h"

namespace ospray {
  namespace cpp_renderer {

    struct ScreenSample
    {
      // input values to 'renderSample'
      ospcommon::vec3i sampleID; /*!< x/y=pixelID,z=accumID/sampleID */
      Ray              ray;      /*!< the primary ray generated by the camera */
      // return values from 'renderSample'
      ospcommon::vec3f rgb {0.f, 0.f, 0.f};
      float alpha{0.f};
      float z{inf};
    };

    struct CPPRaycastRenderer : public Renderer
    {
      std::string toString() const override;
      void commit() override;

      void *beginFrame(FrameBuffer *fb) override;

      void renderTile(void *perFrameData,
                      Tile &tile,
                      size_t jobID) const override;

      void endFrame(void *perFrameData, const int32 fbChannelFlags) override;

    private:

      void traceRay(Ray &ray) const;

      CPPCamera *currentCamera {nullptr};
    };

    // Inlined member functions ///////////////////////////////////////////////

    inline void CPPRaycastRenderer::traceRay(Ray &ray) const
    {
      rtcIntersect(model->embreeSceneHandle, reinterpret_cast<RTCRay&>(ray));
    }

  }// namespace cpp_renderer
}// namespace ospray
