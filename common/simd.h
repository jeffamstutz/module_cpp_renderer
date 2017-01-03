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

// ospcommon
#include "ospcommon/vec.h"
// boost.simd
#include "boost/simd.hpp"
#include "boost/simd/function/enumerate.hpp"
#include "boost/simd/function/all.hpp"
// std
#include <random>

namespace ospray {
  namespace simd {

    using namespace boost::simd;

    // Aliases for vector types based on boost::simd types ////////////////////

    // Pack types //

    template <typename T>
    using pack = boost::simd::pack<T>;

    using vfloat = pack<float>;
    using vint   = pack<int>;
    using vuint  = pack<uint32_t>;

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

    using vec2f = ospcommon::vec_t<vfloat, 2>;
    using vec2i = ospcommon::vec_t<vint,   2>;
    using vec2u = ospcommon::vec_t<vuint,  2>;

    using vec3f = ospcommon::vec_t<vfloat, 3>;
    using vec3i = ospcommon::vec_t<vint,   3>;
    using vec3u = ospcommon::vec_t<vuint,  3>;

    using vec4f = ospcommon::vec_t<vfloat, 4>;
    using vec4i = ospcommon::vec_t<vint,   4>;
    using vec4u = ospcommon::vec_t<vuint,  4>;

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

    template <typename SIMD_T, typename FCN_T>
    inline void vforeach(SIMD_T &v, FCN_T &&fcn)
    {
      // NOTE(jda) - need to static_assert() FCN_T's signature

      for (int i = 0; i < SIMD_T::static_size; ++i) {
        typename SIMD_T::value_type tmp = v[i];
        fcn(tmp, i);
        v[i] = tmp;
      }
    }

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
                    "mask type and simd type. (i.e. can't do maskf with vint)");

      for (int i = 0; i < SIMD_T::static_size; ++i) {
        if (mask_t<typename MASK_T::value_type>{l[i]}) {
          typename SIMD_T::value_type tmp = v[i];
          fcn(tmp, i);
          v[i] = tmp;
        }
      }
    }

    // Helper functions ///////////////////////////////////////////////////////

    static thread_local std::minstd_rand generator;
    static std::uniform_real_distribution<float> distribution {0.f, 1.f};
    static inline simd::vfloat randUniformDist()
    {
      simd::vfloat retval;
      simd::vforeach(retval, [&](float &v, int i) {
        v = distribution(generator);
      });
      return retval;
    }

    // Constants //////////////////////////////////////////////////////////////

    const int  width        = vfloat::static_size;
    const vint programIndex = boost::simd::enumerate<vint>(0, 1);

  }// namespace simd
}// namespace ospray
