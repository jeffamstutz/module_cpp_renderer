## ======================================================================== ##
## Copyright 2009-2016 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##

option(OSPRAY_MODULE_CPP "Build 'C++' module (renderers in all C++ code)" ON)

if (OSPRAY_MODULE_CPP AND
    (NOT OSPRAY_USE_EXTERNAL_EMBREE OR NOT OSPRAY_USE_EMBREE_STREAMS))
  message(WARNING "You must use an external Embree (v2.10.0+) AND enable "
                  "Embree streams...OSPRAY_MODULE_CPP will not be built.")
  set(OSPRAY_MODULE_CPP OFF)
endif()

if (OSPRAY_MODULE_CPP)

  ospray_create_library(module_cpp
    camera/Camera.h
    camera/Perspective.cpp

    common/DifferentialGeometry.h
    common/Ray.h
    common/ScreenSample.h

    geometry/Geometry.h
    geometry/TriangleMesh.cpp

    renderer/Renderer.cpp
    renderer/StreamRenderer.cpp
    renderer/raycast/Raycast.cpp
    renderer/raycast/StreamRaycast.cpp
    renderer/simple_ao/ao_util.cpp
    renderer/simple_ao/SimpleAO.cpp
    renderer/simple_ao/StreamSimpleAO.cpp

    util.cpp
    LINK
    ospray
  )

endif()
