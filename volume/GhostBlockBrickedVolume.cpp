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

namespace ospray {
  namespace cpp_renderer {

    template <typename VOXEL_T>
    struct shift_per
    {
      constexpr static int value = 0;
    };

    template <>
    struct shift_per<int16>
    {
      constexpr static int value = 1;
    };

    template <>
    struct shift_per<uint16>
    {
      constexpr static int value = 1;
    };

    template <>
    struct shift_per<float>
    {
      constexpr static int value = 2;
    };

    template <>
    struct shift_per<double>
    {
      constexpr static int value = 3;
    };

    template <typename VOXEL_T>
    struct scale_per
    {
      constexpr static int value = 1;
    };

    template <>
    struct scale_per<int16>
    {
      constexpr static int value = 2;
    };

    template <>
    struct scale_per<uint16>
    {
      constexpr static int value = 2;
    };

    template <>
    struct scale_per<float>
    {
      constexpr static int value = 4;
    };

    template <>
    struct scale_per<double>
    {
      constexpr static int value = 8;
    };

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
    template<typename VOXEL_T>
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
        + (BRICK_BIT_SCALE_X_HI*scale_per<VOXEL_T>::value)
        - ((BRICK_WIDTH-1)*BRICK_BIT_SCALE_X_LO*scale_per<VOXEL_T>::value)
        : (BRICK_BIT_SCALE_X_LO*scale_per<VOXEL_T>::value);

      addr.voxelOfs_dy = (voxelIdxInBrick.y == (BRICK_WIDTH-1))
        ?
        + (BRICK_BIT_SCALE_Y_HI*scale_per<VOXEL_T>::value)
        - ((BRICK_WIDTH-1)*BRICK_BIT_SCALE_Y_LO*scale_per<VOXEL_T>::value)
        : (BRICK_BIT_SCALE_Y_LO*scale_per<VOXEL_T>::value);

      addr.voxelOfs_dz = (voxelIdxInBrick.z == (BRICK_WIDTH-1))
        ?
        + (BRICK_BIT_SCALE_Z_HI*scale_per<VOXEL_T>::value)
        - ((BRICK_WIDTH-1)*BRICK_BIT_SCALE_Z_LO*scale_per<VOXEL_T>::value)
        : (BRICK_BIT_SCALE_Z_LO*scale_per<VOXEL_T>::value);

      addr.voxelOfs =
        (voxelIdxInBrick.x << (BRICK_BIT_X_LO+shift_per<VOXEL_T>::value)) |
        (voxelIdxInBrick.y << (BRICK_BIT_Y_LO+shift_per<VOXEL_T>::value)) |
        (voxelIdxInBrick.z << (BRICK_BIT_Z_LO+shift_per<VOXEL_T>::value)) |
        (brickIdxInBlock.x << (BRICK_BIT_X_HI+shift_per<VOXEL_T>::value)) |
        (brickIdxInBlock.y << (BRICK_BIT_Y_HI+shift_per<VOXEL_T>::value)) |
        (brickIdxInBlock.z << (BRICK_BIT_Z_HI+shift_per<VOXEL_T>::value));

      return addr;
    }

