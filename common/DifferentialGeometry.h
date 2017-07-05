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

#include "SDK/common/Material.h"
#include "geometry/Geometry.h"

#include "Stream.h"

namespace ospray {
  namespace cpp_renderer {

    typedef enum {
      DG_FACEFORWARD = (1<<0), /*!< face-forward normals */
      DG_NORMALIZE   = (1<<1), /*!< normalize normals */
      //DG_WORLD_SPACE = (1<<2) obsolete: is always done
      DG_NG          = (1<<3), /*!< need geometry normal Ng */
      DG_NS          = (1<<4), /*!< need shading normal Ns */
      //DG_MATERIAL    = (1<<5), /*!< need material pointer; deprecated: is always done*/
      //DG_GEOMETRY    = (1<<6), /*!< need geometry pointer; deprecated: is always done*/
      DG_MATERIALID  = (1<<7), /*!< hack for now - return materialID as
                                  stored in "prim.materialID" array */
      DG_COLOR       = (1<<8), /*!< hack for now - need interpolated vertex color */
      DG_TEXCOORD    = (1<<9), /*!< calculate texture coords st */
      DG_TANGENTS    = (1<<10),/*!< calculate tangents, i.e. the partial
                                 derivatives of position wrt. texture coordinates */
    } DG_PostIntersectFlags;

    /*! differential geometry information that gives more detailed
        information on the actual geometry that a ray has hit */
    struct DifferentialGeometry {
      vec3f P;  //!< location of the hit-point
      vec3f Ng; /*!< geometry normal if DG_NG was set, possibly not
                   normalized/facefordwarded if DG_NORMALIZE and/or
                   DG_FACEFORWARD weren't specified */
      vec3f Ns; /*!< shading normal if DG_NS was set, possibly not
                   normalized/facefordwarded if DG_NORMALIZE and/or
                   DG_FACEFORWARD weren't specified */
      vec3f dPds; //!< tangent, the partial derivative of the hit-point wrt. texcoord s
      vec3f dPdt; //!< bi-tangent, the partial derivative of the hit-point wrt. texcoord t
      vec2f st; //!< texture coordinates if DG_TEXCOORD was set
      vec4f color; /*! interpolated vertex color (rgba) if DG_COLOR was set;
                     defaults to vec4f(1.f) if queried but not present in geometry
                     */
      int32 materialID {-1}; /*!< hack for now - the materialID as stored in
                                 "prim.materialID" array (-1 if that value isn't
                                  specified) */

      ospray::Geometry *geometry{nullptr}; /*! pointer to hit-point's geometry */
      ospray::Material *material{nullptr}; /*! pointer to hit-point's material */
    };

    using DGStream = Stream<DifferentialGeometry>;

  }// namespace cpp_renderer
}// namespace ospray
