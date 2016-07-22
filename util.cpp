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

#include "util.h"

namespace ospray {
  namespace cpp_renderer {

    inline float radicalInverse(uint32 idx, const uint32 base)
    {
      float f = 0.f, g = 1.0f, inv = 1.0f/base;

      while (idx > 0) {
        g *= inv;
        f += (idx % base) * g;
        idx /= base;
      }

      return f;
    }

    inline float radicalInverse2(uint32 idx)
    {
      float f = 0.f, g = 1.0f;

      while (idx > 0) {
        g *= 0.5f;
        f += idx & 1 ? g : 0.f;
        idx >>= 1;
      }

      return f;
    }

    float precomputedHalton[3][NUM_PRECOMPUTED_HALTON_VALUES];
    bool  precomputedHalton_initialized = false;

    void precomputedHalton_create()
    {
      if (precomputedHalton_initialized)
        return;
      precomputedHalton_initialized = true;

      for (int i = 0; i < NUM_PRECOMPUTED_HALTON_VALUES; ++i) {
        precomputedHalton[0][i] = radicalInverse2(i);
        precomputedHalton[1][i] = radicalInverse(i,3);
        precomputedHalton[2][i] = radicalInverse(i,5);
      }
    }

    z_order_t z_order;
    bool z_order_initialized = false;

    void precomputedZOrder_create()
    {
      for(uint i = 0; i < TILE_SIZE*TILE_SIZE; i++) {
        deinterleaveZOrder(i, &z_order.xs[i], &z_order.ys[i]);
        z_order.xyIdx[i] = z_order.xs[i] | (z_order.ys[i] << 16);
      }

      z_order_initialized = true;
    }

  }// namespace cpp_renderer
}// namespace ospray
