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

/*! \brief utility library containing sampling functions */

// convention is to return the sample (vec3f) generated from given
// vec2f 's'ample as last parameter

// sampling functions often come in pairs: sample and pdf (needed for MIS)
// good reference is "Total Compendium" by Philip Dutre
// http://people.cs.kuleuven.be/~philip.dutre/GI/

#include "ospcommon/vec.h"

namespace ospcommon {

  inline vec3f cartesian(float phi, float sinTheta, float cosTheta)
  {
    double sinPhi = sin(phi);
    double cosPhi = cos(phi);
    return vec3f(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
  }

  inline vec3f cartesian(float phi, float cosTheta)
  {
    return cartesian(phi, cos2sin(cosTheta), cosTheta);
  }

  // cosine-weighted sampling of hemisphere oriented along the +z-axis ////////

  inline vec3f cosineSampleHemisphere(const vec2f s)
  {
    const float phi = static_cast<float>(two_pi) * s.x;
    const float cosTheta = sqrt(s.y);
    const float sinTheta = sqrt(1.0f - s.y);
    return cartesian(phi, sinTheta, cosTheta);
  }

  inline float cosineSampleHemispherePDF(const vec3f &dir)
  {
    return dir.z / M_PI;
  }

  inline float cosineSampleHemispherePDF(float cosTheta)
  {
    return cosTheta / M_PI;
  }

  // power cosine-weighted sampling of hemisphere oriented along the +z-axis //

  inline vec3f powerCosineSampleHemisphere(const float n, const vec2f &s)
  {
    const float phi = static_cast<float>(two_pi) * s.x;
    const float cosTheta = pow(s.y, 1.0f / (n + 1.0f));
    return cartesian(phi, cosTheta);
  }

  inline float powerCosineSampleHemispherePDF(const float cosTheta,
                                              const float n)
  {
    return (n + 1.0f) * (0.5f / M_PI) * pow(cosTheta, n);
  }

  inline float powerCosineSampleHemispherePDF(const vec3f& dir,
                                              const float n)
  {
    return (n + 1.0f) * (0.5f / M_PI) * pow(dir.z, n);
  }

  // uniform sampling of cone of directions oriented along the +z-axis ////////

  inline vec3f uniformSampleCone(const float cosAngle, const vec2f &s)
  {
    const float phi = static_cast<float>(two_pi) * s.x;
    const float cosTheta = 1.0f - s.y * (1.0f - cosAngle);
    return cartesian(phi, cosTheta);
  }

  inline float uniformSampleConePDF(const float cosAngle)
  {
    return rcp(static_cast<float>(two_pi)*(1.0f - cosAngle));
  }

  // uniform sampling of disk /////////////////////////////////////////////////

  inline vec3f uniformSampleDisk(const float radius, const vec2f &s)
  {
    const float r = sqrtf(s.x) * radius;
    const float phi = static_cast<float>(two_pi) * s.y;
    double sinPhi = sin(phi);
    double cosPhi = cos(phi);
    return vec3f(r * cosPhi, r * sinPhi, 0.f);
  }

  inline float uniformSampleDiskPDF(const float radius)
  {
    return rcp(M_PI * sqr(radius));
  }

  // uniform sampling of triangle abc /////////////////////////////////////////

  inline vec3f uniformSampleTriangle(const vec3f &a,
                                     const vec3f &b,
                                     const vec3f &c,
                                     const vec2f &s)
  {
    const float su = sqrtf(s.x);
    return c + (1.0f - su) * (a-c) + (s.y*su) * (b-c);
  }

  inline float uniformSampleTrianglePDF(const vec3f &a,
                                        const vec3f &b,
                                        const vec3f &c)
  {
    return 2.0f * rcp(abs(length(cross(a-c, b-c))));
  }

}// namespace ospcommon
