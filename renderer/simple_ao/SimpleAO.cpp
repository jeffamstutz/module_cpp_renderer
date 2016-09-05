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
#include "SimpleAO.h"
#include "ao_util.h"
#include "../../util.h"

#include <random>


namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SimpleAO renderer
    /*! \detailed Since the SimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct SimpleAOMaterial : public ospray::Material {
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

    void SimpleAOMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // SimpleAO definitions ///////////////////////////////////////////////////

    std::string SimpleAORenderer::toString() const
    {
      return "ospray::cpp_renderer::SimpleAORenderer";
    }

    void SimpleAORenderer::commit()
    {
      ospray::cpp_renderer::Renderer::commit();
      samplesPerFrame = getParam1i("aoSamples", 1);
      aoRayLength     = getParam1f("aoOcclusionDistance", 1e20f);
    }

    inline void SimpleAORenderer::shade_ao(vec3f &color,
                                           float &alpha,
                                           const Ray &ray) const
    {
      vec3f superColor{1.f};

      auto dg = postIntersect(ray, DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|
                                   DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      SimpleAOMaterial *mat = dynamic_cast<SimpleAOMaterial*>(dg.material);

      if (mat) {
        superColor = mat->Kd;
#if 0// NOTE(jda) - texture fetches not yet implemented
        if (mat->map_Kd) {
          vec4f Kd_from_map = get4f(mat->map_Kd, dg.st);
          superColor = superColor *
              vec3f(Kd_from_map.x, Kd_from_map.y, Kd_from_map.z);
        }
#endif
      }

      // should be done in material:
      superColor *= vec3f{dg.color.x, dg.color.y, dg.color.z};

      int hits = 0;
      auto aoContext = getAOContext(dg, aoRayLength, epsilon);

      for (int i = 0; i < samplesPerFrame; i++) {
        auto ao_ray = calculateAORay(dg, aoContext);
        if (dot(ao_ray.dir, dg.Ng) < 0.05f || isOccluded(ao_ray))
          hits++;
      }

      float diffuse = ospcommon::abs(dot(dg.Ng, ray.dir));
      color = superColor * (diffuse * (1.0f-float(hits)/samplesPerFrame));
      alpha = 1.f;
    }

    void SimpleAORenderer::renderSample(void *perFrameData,
                                        ScreenSample &sample) const
    {
      UNUSED(perFrameData);
      auto &ray = sample.ray;

      if (traceRay(ray)) {
        shade_ao(sample.rgb, sample.alpha, sample.ray);
      } else {
        sample.rgb = bgColor;
      }
    }

    Material *SimpleAORenderer::createMaterial(const char *type)
    {
      UNUSED(type);
      return new SimpleAOMaterial;
    }

    OSP_REGISTER_RENDERER(SimpleAORenderer, cpp_ao);

  }// namespace cpp_renderer
}// namespace ospray
