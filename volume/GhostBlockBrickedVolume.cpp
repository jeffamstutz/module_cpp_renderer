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
#include "GhostBlockBrickedVolume.h"
#include "ospcommon/tasking/parallel_for.h"

/*! total number of bits per block dimension. '6' would mean 18 bits =
  1/4million voxels per block, which for alots would be 1MB, so should
  all fit into a single 2M page (assuming we do 2M allocs, of course) */
#define BLOCK_BITS (6)
#define BRICK_BITS (2)

/*! @{ number of voxels per block per dimension */
#define BLOCK_WIDTH (1<<BLOCK_BITS)
#define BRICK_WIDTH (1<<BRICK_BITS)
/*! @} */

/*! bit mask for extracting the _brick_ bits from a given per-block
  coordinate */
#define BRICK_MASK (BRICK_WIDTH-1)

/*! size of one block, in voxels */
#define VOXELS_PER_BLOCK (BLOCK_WIDTH*BLOCK_WIDTH*BLOCK_WIDTH)

/*! @{ start bit for given section of address in bricked offset field

  lo is the address part for inside a brick, hi is the remaining part
  inside the blcok ex the brick part
*/
#define BRICK_BIT_X_LO (0)
#define BRICK_BIT_Y_LO (BRICK_BIT_X_LO+BRICK_BITS)
#define BRICK_BIT_Z_LO (BRICK_BIT_Y_LO+BRICK_BITS)
#define BRICK_BIT_X_HI (BRICK_BIT_Z_LO+BRICK_BITS)
#define BRICK_BIT_Y_HI (BRICK_BIT_X_HI+(BLOCK_BITS-BRICK_BITS))
#define BRICK_BIT_Z_HI (BRICK_BIT_Y_HI+(BLOCK_BITS-BRICK_BITS))
/* @} */

/*! @{ how much we have to sacle a given linear coordinate inside a
  brick to end up at the proper bit fields required by the brick
  translation */
#define BRICK_BIT_SCALE_X_LO ((1<<BRICK_BIT_X_LO))
#define BRICK_BIT_SCALE_Y_LO ((1<<BRICK_BIT_Y_LO))
#define BRICK_BIT_SCALE_Z_LO ((1<<BRICK_BIT_Z_LO))
#define BRICK_BIT_SCALE_X_HI ((1<<BRICK_BIT_X_HI))
#define BRICK_BIT_SCALE_Y_HI ((1<<BRICK_BIT_Y_HI))
#define BRICK_BIT_SCALE_Z_HI ((1<<BRICK_BIT_Z_HI))
/* @} */

/*! @{ number of bits we have to SHIFT addresses for given type */
#define shift_per_uint8 0
#define shift_per_int16 1
#define shift_per_uint16 1
#define shift_per_float 2
#define shift_per_double 3
/*! @} */

/*! @{ number of bits we have to MULTIPLY offsets with for given
type */
#define scale_per_uint8 1
#define scale_per_int16 2
#define scale_per_uint16 2
#define scale_per_float 4
#define scale_per_double 8
/*! @} */

namespace ospray {
  namespace cpp_renderer {

    using GBBV = GhostBlockBrickedVolume;

    // Helper functions ///////////////////////////////////////////////////////

    /*! fast 32-bit integer division via floats */
    inline int intDiv(int x)
    {
      return (int)((float)x * (1.f/(BLOCK_WIDTH-1.f)));
    }

    /*! fast 32-bit integer division via floats; for the case where we
      already have a float-version of the original integer */
    inline int intDiv(float x)
    {
      return (int)((float)x * (1.f/(BLOCK_WIDTH-1.f)));
    }

