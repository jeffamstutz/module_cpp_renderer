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
#include "StreamSimpleAO.h"
#include "../util.h"

#include <random>

static thread_local std::minstd_rand generator;

namespace ospray {
  namespace cpp_renderer {

    using SSR = ScreenSampleRef;

    // Material definition ////////////////////////////////////////////////////

    //! \brief Material used by the StreamSimpleAO renderer
    /*! \detailed Since the StreamSimpleAO Renderer only cares about a
        diffuse material component this material only stores diffuse
        and diffuse texture */
    struct StreamSimpleAOMaterial : public ospray::Material {
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

    void StreamSimpleAOMaterial::commit()
    {
      Kd = getParam3f("color", getParam3f("kd", getParam3f("Kd", vec3f(.8f))));
      map_Kd = (Texture2D*)getParamObject("map_Kd",
                                          getParamObject("map_kd", nullptr));
    }

    // Helper functions ///////////////////////////////////////////////////////

    inline vec3f getShadingNormal(const Ray &ray)
    {
      vec3f N = ray.Ng;
      float f = rcp(sqrt(dot(N,N)));
      if (dot(N,ray.dir) >= 0.f) f = -f;
      return f*N;
    }

    inline void getBinormals(vec3f &biNorm0,
                             vec3f &biNorm1,
                             const vec3f &gNormal)
    {
      biNorm0 = vec3f{1.f,0.f,0.f};
      if (abs(dot(biNorm0,gNormal)) > .95f)
        biNorm0 = vec3f{0.f,1.f,0.f};
      biNorm1 = normalize(cross(biNorm0,gNormal));
      biNorm0 = normalize(cross(biNorm1,gNormal));
    }

    inline float rotate(float x, float dx)
    {
      x += dx;
      if (x >= 1.f) x -= 1.f;
      return x;
    }

    inline vec3f getRandomDir(const vec3f &biNorm0,
                              const vec3f &biNorm1,
                              const vec3f &gNormal,
                              float rot_x,
                              float rot_y,
                              float epsilon)
    {
      static std::uniform_real_distribution<float> distribution {0.f, 1.f};
      const vec2f rn = vec2f{distribution(generator), distribution(generator)};
      const float r0 = rotate(rn.x, rot_x);
      const float r1 = rotate(rn.y, rot_y);

      const float w = sqrt(1.f-r1);
      const float x = cos(float((2.f*M_PI)*r0))*w;
      const float y = sin(float((2.f*M_PI)*r0))*w;
      const float z = sqrt(r1) + epsilon;
      return x*biNorm0 + y*biNorm1 + z*gNormal;
    }

    // StreamSimpleAO definitions ///////////////////////////////////////////////////

    std::string StreamSimpleAORenderer::toString() const
    {
      return "ospray::cpp_renderer::StreamSimpleAORenderer";
    }

    void StreamSimpleAORenderer::commit()
    {
      ospray::cpp_renderer::Renderer::commit();
      samplesPerFrame = getParam1i("aoSamples", 1);
      aoRayLength     = getParam1f("aoOcclusionDistance", 1e20f);
    }

    inline void
    StreamSimpleAORenderer::shade_ao(ScreenSampleStream &stream) const
    {
      DGStream dgs = postIntersect(stream.rays,
                                   DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD|
                                   DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      int nActiveRays = ScreenSampleStream::size;

      for_each_sample(stream, [](SSR sample){ sample.alpha = 1.f; });

      // Disable rays which didn't hit anything
      for_each_sample(
        stream,
        [&](SSR sample){
          sample.rgb = bgColor;
          disableRay(sample.ray);
          nActiveRays--;
        },
        rayMiss
      );

      if (nActiveRays <= 0)
        return;

      // Get material color for rays which did hit something
      for_each_sample_n(
        stream,
        [&](SSR sample, int i) {
          auto &dg = dgs[i];

          StreamSimpleAOMaterial *mat =
              dynamic_cast<StreamSimpleAOMaterial*>(dg.material);

          if (mat) {
            sample.rgb = mat->Kd;
#if 0// NOTE(jda) - texture fetches not yet implemented
            if (mat->map_Kd) {
              vec4f Kd_from_map = get4f(mat->map_Kd, dg.st);
              sample.rgbcolor *=
                  vec3f(Kd_from_map.x, Kd_from_map.y, Kd_from_map.z);
            }
#endif
          } else {
            sample.rgb = vec3f{1.f};
          }

          // should be done in material:
          sample.rgb *= vec3f{dg.color.x, dg.color.y, dg.color.z};
        },
        rayHit
      );

      std::array<int, ScreenSampleStream::size> hits;
      std::fill(begin(hits), end(hits), 0);

      RayStream ao_rays;
      std::uniform_real_distribution<float> distribution {0.f, 1.f};

      // Disable AO rays which the primary ray missed
      for_each_sample_n(
        stream,
        [&](SSR /*sample*/, int i) {
          auto &ao_ray = ao_rays[i];
          ao_ray.t0 = inf;
          ao_ray.t  = 0.f;
        },
        rayMiss
      );

      for (int j = 0; j < samplesPerFrame; j++) {
        // Setup AO rays for active "lanes"
        for_each_sample_n(
          stream,
          [&](SSR sample, int i) {
            const float rot_x = 1.f - distribution(generator);
            const float rot_y = 1.f - distribution(generator);

            vec3f biNormU, biNormV;
            auto &dg = dgs[i];
            getBinormals(biNormU, biNormV, dg.Ns);

            auto &ray    = sample.ray;
            auto &ao_ray = ao_rays[i];

            ao_ray.org = (ray.org + ray.t * ray.dir) + (1e-3f * dg.Ns);
            ao_ray.dir = getRandomDir(biNormU, biNormV, dg.Ns,
                                      rot_x, rot_y, epsilon);
            ao_ray.t0  = epsilon;
            ao_ray.t   = aoRayLength - epsilon;
          },
          rayHit
        );

        // Trace AO rays
        occludeRays(ao_rays, RTC_INTERSECT_INCOHERENT);

        // Record occlusion test
        for_each_sample_n(
          stream,
          [&](SSR /*sample*/, int i) {
            auto &ao_ray = ao_rays[i];
            if (dot(ao_ray.dir, dgs[i].Ns) < 0.05f || ao_ray.hitSomething())
              hits[i]++;
          },
          rayHit
        );
      }

      auto writeColor = [&](SSR sample, int i)
      {
        float diffuse = abs(dot(dgs[i].Ns, sample.ray.dir));
        sample.rgb *= diffuse * (1.0f - float(hits[i])/samplesPerFrame);
      };

      for_each_sample_n(stream, writeColor, rayHit);
    }

    void StreamSimpleAORenderer::renderStream(void */*perFrameData*/,
                                              ScreenSampleStream &stream) const
    {
      traceRays(stream.rays, RTC_INTERSECT_COHERENT);
      shade_ao(stream);
    }

    Material *StreamSimpleAORenderer::createMaterial(const char */*type*/)
    {
      return new StreamSimpleAOMaterial;
    }

    OSP_REGISTER_RENDERER(StreamSimpleAORenderer, cpp_ao_stream)

  }// namespace cpp_renderer
}// namespace ospray
