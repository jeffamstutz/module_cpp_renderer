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
#include "boost/simd/function/all.hpp"

namespace ospray {

  namespace simd {

    using namespace boost::simd;

    // Aliases for vector types based on boost::simd types ////////////////////

    using vfloat = boost::simd::pack<float>;
    using vint   = boost::simd::pack<int>;
    using vmaskf = boost::simd::pack<boost::simd::logical<float>>;
    using vmaski = boost::simd::pack<boost::simd::logical<int>>;

    const int width = vfloat::static_size;

    using vec2f = ospcommon::vec_t<vfloat, 2>;
    using vec2i = ospcommon::vec_t<vint,   2>;
    using vec3f = ospcommon::vec_t<vfloat, 3>;
    using vec3i = ospcommon::vec_t<vint,   3>;
    using vec4f = ospcommon::vec_t<vfloat, 4>;
    using vec4i = ospcommon::vec_t<vint,   4>;

    // Cast operatons //

    template <typename NewType, typename OriginalType>
    NewType cast(const OriginalType &t)
    {
      NewType nt;

      for (int i = 0; i < OriginalType::static_size; ++i)
        nt[i] = t[i];

      return nt;
    }
  }
}