    /*! compute address8 for a given voxel index, including bricking */
    template<typename VOXEL_T, int SCALE_PER, int SHIFT_PER>
    inline Address8 brickTranslation(const vec3i &voxelIdxInBlock)
    {
      Address8 addr;

      const vec3i brickIdxInBlock {voxelIdxInBlock.x >> BRICK_BITS,
                                   voxelIdxInBlock.y >> BRICK_BITS,
                                   voxelIdxInBlock.z >> BRICK_BITS};
      const vec3i voxelIdxInBrick {voxelIdxInBlock.x & BRICK_MASK,
                                   voxelIdxInBlock.y & BRICK_MASK,
                                   voxelIdxInBlock.z & BRICK_MASK};

      addr.voxelOfs_dx = (voxelIdxInBrick.x == (BRICK_WIDTH-1))
        ?
        + (BRICK_BIT_SCALE_X_HI*SCALE_PER)
        - ((BRICK_WIDTH-1)*BRICK_BIT_SCALE_X_LO*SCALE_PER)
        : (BRICK_BIT_SCALE_X_LO*SCALE_PER);

      addr.voxelOfs_dy = (voxelIdxInBrick.y == (BRICK_WIDTH-1))
        ?
        + (BRICK_BIT_SCALE_Y_HI*SCALE_PER)
        - ((BRICK_WIDTH-1)*BRICK_BIT_SCALE_Y_LO*SCALE_PER)
        : (BRICK_BIT_SCALE_Y_LO*SCALE_PER);

      addr.voxelOfs_dz = (voxelIdxInBrick.z == (BRICK_WIDTH-1))
        ?
        + (BRICK_BIT_SCALE_Z_HI*SCALE_PER)
        - ((BRICK_WIDTH-1)*BRICK_BIT_SCALE_Z_LO*SCALE_PER)
        : (BRICK_BIT_SCALE_Z_LO*SCALE_PER);

      addr.voxelOfs =
        (voxelIdxInBrick.x << (BRICK_BIT_X_LO+SHIFT_PER)) |
        (voxelIdxInBrick.y << (BRICK_BIT_Y_LO+SHIFT_PER)) |
        (voxelIdxInBrick.z << (BRICK_BIT_Z_LO+SHIFT_PER)) |
        (brickIdxInBlock.x << (BRICK_BIT_X_HI+SHIFT_PER)) |
        (brickIdxInBlock.y << (BRICK_BIT_Y_HI+SHIFT_PER)) |
        (brickIdxInBlock.z << (BRICK_BIT_Z_HI+SHIFT_PER));

      return addr;
    }

    // BlockBrickedVolume definitions /////////////////////////////////////////

    GhostBlockBrickedVolume::~GhostBlockBrickedVolume()
    {
      freeVolumeMemory();
    }

    std::string GBBV::toString() const
    {
      return("ospray::cpp_renderer::GBBV<" + voxelType + ">");
    }

    void GBBV::commit()
    {
      if (!blockMem) constructVolumeMemory();
      StructuredVolume::commit();
    }

    int GBBV::setRegion(
        // points to the first voxel to be copied. The voxels at 'source' MUST
        // have dimensions 'regionSize', must be organized in 3D-array order,and
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

      if (voxel_t == OSP_UNKNOWN)
        constructVolumeMemory();

      switch (voxel_t) {
#if 0
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
#endif
      default:
        throw std::runtime_error("No voxel_t specificed in cpp bbv!");
        break;
      }

      // If we're upsampling finalSource points at the chunk of data allocated by
      // scaleRegion to hold the upsampled volume data and we must free it.
      if (upsampling) {
        free(finalSource);
      }

      return true;
    }

    float GhostBlockBrickedVolume::getVoxel(const vec3i &index) const
    {
      NOT_IMPLEMENTED// NOTE(jda) - not needed...
    }

    float
    GhostBlockBrickedVolume::computeSample(const vec3f &worldCoordinates) const
    {
      NOT_IMPLEMENTED
    }

