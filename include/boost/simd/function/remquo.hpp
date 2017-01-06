//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_FUNCTION_REMQUO_HPP_INCLUDED
#define BOOST_SIMD_FUNCTION_REMQUO_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{

 /*!

    @ingroup group-arithmetic
    Function object implementing remquo capabilities

    Remainder and bits of quotient

    remquo computes the remainder (rem) and a part of the quotient (quo) upon division of
    @c x by @c y. By design, the value of the remainder is the same as that
    computed by the @ref remainder function. The value of the computed quotient has
    the sign of @c x/y and agrees with the actual quotient in at least the low
    order 3 bits.

    @par semantic:

    For any given value @c x, @c y of type @c T:

    @code
    as_integer_t<T>& quo;
    T rem = remquo(x, y, quo);
    @endcode

    or

    @code
    std::pair< T, as_integer_t<T> > p = remquo(x, y);
    @endcode

    computes the two values.

    @par Note

      - This function mimics a standard C library one that was mainly written in its time to
      help computation of periodic functions: three bits of quo allowing to know the
      'octant'

      - This implementation differs from std::remquo as the quotient is not returned as a pointer, and
      his type is not int the the signed integer type associated to the floating one, to allow
      proper SIMD implementation.

      - also note that the double implementation of std::remquo is flawed in GNU C
      Library until version 2.21 (2.22 been corrected).
  **/
  IntegerValue remquo(Value const & v0, Value const& y, IntegerValue const& quo);
} }
#endif

#include <boost/simd/function/scalar/remquo.hpp>
#include <boost/simd/function/simd/remquo.hpp>

#endif
