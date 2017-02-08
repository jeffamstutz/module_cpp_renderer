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

    struct Address
    {
      //! The 1D address of the block in the volume containing the voxel.
      uint32 block;

      //! The 1D offset of the voxel in the enclosing block.
      uint32 voxel;
    };

    /*! struct that encodes all 8 corners in a (ghosted) block by block
      ID, base of the lower-left, and offset's to the +1's in x,y,and
      z. Deltas fully contain the effet of bricking */
    struct Address8
    {
      //! The 1D address of the block in the volume containing the voxel.
      uint32 block;

      //! The 1D offset (in bytes!) of the voxel in the enclosing block.
      uint32 voxelOfs;
      /*! the offset we have to add to voxelofs to go to the next voxel in
        x direction (only works for _one_ step in x, not multiple */
      uint32 voxelOfs_dx;
      /*! the offset we have to add to voxelofs to go to the next voxel in
        y direction (only works for _one_ step in y, not multiple */
      uint32 voxelOfs_dy;
      /*! the offset we have to add to voxelofs to go to the next voxel in
        z direction (only works for _one_ step in z, not multiple */
      uint32 voxelOfs_dz;
    };

    class GhostBlockBrickedVolume : public StructuredVolume
    {
    public:

      ~GhostBlockBrickedVolume();

      std::string toString() const override;

      void commit() override;

      int setRegion(const void *source,
                    const vec3i &index,
                    const vec3i &count) override;

    private:

      // StructuredVolume interface //

      float getVoxel(const vec3i &index) const override;
      float computeSample(const vec3f &worldCoordinates) const override;

      // Helper functions //

      template <typename T, size_t BLOCK_VOXEL_COUNT>
      float getVoxelValue(const Address &address) const;

      template <typename T, size_t BLOCK_VOXEL_COUNT>
      void setVoxelValues(void *_source,
                          const vec3i &targetCoord000,
                          const vec3i &regionSize,
                          size_t taskIndex);

      Address  getIndices(const vec3i &voxelIdxInVolume);
      Address8 getVoxelAddress(const vec3f &indexf, const vec3i &indexi) const;

      void constructVolumeMemory();
      void freeVolumeMemory();

      // Data //

      //! Volume size in blocks per dimension with padding to the nearest block.
      vec3i blockCount;

      //! pointer to the large array of blocks.
      byte_t *blockMem {nullptr};

      //! Voxel type.
      OSPDataType voxel_t {OSP_UNKNOWN};

      //! Voxel size in bytes.
      size_t voxelSize;

    };

    // Inlined definitions ////////////////////////////////////////////////////

    template<typename T, size_t BLOCK_VOXEL_COUNT>
    inline float
    GhostBlockBrickedVolume::getVoxelValue(const Address &address) const
    {
      NOT_IMPLEMENTED
    }

    template<typename T, size_t BLOCK_VOXEL_COUNT>
    inline void
    GhostBlockBrickedVolume::setVoxelValues(void *_source,
                                            const vec3i &targetCoord000,
                                            const vec3i &regionSize,
                                            size_t taskIndex)
    {
      NOT_IMPLEMENTED
    }

  } // ::ospray::cpp_renderer
} // ::ospray

