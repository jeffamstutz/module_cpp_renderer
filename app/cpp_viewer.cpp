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

#include "common/sg/SceneGraph.h"
#include "common/sg/Renderer.h"
#include "common/sg/common/Data.h"
#include "common/sg/geometry/Geometry.h"

#include "exampleViewer/widgets/imguiViewer.h"

#include "cpp_nodes.h"

namespace ospray {
  namespace cpp_renderer {

    using namespace sg;

    static std::vector<ospcommon::FileName> files;

    //parse command line arguments containing the format:
    //  -nodeName:...:nodeName=value,value,value -- changes value
    //  -nodeName:...:nodeName+=name,type        -- adds new child node
    static inline void parseCommandLineSG(int ac, const char **&av,
                                          sg::Node &root)
    {
      for(int i=1;i < ac; i++) {
        std::string arg(av[i]);
        size_t f;
        std::string value("");
        if (arg.size() < 2 || arg[0] != '-')
          continue;

        const std::string orgarg(arg);
        while ((f = arg.find(":")) != std::string::npos ||
               (f = arg.find(",")) != std::string::npos) {
          arg[f] = ' ';
        }

        f = arg.find("+=");
        bool addNode = false;
        if (f != std::string::npos)
        {
          value = arg.substr(f+2,arg.size());
          addNode = true;
        }
        else
        {
          f = arg.find("=");
          if (f != std::string::npos)
            value = arg.substr(f+1,arg.size());
        }

        if (value != "") {
          std::stringstream ss;
          ss << arg.substr(1,f-1);
          std::string child;
          std::reference_wrapper<sg::Node> node_ref = root;
          try {
            while (ss >> child) {
              node_ref = node_ref.get().childRecursive(child);
            }
          } catch (const std::runtime_error &) {
            std::cerr << "Warning: unknown sg::Node '" << child
              << "', ignoring option '" << orgarg << "'." << std::endl;
          }
          auto &node = node_ref.get();

          std::stringstream vals(value);

          if (addNode) {
            std::string name, type;
            vals >> name >> type;
            try {
              node.createChild(name, type);
            } catch (const std::runtime_error &) {
              std::cerr << "Warning: unknown sg::Node type '" << type
                << "', ignoring option '" << orgarg << "'." << std::endl;
            }
          } else { // set node value

            // TODO: more generic implementation
            if (node.valueIsType<std::string>()) {
              node.setValue(value);
            } else if (node.valueIsType<float>()) {
              float x;
              vals >> x;
              node.setValue(x);
            } else if (node.valueIsType<int>()) {
              int x;
              vals >> x;
              node.setValue(x);
            } else if (node.valueIsType<bool>()) {
              bool x;
              vals >> x;
              node.setValue(x);
            } else if (node.valueIsType<ospcommon::vec3f>()) {
              float x,y,z;
              vals >> x >> y >> z;
              node.setValue(ospcommon::vec3f(x,y,z));
            } else if (node.valueIsType<ospcommon::vec2i>()) {
              int x,y;
              vals >> x >> y;
              node.setValue(ospcommon::vec2i(x,y));
            } else try {
              auto &vec = dynamic_cast<sg::DataVector1f&>(node);
              float f;
              while (vals.good()) {
                vals >> f;
                vec.push_back(f);
              }
            } catch(...) {}
          }
        }
      }
    }

    void parseExtraParametersFromComandLine(int ac, const char **&av)
    {
      bool isStream = false;
      bool isSimd   = false;

      for (int i = 1; i < ac; i++) {
        const std::string arg = av[i];
        if (arg == "--simd" || arg == "-simd") {
          isSimd = true;
        } else if (arg == "--stream" || arg == "-stream") {
          isStream = true;
        } else {
          ospcommon::FileName fileName = arg;
          if (fileName.ext() == "obj")
            files.push_back(arg);
          else
            std::cout << "Ignoring non-obj file " << arg << std::endl;
        }
      }

      if (isStream) g_postfix += "_stream";
      if (isSimd)   g_postfix += "_simd";
    }

    static inline void addLightsToScene(sg::Node& renderer)
    {
      auto &lights = renderer["lights"];

    #if 0
      auto &sun = lights.createChild("sun", "DirectionalLight");
      sun["color"] = vec3f(1.f,232.f/255.f,166.f/255.f);
      sun["direction"] = vec3f(0.462f,-1.f,-.1f);
      sun["intensity"] = 1.5f;

      auto &bounce = lights.createChild("bounce", "DirectionalLight");
      bounce["color"] = vec3f(127.f/255.f,178.f/255.f,255.f/255.f);
      bounce["direction"] = vec3f(-.93,-.54f,-.605f);
      bounce["intensity"] = 0.25f;

      auto &ambient = lights.createChild("ambient", "AmbientLight");
      ambient["intensity"] = 0.9f;
      ambient["color"] = vec3f(174.f/255.f,218.f/255.f,255.f/255.f);
    #endif
    }


    extern "C" int main(int ac, const char **av)
    {
      int init_error = ospInit(&ac, av);
      if (init_error != OSP_NO_ERROR) {
        std::cerr << "FATAL ERROR DURING INITIALIZATION!" << std::endl;
        return init_error;
      }

      auto device = ospGetCurrentDevice();
      if (device == nullptr) {
        std::cerr << "FATAL ERROR DURING GETTING CURRENT DEVICE!" << std::endl;
        return 1;
      }

      ospDeviceSetString(device, "logOutput", "cout");
      ospDeviceSetString(device, "errorOutput", "cerr");

      ospDeviceCommit(device);

      // access/load symbols/sg::Nodes dynamically
      loadLibrary("ospray_sg");
      ospLoadModule("cpp");

      parseExtraParametersFromComandLine(ac, av);

      ospray::imgui3D::init(&ac,av);

      // import the patches from the sample files (creates a default
      // patch if no files were specified)
      box3f worldBounds;

      auto renderer_ptr = sg::createNode("renderer", "Renderer");
      auto &renderer = *renderer_ptr;

      auto &win_size = ospray::imgui3D::ImGui3DWidget::defaultInitSize;
      renderer["frameBuffer"]["size"] = win_size;
      renderer["rendererType"].setFlags(sg::NodeFlags::required |
                                        sg::NodeFlags::gui_combo);
      renderer["rendererType"] = std::string(g_prefix + "ao" + g_postfix);

      auto &world = renderer["world"];

      for (auto file : files)
        importOBJ_cpp(world.shared_from_this(), file);

      addLightsToScene(renderer);

      // last, to be able to modify all created SG nodes
      parseCommandLineSG(ac, av, renderer);

      ImGuiViewer window(renderer_ptr);

      auto &viewPort = window.viewPort;
      auto dir = normalize(viewPort.at - viewPort.from);
      auto camera_ptr = sg::createNode("camera", "CppPerspectiveCamera");
      renderer.add(camera_ptr);
      auto &camera = *camera_ptr;
      camera["dir"] = dir;
      camera["pos"] = viewPort.from;
      camera["up"]  = viewPort.up;
      camera["fovy"] = viewPort.openingAngle;
      camera["apertureRadius"] = viewPort.apertureRadius;
      if (camera.hasChild("focusdistance"))
        camera["focusdistance"] = length(viewPort.at - viewPort.from);

      window.create("ospCppViewer: OSPRay C++-rendering Viewer");

      ospray::imgui3D::run();
    }

  } // ::ospray::cpp_renderer
} // ::ospray