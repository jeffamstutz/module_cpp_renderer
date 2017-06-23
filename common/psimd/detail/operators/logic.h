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

#include <type_traits>

#include "../pack.h"

namespace psimd {

  // binary operator==() //

  template <typename T, int W>
  inline mask<W> operator==(const pack<T, W> &p1, const pack<T, W> &p2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] == p2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator==(const pack<T, W> &p1, const OTHER_T &v)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] == v) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator==(const OTHER_T &v, const pack<T, W> &p1)
  {
    return p1 == v;
  }

  // binary operator!=() //

  template <typename T, int W>
  inline mask<W> operator!=(const pack<T, W> &p1, const pack<T, W> &p2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] != p2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator!=(const pack<T, W> &p1, const OTHER_T &v)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] != v) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator!=(const OTHER_T &v, const pack<T, W> &p1)
  {
    return p1 != v;
  }

  // binary operator<() //

  template <typename T, int W>
  inline mask<W> operator<(const pack<T, W> &p1, const pack<T, W> &p2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] < p2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator<(const pack<T, W> &p1, const OTHER_T &v)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] < v) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator<(const OTHER_T &v, const pack<T, W> &p1)
  {
    return pack<T, W>(v) < p1;
  }

  // binary operator<=() //

  template <typename T, int W>
  inline mask<W> operator<=(const pack<T, W> &p1, const pack<T, W> &p2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] <= p2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator<=(const pack<T, W> &p1, const OTHER_T &v)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] <= v) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator<=(const OTHER_T &v, const pack<T, W> &p1)
  {
    return pack<T, W>(v) <= p1;
  }

  // binary operator>() //

  template <typename T, int W>
  inline mask<W> operator>(const pack<T, W> &p1, const pack<T, W> &p2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] > p2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator>(const pack<T, W> &p1, const OTHER_T &v)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] > v) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator>(const OTHER_T &v, const pack<T, W> &p1)
  {
    return pack<T, W>(v) > p1;
  }

  // binary operator>=() //

  template <typename T, int W>
  inline mask<W> operator>=(const pack<T, W> &p1, const pack<T, W> &p2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] >= p2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator>=(const pack<T, W> &p1, const OTHER_T &v)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (p1[i] >= v) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  template <typename T, int W, typename OTHER_T>
  inline typename
  std::enable_if<std::is_convertible<OTHER_T, T>::value, mask<W>>::type
  operator>=(const OTHER_T &v, const pack<T, W> &p1)
  {
    return pack<T, W>(v) >= p1;
  }

  // binary operator&&() //

  template <int W>
  inline mask<W> operator&&(const mask<W> &m1, const mask<W> &m2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (m1[i] && m2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  // binary operator||() //

  template <int W>
  inline mask<W> operator||(const mask<W> &m1, const mask<W> &m2)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = (m1[i] || m2[i]) ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

  // unary operator!() //

  template <int W>
  inline mask<W> operator!(const mask<W> &m)
  {
    mask<W> result;

    #pragma omp simd
    for (int i = 0; i < W; ++i)
      result[i] = !m[i] ? 0xFFFFFFFF : 0x00000000;

    return result;
  }

} // ::psimd