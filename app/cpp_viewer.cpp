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

#include "widgets/OSPGlutViewer.h"
#include "commandline/Utility.h"
#include "commandline/CameraParser.h"
#include "commandline/SceneParser/trianglemesh/TriangleMeshSceneParser.h"
#include "commandline/LightsParser.h"
#include "commandline/RendererParser.h"

#include "ospray_cpp/Data.h"

class CppRendererParser : public DefaultRendererParser
{
public:
  CppRendererParser() { rendererType = "cpp_scivis_stream"; }
};

class CppCameraParser : public DefaultCameraParser
{
public:
  CppCameraParser() { cameraType = "cpp_perspective"; }
};

class CppSceneParser : public TriangleMeshSceneParser
{
public:
  CppSceneParser(ospray::cpp::Renderer renderer,
                 std::string geometryType = "cpp_triangles") :
    TriangleMeshSceneParser(renderer, geometryType) {}
};

class CppLightsParser : public LightsParser
{
public:
  CppLightsParser(ospray::cpp::Renderer renderer)
  {
    std::vector<OSPLight> lights;

    auto ospLight = renderer.newLight("cpp_directional");
    if (ospLight.handle() == nullptr) {
      throw std::runtime_error("Failed to create a 'DirectionalLight'!");
    }
    ospLight.set("name", "sun");
    ospLight.set("color", 1.f, .94f, .88f);
    ospLight.set("direction", ospcommon::vec3f(.3, -1, -.2));
    ospLight.set("intensity", 3.14f);
    ospLight.set("angularDiameter", 0.53f);
    ospLight.commit();
    lights.push_back(ospLight.handle());

    ospcommon::vec4f ambient(.85, .9, 1, .2*3.14);

    if (ambient.w > 0.f && reduce_max(ambient) > 0.f) {
      auto ambLight = renderer.newLight("cpp_ambient");
      if (ambLight.handle() == nullptr) {
        throw std::runtime_error("Failed to create a 'AmbientLight'!");
      }
      ambLight.set("name", "ambient");
      ambLight.set("color", ambient.x, ambient.y, ambient.z);
      ambLight.set("intensity", ambient.w);
      ambLight.commit();
      lights.push_back(ambLight.handle());
    }

    auto lightArray = ospray::cpp::Data(lights.size(),
                                        OSP_OBJECT,
                                        lights.data());
    //lightArray.commit();
    renderer.set("lights", lightArray);
  }

  bool parse(int ac, const char **&av) {}
};

int main(int ac, const char **av)
{
  ospInit(&ac,av);
  ospray::glut3D::initGLUT(&ac,av);

  ospLoadModule("cpp");
  auto ospObjs = parseCommandLine<CppRendererParser,
                                  CppCameraParser,
                                  CppSceneParser,
                                  CppLightsParser>(ac, av);

  std::deque<ospcommon::box3f>   bbox;
  std::deque<ospray::cpp::Model> model;
  ospray::cpp::Renderer          renderer;
  ospray::cpp::Camera            camera;

  std::tie(bbox, model, renderer, camera) = ospObjs;

  ospray::OSPGlutViewer window(bbox, model, renderer, camera);
  window.create("ospCppViewer: OSPRay C++-rendering Viewer");

  ospray::glut3D::runGLUT();
}
