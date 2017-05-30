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

#include "ospcommon/TypeTraits.h"

#include <boost/fiber/all.hpp>

namespace ospray {
  namespace cpp_renderer {

    // NOTE(jda) - This abstraction wraps "fork-join" parallelism, with an
    //             implied synchronizsation after all of the tasks have run.
    template<typename FCN_T>
    inline void concurrent_for(int nFibers, FCN_T&& fcn)
    {
      using namespace ospcommon::traits;
      static_assert(has_operator_method_with_integral_param<FCN_T>::value,
                    "ospray::cpp_renderer::concurrent_for() requires FCN_T"
                    " to implement operator() with a single 'int' parameter.");

      std::vector<boost::fibers::fiber> fibers;

      for (int i = 0; i < nFibers; ++i)
        fibers.emplace_back(fcn, i);

      for (auto &f : fibers)
        f.join();
    }

  } // ::ospray::cpp_renderer
} //::ospray
