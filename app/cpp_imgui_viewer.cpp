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

#include "../../ospImGui/widgets/imguiViewer.h"
#include "commandline/Utility.h"
#include "commandline/CameraParser.h"
#include "commandline/SceneParser/trianglemesh/TriangleMeshSceneParser.h"
#include "commandline/SceneParser/volume/VolumeSceneParser.h"
#include "commandline/LightsParser.h"
#include "commandline/RendererParser.h"

#include <ospray/ospray_cpp/Data.h>

static std::string g_prefix  = "cpp_";
static std::string g_postfix = "";

static bool isSimd   = false;
static bool isStream = false;

static ospcommon::vec3f translate;
static ospcommon::vec3f scale;
static bool lockFirstFrame = false;

class CppRendererParser : public DefaultRendererParser
{
public:
  CppRendererParser() { rendererType = g_prefix + "scivis" + g_postfix; }
};

class CppCameraParser : public DefaultCameraParser
{
public:
  CppCameraParser() { cameraType = g_prefix + "perspective" + g_postfix; }
};

class CppSceneParser : public SceneParser
{
public:
  CppSceneParser(ospray::cpp::Renderer _renderer)
    : renderer(_renderer) {}

  std::deque<ospray::cpp::Model> model() const override { return sceneModels; }
  std::deque<ospcommon::box3f>   bbox()  const override { return sceneBboxes; }

  bool parse(int ac, const char **&av)
  {
    auto triangles_name = g_prefix + "perspective" + g_postfix;
    TriangleMeshSceneParser triangleMeshParser(renderer, triangles_name);

    VolumeSceneParser volumeParser(renderer);

    bool gotTriangleMeshScene = triangleMeshParser.parse(ac, av);
    bool gotVolumeScene       = volumeParser.parse(ac, av);

    SceneParser *parser = nullptr;

    if (gotTriangleMeshScene)
      parser = &triangleMeshParser;
    else if (gotVolumeScene)
      parser = &volumeParser;

    if (parser) {
      sceneModels = parser->model();
      sceneBboxes = parser->bbox();
    } else {
      auto model = ospray::cpp::Model();
      model.commit();
      sceneModels.push_back(model);
    }

    return parser != nullptr;
  }

private:

  ospray::cpp::Renderer renderer;
  std::deque<ospray::cpp::Model> sceneModels;
  std::deque<ospcommon::box3f>   sceneBboxes;
};

class CppLightsParser : public LightsParser
{
public:
  CppLightsParser(ospray::cpp::Renderer renderer)
  {
    std::vector<OSPLight> lights;

    auto ospLight = renderer.newLight(g_prefix + "directional");
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
      auto ambLight = renderer.newLight(g_prefix + "ambient");
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

  bool parse(int ac, const char **&av) { return false; }
};

void parseExtraParametersFromComandLine(int ac, const char **&av)
{
  for (int i = 1; i < ac; i++) {
    const std::string arg = av[i];
    if (arg == "--translate") {
      translate.x = atof(av[++i]);
      translate.y = atof(av[++i]);
      translate.z = atof(av[++i]);
    } else if (arg == "--scale") {
      scale.x = atof(av[++i]);
      scale.y = atof(av[++i]);
      scale.z = atof(av[++i]);
    } else if (arg == "--lockFirstFrame") {
      lockFirstFrame = true;
    } else if (arg == "--simd" || arg == "-simd") {
      isSimd = true;
    } else if (arg == "--stream" || arg == "-stream") {
      isStream = true;
    }
  }

  if (isStream) g_postfix += "_stream";
  if (isSimd)   g_postfix += "_simd";
}

int main(int ac, const char **av)
{
  parseExtraParametersFromComandLine(ac, av);

  ospInit(&ac,av);
  ospray::imgui3D::init(&ac,av);

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

  ospray::ImGuiViewer window(bbox, model, renderer, camera);
  window.setScale(scale);
  window.setLockFirstAnimationFrame(lockFirstFrame);
  window.setTranslation(translate);
  window.create("ospCppViewer: OSPRay C++-rendering Viewer");

  ospray::imgui3D::run();
}
