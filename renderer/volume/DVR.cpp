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

#include "DVR.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    struct DVMaterial : public ospray::Material
    {
      void commit() override;

      float d;
      vec3f Kd;
      vec3f Ks;
      float Ns;

      Ref<Texture2D> map_d;
      Ref<Texture2D> map_Kd;
      Ref<Texture2D> map_Ks;
      Ref<Texture2D> map_Ns;
    };

    void DVMaterial::commit()
    {
      map_d  = (Texture2D*)getParamObject("map_d", nullptr);
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
      map_Ks = (Texture2D*)getParamObject("map_Ks",
                                          getParamObject("map_ks", nullptr));
      map_Ns = (Texture2D*)getParamObject("map_Ns",
                                          getParamObject("map_ns", nullptr));

      d  = getParam1f("d", 1.f);
      Kd = getParam3f("kd", getParam3f("Kd", vec3f(.8f)));
      Ks = getParam3f("ks", getParam3f("Ks", vec3f(0.f)));
      Ns = getParam1f("ns", getParam1f("Ns", 10.f));
    }

    // DVR definitions ////////////////////////////////////////////////////////

    std::string DVRenderer::toString() const
    {
      return "ospray::cpp_renderer::DVRenderer";
    }

    void DVRenderer::commit()
    {
      cpp_renderer::Renderer::commit();
    }

    void *DVRenderer::beginFrame(FrameBuffer *fb)
    {
      auto &volumes = model->volume;

      if (!volumes.empty()) {
        currentVolume = dynamic_cast<cpp_renderer::Volume*>(volumes[0].ptr);
        //PRINT(currentVolume);
      }

      return cpp_renderer::Renderer::beginFrame(fb);
    }

    void DVRenderer::renderSample(void *perFrameData,
                                  ScreenSample &sample) const
    {
      UNUSED(perFrameData);

      sample.rgb = bgColor;

      auto &ray = sample.ray;

      if (currentVolume != nullptr) {
        auto hits = intersectBox(ray, currentVolume->boundingBox);

        if (hits.first < hits.second && hits.first < ray.t) {
#if 0
          volumeRay.t0 = t0;
          volumeRay.t = t1;
#else
          sample.rgb = vec3f{0.25f};
#endif
        }
      }
    }

    Material *DVRenderer::createMaterial(const char *type)
    {
      UNUSED(type);
      return new DVMaterial;
    }

    OSP_REGISTER_RENDERER(DVRenderer, cpp_dvr);

  }// namespace cpp_renderer
}// namespace ospray
