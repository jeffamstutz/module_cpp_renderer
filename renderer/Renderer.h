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

#include "../camera/Camera.h"
#include "../common/DifferentialGeometry.h"
#include "../common/ScreenSample.h"
#include "../geometry/Geometry.h"

namespace ospray {
  namespace cpp_renderer {

    struct Renderer : public ospray::Renderer
    {
      virtual std::string toString() const override;
      virtual void commit() override;

      virtual void *beginFrame(FrameBuffer *fb) override;

      virtual void renderTile(void *perFrameData,
                              Tile &tile,
                              size_t jobID) const override;

      virtual void renderSample(void *perFrameData,
                                ScreenSample &screenSample) const = 0;

      virtual void endFrame(void *perFrameData,
                            const int32 fbChannelFlags) override;

    protected:

      void traceRay(Ray &ray) const;
      bool isOccluded(Ray &ray) const;

      DifferentialGeometry postIntersect(const Ray &ray, int flags) const;

    private:

      ospray::cpp_renderer::Camera *currentCamera {nullptr};
    };

    // Inlined member functions ///////////////////////////////////////////////

    inline void Renderer::traceRay(Ray &ray) const
    {
      rtcIntersect(model->embreeSceneHandle, reinterpret_cast<RTCRay&>(ray));
    }

    inline bool Renderer::isOccluded(Ray &ray) const
    {
      rtcOccluded(model->embreeSceneHandle, reinterpret_cast<RTCRay&>(ray));
      return ray.geomID != RTC_INVALID_GEOMETRY_ID;
    }

    inline DifferentialGeometry Renderer::postIntersect(const Ray &ray,
                                                        int flags) const
    {
      DifferentialGeometry dg;

      if (flags & DG_COLOR)
        dg.color = vec4f{1.f};

      dg.P = ray.org + ray.t * ray.dir;

      // a first hack for instancing: problem is that ospray assumes that
      // 'ray.geomid' specifies the respective sub-geometry of a model
      // that was hit, but for instances embree actually stores this value
      // in ray.instID, so the model would have to know if it's a model
      // over instances or a model over geometries, which is kind of ugly
      // ... will eventually have to fix this properly by implementing
      // instances directly in ospray, but for now let's try this hack
      // here:
      if (ray.instID < 0) {
        // a regular geometry
        auto *geom =
            dynamic_cast<Geometry*>(model->geometry[ray.geomID].ptr);
        dg.geometry = geom;
        dg.material = geom->material.ptr;
        geom->postIntersect(dg, ray, flags);
      } else {
        // instanced geometry: create copy of ray, iterate over
        // ray.instIDs, and remove that instancing info from the ray (so
        // the next level of model doesn't get confused by it)
#if 0// NOTE(jda) --> C++ version not yet implemented
        Ray newRay = ray;
        foreach_unique(instID in ray.instID) {
          uniform Geometry *uniform instGeom = model->geometry[instID];
          dg.geometry = instGeom;
          dg.material = instGeom->material;
          newRay.instID = -1;
          instGeom->postIntersect(instGeom,model,dg,newRay,flags);
        }
#endif
      }

#define  DG_NG_FACEFORWARD (DG_NG | DG_FACEFORWARD)
#define  DG_NS_FACEFORWARD (DG_NS | DG_FACEFORWARD)
#define  DG_NG_NORMALIZE   (DG_NG | DG_NORMALIZE)
#define  DG_NS_NORMALIZE   (DG_NS | DG_NORMALIZE)

      if ((flags & DG_NG_NORMALIZE) == DG_NG_NORMALIZE)
        dg.Ng = normalize(dg.Ng);
      if ((flags & DG_NS_NORMALIZE) == DG_NS_NORMALIZE)
        dg.Ns = normalize(dg.Ns);

      if ((flags & DG_NG_FACEFORWARD) == DG_NG_FACEFORWARD &&
          (dot(ray.dir,dg.Ng) >= 0.f))
        dg.Ng = -dg.Ng;

      if ((flags & DG_NS_FACEFORWARD) == DG_NS_FACEFORWARD &&
          (dot(ray.dir,dg.Ns) >= 0.f))
        dg.Ns = -dg.Ns;

#undef  DG_NG_FACEFORWARD
#undef  DG_NS_FACEFORWARD
#undef  DG_NG_NORMALIZE
#undef  DG_NS_NORMALIZE

      return dg;
    }

  }// namespace cpp_renderer
}// namespace ospray
