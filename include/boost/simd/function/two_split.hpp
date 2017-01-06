//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_FUNCTION_TWO_SPLIT_HPP_INCLUDED
#define BOOST_SIMD_FUNCTION_TWO_SPLIT_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{

 /*!

    @ingroup group-arithmetic
    Function object implementing two_split capabilities

    For any real @c x, two_split computes two reals @c r0 and @c r1 (in an std::pair)
    such that:

    @code
    x == r0+r1
    @endcode

    and where @c r0 and @c r1 bit pattern don't overlap.

  **/
  std::pair<Value, Value> two_split(Value const& x);
} }
#endif

#include <boost/simd/function/scalar/two_split.hpp>
#include <boost/simd/function/simd/two_split.hpp>

#endif
