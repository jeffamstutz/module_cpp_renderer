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
#include <functional>

namespace ospray {
  namespace cpp_renderer {
    namespace detail {
      extern thread_local std::function<void(int)> currentFiberTask;
      extern thread_local bool newTask;
      extern thread_local bool cancelFibers;
      extern thread_local int numFinishedFibers;

      struct FiberGroup
      {
        inline ~FiberGroup() { clearFibers(); }

        inline void clearFibers()
        {
          if (!activeFibers.empty()) {
            cancelFibers = true;
            newTask = false;

            for (auto &f : activeFibers)
              f.join();

            activeFibers.clear();
          }
        }

        std::vector<boost::fibers::fiber> activeFibers;
      };

      extern thread_local FiberGroup fibers;

      static void fiber_fcn(int whichFiber, int numSiblingFibers)
      {
        while (true) {
          if (cancelFibers)
            return;

          if (!newTask) {
            boost::this_fiber::yield();
            continue;
          }

          currentFiberTask(whichFiber);

          numFinishedFibers++;

          if (numFinishedFibers == numSiblingFibers)
            newTask = false;
        }
      }
    } // ::ospray::cpp_renderer::detail

    template<typename FCN_T>
    inline void concurrent_for(int nFibers, FCN_T&& fcn)
    {
      using namespace ospcommon::traits;
      static_assert(has_operator_method_with_integral_param<FCN_T>::value,
                    "ospray::cpp_renderer::concurrent_for() requires FCN_T"
                    " to implement operator() with a single 'int' parameter.");

      if (detail::fibers.activeFibers.size() != size_t(nFibers)) {
        detail::fibers.activeFibers.resize(nFibers);

        int whichFiber = 0;
        for (auto &fiber : detail::fibers.activeFibers)
          fiber = boost::fibers::fiber(boost::fibers::launch::post,
                                       detail::fiber_fcn,
                                       whichFiber++,
                                       nFibers);
      }

      detail::numFinishedFibers = 0;

      detail::currentFiberTask = fcn;
      detail::newTask = true;

      while (detail::numFinishedFibers < nFibers)
        boost::this_fiber::yield();
    }

  } // ::ospray::cpp_renderer
} //::ospray
