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

// cpp_renderer
#include "SimdRenderer.h"
// embree
#include "embree2/rtcore.h"
#include "embree2/rtcore_scene.h"

namespace ospray {
  namespace cpp_renderer {

    using mask_stream = Stream<simd::vmaski>;

    struct StreamSimdRenderer : public ospray::cpp_renderer::SimdRenderer
    {
      virtual std::string toString() const override;

      virtual void renderTile(void *perFrameData,
                              Tile &tile,
                              size_t jobID) const override;

      void renderSample(simd::vmaski active,
                        void *perFrameData,
                        ScreenSampleN &screenSample) const override;

      virtual void renderStream(const mask_stream &active,
                                void *perFrameData,
                                ScreenSampleNStream &stream) const = 0;

    protected:

      void traceRays(RayNStream &rays, RTCIntersectFlags flags) const;
      void occludeRays(RayNStream &rays, RTCIntersectFlags flags) const;

      DGNStream postIntersect(const RayNStream &rays, int flags) const;
    };

    // Inlined member functions ///////////////////////////////////////////////

    inline void StreamSimdRenderer::traceRays(RayNStream &rays,
                                              RTCIntersectFlags flags) const
    {
#if 0
#if USE_EMBREE_STREAMS
      RTCIntersectContext ctx{flags, nullptr};
      rtcIntersect1M(model->embreeSceneHandle, &ctx,
                     (RTCRay*)&rays, rays.size(), sizeof(Ray));
#else
      UNUSED(flags);
      for (int i = 0; i < ScreenSampleStream::size; ++i) {
        auto &ray = rays[i];
        if (rayIsActive(ray))
          traceRay(ray);
      }
#endif
#endif
    }

    inline void StreamSimdRenderer::occludeRays(RayNStream &rays,
                                                RTCIntersectFlags flags) const
    {
#if 0
#if USE_EMBREE_STREAMS
      RTCIntersectContext ctx{flags, nullptr};
      rtcOccluded1M(model->embreeSceneHandle, &ctx,
                    (RTCRay*)&rays, rays.size(), sizeof(Ray));
#else
      UNUSED(flags);
      for (int i = 0; i < ScreenSampleStream::size; ++i) {
        auto &ray = rays[i];
        if (rayIsActive(ray))
          isOccluded(ray);
      }
#endif
#endif
    }

    inline DGNStream StreamSimdRenderer::postIntersect(const RayNStream &rays,
                                                       int flags) const
    {
#if 0
      DGNStream dgs;

      for (int i = 0; i < ScreenSampleStream::size; ++i) {
        if (rays[i].hitSomething())
          dgs[i] = cpp_renderer::Renderer::postIntersect(rays[i], flags);
      }

      return dgs;
#endif
    }


  }// namespace cpp_renderer
}// namespace ospray