    Address GhostBlockBrickedVolume::getIndices(const vec3i &voxelIdxInVolume)
    {
      Address address;

      /* Compute the 3D index of the block containing the brick containing the voxel. */
      const vec3i blockIndex {intDiv(voxelIdxInVolume.x),
                              intDiv(voxelIdxInVolume.y),
                              intDiv(voxelIdxInVolume.z)};

      /* Compute the 1D address of the block in the volume. */
      address.block = blockIndex.x
                      + blockCount.x * (blockIndex.y
                                        + blockCount.y * blockIndex.z);

      /* Compute the 3D offset of the brick within the block containing the voxel. */
      const vec3i voxelIdxInBlock
        = voxelIdxInVolume - blockIndex * vec3i{BLOCK_WIDTH - 1};

      const vec3i brickIdxInBlock {voxelIdxInBlock.x >> BRICK_BITS,
                                   voxelIdxInBlock.y >> BRICK_BITS,
                                   voxelIdxInBlock.z >> BRICK_BITS};
      const vec3i voxelIdxInBrick {voxelIdxInBlock.x & BRICK_MASK,
                                   voxelIdxInBlock.y & BRICK_MASK,
                                   voxelIdxInBlock.z & BRICK_MASK};

      address.voxel
        = (voxelIdxInBrick.x << (0*BRICK_BITS))
        | (voxelIdxInBrick.y << (1*BRICK_BITS))
        | (voxelIdxInBrick.z << (2*BRICK_BITS))
        | (brickIdxInBlock.x << (3*BRICK_BITS+0*(BLOCK_BITS-BRICK_BITS)))
        | (brickIdxInBlock.y << (3*BRICK_BITS+1*(BLOCK_BITS-BRICK_BITS)))
        | (brickIdxInBlock.z << (3*BRICK_BITS+2*(BLOCK_BITS-BRICK_BITS)));

      return address;
    }

    Address8 GBBV::getVoxelAddress(const vec3f &indexf,
                                   const vec3i &indexi) const
    {
      Address8 address;

      /* Compute the 3D index of the block containing the brick containing the voxel. */
      const vec3i blockIndex = vec3i{intDiv(indexf.x),
                                     intDiv(indexf.y),
                                     intDiv(indexf.z)};

      /* Compute the 1D address of the block in the volume. */
      uint32 block = blockIndex.x
                     + blockCount.x * (blockIndex.y
                                       + blockCount.y * blockIndex.z);

      /* Compute the 3D offset of the brick within the block containing the voxel. */
      const vec3i voxelIdxInBlock = indexi - blockIndex * vec3i{BLOCK_WIDTH-1};

      switch (voxel_t) {
      case OSP_UCHAR:
        address = brickTranslation<uint8,
                                   scale_per_uint8,
                                   shift_per_uint8>(voxelIdxInBlock);
        break;
      case OSP_SHORT:
        address = brickTranslation<int16,
                                   scale_per_int16,
                                   shift_per_int16>(voxelIdxInBlock);
        break;
      case OSP_USHORT:
        address = brickTranslation<uint16,
                                   scale_per_uint16,
                                   shift_per_uint16>(voxelIdxInBlock);
        break;
      case OSP_FLOAT:
        address = brickTranslation<float,
                                   scale_per_float,
                                   shift_per_float>(voxelIdxInBlock);
        break;
      case OSP_DOUBLE:
        address = brickTranslation<double,
                                   scale_per_double,
                                   shift_per_double>(voxelIdxInBlock);
        break;
      default:
        break;
      }

      address.block = block;

      return address;
    }

    void GhostBlockBrickedVolume::constructVolumeMemory()
    {
      NOT_IMPLEMENTED
    }

    void GhostBlockBrickedVolume::freeVolumeMemory()
    {
      if (blockMem) delete [] blockMem;
    }

#if 0//def EXP_NEW_BB_VOLUME_KERNELS
    /*! in new bb kernel mode we'll be using the code in
        GhostBlockBrickedVolume.* */
#else
    // A volume type with 64-bit addressing and multi-level bricked storage order.
    OSP_REGISTER_VOLUME(GhostBlockBrickedVolume, cpp_ghost_block_bricked_volume);
    OSP_REGISTER_VOLUME(GhostBlockBrickedVolume, cpp_gbbv);
#endif

  } // ::ospray::cpp_renderer
} // ::ospray

