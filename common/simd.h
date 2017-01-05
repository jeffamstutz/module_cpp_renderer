// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
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

// boost.simd
#include "boost/simd.hpp"
#include "boost/simd/arithmetic.hpp"
#include "boost/simd/function/all.hpp"
#include "boost/simd/function/enumerate.hpp"
#include "boost/simd/function/store.hpp"
// ospcommon
namespace ospcommon {
  using namespace boost::simd;

  // Pack types //

  template <typename T>
  using pack = boost::simd::pack<T>;

  using vfloat = pack<float>;
  using vint   = pack<int>;
  using vuint  = pack<uint32_t>;

  inline vfloat rsqrt(const vfloat &val)
  {
    return boost::simd::rsqrt(val);
  }
}
#include "ospcommon/vec.h"
// std
#include <random>

namespace ospray {
  namespace simd {

    using namespace boost::simd;

    // Constants //////////////////////////////////////////////////////////////

    constexpr auto width    = boost::simd::pack<float>::static_size;
    const auto programIndex =
        boost::simd::enumerate<boost::simd::pack<int>>(0, 1);

    // Aliases for vector types based on boost::simd types ////////////////////

    // Pack types //

    template <typename T>
    using pack = boost::simd::pack<T>;

    using vfloat = pack<float>;
    using vint   = pack<int>;
    using vuint  = pack<uint32_t>;

    template <typename T>
    using vptr = std::array<T*, width>;

    // Mask types //

    template <typename T>
    using mask_t = boost::simd::logical<T>;

    using maskf = mask_t<float>;
    using maski = mask_t<int>;
    using masku = mask_t<uint32_t>;

    using vmaskf = pack<maskf>;
    using vmaski = pack<maski>;
    using vmasku = pack<masku>;

    // Vector math types //

    template <typename T, int N>
    using vec_t = ospcommon::vec_t<T, N>;

    using vec2f = vec_t<vfloat, 2>;
    using vec2i = vec_t<vint,   2>;
    using vec2u = vec_t<vuint,  2>;

    using vec3f = vec_t<vfloat, 3>;
    using vec3i = vec_t<vint,   3>;
    using vec3u = vec_t<vuint,  3>;

    using vec4f = vec_t<vfloat, 4>;
    using vec4i = vec_t<vint,   4>;
    using vec4u = vec_t<vuint,  4>;

    // Cast operatons /////////////////////////////////////////////////////////

    template <typename NewType, typename OriginalType>
    NewType cast(const OriginalType &t)
    {
      NewType nt;

      for (int i = 0; i < OriginalType::static_size; ++i)
        nt[i] = t[i];

      return nt;
    }

    // Algorithms /////////////////////////////////////////////////////////////

    // foreach //

    template <typename SIMD_T, typename FCN_T>
    inline void foreach_v(SIMD_T &v, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature

      for (int i = 0; i < SIMD_T::static_size; ++i) {
        typename SIMD_T::value_type tmp = v[i];
        fcn(tmp, i);
        v[i] = tmp;
      }
    }

    // foreach_active //

    template <typename MASK_T, typename FCN_T>
    inline void foreach_active(const MASK_T &l, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature

      for (int i = 0; i < MASK_T::static_size; ++i)
        if (mask_t<typename MASK_T::value_type>{l[i]}) fcn(i);
    }

    template <typename MASK_T, typename SIMD_T, typename FCN_T>
    inline void foreach_active(const MASK_T &l, SIMD_T &v, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature
      static_assert(std::is_same<typename MASK_T::value_type,
                                 typename SIMD_T::value_type>::value,
                    "The LOGICAL_T and SIMD_T types provided must be of the "
                    "same value type. In other words, you can't mismatch the "
                    "mask type and simd type. (ex: can't do vmaskf with vint)");

      for (int i = 0; i < SIMD_T::static_size; ++i) {
        if (mask_t<typename MASK_T::value_type>{l[i]}) {
          typename SIMD_T::value_type tmp = v[i];
          fcn(tmp, i);
          v[i] = tmp;
        }
      }
    }

    // select //

    template <typename MASK_T, typename T1, typename T2>
    inline auto select(const MASK_T &m, const T1 &t, const T2& f)
      -> decltype(boost::simd::if_else(m, t, f))
    {
      return boost::simd::if_else(m, t, f);
    }

    // NOTE(jda) - Add variants which allow scalar values for either t or f
    //             types.

    template <typename MASK_T, typename T>
    inline vec_t<T, 2> select(const MASK_T &m,
                              const vec_t<T, 2> &t,
                              const vec_t<T, 2> &f)
    {
      return {select(m, t.x, f.x), select(m, t.y, f.y)};
    }

    template <typename MASK_T, typename T>
    inline vec_t<T, 3> select(const MASK_T &m,
                              const vec_t<T, 3> &t,
                              const vec_t<T, 3> &f)
    {
      return {select(m, t.x, f.x), select(m, t.y, f.y), select(m, t.z, f.z)};
    }

    template <typename MASK_T, typename T>
    inline vec_t<T, 4> select(const MASK_T &m,
                              const vec_t<T, 4> &t,
                              const vec_t<T, 4> &f)
    {
      return {select(m, t.x, f.x), select(m, t.y, f.y),
              select(m, t.z, f.z), select(m, t.w, f.w)};
    }

    // Helper functions ///////////////////////////////////////////////////////

    // NOTE(jda) - This adds a random number generator per C++ translation unit,
    //             which means random number sequences between different .cpp
    //             files may overlap or be identical...beware!
    static thread_local std::minstd_rand generator;
    static std::uniform_real_distribution<float> distribution {0.f, 1.f};
    static inline simd::vfloat randUniformDist()
    {
      simd::vfloat retval;
      simd::foreach_v(retval, [&](float &v, int i) {
        v = distribution(generator);
      });
      return retval;
    }

  }// namespace simd
}// namespace ospray
