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
#include "SDK/volume/Volume.h"
// cpp_renderer
#include "../common/Ray.h"
#include "../transferFunction/TransferFunction.h"

namespace ospray {
  namespace cpp_renderer {

    class Volume : public ospray::Volume
    {
    public:

      virtual ~Volume() = default;

      virtual std::string toString() const override;

      virtual void commit() override;

      //! Copy voxels into the volume at the given index (non-zero return value
      //!  indicates success).
      virtual int setRegion(const void *source,
                            const vec3i &index,
                            const vec3i &count) override = 0;

      //! Compute samples at the given world coordinates.
      virtual void computeSamples(float **results,
                                  const vec3f *worldCoordinates,
                                  const size_t &count) override= 0;

      virtual float computeSample(const vec3f &worldCoordinates) const = 0;

      virtual vec3f computeGradient(const vec3f &worldCoordinates) const = 0;

      virtual bool intersect(Ray &ray) const = 0;

      virtual void advance(Ray &ray) const = 0;
      virtual void advanceAdaptive(Ray &ray) const = 0;

      virtual void intersectIsosurface(const std::vector<float> &isovalues,
                                       Ray &ray) const = 0;

      // Data //

      Ref<TransferFunction> transferFunction;

      bool gradientShadingEnabled  {false};
      bool preIntegrationEnabled   {false};
      bool singleShadingEnabled    {true};
      bool adaptiveSamplingEnabled {true};

      float adaptiveScalar          {15.f};
      float adaptiveMaxSamplingRate {0.7f};
      float adaptiveBacktrack       {0.03f};

      float samplingRate {1.f};
      float samplingStep {1.f};
      vec3f specular     {0.3f};

      box3f boundingBox {vec3f{0.f}, vec3f{1.f}};
      box3f volumeClippingBox;
    };

  } // ::ospray::cpp_renderer
} // ::ospray
