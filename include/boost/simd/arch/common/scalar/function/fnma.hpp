//==================================================================================================
/*!
  @file

  @copyright 2015 NumScale SAS
  @copyright 2015 J.T. Lapreste

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_FNMA_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_FNMA_HPP_INCLUDED

#include <boost/simd/function/fma.hpp>
#include <boost/simd/function/minus.hpp>
#include <boost/simd/function/multiplies.hpp>
#include <boost/simd/function/unary_minus.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/config.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  BOOST_DISPATCH_OVERLOAD ( fnma_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::unspecified_<A0> >
                          , bd::scalar_< bd::unspecified_<A0> >
                          , bd::scalar_< bd::unspecified_<A0> >
                                    )
  {
    BOOST_FORCEINLINE
    A0 operator() ( A0 a0, A0 a1, A0 a2) const BOOST_NOEXCEPT
    {
     return -multiplies(a0, a1)-a2;
    }
  };

   BOOST_DISPATCH_OVERLOAD ( fnma_
                           , (typename A0)
                           , bd::cpu_
                           , bs::pedantic_tag
                           , bd::scalar_< bd::unspecified_<A0> >
                           , bd::scalar_< bd::unspecified_<A0> >
                           , bd::scalar_< bd::unspecified_<A0> >
                           )
  {
    BOOST_FORCEINLINE
    A0 operator() ( A0 a0, A0 a1, A0 a2) const BOOST_NOEXCEPT
    {
      return -pedantic_(fma)(a0, a1, a2);
    }
  };

} } }


#endif
