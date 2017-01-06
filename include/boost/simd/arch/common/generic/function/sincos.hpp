//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_GENERIC_FUNCTION_SINCOS_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_GENERIC_FUNCTION_SINCOS_HPP_INCLUDED


#include <boost/simd/arch/common/detail/generic/trigo.hpp>
#include <boost/simd/function/restricted.hpp>
#include <boost/simd/meta/is_not_scalar.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/config.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  namespace bs = boost::simd;

  BOOST_DISPATCH_OVERLOAD ( sincos_
                          , (typename A0)
                          , bd::cpu_
                          , bd::generic_< bd::floating_<A0> >
                          )
  {
    BOOST_FORCEINLINE std::pair<A0, A0> operator() ( A0 const& a0) const
    {
      return detail::trig_base <A0,tag::radian_tag,is_not_scalar_t<A0>,tag::big_tag>::sincosa(a0);
    }
  };

  BOOST_DISPATCH_OVERLOAD ( sincos_
                          , (typename A0)
                          , bd::cpu_
                          , bs::restricted_tag
                          , bd::generic_< bd::floating_<A0> >
                          )
  {
    BOOST_FORCEINLINE std::pair<A0, A0> operator() (const restricted_tag &,  A0 const& a0) const BOOST_NOEXCEPT
    {
      return detail::trig_base<A0, tag::radian_tag,is_not_scalar_t<A0>,tag::clipped_pio4_tag>::sincosa(a0);
    }
  };
} } }


#endif
