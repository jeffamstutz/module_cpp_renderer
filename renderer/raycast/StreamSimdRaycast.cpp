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

// ospray
#include "StreamSimdRaycast.h"
#include "../../util.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SimpleAO renderer
    /*! \detailed Since the SimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct StreamSimdRaycastMaterial : public ospray::Material {
      /*! \brief commit the object's outstanding changes
       *         (such as changed parameters etc) */
      void commit() override;

      // -------------------------------------------------------
      // member variables
      // -------------------------------------------------------

      //! \brief diffuse material component, that's all we care for
      vec3f Kd;

      //! \brief diffuse texture, if available
      Ref<Texture2D> map_Kd;
    };

    void StreamSimdRaycastMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // StreamRaycastRenderer definitions //////////////////////////////////////

    std::string StreamSimdRaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamRaycastRenderer";
    }

    void
    StreamSimdRaycastRenderer::renderStream(const mask_stream &activeStream,
                                            void */*perFrameData*/,
                                            ScreenSampleNStream &stream) const
    {
      traceRays(stream.rays, RTC_INTERSECT_COHERENT);

      auto dgs = postIntersect(stream.rays, DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      // Shade rays
      for_each_sample_i(
        stream,
        [&](ScreenSampleNRef sample, int i) {
          const auto &ray = sample.ray;
          const auto c =
              0.2f + 0.8f * simd::abs(dot(normalize(ray.Ng), ray.dir));

          simd::vec3f col{c};

          auto &active = activeStream[i];
          auto hit     = ray.hitSomething() && active;
          auto miss    = !hit && active;

          auto &dg = dgs[i];

          simd::foreach_active(hit, [&](int j) {
            auto *mat =
                dynamic_cast<StreamSimdRaycastMaterial*>(dg.material[j]);

            auto eye_col = c[j];
            if (mat) {
              col.x[j] = eye_col * mat->Kd.x;
              col.y[j] = eye_col * mat->Kd.y;
              col.z[j] = eye_col * mat->Kd.z;
            }
          });

          sample.rgb   = simd::select(hit, col,   simd::vec3f{bgColor});
          sample.z     = simd::select(hit, ray.t, sample.z);
          sample.alpha = simd::select(hit, 1.f,   sample.alpha);
        }
      );
    }

    Material *StreamSimdRaycastRenderer::createMaterial(const char */*type*/)
    {
      return new StreamSimdRaycastMaterial;
    }

    OSP_REGISTER_RENDERER(StreamSimdRaycastRenderer, cpp_raycast_stream_simd);

    // NOTE(jda) - Remove this alias once there's a real StreamSimdSciVisRenderer
    OSP_REGISTER_RENDERER(StreamSimdRaycastRenderer, cpp_scivis_stream_simd);

  }// namespace cpp_renderer
}// namespace ospray
