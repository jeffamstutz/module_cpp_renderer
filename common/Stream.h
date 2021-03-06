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

#include "simd.h"
#include <array>

namespace ospray {
  namespace cpp_renderer {

    template <typename T, int N>
    using StreamN = std::array<T, N>;

    template <typename T>
    using Stream = StreamN<T, STREAM_SIZE>;

    constexpr auto SIMD_STREAM_SIZE = STREAM_SIZE / simd::width;

    template <typename T>
    using SimdStream = StreamN<T, SIMD_STREAM_SIZE>;

  }// namespace cpp_renderer
}// namespace ospray
