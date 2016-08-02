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

#include "Ray.h"

#include <array>

namespace ospray {
  namespace cpp_renderer {

    struct ScreenSampleRef;

    struct ScreenSample
    {
      // input values to 'renderSample'
      vec3i sampleID; /*!< x/y=pixelID,z=accumID/sampleID */
      Ray   ray;      /*!< the primary ray generated by the camera */
      // return values from 'renderSample'
      vec3f rgb {0.f, 0.f, 0.f};
      float alpha{0.f};
      float z{inf};
      int tileOffset{-1};// linear value --> comes from tileX,tileY
    };

    struct ScreenSampleRef
    {
      vec3i &sampleID;
      Ray   &ray;
      vec3f &rgb;
      float &alpha;
      float &z;
      int   &tileOffset;
    };

    template <int SIZE>
    struct ScreenSampleStreamN
    {
      static constexpr int size = SIZE;

      std::array<vec3i, SIZE> sampleID;

      RayStreamN<SIZE> rays;

      std::array<vec3f, SIZE> rgb;
      std::array<float, SIZE> alpha;
      std::array<float, SIZE> z;
      std::array<int, SIZE> tileOffset;

      // Member functions //

      ScreenSampleRef get(int i);
    };

    using ScreenSampleStream = ScreenSampleStreamN<RENDERTILE_PIXELS_PER_JOB>;

    // Inlined function definitions ///////////////////////////////////////////

    template <int SIZE>
    inline ScreenSampleRef ScreenSampleStreamN<SIZE>::get(int i)
    {
      return {sampleID[i], rays[i], rgb[i], alpha[i], z[i], tileOffset[i]};
    }

    // Inlined helper functions ///////////////////////////////////////////////

    template <int SIZE, typename FCN_T>
    inline void
    for_each_sample(ScreenSampleStreamN<SIZE> &stream, const FCN_T &fcn)
    {
      // TODO: Add static_assert() check for signature of FCN_T, similar to
      //       the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i)
        fcn(stream.get(i));
    }

    template <int SIZE, typename FCN_T, typename PRED_T>
    inline void
    for_each_sample(ScreenSampleStreamN<SIZE> &stream,
                    const FCN_T &fcn,
                    const PRED_T &pred)
    {
      // TODO: Add static_assert() check for signature of FCN_T and PRED_T,
      //       similar to the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i) {
        auto sample = stream.get(i);
        if (pred(sample)) {
          fcn(sample);
        }
      }
    }

    template <int SIZE, typename FCN_T>
    inline void
    for_each_sample_n(ScreenSampleStreamN<SIZE> &stream, const FCN_T &fcn)
    {
      // TODO: Add static_assert() check for signature of FCN_T, similar to
      //       the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i)
        fcn(stream.get(i), i);
    }

    template <int SIZE, typename FCN_T, typename PRED_T>
    inline void
    for_each_sample_n(ScreenSampleStreamN<SIZE> &stream,
                      const FCN_T &fcn,
                      const PRED_T &pred)
    {
      // TODO: Add static_assert() check for signature of FCN_T and PRED_T,
      //       similar to the way TASK_T is checked for ospray::parallel_for()

      for (int i = 0; i < SIZE; ++i) {
        auto sample = stream.get(i);
        if (pred(sample)) {
          fcn(sample, i);
        }
      }
    }

    // Predefined predicates //////////////////////////////////////////////////

    inline bool sampleEnabled(const ScreenSampleRef &sample)
    {
      return sample.tileOffset >= 0;
    }

    inline bool rayHit(const ScreenSampleRef &sample)
    {
      return sample.ray.hitSomething();
    }

    inline bool rayMiss(const ScreenSampleRef &sample)
    {
      return !sample.ray.hitSomething();
    }

  }// namespace cpp_renderer
}// namespace ospray
