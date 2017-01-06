//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_EXPONENT_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_EXPONENT_HPP_INCLUDED

#include <boost/simd/detail/constant/maxexponent.hpp>
#include <boost/simd/constant/nbmantissabits.hpp>
#include <boost/simd/constant/zero.hpp>
#include <boost/simd/function/exponentbits.hpp>
#include <boost/simd/function/if_else_zero.hpp>
#include <boost/simd/function/is_eqz.hpp>
#include <boost/simd/function/is_invalid.hpp>
#include <boost/simd/function/shr.hpp>
#include <boost/simd/detail/math.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/simd/detail/dispatch/meta/as_integer.hpp>
#include <boost/config.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  BOOST_DISPATCH_OVERLOAD ( exponent_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::integer_<A0> >
                          )
  {
    using result_t = bd::as_integer_t<A0, signed>;
    BOOST_FORCEINLINE result_t operator() ( A0) const BOOST_NOEXCEPT
    {
      return Zero<result_t>();
    }
  };

#ifdef BOOST_SIMD_HAS_ILOGB
  BOOST_DISPATCH_OVERLOAD ( exponent_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::double_<A0> >
                          )
  {
    using result_t = bd::as_integer_t<A0, signed>;
    BOOST_FORCEINLINE result_t operator() ( A0 a0) const BOOST_NOEXCEPT
    {
      if (is_invalid(a0) || is_eqz(a0)) return Zero<result_t>();
      return ::ilogb(a0);
    }
  };
#endif

#ifdef BOOST_SIMD_HAS_ILOGBF
  BOOST_DISPATCH_OVERLOAD ( exponent_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::single_<A0> >
                          )
  {
    using result_t = bd::as_integer_t<A0, signed>;
    BOOST_FORCEINLINE result_t operator() ( A0 a0) const BOOST_NOEXCEPT
    {
      if (is_invalid(a0) || is_eqz(a0)) return Zero<result_t>();
      return ::ilogbf(a0);
    }
  };
#endif

  BOOST_DISPATCH_OVERLOAD ( exponent_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::floating_<A0> >
                          )
  {
    using result_t = bd::as_integer_t<A0, signed>;
    BOOST_FORCEINLINE result_t operator() ( A0 a0) const BOOST_NOEXCEPT
    {
      if (is_invalid(a0) || is_eqz(a0)) return Zero<result_t>();
      const int nmb = int(Nbmantissabits<A0>());
      const result_t x = shr(exponentbits(a0), nmb);
      return x-if_else_zero(a0, Maxexponent<A0>());
    }
  };
} } }


#endif
