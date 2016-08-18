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
#include "StreamRaycast.h"
#include "../../util.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SimpleAO renderer
    /*! \detailed Since the SimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct StreamRaycastMaterial : public ospray::Material {
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

    void StreamRaycastMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // StreamRaycastRenderer definitions //////////////////////////////////////

    std::string StreamRaycastRenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamRaycastRenderer";
    }

    void StreamRaycastRenderer::renderStream(void */*perFrameData*/,
                                             ScreenSampleStream &stream) const
    {
      traceRays(stream.rays, RTC_INTERSECT_COHERENT);

      DGStream dgs = postIntersect(stream.rays,
                                   DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      // Shade rays
      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          const auto &ray = sample.ray;
          const float c = 0.2f + 0.8f * abs(dot(normalize(ray.Ng), ray.dir));

          auto *mat = dynamic_cast<StreamRaycastMaterial*>(dgs[i].material);

          if (!ray.hitSomething()) {
            sample.rgb = bgColor;
            return;
          }

          sample.rgb   = (mat != nullptr) ? c * mat->Kd : vec3f{c};
          sample.z     = ray.t;
          sample.alpha = 1.f;
        }
      );
    }

    Material *StreamRaycastRenderer::createMaterial(const char */*type*/)
    {
      return new StreamRaycastMaterial;
    }

    OSP_REGISTER_RENDERER(StreamRaycastRenderer, cpp_raycast_stream)

  }// namespace cpp_renderer
}// namespace ospray
