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
#include "../util.h"

#include <random>

static thread_local std::minstd_rand generator;

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

    // Helper functions ///////////////////////////////////////////////////////

    inline vec3f getShadingNormal(const Ray &ray)
    {
      vec3f N = ray.Ng;
      float f = rcp(sqrt(dot(N,N)));
      if (dot(N,ray.dir) >= 0.f) f = -f;
      return f*N;
    }

    void getBinormals(vec3f &biNorm0, vec3f &biNorm1, const vec3f &gNormal)
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
                                           const Ray &ray,
                                           const float rot_x,
                                           const float rot_y) const
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
      vec3f biNormU, biNormV;
      const vec3f &N = dg.Ns;
      getBinormals(biNormU, biNormV, N);

      for (int i = 0; i < samplesPerFrame; i++) {
        Ray ao_ray;
        ao_ray.org = (ray.org + ray.t * ray.dir) + (1e-3f * N);
        ao_ray.dir = getRandomDir(biNormU, biNormV, N, rot_x, rot_y, epsilon);
        ao_ray.t0 = epsilon;
        ao_ray.t  = aoRayLength - epsilon;

        if (dot(ao_ray.dir, N) < 0.05f || isOccluded(ao_ray))
          hits++;
      }

      float diffuse = abs(dot(N,ray.dir));
      color = superColor * vec3f{diffuse * (1.0f-float(hits)/samplesPerFrame)};
      alpha = 1.f;
    }

    void SimpleAORenderer::renderSample(void */*perFrameData*/,
                                        ScreenSample &sample) const
    {
      auto &ray = sample.ray;

      traceRay(ray);

      if (ray.geomID != RTC_INVALID_GEOMETRY_ID) {
        std::uniform_real_distribution<float> distribution {0.f, 1.f};
        const float rot_x = 1.f - distribution(generator);
        const float rot_y = 1.f - distribution(generator);
        shade_ao(sample.rgb, sample.alpha, sample.ray, rot_x,rot_y);
      } else {
        sample.rgb = bgColor;
      }
    }

    Material *SimpleAORenderer::createMaterial(const char */*type*/)
    {
      return new SimpleAOMaterial;
    }

    OSP_REGISTER_RENDERER(SimpleAORenderer, cpp_ao)

  }// namespace cpp_renderer
}// namespace ospray
