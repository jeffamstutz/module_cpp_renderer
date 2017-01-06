//==================================================================================================
/*!
  @file

  @copyright 2015 NumScale SAS
  @copyright 2015 J.T. Lapreste

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_INEARBYINT_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_INEARBYINT_HPP_INCLUDED

#include <boost/simd/function/pedantic.hpp>
#include <boost/simd/function/nearbyint.hpp>
#include <boost/simd/function/toint.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/simd/detail/dispatch/meta/as_integer.hpp>
#include <boost/config.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  BOOST_DISPATCH_OVERLOAD ( inearbyint_
                          , (typename A0)
                          , bd::cpu_
                          , bs::pedantic_tag
                          , bd::scalar_<bd::integer_<A0> >
                          )
  {
    BOOST_FORCEINLINE A0 operator() (pedantic_tag const &
                                    , A0 a0) const BOOST_NOEXCEPT
    {
      return a0;
    }
  };
  BOOST_DISPATCH_OVERLOAD ( inearbyint_
                          , (typename A0)
                          , bd::cpu_
                          , bs::pedantic_tag
                          , bd::scalar_<bd::floating_<A0> >
                          )
  {
    BOOST_FORCEINLINE bd::as_integer_t<A0> operator() (pedantic_tag const &
                                                      , A0 a0) const BOOST_NOEXCEPT
    {
      return saturated_(toint)(nearbyint(a0));
    }
  };
  BOOST_DISPATCH_OVERLOAD ( inearbyint_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_<bd::integer_<A0> >
                          )
  {
    BOOST_FORCEINLINE A0 operator() (A0 a0 ) const BOOST_NOEXCEPT
    {
      return a0;
    }
  };
  BOOST_DISPATCH_OVERLOAD ( inearbyint_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_<bd::floating_<A0> >
                          )
  {
    BOOST_FORCEINLINE bd::as_integer_t<A0> operator() ( A0 a0 ) const BOOST_NOEXCEPT
    {
      return toint(nearbyint(a0));
    }
  };
} } }


#endif
