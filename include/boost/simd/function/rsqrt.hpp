//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_FUNCTION_RSQRT_HPP_INCLUDED
#define BOOST_SIMD_FUNCTION_RSQRT_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{

 /*!

    @ingroup group-arithmetic
    Function object implementing rsqrt capabilities

    Returns the inverse of the square root of the input.

    @par semantic:

    For any given value @c x of floating type @c T:

    @code
    T r = rsqrt(x);
    @endcode

    For signed type is similar to:

    @code
    T r = T(1)/sqrt(x)
    @endcode

    @par Decorators

    - raw_ for floating entries: if full accuracy is not needed a sometimes rawer less accurate version of the function
    can be obtained using the the raw_ decorator : raw_(rsqrt)(x).

    @par Decorators

  **/
  Value rsqrt(Value const & v0);
} }
#endif

#include <boost/simd/function/scalar/rsqrt.hpp>
#include <boost/simd/function/scalar/rsqrt.hpp>
#include <boost/simd/function/simd/rsqrt.hpp>

#endif
