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

//ospray
#include "BlockBrickedVolume.h"
#include "ospcommon/tasking/parallel_for.h"

//! The number of bits used to represent the width of a Block in voxels.
#define BLOCK_VOXEL_WIDTH_BITCOUNT (6)

//! The number of bits used to represent the width of a brick in voxels.
#define BRICK_VOXEL_WIDTH_BITCOUNT (2)

//! The number of bits used to represent the width of a block in bricks.
#define BLOCK_BRICK_WIDTH_BITCOUNT (BLOCK_VOXEL_WIDTH_BITCOUNT - BRICK_VOXEL_WIDTH_BITCOUNT)

//! The width of a block in voxels.
#define BLOCK_VOXEL_WIDTH (1 << BLOCK_VOXEL_WIDTH_BITCOUNT)

//! The width of a brick in voxels.
#define BRICK_VOXEL_WIDTH (1 << BRICK_VOXEL_WIDTH_BITCOUNT)

//! The width of a block in bricks.
#define BLOCK_BRICK_WIDTH (1 << BLOCK_BRICK_WIDTH_BITCOUNT)

//! The bits denoting the offset of a brick within a block.
#define BLOCK_BRICK_BITMASK (BLOCK_BRICK_WIDTH - 1)

//! The bits denoting the offset of a voxel within a brick.
#define BRICK_VOXEL_BITMASK (BRICK_VOXEL_WIDTH - 1)

//! The number of voxels contained in a block.
#define BLOCK_VOXEL_COUNT (BLOCK_VOXEL_WIDTH * BLOCK_VOXEL_WIDTH * BLOCK_VOXEL_WIDTH)

namespace ospray {
  namespace cpp_renderer {

    using BBV = BlockBrickedVolume;

    // BlockBrickedVolume definitions /////////////////////////////////////////

    BlockBrickedVolume::~BlockBrickedVolume()
    {
      if (blockMem) delete [] blockMem;
    }

    std::string BBV::toString() const
    {
      return("ospray::cpp_renderer::BBV<" + voxelType + ">");
    }

    void BBV::commit()
    {
      if (!finished) {
        // Get the voxel type.
        voxelType = getParamString("voxelType", "unspecified");
        voxel_t   = getVoxelType();
        voxelSize = sizeOf(voxel_t);

        // Get the volume dimensions.
        this->dimensions = getParam3i("dimensions", vec3i(0));
        exitOnCondition(reduce_min(this->dimensions) <= 0,
                        "invalid volume dimensions (must be set before "
                        "calling ospSetRegion())");

        // Volume size in blocks per dimension with padding to the nearest block
        blockCount = (dimensions + BLOCK_VOXEL_WIDTH - 1) / BLOCK_VOXEL_WIDTH;

        // Volume size in blocks with padding.
        const size_t numBlocks = blockCount.x * blockCount.y * blockCount.z;

        // allocate the large array of blocks
        size_t blockSize = BLOCK_VOXEL_COUNT * voxelSize;
        blockMem = new byte_t[blockSize * numBlocks];
      }

      StructuredVolume::commit();
    }

