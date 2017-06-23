// ========================================================================== //
// The MIT License (MIT)                                                      //
//                                                                            //
// Copyright (c) 2017 Jefferson Amstutz                                       //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// in all copies or substantial portions of the Software.                     //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
// ========================================================================== //

#pragma once

#include "../pack.h"

namespace psimd {

  // load() //

  template <typename PACK_T>
  inline PACK_T load(void* _src)
  {
    auto *src = (typename PACK_T::type*) _src;
    PACK_T result;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      result[i] = src[i];

    return result;
  }

  template <typename PACK_T>
  inline PACK_T load(void* _src,
                     const mask<PACK_T::static_size> &m)
  {
    auto *src = (typename PACK_T::type*) _src;
    PACK_T result;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      if (m[i])
        result[i] = src[i];

    return result;
  }

  // gather() //

  template <typename PACK_T, typename OFFSET_T>
  inline PACK_T gather(void* _src, const pack<OFFSET_T, PACK_T::static_size> &o)
  {
    auto *src = (typename PACK_T::type*) _src;
    PACK_T result;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      result[i] = src[o[i]];

    return result;
  }

  template <typename PACK_T, typename OFFSET_T>
  inline PACK_T gather(void* _src,
                       const pack<OFFSET_T, PACK_T::static_size> &o,
                       const mask<PACK_T::static_size> &m)
  {
    auto *src = (typename PACK_T::type*) _src;
    PACK_T result;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      if(m[i])
        result[i] = src[o[i]];

    return result;
  }

  // store() //

  template <typename PACK_T>
  inline void store(const PACK_T &p, void* _dst)
  {
    auto *dst = (typename PACK_T::type*) _dst;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      dst[i] = p[i];
  }

  template <typename PACK_T>
  inline void store(const PACK_T &p,
                    void* _dst,
                    const mask<PACK_T::static_size> &m)
  {
    auto *dst = (typename PACK_T::type*) _dst;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      if (m[i])
        dst[i] = p[i];
  }

  // scatter() //

  template <typename PACK_T, typename OFFSET_T>
  inline void scatter(const PACK_T &p,
                      void* _dst,
                      const pack<OFFSET_T, PACK_T::static_size> &o)
  {
    auto *dst = (typename PACK_T::type*) _dst;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      dst[o[i]] = p[i];
  }

  template <typename PACK_T, typename OFFSET_T>
  inline void scatter(const PACK_T &p,
                      void* _dst,
                      const pack<OFFSET_T, PACK_T::static_size> &o,
                      const mask<PACK_T::static_size> &m)
  {
    auto *dst = (typename PACK_T::type*) _dst;

    #pragma omp simd
    for (int i = 0; i < PACK_T::static_size; ++i)
      if (m[i])
        dst[o[i]] = p[i];
  }

} // ::psimd