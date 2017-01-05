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
#include "SimdRaycast.h"
#include "../../util.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SimpleAO renderer
    /*! \detailed Since the SimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct SimdRaycastMaterial : public ospray::Material {
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

    void SimdRaycastMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // RaycastRenderer definitions ////////////////////////////////////////////

    std::string SimdRaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::RaycastRenderer";
    }

    void SimdRaycastRenderer::renderSample(simd::vmaski active,
                                           void */*perFrameData*/,
                                           ScreenSampleN &screenSample) const
    {
      auto &ray = screenSample.ray;

      auto hit  = traceRay(active, ray);
      auto miss = !hit;

      if (simd::any(hit)) {
        const auto c = 0.2f + 0.8f * simd::abs(dot(normalize(ray.Ng), ray.dir));
        auto dg = postIntersect(hit,ray,DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

        simd::vec3f col{c};

        simd::foreach_active(hit, [&](int i) {
          auto *mat = dynamic_cast<SimdRaycastMaterial*>(dg.material[i]);

          auto eye_col = c[i];
          if (mat) {
            col.x[i] = eye_col * mat->Kd.x;
            col.y[i] = eye_col * mat->Kd.y;
            col.z[i] = eye_col * mat->Kd.z;
          }
        });

        // NOTE(jda) - use vec_t select!
        screenSample.rgb.x = simd::select(hit, col.x, bgColor.x);
        screenSample.rgb.y = simd::select(hit, col.y, bgColor.y);
        screenSample.rgb.z = simd::select(hit, col.z, bgColor.z);

        screenSample.z     = simd::select(hit, ray.t, screenSample.z);
        screenSample.alpha = simd::select(hit, ray.t, screenSample.alpha);
      } else {
        screenSample.rgb = simd::vec3f{simd::vfloat{bgColor.x},
                                       simd::vfloat{bgColor.y},
                                       simd::vfloat{bgColor.z}};
      }
    }

    Material *SimdRaycastRenderer::createMaterial(const char */*type*/)
    {
      return new SimdRaycastMaterial;
    }

    OSP_REGISTER_RENDERER(SimdRaycastRenderer, cpp_raycast_simd);

  }// namespace cpp_renderer
}// namespace ospray
