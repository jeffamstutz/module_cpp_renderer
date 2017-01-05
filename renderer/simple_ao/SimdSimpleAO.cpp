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
#include "SimdSimpleAO.h"
#include "ao_util_simd.h"
#include "../../util.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SimpleAO renderer
    /*! \detailed Since the SimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct SimdSimpleAOMaterial : public ospray::Material {
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

    void SimdSimpleAOMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // SimpleAO definitions ///////////////////////////////////////////////////

    std::string SimdSimpleAORenderer::toString() const
    {
      return "ospray::cpp_renderer::SimpleAORenderer";
    }

    void SimdSimpleAORenderer::commit()
    {
      ospray::cpp_renderer::SimdRenderer::commit();
      samplesPerFrame = getParam1i("aoSamples", 1);
      aoRayLength     = getParam1f("aoOcclusionDistance", 1e20f);
    }

    inline void SimdSimpleAORenderer::shade_ao(simd::vmaski active,
                                               simd::vec3f &color,
                                               simd::vfloat &alpha,
                                               const RayN &ray) const
    {
      auto superColor = simd::make_vec3f(1.f, 1.f, 1.f);

      auto dg = postIntersect(active, ray,
                              DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|
                              DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      simd::foreach_active(active, [&](int i) {
        auto *mat = dynamic_cast<SimdSimpleAOMaterial*>(dg.material[i]);

        if (mat) {
          superColor.x[i] = mat->Kd.x;
          superColor.y[i] = mat->Kd.y;
          superColor.z[i] = mat->Kd.z;
#if 0// NOTE(jda) - texture fetches not yet implemented
          if (mat->map_Kd) {
            vec4f Kd_from_map = get4f(mat->map_Kd, dg.st);
            superColor = superColor *
                vec3f(Kd_from_map.x, Kd_from_map.y, Kd_from_map.z);
          }
#endif
        }
      });

      // should be done in material:
      superColor *= simd::vec3f{dg.color.x, dg.color.y, dg.color.z};

      simd::vfloat hits {0.f};
      auto aoContext = getAOContext(dg, aoRayLength, epsilon);

      for (int i = 0; i < samplesPerFrame; i++) {
        auto ao_ray = calculateAORay(dg, aoContext);

        auto rayOccluded = isOccluded(active, ao_ray) ||
                           dot(ao_ray.dir, dg.Ng) < 0.05f;

        hits = simd::select(rayOccluded, hits+1, hits);
      }

      auto diffuse = simd::abs(dot(dg.Ng, ray.dir));
      color = simd::select(active,
                           superColor * (diffuse * (1.f-hits/samplesPerFrame)),
                           simd::vec3f{bgColor});
      alpha = simd::select(active, simd::vfloat{1.f}, alpha);
    }

    void SimdSimpleAORenderer::renderSample(simd::vmaski active,
                                            void *perFrameData,
                                            ScreenSampleN &sample) const
    {
      UNUSED(perFrameData);
      auto &ray = sample.ray;

      auto activeRays = traceRay(active, ray);

      if (simd::any(activeRays)) {
        shade_ao(activeRays, sample.rgb, sample.alpha, sample.ray);
      } else {
        sample.rgb = simd::vec3f{bgColor};
      }
    }

    Material *SimdSimpleAORenderer::createMaterial(const char *type)
    {
      UNUSED(type);
      return new SimdSimpleAOMaterial;
    }

    OSP_REGISTER_RENDERER(SimdSimpleAORenderer, cpp_ao_simd);

  }// namespace cpp_renderer
}// namespace ospray
