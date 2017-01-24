// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
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

#include "StructuredVolume.h"

namespace ospray {
  namespace cpp_renderer {

    class BlockBrickedVolume : public StructuredVolume
    {
    public:

      ~BlockBrickedVolume();

      std::string toString() const override;

      void commit() override;

      int setRegion(const void *source,
                    const vec3i &index,
                    const vec3i &count) override;

    private:

      // Helper types //

      struct Address
      {
        //! The 1D address of the block in the volume containing the voxel.
        uint32 block;

        //! The 1D offset of the voxel in the enclosing block.
        uint32 voxel;
      };

      // StructuredVolume interface //

      float getVoxel(const vec3i &index) const override;

      // Helper functions //

      template <typename T, size_t BLOCK_VOXEL_COUNT>
      float getVoxelValue(const Address &address) const;

      template <typename T, size_t BLOCK_VOXEL_COUNT>
      void setVoxelValues(void *_source,
                          const vec3i &targetCoord000,
                          const vec3i &regionSize,
                          size_t taskIndex);

      Address getVoxelAddress(const vec3i &index) const;

      // Data //

      //! Volume size in blocks per dimension with padding to the nearest block.
      vec3i blockCount;

      //! pointer to the large array of blocks.
      byte_t *blockMem {nullptr};

      //! Voxel type.
      OSPDataType voxel_t;

      //! Voxel size in bytes.
      size_t voxelSize;

    };

    // Inlined definitions ////////////////////////////////////////////////////

    template<typename T, size_t BLOCK_VOXEL_COUNT>
    inline float BlockBrickedVolume::getVoxelValue(const Address &address) const
    {
      T *blockPtr = (T*)blockMem + (BLOCK_VOXEL_COUNT * address.block);
      return float(blockPtr[address.voxel]);
    }

    template<typename T, size_t BLOCK_VOXEL_COUNT>
    inline void BlockBrickedVolume::setVoxelValues(void *_source,
                                                   const vec3i &targetCoord000,
                                                   const vec3i &regionSize,
                                                   size_t taskIndex)
    {
      const uint32 region_y = taskIndex % regionSize.y;
      const uint32 region_z = taskIndex / regionSize.y;
      const uint64 runOfs = (uint64)regionSize.x *
                            (region_y + (uint64)regionSize.y * region_z);
      const T *run = (const T *)_source + runOfs;
      vec3i coord = targetCoord000 + vec3i{0, region_y, region_z};
      for(int x = 0; x < regionSize.x; ++x) {
        coord.x = targetCoord000.x + x;
        if (coord.x < 0 ||
            coord.y < 0 ||
            coord.z < 0 ||
            coord.x >= dimensions.x ||
            coord.y >= dimensions.y ||
            coord.z >= dimensions.z
            )
          continue;

        Address address = getVoxelAddress(coord);
        T *blockPtr = (T*)blockMem + address.block * BLOCK_VOXEL_COUNT;
        blockPtr[address.voxel] = run[x];
      }
    }

  } // ::ospray::cpp_renderer
} // ::ospray

