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
#include "StructuredVolume.h"

namespace ospray {
  namespace cpp_renderer {

    std::string StructuredVolume::toString() const
    {
      return("ospray::cpp_renderer::StructuredVolume<" + voxelType + ">");
    }

    void StructuredVolume::commit()
    {
      cpp_renderer::Volume::commit();

      gridOrigin = getParam3f("gridOrigin", vec3f(0.f));

      dimensions = getParam3i("dimensions", vec3i(0));
      exitOnCondition(reduce_min(this->dimensions) <= 0,
                      "invalid volume dimensions");

      gridSpacing = getParam3f("gridSpacing", vec3f(1.f));

      scaleFactor = getParam3f("scaleFactor", vec3f(-1.f));

      localCoordinatesUpperBound = vec3f{nextafter(dimensions.x - 1, 0),
                                         nextafter(dimensions.y - 1, 0),
                                         nextafter(dimensions.z - 1, 0)};

      if (!finished) {
        voxelRange = getParam2f("voxelRange", voxelRange);

        boundingBox = box3f{gridOrigin,
                            gridOrigin + vec3f{dimensions - 1} * gridSpacing};

        buildAccelerator();
        finished = true;
      }
    }

    void StructuredVolume::computeSamples(float **results,
                                          const vec3f *worldCoordinates,
                                          const size_t &count)
    {
      NOT_IMPLEMENTED
    }

    float StructuredVolume::computeSample(const vec3f &worldCoordinates) const
    {
      vec3f localCoordinates = transformWorldToLocal(worldCoordinates);

      const vec3f clampedLocalCoordinates = clamp(localCoordinates,
                                                  vec3f{0.0f},
                                                  localCoordinatesUpperBound);

      // Lower and upper corners of the box straddling the voxels to be
      // interpolated. "vi" means "voxelIndex"
      const vec3i vi_0 {clampedLocalCoordinates.x,
                        clampedLocalCoordinates.y,
                        clampedLocalCoordinates.z};
      const vec3i vi_1 = vi_0 + 1;

      // Fractional coordinates within the lower corner voxel used during
      // interpolation. "flc" means "fractionalLocalCoordinates"
      const vec3f flc = clampedLocalCoordinates - vec3f{vi_0.x, vi_0.y, vi_0.z};

      // Look up the voxel values to be interpolated. "vv" means "voxelValue"
      float vv_000 = getVoxel(vec3i{vi_0.x, vi_0.y, vi_0.z});
      float vv_001 = getVoxel(vec3i{vi_1.x, vi_0.y, vi_0.z});
      float vv_010 = getVoxel(vec3i{vi_0.x, vi_1.y, vi_0.z});
      float vv_011 = getVoxel(vec3i{vi_1.x, vi_1.y, vi_0.z});
      float vv_100 = getVoxel(vec3i{vi_0.x, vi_0.y, vi_1.z});
      float vv_101 = getVoxel(vec3i{vi_1.x, vi_0.y, vi_1.z});
      float vv_110 = getVoxel(vec3i{vi_0.x, vi_1.y, vi_1.z});
      float vv_111 = getVoxel(vec3i{vi_1.x, vi_1.y, vi_1.z});

      // Interpolate the voxel values.
      const float vv_00 = vv_000 + flc.x * (vv_001 - vv_000);
      const float vv_01 = vv_010 + flc.x * (vv_011 - vv_010);
      const float vv_10 = vv_100 + flc.x * (vv_101 - vv_100);
      const float vv_11 = vv_110 + flc.x * (vv_111 - vv_110);
      const float vv_0  = vv_00  + flc.y * (vv_01  - vv_00 );
      const float vv_1  = vv_10  + flc.y * (vv_11  - vv_10 );
      const float volumeSample = vv_0 + flc.z * (vv_1 - vv_0);

      return volumeSample;
    }

    vec3f StructuredVolume::computeGradient(const vec3f &worldCoordinates) const
    {
      // Gradient step in each dimension (world coordinates).
      const vec3f &gradientStep = gridSpacing;

      // The gradient will be computed using central differences.
      vec3f gradient;

      // Sample at gradient location.
      float sample = computeSample(worldCoordinates);

      const auto step_x = vec3f{gradientStep.x, 0.0f, 0.0f};
      const auto step_y = vec3f{0.0f, gradientStep.y, 0.0f};
      const auto step_z = vec3f{0.0f, 0.0f, gradientStep.z};

      // Gradient magnitude in the X direction.
      gradient.x = computeSample(worldCoordinates + step_x) - sample;

      // Gradient magnitude in the Y direction.
      gradient.y = computeSample(worldCoordinates + step_y) - sample;

      // Gradient magnitude in the Z direction.
      gradient.z = computeSample(worldCoordinates + step_z) - sample;

      // This approximation may yield image artifacts.
      return gradient / gradientStep;
    }