    template<typename T>
    inline float accessArrayWithOffset(const T *basePtr, uint32 offset)
    {
      return *((T*)((uint8 *)basePtr+offset));
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
      case OSP_UCHAR:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<uint8, VOXELS_PER_BLOCK>(finalSource,
                                                  finalRegionCoords,
                                                  finalRegionSize,
                                                  taskIndex);
        });
        break;
      case OSP_SHORT:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<int16, VOXELS_PER_BLOCK>(finalSource,
                                                  finalRegionCoords,
                                                  finalRegionSize,
                                                  taskIndex);
        });
        break;
      case OSP_USHORT:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<uint16, VOXELS_PER_BLOCK>(finalSource,
                                                   finalRegionCoords,
                                                   finalRegionSize,
                                                   taskIndex);
        });
        break;
      case OSP_FLOAT:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<float, VOXELS_PER_BLOCK>(finalSource,
                                                  finalRegionCoords,
                                                  finalRegionSize,
                                                  taskIndex);
        });
        break;
      case OSP_DOUBLE:
        parallel_for(NTASKS, [&](size_t taskIndex) {
          setVoxelValues<double, VOXELS_PER_BLOCK>(finalSource,
                                                   finalRegionCoords,
                                                   finalRegionSize,
                                                   taskIndex);
        });
        break;
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
      switch (voxel_t) {
      case OSP_UCHAR:
        return computeSample_T<uint8>(worldCoordinates);
        break;
      case OSP_SHORT:
        return computeSample_T<int16>(worldCoordinates);
        break;
      case OSP_USHORT:
        return computeSample_T<uint16>(worldCoordinates);
        break;
      case OSP_FLOAT:
        return computeSample_T<float>(worldCoordinates);
        break;
      case OSP_DOUBLE:
        return computeSample_T<double>(worldCoordinates);
        break;
      default:
        break;
      }

      return inf;
    }

    template<typename T>
    float
    GhostBlockBrickedVolume::computeSample_T(const vec3f &worldCoordinates) const
    {
      /* Transform the sample location into the local coordinate system. */
      vec3f localCoordinates = transformWorldToLocal(worldCoordinates);

      /* Coordinates outside the volume are clamped to the volume bounds. */
      const vec3f clampedLocalCoordinates = clamp(localCoordinates,
                                                  vec3f{0.0f},
                                                  localCoordinatesUpperBound);

      // "vi" means "voxelIndex"
      const vec3i vi_0 {clampedLocalCoordinates.x,
                        clampedLocalCoordinates.y,
                        clampedLocalCoordinates.z};

      const vec3f flc = clampedLocalCoordinates - vec3f{vi_0.x, vi_0.y, vi_0.z};

      /* Compute the 1D address of the block in the volume and the voxel in the block. */
      Address8 address8 = getVoxelAddress(clampedLocalCoordinates, vi_0);

      /* split block into 24+8 bits; we'll have to do a foreach_unique */
      /* over the upper 24 bits to satify 64-bit addressing, but at */
      /* least the lower bits we can merge into a single loop */
      const uint32 block_lo = address8.block & 255;
      const uint32 block_hi = address8.block ^ block_lo;

      const T *blockPtrHi = (const T*)blockMem
                            + ((uint64)block_hi) * (VOXELS_PER_BLOCK);
      /* TODO: interleave loads with the address computations ... or at */
      /* least, prefetch upon the first address being known. */
      const uint32 ofs000 = address8.voxelOfs
        + block_lo*(VOXELS_PER_BLOCK*scale_per<T>::value);
      const uint32 ofs001 = ofs000+address8.voxelOfs_dx;
      const float val000  = accessArrayWithOffset(blockPtrHi,ofs000);
      const float val001  = accessArrayWithOffset(blockPtrHi,ofs001);
      const float val00   = val000 + flc.x * (val001 - val000);

      const uint32 ofs010 = ofs000+address8.voxelOfs_dy;
      const uint32 ofs011 = ofs001+address8.voxelOfs_dy;
      const float val010  = accessArrayWithOffset(blockPtrHi,ofs010);
      const float val011  = accessArrayWithOffset(blockPtrHi,ofs011);
      const float val01   = val010 + flc.x * (val011 - val010);

      const uint32 ofs100 = ofs000+address8.voxelOfs_dz;
      const uint32 ofs101 = ofs001+address8.voxelOfs_dz;
      const float val100  = accessArrayWithOffset(blockPtrHi,ofs100);
      const float val101  = accessArrayWithOffset(blockPtrHi,ofs101);
      const float val10   = val100 + flc.x * (val101 - val100);

      const uint32 ofs110 = ofs010+address8.voxelOfs_dz;
      const uint32 ofs111 = ofs011+address8.voxelOfs_dz;
      const float val110  = accessArrayWithOffset(blockPtrHi,ofs110);
      const float val111  = accessArrayWithOffset(blockPtrHi,ofs111);
      const float val11   = val110 + flc.x * (val111 - val110);

      /* Interpolate the voxel values. */
      const float val0    = val00  + flc.y * (val01  - val00 );
      const float val1    = val10  + flc.y * (val11  - val10 );
      const float val     = val0   + flc.z * (val1   - val0  );

      return val;
    }

    Address GhostBlockBrickedVolume::getIndices(const vec3i &voxelIdxInVolume) const
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

    bool GhostBlockBrickedVolume::getGhostIndices(const vec3i &voxelIdxInVolume,
                                                  const vec3i &delta,
                                                  Address     &address) const
    {
      // do not set offset value for original sample
      if ((delta.x == 0) && (delta.y == 0) && (delta.z == 0))
        return false;

      // Compute the 3D index of the block containing the brick containing the voxel.
      // vec3i blockIndex = voxelIdxInVolume / make_vec3i(BLOCK_WIDTH-1);
      vec3i blockIndex = vec3i{intDiv(voxelIdxInVolume.x),
                               intDiv(voxelIdxInVolume.y),
                               intDiv(voxelIdxInVolume.z)};

      // Compute the 3D offset of the brick within the block containing the voxel.
      vec3i voxelIdxInBlock =
                      voxelIdxInVolume - blockIndex * vec3i{BLOCK_WIDTH-1};

      bool valid = false;
      if ((voxelIdxInBlock.x == 0) && (delta.x == 1)) {
        voxelIdxInBlock.x = BLOCK_WIDTH-1;
        --blockIndex.x;
        valid = true;
      }
      if ((voxelIdxInBlock.y == 0) && (delta.y == 1)) {
        voxelIdxInBlock.y = BLOCK_WIDTH-1;
        --blockIndex.y;
        valid = true;
      }
      if ((voxelIdxInBlock.z == 0) && (delta.z == 1)) {
        voxelIdxInBlock.z = BLOCK_WIDTH-1;
        --blockIndex.z;
        valid = true;
      }

      // Compute the 1D address of the block in the volume.
      address.block = blockIndex.x
                      + blockCount.x * (blockIndex.y
                                        + blockCount.y * blockIndex.z);

      const vec3i brickIdxInBlock {voxelIdxInBlock.x >> BRICK_BITS,
                                   voxelIdxInBlock.y >> BRICK_BITS,
                                   voxelIdxInBlock.z >> BRICK_BITS};
      const vec3i voxelIdxInBrick {voxelIdxInBlock.x & BRICK_MASK,
                                   voxelIdxInBlock.y & BRICK_MASK,
                                   voxelIdxInBlock.z & BRICK_MASK};

      // Compute the 1D address of the voxel in the block.
      address.voxel =
        (voxelIdxInBrick.x << (BRICK_BIT_X_LO)) |
        (voxelIdxInBrick.y << (BRICK_BIT_Y_LO)) |
        (voxelIdxInBrick.z << (BRICK_BIT_Z_LO)) |
        (brickIdxInBlock.x << (BRICK_BIT_X_HI)) |
        (brickIdxInBlock.y << (BRICK_BIT_Y_HI)) |
        (brickIdxInBlock.z << (BRICK_BIT_Z_HI));

      return valid
             && (blockIndex.x >= 0)
             && (blockIndex.y >= 0)
             && (blockIndex.z >= 0);
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
        address = brickTranslation<uint8>(voxelIdxInBlock);
        break;
      case OSP_SHORT:
        address = brickTranslation<int16>(voxelIdxInBlock);
        break;
      case OSP_USHORT:
        address = brickTranslation<uint16>(voxelIdxInBlock);
        break;
      case OSP_FLOAT:
        address = brickTranslation<float>(voxelIdxInBlock);
        break;
      case OSP_DOUBLE:
        address = brickTranslation<double>(voxelIdxInBlock);
        break;
      default:
        break;
      }

      address.block = block;

      return address;
    }

    void GhostBlockBrickedVolume::constructVolumeMemory()
    {
      freeVolumeMemory();

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
      blockCount = (dimensions + (BLOCK_WIDTH-1) - 1) / (BLOCK_WIDTH-1);

      // Volume size in blocks with padding.
      const size_t numBlocks = blockCount.x * blockCount.y * blockCount.z;

      // allocate the large array of blocks
      size_t blockSize = VOXELS_PER_BLOCK * voxelSize;
      blockMem = new byte_t[blockSize * numBlocks];
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

