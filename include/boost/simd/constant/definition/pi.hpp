//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_CONSTANT_DEFINITION_PI_HPP_INCLUDED
#define BOOST_SIMD_CONSTANT_DEFINITION_PI_HPP_INCLUDED

#include <boost/simd/config.hpp>
#include <boost/simd/detail/brigand.hpp>
#include <boost/simd/detail/dispatch.hpp>
#include <boost/simd/detail/constant_traits.hpp>
#include <boost/simd/detail/dispatch/function/make_callable.hpp>
#include <boost/simd/detail/dispatch/hierarchy/functions.hpp>
#include <boost/simd/detail/dispatch/as.hpp>

namespace boost { namespace simd
{
  namespace tag
  {
    struct pi_ : boost::dispatch::constant_value_<pi_>
    {
      BOOST_DISPATCH_MAKE_CALLABLE(ext,pi_,boost::dispatch::constant_value_<pi_>);
      BOOST_SIMD_REGISTER_CONSTANT(3, 0x40490FDBUL, 0x400921FB54442D18ULL);
    };
  }

  namespace ext
  {
    BOOST_DISPATCH_FUNCTION_DECLARATION(tag, pi_)
  }

  namespace detail
  {
    BOOST_DISPATCH_CALLABLE_DEFINITION(tag::pi_,pi);
  }

  template<typename T> BOOST_FORCEINLINE auto Pi()
  BOOST_NOEXCEPT_DECLTYPE(detail::pi( boost::dispatch::as_<T>{}))
  {
    return detail::pi( boost::dispatch::as_<T>{} );
  }
} }

#endif