    bool StructuredVolume::intersect(Ray &ray) const
    {
      auto hits = intersectBox(ray, boundingBox);

      if (hits.first < hits.second &&  hits.first < ray.t) {
        ray.t0 = hits.first;
        ray.t  = hits.second;
        return true;
      } else {
        return false;
      }
    }

    void StructuredVolume::stepRay(Ray &ray) const
    {
      // The recommended step size for ray casting based volume renderers.
      const float step = samplingStep / samplingRate;

#if 0
      // Compute the next hit point using a spatial acceleration structure.
      GridAccelerator_intersect(volume->accelerator, step, ray);
#else
      ray.t0 += step;
#endif
    }

    void StructuredVolume::stepRayAdaptive(Ray &ray) const
    {
      NOT_IMPLEMENTED
    }

    void
    StructuredVolume::intersectIsosurface(const std::vector<float> &isovalues,
                                          Ray &ray) const
    {
      NOT_IMPLEMENTED
    }

    vec3f
    StructuredVolume::transformLocalToWorld(const vec3f &localCoords) const
    {
      return gridOrigin + localCoords * gridSpacing;
    }

    vec3f
    StructuredVolume::transformWorldToLocal(const vec3f &worldCoords) const
    {
      return rcp(gridSpacing) * (worldCoords - gridOrigin);
    }

    bool StructuredVolume::scaleRegion(const void *source, void *&out,
                                       vec3i &regionSize, vec3i &regionCoords)
    {
#if 0
      this->scaleFactor = getParam3f("scaleFactor", vec3f(-1.f));
      const bool upsampling = scaleFactor.x > 0
                              && scaleFactor.y > 0
                              && scaleFactor.z > 0;
      vec3i scaledRegionSize = vec3i(scaleFactor * vec3f(regionSize));
      vec3i scaledRegionCoords = vec3i(scaleFactor * vec3f(regionCoords));

      if (upsampling) {
        if (voxelType == "uchar") {
          out = malloc(sizeof(unsigned char) * size_t(scaledRegionSize.x) *
              size_t(scaledRegionSize.y) * size_t(scaledRegionSize.z));
          upsampleRegion((unsigned char *)source, (unsigned char *)out,
                         regionSize, scaledRegionSize);
        }
        else if (voxelType == "short") {
          out = malloc(sizeof(short) * size_t(scaledRegionSize.x) *
              size_t(scaledRegionSize.y) * size_t(scaledRegionSize.z));
          upsampleRegion((unsigned short *)source, (unsigned short *)out,
                         regionSize, scaledRegionSize);
        }
        else if (voxelType == "ushort") {
          out = malloc(sizeof(unsigned short) * size_t(scaledRegionSize.x) *
              size_t(scaledRegionSize.y) * size_t(scaledRegionSize.z));
          upsampleRegion((unsigned short *)source, (unsigned short *)out,
                         regionSize, scaledRegionSize);
        }
        else if (voxelType == "float") {
          out = malloc(sizeof(float) * size_t(scaledRegionSize.x) *
              size_t(scaledRegionSize.y) * size_t(scaledRegionSize.z));
          upsampleRegion((float *)source, (float *)out, regionSize,
                         scaledRegionSize);
        }
        else if (voxelType == "double") {
          out = malloc(sizeof(double) * size_t(scaledRegionSize.x) *
              size_t(scaledRegionSize.y) * size_t(scaledRegionSize.z));
          upsampleRegion((double *)source, (double *)out, regionSize,
                         scaledRegionSize);
        }
        regionSize = scaledRegionSize;
        regionCoords = scaledRegionCoords;
      }
      return upsampling;
#else
      return false;
#endif
    }

    void StructuredVolume::buildAccelerator()
    {
#if 0
      // Create instance of volume accelerator.
      void *accel = ispc::StructuredVolume_createAccelerator(ispcEquivalent);

      vec3i brickCount;
      brickCount.x = ispc::GridAccelerator_getBrickCount_x(accel);
      brickCount.y = ispc::GridAccelerator_getBrickCount_y(accel);
      brickCount.z = ispc::GridAccelerator_getBrickCount_z(accel);

      // Build volume accelerator.
      const int NTASKS = brickCount.x * brickCount.y * brickCount.z;
      parallel_for(NTASKS, [&](int taskIndex){
        ispc::GridAccelerator_buildAccelerator(ispcEquivalent, taskIndex);
      });
#endif
    }

    OSPDataType StructuredVolume::getVoxelType()
    {
      return finished ? typeForString(getParamString("voxelType","unspecified"))
                        : typeForString(voxelType.c_str());
    }

  } // ::ospray::cpp_renderer
} // ::ospray

