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

#include "common/OSPCommon.h"

namespace ospray {
  namespace cpp_renderer {

    /*! \file ospray/render/util.ih \brief Utility-functions for shaders */

    inline vec3f make_random_color(const int i)
    {
      const int mx = 13*17*43;
      const int my = 11*29;
      const int mz = 7*23*63;
      const uint32 g = (i * (3*5*127)+12312314);
      return {(g % mx)*(1.f/(mx-1)),
              (g % my)*(1.f/(my-1)),
              (g % mz)*(1.f/(mz-1))};
    }

    /*! struct that stores a precomputed z-order for tiles of
     *  (TILE_SIZExTILE_SIZE) pixels */
    struct z_order_t {
      /*! 32-bit field specifying both x and y coordinate of the z-order,
          with upper 16 bits for the y coordinate, and lower 16 for the x
          coordinate. Compared to using two uint32-arrays, this saves on
          gather-loop */
      uint xyIdx[TILE_SIZE*TILE_SIZE];
      uint xs[TILE_SIZE*TILE_SIZE];
      uint ys[TILE_SIZE*TILE_SIZE];
    };

    inline uint getZOrderX(const uint &xs16_ys16)
    {
      return xs16_ys16 & (0xffff);
    }

    inline uint getZOrderY(const uint &xs16_ys16)
    {
      return xs16_ys16 >> 16;
    }

    extern z_order_t z_order;
    extern bool z_order_initialized;

    inline uint partitionZOrder(uint n) {
      n &= 0x0000FFFF;
      n = (n | (n << 8)) & 0x00FF00FF;
      n = (n | (n << 4)) & 0x0F0F0F0F;
      n = (n | (n << 2)) & 0x33333333;
      n = (n | (n << 1)) & 0x55555555;
      return n;
    }

    inline uint unpartitionZOrder(uint n) {
      n &= 0x55555555;
      n = (n ^ (n >> 1)) & 0x33333333;
      n = (n ^ (n >> 2)) & 0x0F0F0F0F;
      n = (n ^ (n >> 4)) & 0x00FF00FF;
      n = (n ^ (n >> 8)) & 0x0000FFFF;
      return n;
    }

    inline uint interleaveZOrder(uint x, uint y) {
      return partitionZOrder(x) | (partitionZOrder(y) << 1);
    }

    inline void deinterleaveZOrder(uint z,  uint *x, uint *y) {
      assert(x); assert(y);
      *x = *y = 0;
      *x = unpartitionZOrder(z);
      *y = unpartitionZOrder(z >> 1);
    }

    /*! precompute a per-pixel z-order to be used within a tile */
    extern void precomputedZOrder_create();

    /*! precompute a per-pixel z-order to be used within a tile */
    inline void precomputeZOrder()
    {
      if (!z_order_initialized) precomputedZOrder_create();
    }

  }// namespace cpp_renderer
}// namespace ospray
