//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_FUNCTION_IFREXP_HPP_INCLUDED
#define BOOST_SIMD_FUNCTION_IFREXP_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{

 /*!

    @ingroup group-ieee
    Function object implementing ifrexp capabilities

    Computes a mantissa and an exponent pair for the input

    @par Semantic:

    For every parameter of floating type @c T

    @code
    std::tie(m, e)= ifrexp(x);
    @endcode

    is similar to:

    @code
    as_integer_t<T> e = exponent(x)+1;
    T m = copysign(mantissa(x)/2, x);
    @endcode

    The call

    @code
    std:pair<T,as_integer_t<T>> p = ifrexp(x);
    @endcode

    can also be used.

    @par Note:

    This function splits a floating point value @c v f in a signed mantissa @c m and
    an exponent @c e so that:  @f$v = m\times 2^e@f$,
    with absolute value of @c m \f$\in [1/2, 1[\f$

    @warninbox{Take care that these results differ from the returns of the functions @ref mantissa
    and @ref exponent}

    The decorators pedantic_ and std_ can be used.

    without pedantic_ the call is speedier, but special values as Nan or Inf are not handled properly.
    std_ transmit the call to std::ifrexp. That implies that simd is ever emulated.

    @see exponent, mantissa, copysign

  **/
  std::pair<T, as_integer_t<Value>> ifrexp(Value const & v0);
} }
#endif

#include <boost/simd/function/scalar/ifrexp.hpp>
#include <boost/simd/function/simd/ifrexp.hpp>

#endif
