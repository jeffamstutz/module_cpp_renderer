//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_CONSTANT_INVEPS_HPP_INCLUDED
#define BOOST_SIMD_CONSTANT_INVEPS_HPP_INCLUDED

#if defined(DOXYGEN_ONLY)
namespace boost { namespace simd
{
  /*!
    @ingroup group-constant

    Generate  value \f$\sqrt2\f$

    @par Semantic:

    @code
    T r = inveps<T>();
    @endcode

    is similar to:

    @code
    T r = rec(Eps<T>());
    @endcode

    @return The inveps constant for the proper type
  **/
  template<typename T> T inveps();

  namespace functional
  {
    /*!
      @ingroup group-callable-constant
      Generate the  constant inveps.

      @return The inveps constant for the proper type
    **/
    const boost::dispatch::functor<tag::inveps_> inveps = {};
  }
} }
#endif

#include <boost/simd/constant/definition/inveps.hpp>
#include <boost/simd/arch/common/scalar/constant/constant_value.hpp>
#include <boost/simd/arch/common/simd/constant/constant_value.hpp>

#endif
