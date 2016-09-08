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

#include "StreamSciVis.h"
#include "../simple_ao/ao_util.h"
#include "../../util.h"

#include "common/Data.h"

namespace ospray {
  namespace cpp_renderer {

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the SciVis renderer
    /*! \detailed Since the SciVis Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct StreamSciVisMaterial : public ospray::Material {
      /*! \brief commit the object's outstanding changes
       *         (such as changed parameters etc) */
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

    void StreamSciVisMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
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

    // SciVis definitions /////////////////////////////////////////////////////

    std::string StreamSciVisRenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamSciVisRenderer";
    }

    void StreamSciVisRenderer::commit()
    {
      cpp_renderer::Renderer::commit();

      // shadow parameters
      shadowsEnabled      = getParam1i("shadowsEnabled", 1);
      singleSidedLighting = getParam1i("oneSidedLighting", 1);

      // ao parameters
      samplesPerFrame = getParam1i("aoSamples", 1);
      aoRayLength     = getParam1f("aoOcclusionDistance", 1e20f);
      aoWeight        = getParam1f("aoWeight", 0.25f);

      aoWeight *= static_cast<float>(pi);

      auto *lightData = (Data*)getParamData("lights");

      lights.clear();

      if (lightData) {
        auto **lightArray = (cpp_renderer::Light**)lightData->data;
        for (uint32_t i = 0; i < lightData->size(); i++)
          lights.push_back(lightArray[i]);
      }
    }

    void StreamSciVisRenderer::renderStream(void *perFrameData,
                                            ScreenSampleStream &stream) const
    {
      UNUSED(perFrameData);
      traceRays(stream.rays, RTC_INTERSECT_COHERENT);

      DGStream dgs = postIntersect(stream.rays,
                                   DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|
                                   DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      int nActiveRays = ScreenSampleStream::size;

      for_each_sample(stream,[](ScreenSampleRef sample){ sample.alpha = 1.f; });

      // Disable rays which didn't hit anything
      for_each_sample(
        stream,
        [&](ScreenSampleRef sample){
          sample.rgb = bgColor;
          disableRay(sample.ray);
          nActiveRays--;
        },
        rayMiss
      );

      if (nActiveRays <= 0)
        return;

      const auto ss = computeShadingInfo(stream, dgs);

#if 0//NOTE(jda) - stack size problem?...wacky periodic performance issues...
      const auto aoColors    = shade_ao(stream, dgs, ss);
      const auto lightColors = shade_lights(stream, dgs, ss, 0);

      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          sample.rgb = aoColors[i] + lightColors[i];
        },
        rayHit
      );
#else
      auto colors = shade_ao(stream, dgs, ss);

      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          sample.rgb = colors[i];
        },
        rayHit
      );

      colors = shade_lights(stream, dgs, ss, 0);

      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          sample.rgb += colors[i];
        },
        rayHit
      );
