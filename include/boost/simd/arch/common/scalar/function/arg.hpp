//==================================================================================================
/*!
  @file

  @copyright 2015 NumScale SAS
  @copyright 2015 J.T. Lapreste

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_ARG_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_ARG_HPP_INCLUDED

#ifndef BOOST_SIMD_NO_NANS
#include <boost/simd/function/if_allbits_else.hpp>
#include <boost/simd/function/is_nan.hpp>
#endif
#include <boost/simd/constant/pi.hpp>
#include <boost/simd/function/if_else_zero.hpp>
#include <boost/simd/function/pedantic.hpp>
#include <boost/simd/function/is_ltz.hpp>
#include <boost/simd/function/is_negative.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/config.hpp>

namespace boost { namespace simd
 {
   namespace ext
   {
     namespace bd = boost::dispatch;
     namespace bs = boost::simd;
     BOOST_DISPATCH_OVERLOAD ( arg_
                             , (typename A0)
                             , bd::cpu_
                             , bs::pedantic_tag
                             , bd::scalar_< bd::floating_<A0> >
                             )
     {
       BOOST_FORCEINLINE A0 operator() (const pedantic_tag &,
                                        A0 a0) const BOOST_NOEXCEPT
       {
         A0 r = if_else_zero(is_negative(a0),Pi<A0>());
       #ifndef BOOST_SIMD_NO_NANS
         return if_allbits_else(is_nan(a0),r);
       #else
         return r;
       #endif
       }
     };

     BOOST_DISPATCH_OVERLOAD ( arg_
                             , (typename A0)
                             , bd::cpu_
                             , bd::scalar_< bd::floating_<A0> >
                             )
     {
       BOOST_FORCEINLINE A0 operator() ( A0 a0 ) const BOOST_NOEXCEPT
       {
         return if_else_zero(is_negative(a0),Pi<A0>());
       }
     };

} } }


#endif