    int BBV::setRegion(
        // points to the first voxel to be copied. The voxels at 'source' MUST
        // have dimensions 'regionSize', must be organized in 3D-array order, and
        // must have the same voxel type as the volume.
        const void *source,
        // coordinates of the lower, left, front corner of the target region
        const vec3i &regionCoords,
        // size of the region that we're writing to, MUST be the same as the
        // dimensions of source[][][]
        const vec3i &regionSize)
    {
      vec3i finalRegionSize   = regionSize;
      vec3i finalRegionCoords = regionCoords;
      void *finalSource       = const_cast<void*>(source);

      const bool upsampling = scaleRegion(source, finalSource,
                                          finalRegionSize, finalRegionCoords);
      // Copy voxel data into the volume.
      const size_t NTASKS = finalRegionSize.y * finalRegionSize.z;

      switch (voxel_t) {
      case OSP_UCHAR:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<uint8, BLOCK_VOXEL_COUNT>(finalSource,
                                                   finalRegionCoords,
                                                   finalRegionSize,
                                                   taskIndex);
        });
        break;
      case OSP_SHORT:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<int16, BLOCK_VOXEL_COUNT>(finalSource,
                                                   finalRegionCoords,
                                                   finalRegionSize,
                                                   taskIndex);
        });
        break;
      case OSP_USHORT:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<uint16, BLOCK_VOXEL_COUNT>(finalSource,
                                                    finalRegionCoords,
                                                    finalRegionSize,
                                                    taskIndex);
        });
        break;
      case OSP_FLOAT:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<float, BLOCK_VOXEL_COUNT>(finalSource,
                                                   finalRegionCoords,
                                                   finalRegionSize,
                                                   taskIndex);
        });
        break;
      case OSP_DOUBLE:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<double, BLOCK_VOXEL_COUNT>(finalSource,
                                                    finalRegionCoords,
                                                    finalRegionSize,
                                                    taskIndex);
        });
        break;
      default:
        break;
      }

      // If we're upsampling finalSource points at the chunk of data allocated by
      // scaleRegion to hold the upsampled volume data and we must free it.
      if (upsampling) {
        free(finalSource);
      }

      return true;
    }

    float BBV::getVoxel(const vec3i &index) const
    {
      /* Compute the 1D address of the block in the volume
       and the voxel in the block. */
      Address address = getVoxelAddress(index);

      switch (voxel_t) {
      case OSP_UCHAR:
        return getVoxelValue<uint8, BLOCK_VOXEL_COUNT>(address);
        break;
      case OSP_SHORT:
        return getVoxelValue<int16, BLOCK_VOXEL_COUNT>(address);
        break;
      case OSP_USHORT:
        return getVoxelValue<uint16, BLOCK_VOXEL_COUNT>(address);
        break;
      case OSP_FLOAT:
        return getVoxelValue<float, BLOCK_VOXEL_COUNT>(address);
        break;
      case OSP_DOUBLE:
        return getVoxelValue<double, BLOCK_VOXEL_COUNT>(address);
        break;
      default:
        break;
      }

      return 0.f;
    }

    BBV::Address BBV::getVoxelAddress(const vec3i &index) const
    {
      Address address;

      // Compute the 3D index of the block containing the brick containing the
      // voxel.
      const vec3i blockIndex {index.x >> BLOCK_VOXEL_WIDTH_BITCOUNT,
                              index.y >> BLOCK_VOXEL_WIDTH_BITCOUNT,
                              index.z >> BLOCK_VOXEL_WIDTH_BITCOUNT};

      // Compute the 1D address of the block in the volume.
      address.block =
          blockIndex.x + blockCount.x * (blockIndex.y + blockCount.y * blockIndex.z);

      // Compute the 3D offset of the brick within the block containing the voxel.
      const vec3i brickOffset {blockIndex.x & BLOCK_BRICK_BITMASK,
                               blockIndex.y & BLOCK_BRICK_BITMASK,
                               blockIndex.z & BLOCK_BRICK_BITMASK};

      // Compute the 1D address of the brick in the block.
      const uint32 brickAddress
        = brickOffset.x
        + (brickOffset.y << BLOCK_BRICK_WIDTH_BITCOUNT)
        + (brickOffset.z << 2 * BLOCK_BRICK_WIDTH_BITCOUNT);

      // Compute the 3D offset of the voxel in the brick.
      const vec3i voxelOffset {index.x & BRICK_VOXEL_BITMASK,
                               index.y & BRICK_VOXEL_BITMASK,
                               index.z & BRICK_VOXEL_BITMASK};

      // Compute the 1D address of the voxel in the block.
      address.voxel
        = brickAddress  << (3 * BRICK_VOXEL_WIDTH_BITCOUNT)
        | voxelOffset.z << (2 * BRICK_VOXEL_WIDTH_BITCOUNT)
        | voxelOffset.y << BRICK_VOXEL_WIDTH_BITCOUNT
        | voxelOffset.x;

      return address;
    }

#if 0//def EXP_NEW_BB_VOLUME_KERNELS
    /*! in new bb kernel mode we'll be using the code in
        GhostBlockBrickedVolume.* */
#else
    // A volume type with 64-bit addressing and multi-level bricked storage order.
    OSP_REGISTER_VOLUME(BlockBrickedVolume, cpp_block_bricked_volume);
    OSP_REGISTER_VOLUME(BlockBrickedVolume, cpp_bbv);
#endif

  } // ::ospray::cpp_renderer
} // ::ospray

