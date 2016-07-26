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

// ospray
#include "TriangleMesh.h"
#include "common/Model.h"
#include "common/Data.h"
// embree
#include "embree2/rtcore.h"
#include "embree2/rtcore_scene.h"
#include "embree2/rtcore_geometry.h"

using std::cout;
using std::endl;

namespace ospray {
  namespace cpp_renderer {

    std::string TriangleMesh::toString() const
    {
      return "ospray::TriangleMesh";
    }

    void TriangleMesh::finalize(Model *model)
    {
      static int numPrints = 0;
      numPrints++;

      if (logLevel >= 2)
        if (numPrints < 5)
          std::cout << "ospray: finalizing triangle mesh ..." << std::endl;

      Assert(model && "invalid model pointer");

      RTCScene embreeSceneHandle = model->embreeSceneHandle;

      vertexData = getParamData("vertex",getParamData("position"));
      normalData = getParamData("vertex.normal",getParamData("normal"));
      colorData  = getParamData("vertex.color",getParamData("color"));
      texcoordData = getParamData("vertex.texcoord",getParamData("texcoord"));
      indexData  = getParamData("index",getParamData("triangle"));
      prim_materialIDData = getParamData("prim.materialID");
      materialListData = getParamData("materialList");
      geom_materialID = getParam1i("geom.materialID",-1);

      Assert2(vertexData != NULL,
              "triangle mesh geometry does not have either 'position'"
              " or 'vertex' array");
      Assert2(indexData != NULL,
              "triangle mesh geometry does not have either 'index'"
              " or 'triangle' array");

      this->index = (int*)indexData->data;
      this->vertex = (float*)vertexData->data;
      this->normal = normalData ? (float*)normalData->data : NULL;
      this->color  = colorData ? (vec4f*)colorData->data : NULL;
      this->texcoord = texcoordData ? (vec2f*)texcoordData->data : NULL;
      this->prim_materialID  = prim_materialIDData ? (uint32*)prim_materialIDData->data : NULL;
      this->materialList  = materialListData ? (ospray::Material**)materialListData->data : NULL;

#if 0
      if (materialList && !ispcMaterialPtrs) {
        const int num_materials = materialListData->numItems;
        ispcMaterialPtrs = new void*[num_materials];
        for (int i = 0; i < num_materials; i++) {
          assert(this->materialList[i] != NULL && "Materials in list should never be NULL");
          this->ispcMaterialPtrs[i] = this->materialList[i]->getIE();
        }
      }
#endif

      size_t numTris  = -1;
      size_t numVerts = -1;

      size_t numCompsInTri = 0;
      size_t numCompsInVtx = 0;
      size_t numCompsInNor = 0;
      switch (indexData->type) {
      case OSP_INT:
      case OSP_UINT:  numTris = indexData->size() / 3; numCompsInTri = 3; break;
      case OSP_INT3:
      case OSP_UINT3: numTris = indexData->size(); numCompsInTri = 3; break;
      case OSP_UINT4:
      case OSP_INT4:  numTris = indexData->size(); numCompsInTri = 4; break;
      default:
        throw std::runtime_error("unsupported trianglemesh.index data type");
      }

      switch (vertexData->type) {
      case OSP_FLOAT:   numVerts = vertexData->size() / 4; numCompsInVtx = 4; break;
      case OSP_FLOAT3:  numVerts = vertexData->size(); numCompsInVtx = 3; break;
      case OSP_FLOAT3A: numVerts = vertexData->size(); numCompsInVtx = 4; break;
      case OSP_FLOAT4 : numVerts = vertexData->size(); numCompsInVtx = 4; break;
      default:
        throw std::runtime_error("unsupported trianglemesh.vertex data type");
      }
      if (normalData) switch (normalData->type) {
      case OSP_FLOAT3:  numCompsInNor = 3; break;
      case OSP_FLOAT:
      case OSP_FLOAT3A: numCompsInNor = 4; break;
      default:
        throw std::runtime_error("unsupported trianglemesh.vertex.normal data type");
      }


      eMesh = rtcNewTriangleMesh(embreeSceneHandle,RTC_GEOMETRY_STATIC,
                                 numTris,numVerts);

      rtcSetBuffer(embreeSceneHandle,eMesh,RTC_VERTEX_BUFFER,
                   (void*)this->vertex,0,
                   sizeOf(vertexData->type));
      rtcSetBuffer(embreeSceneHandle,eMesh,RTC_INDEX_BUFFER,
                   (void*)this->index,0,
                   sizeOf(indexData->type));

      bounds = empty;

      for (int i = 0; i < numVerts*numCompsInVtx; i += numCompsInVtx)
        bounds.extend(*(vec3f*)(vertex + i));

      if (logLevel >= 2) {
        if (numPrints < 5) {
          cout << "  created triangle mesh (" << numTris << " tris "
               << ", " << numVerts << " vertices)" << endl;
          cout << "  mesh bounds " << bounds << endl;
        }
      }
    }

    void TriangleMesh::postIntersect(DifferentialGeometry &dg,
                                     const Ray &ray,
                                     int flags) const
    {
      std::cout << "postIntersect()" << std::endl;
    }

    OSP_REGISTER_GEOMETRY(TriangleMesh, cpp_triangles)
    OSP_REGISTER_GEOMETRY(TriangleMesh, cpp_trianglemesh)

  }// namespace cpp_renderer
}// namespace ospray
