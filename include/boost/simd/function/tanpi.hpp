//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_FUNCTION_TANPI_HPP_INCLUDED
#define BOOST_SIMD_FUNCTION_TANPI_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{

 /*!

    @ingroup group-trigonometric
    Function object implementing tanpi capabilities

    tangent of angle in \f$\pi\f$ multiples: \f$\cos(\pi x)/sin(\pi x)\f$.

    @par Semantic:

    For every parameter of floating type

    @code
    auto r = tanpi(x);
    @endcode


    As most other trigonometric function tanpi can be called with a second optional parameter
    which is a tag on speed and accuracy (see @ref cos for further details)

    @see tan, tand

  **/
  Value tanpi(Value const & v0);
} }
#endif

#include <boost/simd/function/scalar/tanpi.hpp>
#include <boost/simd/function/simd/tanpi.hpp>

#endif
