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
#include "Raycast.h"
#include "../../util.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SimpleAO renderer
    /*! \detailed Since the SimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct RaycastMaterial : public ospray::Material {
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

    void RaycastMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // RaycastRenderer definitions ////////////////////////////////////////////

    std::string RaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::RaycastRenderer";
    }

    void RaycastRenderer::renderSample(void */*perFrameData*/,
                                       ScreenSample &screenSample) const
    {
      auto &ray = screenSample.ray;

      if (traceRay(ray)) {
#if 1
        const float c =
            0.2f + 0.8f * ospcommon::abs(dot(normalize(ray.Ng), ray.dir));
        auto dg = postIntersect(ray, DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

        auto *mat = dynamic_cast<RaycastMaterial*>(dg.material);
        if (mat)
          screenSample.rgb = c * mat->Kd;
        else
          screenSample.rgb = vec3f{c};
#else
        screenSample.rgb = c * make_random_color(ray.primID);
#endif
        screenSample.z     = ray.t;
        screenSample.alpha = 1.f;
      } else {
        screenSample.rgb = bgColor;
      }
    }

    Material *RaycastRenderer::createMaterial(const char */*type*/)
    {
      return new RaycastMaterial;
    }

    OSP_REGISTER_RENDERER(RaycastRenderer, cpp_raycast);

    extern "C" void ospray_init_module_cpp()
    {
      printf("Loaded plugin 'cpp' ...\n");
    }

  }// namespace cpp_renderer
}// namespace ospray
