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

namespace ospray {
  namespace cpp_renderer {

    struct RandomTEA
    {
      RandomTEA(unsigned int idx, unsigned int seed) :
        v0(idx), v1(seed){}

      vec2f getFloats()
      {
        unsigned int sum = 0;

        for (int i = 0; i < 8; i++) { // just 8 instead of 32 rounds
          sum += 0x9e3779b9;
          v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + sum) ^ ((v1 >> 5) + 0xc8013ea4);
          v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + sum) ^ ((v0 >> 5) + 0x7e95761e);
        }

        const float tofloat = 2.3283064365386962890625e-10f; // 1/2^32
        return vec2f{v0 * tofloat, v1 * tofloat};
      }

      unsigned int v0, v1;
    };

    std::string SimpleAORenderer::toString() const
    {
      return "ospray::cpp_renderer::SimpleAORenderer";
    }

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

    inline vec3f getRandomDir(RandomTEA &rng,
                              const vec3f biNorm0,
                              const vec3f biNorm1,
                              const vec3f gNormal,
                              const float rot_x,
                              const float rot_y,
                              const float epsilon)
    {
      const vec2f rn = rng.getFloats();
      const float r0 = rotate(rn.x, rot_x);
      const float r1 = rotate(rn.y, rot_y);

      const float w = sqrt(1.f-r1);
      const float x = cos(float((2.f*M_PI)*r0))*w;
      const float y = sin(float((2.f*M_PI)*r0))*w;
      const float z = sqrt(r1) + epsilon;
      return x*biNorm0 + y*biNorm1 + z*gNormal;
    }

    inline void SimpleAORenderer::shade_ao(vec3f &color,
                                           float &alpha,
                                           const int accumID,
                                           const Ray &ray,
                                           const int32 pixel_x,
                                           const int32 pixel_y,
                                           const float rot_x,
                                           const float rot_y) const
    {
#if 0
      DifferentialGeometry dg;
      postIntersect(self->super.model,dg,ray,
                    DG_NG|DG_NS|DG_NORMALIZE|DG_FACEFORWARD
                    |DG_MATERIALID|DG_COLOR|DG_TEXCOORD);

      uniform SimpleAOMaterial *mat = ((uniform SimpleAOMaterial*)dg.material);
      vec3f superColor = make_vec3f(1.f);
      if (mat) {
        foreach_unique(m in mat) {
          superColor = m->Kd;
          if (m->map_Kd) {
            vec4f Kd_from_map = get4f(m->map_Kd,dg.st);
            superColor = superColor * make_vec3f(Kd_from_map);
          }
        }
      }
      // should be done in material:
      superColor = superColor * make_vec3f(dg.color);
#else
      vec3f superColor{1.0f};
#endif

      // init TEA RNG //
      RandomTEA rng{(currentFB->size.x * pixel_y) + pixel_x, accumID};

      int hits = 0;
      vec3f biNormU, biNormV;
      const vec3f N = normalize(getShadingNormal(ray));//dg.Ns
      getBinormals(biNormU, biNormV, N);

      for (int i = 0; i < samplesPerFrame; i++) {
        Ray ao_ray;
        ao_ray.org = (ray.org + ray.t * ray.dir) + (1e-3f * N);
        ao_ray.dir =  getRandomDir(rng, biNormU, biNormV, N,
                                   rot_x, rot_y, epsilon);
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
        const int accumID = sample.sampleID.z * samplesPerFrame;
        const float rot_x = 1.f - precomputedHalton3(accumID);
        const float rot_y = 1.f - precomputedHalton5(accumID);

        shade_ao(sample.rgb,
                 sample.alpha,
                 accumID,
                 sample.ray,
                 sample.sampleID.x,
                 sample.sampleID.y,
                 rot_x,rot_y);
      } else {
        sample.rgb = vec3f{1.f};
      }
    }

    OSP_REGISTER_RENDERER(SimpleAORenderer, cpp_ao)

  }// namespace cpp_renderer
}// namespace ospray
