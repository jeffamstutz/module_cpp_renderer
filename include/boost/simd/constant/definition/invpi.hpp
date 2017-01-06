//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_CONSTANT_DEFINITION_INVPI_HPP_INCLUDED
#define BOOST_SIMD_CONSTANT_DEFINITION_INVPI_HPP_INCLUDED

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
    struct invpi_ : boost::dispatch::constant_value_<invpi_>
    {
      BOOST_DISPATCH_MAKE_CALLABLE(ext,invpi_,boost::dispatch::constant_value_<invpi_>);
      BOOST_SIMD_REGISTER_CONSTANT(0, 0X3EA2F983, 0X3FD45F306DC9C883LL);
    };
  }

  namespace ext
  {
    BOOST_DISPATCH_FUNCTION_DECLARATION(tag, invpi_)
  }

  namespace detail
  {
    BOOST_DISPATCH_CALLABLE_DEFINITION(tag::invpi_,invpi);
  }

  template<typename T> BOOST_FORCEINLINE auto Invpi()
  BOOST_NOEXCEPT_DECLTYPE(detail::invpi( boost::dispatch::as_<T>{}))
  {
    return detail::invpi( boost::dispatch::as_<T>{} );
  }
} }

#endif
