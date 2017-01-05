//==================================================================================================
/*!
  @file

  @copyright 2015 NumScale SAS
  @copyright 2015 J.T. Lapreste

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_SECPI_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_SECPI_HPP_INCLUDED

#include <boost/simd/function/restricted.hpp>
#include <boost/simd/constant/half.hpp>
#include <boost/simd/function/if_nan_else.hpp>
#include <boost/simd/function/is_flint.hpp>
#include <boost/simd/function/rec.hpp>
#include <boost/simd/function/cospi.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/config.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  namespace bs = boost::simd;
  BOOST_DISPATCH_OVERLOAD ( secpi_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::floating_<A0> >
                          )
  {
    BOOST_FORCEINLINE A0 operator() ( A0 a0) const BOOST_NOEXCEPT
    {
      return secpi(a0, tag::big_);
    }
  };
  BOOST_DISPATCH_OVERLOAD ( secpi_
                          , (typename A0, typename A1)
                          , bd::cpu_
                          , bd::scalar_< bd::floating_<A0> >
                          , bd::scalar_ < bd::unspecified_<A1> >
                          )
  {
    BOOST_FORCEINLINE A0 operator() ( A0 a0, A1 const&) const BOOST_NOEXCEPT
    {
      return if_nan_else(is_flint(a0-Half<A0>()), rec(cospi(a0, A1())));
    }
  };
  BOOST_DISPATCH_OVERLOAD ( secpi_
                          , (typename A0)
                          , bd::cpu_
                          , bs::restricted_tag
                          , bd::scalar_< bd::floating_<A0> >
                          )
  {
    BOOST_FORCEINLINE A0 operator() (const restricted_tag &,  A0 a0) const BOOST_NOEXCEPT
    {
      return rec(restricted_(cospi)(a0));
    }
  };
} } }


#endif
