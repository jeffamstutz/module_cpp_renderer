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

#include "sg/camera/PerspectiveCamera.h"
#include "sg/geometry/TriangleMesh.h"

namespace ospray {
  namespace sg {

    struct CppTriangleMesh : public sg::TriangleMesh
    {
      CppTriangleMesh();
    };

    struct CppPerspectiveCamera : public sg::Camera
    {
      CppPerspectiveCamera();
    };

    void importOBJ_cpp(const std::shared_ptr<Node> &world,
                       const FileName &fileName);

    extern std::string g_prefix;
    extern std::string g_postfix;

  } // ::ospray::sg
} // ::ospray