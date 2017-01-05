//==================================================================================================
/**
  Copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
**/
//==================================================================================================
#ifndef BOOST_SIMD_DETAIL_CONSTANT_INVLOG_2LO_HPP_INCLUDED
#define BOOST_SIMD_DETAIL_CONSTANT_INVLOG_2LO_HPP_INCLUDED

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
    struct invlog_2lo_ : boost::dispatch::constant_value_<invlog_2lo_>
    {
      BOOST_DISPATCH_MAKE_CALLABLE(ext,invlog_2lo_,boost::dispatch::constant_value_<invlog_2lo_>);
      BOOST_SIMD_REGISTER_CONSTANT(0,0xb9389ad4UL, 0x3de705fc2eefa200ULL);
    };
  }

  namespace ext
  {
    BOOST_DISPATCH_FUNCTION_DECLARATION(tag, invlog_2lo_)
  }

  namespace detail
  {
    BOOST_DISPATCH_CALLABLE_DEFINITION(tag::invlog_2lo_,invlog_2lo);
  }

  template<typename T> BOOST_FORCEINLINE auto Invlog_2lo()
  BOOST_NOEXCEPT_DECLTYPE(detail::invlog_2lo( boost::dispatch::as_<T>{}))
  {
    return detail::invlog_2lo( boost::dispatch::as_<T>{} );
  }
} }

#include <boost/simd/arch/common/scalar/constant/constant_value.hpp>
#include <boost/simd/arch/common/simd/constant/constant_value.hpp>

#endif
