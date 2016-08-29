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
      return "ospray::cpp_renderer::TriangleMesh";
    }

    void TriangleMesh::finalize(Model *model)
    {
      static int numPrints = 0;
      numPrints++;

      if (logLevel >= 2)
        if (numPrints < 5)
          std::cout << "ospray: finalizing triangle mesh ..." << std::endl;

      RTCScene embreeSceneHandle = model->embreeSceneHandle;

      vertexData = getParamData("vertex",getParamData("position"));
      normalData = getParamData("vertex.normal",getParamData("normal"));
      colorData  = getParamData("vertex.color",getParamData("color"));
      texcoordData = getParamData("vertex.texcoord",getParamData("texcoord"));
      indexData  = getParamData("index",getParamData("triangle"));
      prim_materialIDData = getParamData("prim.materialID");
      materialListData = getParamData("materialList");
      geom_materialID = getParam1i("geom.materialID",-1);

      this->index = (int*)indexData->data;
      this->vertex = (float*)vertexData->data;
      this->normal = normalData ? (float*)normalData->data : nullptr;
      this->color  = colorData ? (vec4f*)colorData->data : nullptr;
      this->texcoord = texcoordData ? (vec2f*)texcoordData->data : nullptr;
      this->prim_materialID  =
          prim_materialIDData ? (uint32*)prim_materialIDData->data : nullptr;
      this->materialList =
          materialListData ? (ospray::Material**)materialListData->data :
                             nullptr;

#if 0
      if (materialList && !ispcMaterialPtrs) {
        const int num_materials = materialListData->numItems;
        ispcMaterialPtrs = new void*[num_materials];
        for (int i = 0; i < num_materials; i++) {
          assert(this->materialList[i] != nullptr &&
                 "Materials in list should never be NULL");
          this->ispcMaterialPtrs[i] = this->materialList[i]->getIE();
        }
      }
#endif

      size_t numVerts = -1;
      switch (indexData->type) {
      case OSP_INT:
      case OSP_UINT:  numTris = indexData->size() / 3; idxSize = 3; break;
      case OSP_INT3:
      case OSP_UINT3: numTris = indexData->size(); idxSize = 3; break;
      case OSP_UINT4:
      case OSP_INT4:  numTris = indexData->size(); idxSize = 4; break;
      default:
        throw std::runtime_error("unsupported trianglemesh.index data type");
      }

      switch (vertexData->type) {
      case OSP_FLOAT:   numVerts = vertexData->size() / 4; vtxSize = 4; break;
      case OSP_FLOAT3:  numVerts = vertexData->size(); vtxSize = 3; break;
      case OSP_FLOAT3A: numVerts = vertexData->size(); vtxSize = 4; break;
      case OSP_FLOAT4 : numVerts = vertexData->size(); vtxSize = 4; break;
      default:
        throw std::runtime_error("unsupported trianglemesh.vertex data type");
      }
      if (normalData) switch (normalData->type) {
      case OSP_FLOAT3:  norSize = 3; break;
      case OSP_FLOAT:
      case OSP_FLOAT3A: norSize = 4; break;
      default:
        throw std::runtime_error("unsupported trianglemesh.vertex.normal"
                                 " data type");
      }


      eMesh = rtcNewTriangleMesh(embreeSceneHandle, RTC_GEOMETRY_STATIC,
                                 numTris, numVerts);

      rtcSetBuffer(embreeSceneHandle,eMesh,RTC_VERTEX_BUFFER,
                   (void*)this->vertex,0,
                   sizeOf(vertexData->type));
      rtcSetBuffer(embreeSceneHandle,eMesh,RTC_INDEX_BUFFER,
                   (void*)this->index,0,
                   sizeOf(indexData->type));

      bounds = empty;

      for (size_t i = 0; i < numVerts*vtxSize; i += vtxSize)
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
      dg.Ng = dg.Ns = ray.Ng;
      const int base = idxSize * ray.primID;
      const vec3i idx = vec3i{index[base+0], index[base+1], index[base+2]};

      if ((flags & DG_NS) && normal) {
        const vec3f &n0 = normal[idx.x * norSize];
        const vec3f &n1 = normal[idx.y * norSize];
        const vec3f &n2 = normal[idx.z * norSize];
        dg.Ns = (1.f-ray.u-ray.v) * n0 + (ray.u * n1) + (ray.v * n2);
      }

      if ((flags & DG_COLOR) && color) {
        dg.color = (1.f-ray.u-ray.v) * (color[idx.x])
                   + ray.u * (color[idx.y])
                   + ray.v * (color[idx.z]);
      }

      if (flags & DG_TEXCOORD && texcoord) {
        //calculate texture coordinate using barycentric coordinates
        dg.st = (1.f-ray.u-ray.v) * (texcoord[idx.x])
                + ray.u * (texcoord[idx.y])
                + ray.v * (texcoord[idx.z]);
      } else {
        dg.st = vec2f{0.0f};
      }

      if (flags & DG_TANGENTS) {
        bool fallback = true;
        if (texcoord) {
          const vec2f dst02 = texcoord[idx.x] - texcoord[idx.z];
          const vec2f dst12 = texcoord[idx.y] - texcoord[idx.z];
          const float det = dst02.x * dst12.y - dst02.y * dst12.x;

          if (det != 0.f) {
            const float invDet = rcp(det);
            const vec3f &v0 = vertex[idx.x * vtxSize];
            const vec3f &v1 = vertex[idx.y * vtxSize];
            const vec3f &v2 = vertex[idx.z * vtxSize];
            const vec3f dp02 = v0 - v2;
            const vec3f dp12 = v1 - v2;
            dg.dPds = (dst12.y * dp02 - dst02.y * dp12) * invDet;
            dg.dPdt = (dst02.x * dp12 - dst12.x * dp02) * invDet;
            fallback = false;
          }
        }
        if (fallback) {
          linear3f f = frame(dg.Ng);
          dg.dPds = f.vx;
          dg.dPdt = f.vy;
        }
      }

      if (flags & DG_MATERIALID) {
        if (prim_materialID) {
          dg.materialID = prim_materialID[ray.primID];
        }
        else {
          dg.materialID = geom_materialID;
        }

        if(materialList) {
          Material *myMat = materialList[dg.materialID < 0 ? 0 : dg.materialID];
          dg.material = myMat;
        }
      }
    }

    OSP_REGISTER_GEOMETRY(TriangleMesh, cpp_triangles);
    OSP_REGISTER_GEOMETRY(TriangleMesh, cpp_trianglemesh);

  }// namespace cpp_renderer
}// namespace ospray
