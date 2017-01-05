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

// ospray_cpp
#include "../camera/CameraN.h"
#include "../common/DifferentialGeometryN.h"
#include "../common/ScreenSampleN.h"
#include "Renderer.h"
// embree
#include "embree2/rtcore.h"
#include "embree2/rtcore_scene.h"

namespace ospray {
  namespace cpp_renderer {

    struct SimdRenderer : public ospray::cpp_renderer::Renderer
    {
      virtual std::string toString() const override;

      virtual void commit() override;

      virtual void *beginFrame(FrameBuffer *fb) override;

      virtual void renderTile(void *perFrameData,
                              Tile &tile,
                              size_t jobID) const override;

      virtual void renderSample(simd::vmaski active,
                                void *perFrameData,
                                ScreenSampleN &screenSample) const = 0;

      void renderSample(void *perFrameData,
                        ScreenSample &screenSample) const override;

    protected:

      simd::vmaski traceRay(simd::vmaski active, RayN &ray) const;
      simd::vmaski isOccluded(simd::vmaski active, RayN &ray) const;

      DifferentialGeometryN postIntersect(simd::vmaski active,
                                          const RayN &ray,
                                          int flags) const;

      // Data //

      ospray::cpp_renderer::CameraN *currentCameraN {nullptr};

    private:

      template <int SIMD_W>
      simd::vmaski traceRayImpl(simd::vmaski active, RayN &ray) const;

      template <int SIMD_W>
      simd::vmaski isOccludedImpl(simd::vmaski active, RayN &ray) const;
    };

    // Inlined member functions ///////////////////////////////////////////////

    // traceRay() definitions //

    template <>
    inline simd::vmaski
    SimdRenderer::traceRayImpl<4>(simd::vmaski active, RayN &ray) const
    {
      rtcIntersect4(reinterpret_cast<int*>(&active),
                    model->embreeSceneHandle,
                    reinterpret_cast<RTCRay4&>(ray));
      return ray.hitSomething();
    }

    template <>
    inline simd::vmaski
    SimdRenderer::traceRayImpl<8>(simd::vmaski active, RayN &ray) const
    {
      rtcIntersect8(reinterpret_cast<int*>(&active),
                    model->embreeSceneHandle,
                    reinterpret_cast<RTCRay8&>(ray));
      return ray.hitSomething();
    }

    template <>
    inline simd::vmaski
    SimdRenderer::traceRayImpl<16>(simd::vmaski active, RayN &ray) const
    {
      rtcIntersect16(reinterpret_cast<int*>(&active),
                     model->embreeSceneHandle,
                     reinterpret_cast<RTCRay16&>(ray));
      return ray.hitSomething();
    }

    inline simd::vmaski
    SimdRenderer::traceRay(simd::vmaski active, RayN &ray) const
    {
      return traceRayImpl<simd::width>(active, ray);
    }

    // isOccluded() definitions //

    template <>
    inline simd::vmaski
    SimdRenderer::isOccludedImpl<4>(simd::vmaski active, RayN &ray) const
    {
      rtcOccluded4(reinterpret_cast<int*>(&active),
                   model->embreeSceneHandle,
                   reinterpret_cast<RTCRay4&>(ray));
      return ray.hitSomething();
    }

    template <>
    inline simd::vmaski
    SimdRenderer::isOccludedImpl<8>(simd::vmaski active, RayN &ray) const
    {
      rtcOccluded8(reinterpret_cast<int*>(&active),
                   model->embreeSceneHandle,
                   reinterpret_cast<RTCRay8&>(ray));
      return ray.hitSomething();
    }

    template <>
    inline simd::vmaski
    SimdRenderer::isOccludedImpl<16>(simd::vmaski active, RayN &ray) const
    {
      rtcOccluded16(reinterpret_cast<int*>(&active),
                    model->embreeSceneHandle,
                    reinterpret_cast<RTCRay16&>(ray));
      return ray.hitSomething();
    }

    inline simd::vmaski
    SimdRenderer::isOccluded(simd::vmaski active, RayN &ray) const
    {
      return isOccludedImpl<simd::width>(active, ray);
    }

    // Other Definitions //

    inline DifferentialGeometryN
    SimdRenderer::postIntersect(simd::vmaski active,
                                const RayN &ray,
                                int flags) const
    {
      DifferentialGeometryN dg;

      if (flags & DG_COLOR)
        dg.color = simd::vec4f{simd::vfloat{1.f}};

      dg.P = ray.org + ray.t * ray.dir;

      // a first hack for instancing: problem is that ospray assumes that
      // 'ray.geomid' specifies the respective sub-geometry of a model
      // that was hit, but for instances embree actually stores this value
      // in ray.instID, so the model would have to know if it's a model
      // over instances or a model over geometries, which is kind of ugly
      // ... will eventually have to fix this properly by implementing
      // instances directly in ospray, but for now let's try this hack
      // here:
      auto regularGeometry = ray.instID < 0 && active;
      if (simd::any(regularGeometry)) {
        simd::foreach_active(regularGeometry, [&](int i) {
          auto geomID = ray.geomID[i];
          auto *geom = dynamic_cast<Geometry*>(model->geometry[geomID].ptr);
          dg.geometry[i] = geom;
          dg.material[i] = geom->material.ptr;
          // TODO: implement load/store scalar dg (and other structures, too)
#if 0
          geom->postIntersect(dg, ray, flags);
#endif
        });
      }
#if 0// NOTE(jda) --> C++ version not yet implemented
      else {
        // instanced geometry: create copy of ray, iterate over
        // ray.instIDs, and remove that instancing info from the ray (so
        // the next level of model doesn't get confused by it)
        Ray newRay = ray;
        foreach_unique(instID in ray.instID) {
          uniform Geometry *uniform instGeom = model->geometry[instID];
          dg.geometry = instGeom;
          dg.material = instGeom->material;
          newRay.instID = -1;
          instGeom->postIntersect(instGeom,model,dg,newRay,flags);
        }
      }
#endif

#define  DG_NG_FACEFORWARD (DG_NG | DG_FACEFORWARD)
#define  DG_NS_FACEFORWARD (DG_NS | DG_FACEFORWARD)
#define  DG_NG_NORMALIZE   (DG_NG | DG_NORMALIZE)
#define  DG_NS_NORMALIZE   (DG_NS | DG_NORMALIZE)

      if ((flags & DG_NG_NORMALIZE) == DG_NG_NORMALIZE)
        dg.Ng = normalize(dg.Ng);
      if ((flags & DG_NS_NORMALIZE) == DG_NS_NORMALIZE)
        dg.Ns = normalize(dg.Ns);

      if ((flags & DG_NG_FACEFORWARD) == DG_NG_FACEFORWARD)
        dg.Ng = simd::select(dot(ray.dir, dg.Ng) >= 0.f, -dg.Ng, dg.Ng);

      if ((flags & DG_NS_FACEFORWARD) == DG_NS_FACEFORWARD)
        dg.Ns = simd::select(dot(ray.dir, dg.Ns) >= 0.f, -dg.Ns, dg.Ns);

#undef  DG_NG_FACEFORWARD
#undef  DG_NS_FACEFORWARD
#undef  DG_NG_NORMALIZE
#undef  DG_NS_NORMALIZE

      return dg;
    }

  }// namespace cpp_renderer
}// namespace ospray
