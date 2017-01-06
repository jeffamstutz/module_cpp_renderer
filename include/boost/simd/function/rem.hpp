//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_FUNCTION_REM_HPP_INCLUDED
#define BOOST_SIMD_FUNCTION_REM_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{

 /*!

    @ingroup group-oerator
    Function object implementing rem capabilities

    Computes the remainder of division.
    The return value is x-n*y, where n is the value x/y,
    rounded toward zero.

    @par semantic:
    For any given value @c x, @c y of type @c T:

    @code
    T r = rem({option, }x, y);
    @endcode

    if there is no option the call is similar to:
    @code
    T r = x-div(fix, x, y)*y;
    @endcode

    else option can be ceil, floor, fix, round, nearbyint (in the namespace booost::simd)
    and the code is similar to :

    @code
    T r = x-div(option, x, y)*y;
    @endcode

    @Notes

    -Supported types
     unsigned types are not supported but for the option @c fix, as in other cases result can be negative

    -Limiting values for floating entries:

       -  if x is +/-inf , Nan is returned
       -  if x is +/-0 and y is not 0, 0 is returned (if the sign of x matters the pedantic_ decorated version returns x)
       -  If y is +/-0, Nan is returned
       -  If either argument is NaN, Nan is returned

    @par Decorators

    with floating entries decorator std_ calls the stdlibc++ corresponding function (caution : no simd acceleration)

      - pedantic_ insure better limiting cases (see above note)

      - std_ allow direct calls to some libc++ function:

        -option fix        calls std::fmod
        -option nearbyint  calls std::remainder
         The other options have no standard correspondant

  **/
  Value rem(Value const & v0, value const& y);
} }
#endif

#include <boost/simd/function/scalar/rem.hpp>
#include <boost/simd/function/simd/rem.hpp>

#endif
