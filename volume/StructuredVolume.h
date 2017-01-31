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

// ospray
#if 0
#include "ospcommon/tasking/parallel_for.h"
#endif

#include "Volume.h"

namespace ospray {
  namespace cpp_renderer {

    class StructuredVolume : public Volume
    {
    public:

      StructuredVolume() = default;
      virtual ~StructuredVolume() = default;

      virtual std::string toString() const override;

      virtual void commit() override;

      virtual int setRegion(const void *source_pointer,
                            const vec3i &target_index,
                            const vec3i &source_count) override = 0;

      void computeSamples(float **results,
                          const vec3f *worldCoordinates,
                          const size_t &count) override;

      // cpp_renderer::Volume interface //

      float computeSample(const vec3f &worldCoordinates) const;

      vec3f computeGradient(const vec3f &worldCoordinates) const;

      bool intersect(Ray &ray) const;

      void advance(Ray &ray) const;
      void advanceAdaptive(Ray &ray) const;

      void intersectIsosurface(const std::vector<float> &isovalues,
                               Ray &ray) const;

    protected:

      // Internal interface //

      virtual float getVoxel(const vec3i &index) const = 0;

      // Interal methods //

      vec3f transformLocalToWorld(const vec3f &localCoords) const;
      vec3f transformWorldToLocal(const vec3f &worldCoords) const;

#if 0
      template<typename T>
      void upsampleRegion(const T *source,
                          T *out,
                          const vec3i &regionSize,
                          const vec3i &scaledRegionSize);
#endif

      bool scaleRegion(const void *source, void *&out,
                       vec3i &regionSize, vec3i &regionCoords);

      //! build the accelerator - allows child class (data distributed) to avoid
      //! building..
      virtual void buildAccelerator();

      //! Get the OSPDataType enum corresponding to the voxel type string.
      OSPDataType getVoxelType();

      // Data //

      // TODO: ptr|ref to GridAccelerator

      //! Volume size in voxels per dimension.
      vec3i dimensions;

      //! Grid origin.
      vec3f gridOrigin;

      //! Grid spacing in each dimension.
      vec3f gridSpacing;

      //! Indicate that the volume is fully initialized.
      bool finished {false};

      //! Voxel value range (will be computed if not provided as a parameter).
      vec2f voxelRange {FLT_MAX, -FLT_MAX};

      //! Voxel type.
      std::string voxelType;

      /*! Scale factor for the volume, mostly for internal use or data scaling
          benchmarking. Note that this must be set **before** calling
          'ospSetRegion' on the volume as the scaling is applied in that
          function.
       */
      vec3f scaleFactor;

      //! The largest coordinate value (in local coordinates) still inside the volume.
      vec3f localCoordinatesUpperBound;
    };

// Inlined member functions ///////////////////////////////////////////////////

#if 0
    template<typename T>
    void StructuredVolume::upsampleRegion(const T *source,
                                          T *out,
                                          const vec3i &regionSize,
                                          const vec3i &scaledRegionSize)
    {
      for (int z = 0; z < scaledRegionSize.z; ++z) {
        parallel_for(scaledRegionSize.x * scaledRegionSize.y, [&](int taskID){
          int x = taskID % scaledRegionSize.x;
          int y = taskID / scaledRegionSize.x;
          const int idx =
              static_cast<int>(z / scaleFactor.z) * regionSize.x * regionSize.y
              + static_cast<int>(y / scaleFactor.y) * regionSize.x
              + static_cast<int>(x / scaleFactor.x);
          const auto outIdx = z * scaledRegionSize.y * scaledRegionSize.x
                              + y * scaledRegionSize.x
                              + x
          out[outIdx] = source[idx];
        });
      }
    }
#endif
  } // ::ospray::cpp_renderer
} // ::ospray