#endif
    }

    Material *StreamSciVisRenderer::createMaterial(const char *type)
    {
      UNUSED(type);
      return new StreamSciVisMaterial;
    }

    ShadingStream
    StreamSciVisRenderer::computeShadingInfo(ScreenSampleStream &stream,
                                             const DGStream &dgs) const
    {
      ShadingStream ss;

      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          auto &info = ss[i];
          auto &dg   = dgs[i];

          auto *mat = dynamic_cast<StreamSciVisMaterial*>(dg.material);

          if (mat) {
            // textures modify (mul) values, see
            //   http://paulbourke.net/dataformats/mtl/
            info.Kd = mat->Kd * vec3f{dg.color.x, dg.color.y, dg.color.z};
#if 0// NOTE(jda) - texture fetches not yet implemented
            info.d = mat->d * get1f(mat->map_d, dg.st, 1.f);
            if (mat->map_Kd) {
              vec4f Kd_from_map = get4f(mat->map_Kd, dg.st);
              info.Kd = info.Kd * make_vec3f(Kd_from_map);
              info.d *= Kd_from_map.w;
            }
            info.Ks = mat->Ks * get3f(mat->map_Ks, dg.st, make_vec3f(1.f));
            info.Ns = mat->Ns * get1f(mat->map_Ns, dg.st, 1.f);
#else
            info.d  = mat->d;
            info.Ks = mat->Ks;
            info.Ns = mat->Ns;
#endif
          } else {
            info.Kd = vec3f{dg.color.x, dg.color.y, dg.color.z};
          }

          // BRDF normalization
          info.Kd *= static_cast<float>(one_over_pi);
          info.Ks *= (info.Ns + 2.f) * static_cast<float>(one_over_two_pi);
        },
        rayHit
      );

      return ss;
    }

    RGBStream StreamSciVisRenderer::shade_ao(ScreenSampleStream &stream,
                                             const DGStream &dgs,
                                             const ShadingStream &ss) const
    {
      RGBStream colors;

      Stream<int> hits;
      std::fill(begin(hits), end(hits), 0);

      Stream<ao_context> ao_ctxs;

      RayStream ao_rays;

      for (int j = 0; j < samplesPerFrame; j++) {
        // Setup AO rays for active "lanes"
        for_each_sample_i(
          stream,
          [&](ScreenSampleRef /*sample*/, int i) {
            auto &dg  = dgs[i];
            auto &ctx = ao_ctxs[i];
            ctx = getAOContext(dg, aoRayLength, epsilon);
            ao_rays[i] = calculateAORay(dg, ctx);
          },
          rayHit
        );

        // Trace AO rays
        occludeRays(ao_rays, RTC_INTERSECT_INCOHERENT);

        // Record occlusion test
        for_each_sample_i(
          stream,
          [&](ScreenSampleRef sample, int i) {
            UNUSED(sample);
            auto &ao_ray = ao_rays[i];
            if (dot(ao_ray.dir, dgs[i].Ng) < 0.05f || ao_ray.hitSomething())
              hits[i]++;
          },
          rayHit
        );
      }

      // Write pixel colors
      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          float diffuse = ospcommon::abs(dot(dgs[i].Ng, sample.ray.dir));
          auto &info = ss[i];
          colors[i] = info.Kd *
                      (diffuse * (1.0f - float(hits[i])/samplesPerFrame));
        },
        rayHit
      );

      return colors;
    }

    RGBStream StreamSciVisRenderer::shade_lights(ScreenSampleStream &stream,
                                                 const DGStream &dgs,
                                                 const ShadingStream &ss,
                                                 int path_depth) const
    {
      RGBStream colors;

      for_each_sample_i(
        stream,
        [&](ScreenSampleRef sample, int i) {
          UNUSED(sample);

          const auto &ray  = stream.rays[i];
          const auto &dg   = dgs[i];
          const auto &info = ss[i];

          const vec3f R = ray.dir - ((2.f * dot(ray.dir, dg.Ng)) * dg.Ng);

          //NOTE(jda) - default epsilon doesn't seem to work here...(FIU)
          const float epsilon = 1e-3f;
          const vec3f P = dg.P + epsilon * dg.Ng;

          auto &color = colors[i] = vec3f{0.f};

          //calculate shading for all lights
          for (const auto *l : lights) {
            const auto light = l->sample(dg, vec2f{0.5f});

            if (reduce_max(light.weight) > 0.f) { // any potential contribution?
              float cosNL = dot(light.dir, dg.Ng);

              if (singleSidedLighting) {
                if (cosNL < 0.0f)
                  continue;
              }
              else
                cosNL = fabs(cosNL);

              const float cosLR = max(0.f, dot(light.dir, R));
              const vec3f brdf = info.Kd * cosNL +
                                 info.Ks * powf(cosLR, info.Ns);
              const vec3f light_contrib = brdf * light.weight;

              if (shadowsEnabled) {
                const float max_contrib = reduce_max(light_contrib);
                if (max_contrib > .01f) {
                  Ray shadowRay;
                  shadowRay.org = P;
                  shadowRay.dir = light.dir;
#if 0
                  const float light_alpha = lightAlpha(shadowRay,
                                                       model,
                                                       max_contrib,
                                                       maxDepth - path_depth,
                                                       epsilon);
#else
                  float light_alpha = 1.0f;
                  if (isOccluded(shadowRay)) {
                    light_alpha = 0.f;
                  }
#endif
                  color += light_alpha * light_contrib;
                }
              } else {
                color += light_contrib;
              }
            }
          }
        },
        rayHit
      );

      return colors;
    }

    OSP_REGISTER_RENDERER(StreamSciVisRenderer, cpp_scivis_stream);
    OSP_REGISTER_RENDERER(StreamSciVisRenderer, cpp_sv_stream);

  }// namespace cpp_renderer
}// namespace ospray
