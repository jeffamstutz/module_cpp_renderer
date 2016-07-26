// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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

#include "../../ospray/geometry/TriangleMesh.h"
#include "../geometry/Geometry.h"

namespace ospray {
  namespace cpp_renderer {

    struct TriangleMesh :
        public ospray::TriangleMesh,
        public ospray::cpp_renderer::Geometry
    {
      void postIntersect() const override;
    };

  }// namespace cpp_renderer
}// namespace ospray
