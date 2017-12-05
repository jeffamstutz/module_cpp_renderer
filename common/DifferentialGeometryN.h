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

#include "simd.h"
#include "DifferentialGeometry.h"

namespace ospray {
  namespace cpp_renderer {

    /*! differential geometry information that gives more detailed
        information on the actual geometry that a ray has hit */
    struct DifferentialGeometryN {
      simd::vec3f P;  //!< location of the hit-point
      simd::vec3f Ng; /*!< geometry normal if DG_NG was set, possibly not
                           normalized/facefordwarded if DG_NORMALIZE and/or
                           DG_FACEFORWARD weren't specified */
      simd::vec3f Ns; /*!< shading normal if DG_NS was set, possibly not
                   normalized/facefordwarded if DG_NORMALIZE and/or
                   DG_FACEFORWARD weren't specified */
      simd::vec3f dPds; //!< tangent, the partial derivative of the hit-point wrt.
                        //   texcoord s
      simd::vec3f dPdt; //!< bi-tangent, the partial derivative of the hit-point
                        //   wrt. texcoord t
      simd::vec2f st; //!< texture coordinates if DG_TEXCOORD was set
      simd::vec4f color; /*! interpolated vertex color (rgba) if DG_COLOR was set;
                     defaults to vec4f(1.f) if queried but not present in geometry
                     */
      simd::vint materialID {-1}; /*!< hack for now - the materialID as stored in
                                 "prim.materialID" array (-1 if that value isn't
                                  specified) */

      /*! pointer to hit-point's geometry */
      simd::vptr<ospray::Geometry> geometry{nullptr};
      /*! pointer to hit-point's material */
      simd::vptr<ospray::Material> material{nullptr};
    };

    using DGNStream = Stream<DifferentialGeometryN>;

  }// namespace cpp_renderer
}// namespace ospray
