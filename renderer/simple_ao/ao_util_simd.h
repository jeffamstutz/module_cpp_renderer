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

#include "../SimdRenderer.h"

namespace ospray {
  namespace cpp_renderer {

    // AO helper functions ////////////////////////////////////////////////////

    inline void getBinormals(simd::vec3f &biNorm0,
                             simd::vec3f &biNorm1,
                             const simd::vec3f &gNormal)
    {
      biNorm0 = simd::select(simd::abs(gNormal.x) > .95f,
                             simd::make_vec3f(0.f, 1.f, 0.f),
                             simd::make_vec3f(1.f, 0.f, 0.f));
      biNorm1 = normalize(cross(biNorm0,gNormal));
      biNorm0 = normalize(cross(biNorm1,gNormal));
    }

    struct ao_contextN
    {
      simd::vec3f biNormU, biNormV;
      float rayLength;
      float epsilon;
    };

    inline ao_contextN getAOContext(const DifferentialGeometryN &dg,
                                    float rayLength = 1e20f,
                                    float epsilon   = 1e-6)
    {
      ao_contextN ctx;
      getBinormals(ctx.biNormU, ctx.biNormV, dg.Ng);
      ctx.rayLength = rayLength;
      ctx.epsilon   = epsilon;
      return ctx;
    }

    inline simd::vfloat rotate(simd::vfloat x, simd::vfloat dx)
    {
      x += dx;
      return simd::select(x >= 1.f, x - 1.f, x);
    }

    inline simd::vec3f getRandomDir(const simd::vec3f &biNorm0,
                                    const simd::vec3f &biNorm1,
                                    const simd::vec3f &gNormal,
                                    float epsilon)
    {
      const auto rot_x = 1.f - simd::randUniformDist3();
      const auto rot_y = 1.f - simd::randUniformDist5();

      const auto rn = simd::vec2f{simd::randUniformDist2(),
                                  simd::randUniformDist2()};
      const auto r0 = rotate(rn.x, rot_x);
      const auto r1 = rotate(rn.y, rot_y);

      const auto w = simd::sqrt(1.f-r1);
      const auto x = simd::cos((2.f*simd::vfloat{M_PI}*r0))*w;
      const auto y = simd::sin((2.f*simd::vfloat{M_PI}*r0))*w;
      const auto z = simd::sqrt(r1) + epsilon;
      return x*biNorm0 + y*biNorm1 + z*gNormal;
    }

    // NOTE(jda) - RandomTEA variant of getRandomDir()
    template <typename RANDOM_TEA_T>
    inline simd::vec3f getRandomDir(RANDOM_TEA_T &rng,
                                    const simd::vec3f &biNorm0,
                                    const simd::vec3f &biNorm1,
                                    const simd::vec3f &gNormal,
                                    float epsilon)
    {
      const auto rot_x = 1.f - simd::randUniformDist3();
      const auto rot_y = 1.f - simd::randUniformDist5();
      const auto rn = rng.getFloats();

      const auto r0 = rotate(rn.x, rot_x);
      const auto r1 = rotate(rn.y, rot_y);

      const auto w = simd::sqrt(1.f-r1);
      const auto x = simd::cos((2.f*simd::vfloat{M_PI}*r0))*w;
      const auto y = simd::sin((2.f*simd::vfloat{M_PI}*r0))*w;
      const auto z = simd::sqrt(r1) + epsilon;
      return x*biNorm0 + y*biNorm1 + z*gNormal;
    }

    inline RayN calculateAORay(const DifferentialGeometryN &dg,
                               const ao_contextN &ctx)
    {
      RayN ao_ray;
      ao_ray.org = dg.P + (simd::vfloat{1e-3f} * dg.Ns);
      ao_ray.dir = getRandomDir(ctx.biNormU, ctx.biNormV, dg.Ns, ctx.epsilon);
      ao_ray.t0  = ctx.epsilon;
      ao_ray.t   = ctx.rayLength - ctx.epsilon;
      return ao_ray;
    }

    // NOTE(jda) - RandomTEA variant of calculateAORay()
    template <typename RANDOM_TEA_T>
    inline RayN calculateAORay(const DifferentialGeometryN &dg,
                               const ao_contextN &ctx,
                               RANDOM_TEA_T &rng)
    {
      RayN ao_ray;
      ao_ray.org = dg.P + (simd::vfloat{1e-3f} * dg.Ns);
      ao_ray.dir = getRandomDir(rng, ctx.biNormU, ctx.biNormV,
                                dg.Ns, ctx.epsilon);
      ao_ray.t0  = ctx.epsilon;
      ao_ray.t   = ctx.rayLength - ctx.epsilon;
      return ao_ray;
    }

  }// namespace cpp_renderer
}// namespace ospray
