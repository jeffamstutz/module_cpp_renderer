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

#pragma once

#include "../Renderer.h"


namespace ospray {
  namespace cpp_renderer {

    // AO helper functions ////////////////////////////////////////////////////

    extern thread_local std::minstd_rand ao_rng;

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
                              float epsilon)
    {
      static std::uniform_real_distribution<float> distribution {0.f, 1.f};

      const float rot_x = 1.f - distribution(ao_rng);
      const float rot_y = 1.f - distribution(ao_rng);

      const vec2f rn = vec2f{distribution(ao_rng), distribution(ao_rng)};
      const float r0 = rotate(rn.x, rot_x);
      const float r1 = rotate(rn.y, rot_y);

      const float w = sqrt(1.f-r1);
      const float x = cos(float((2.f*M_PI)*r0))*w;
      const float y = sin(float((2.f*M_PI)*r0))*w;
      const float z = sqrt(r1) + epsilon;
      return x*biNorm0 + y*biNorm1 + z*gNormal;
    }

    struct ao_context
    {
      vec3f biNormU, biNormV;
      float rayLength;
      float epsilon;
    };

    inline ao_context getAOContext(const DifferentialGeometry &dg,
                                   float rayLength = 1e20f,
                                   float epsilon = 1e-6)
    {
      ao_context ctx;
      getBinormals(ctx.biNormU, ctx.biNormV, dg.Ng);
      ctx.rayLength = rayLength;
      ctx.epsilon   = epsilon;
      return ctx;
    }

    inline Ray calculateAORay(const DifferentialGeometry &dg,
                              const ao_context &ctx)
    {
      Ray ao_ray;
      ao_ray.org = dg.P + (1e-3f * dg.Ng);
      ao_ray.dir = getRandomDir(ctx.biNormU, ctx.biNormV, dg.Ng, ctx.epsilon);
      ao_ray.t0  = ctx.epsilon;
      ao_ray.t   = ctx.rayLength - ctx.epsilon;

      return ao_ray;
    }

  }// namespace cpp_renderer
}// namespace ospray
