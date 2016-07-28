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

if (OSPRAY_MODULE_CPP)

  ospray_create_library(module_cpp
    camera/Camera.h
    camera/Perspective.cpp

    common/DifferentialGeometry.h
    common/Ray.h
    common/ScreenSample.h

    geometry/Geometry.h
    geometry/TriangleMesh.cpp

    renderer/Raycast.cpp
    renderer/Renderer.cpp
    renderer/SimpleAO.cpp
    renderer/StreamRaycast.cpp
    renderer/StreamRenderer.cpp
    renderer/StreamSimpleAO.cpp

    util.cpp
    LINK
    ospray
  )

endif()
