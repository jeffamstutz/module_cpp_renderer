//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_LOGICAL_ORNOT_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_LOGICAL_ORNOT_HPP_INCLUDED

#include <boost/simd/detail/overload.hpp>
#include <boost/simd/detail/traits.hpp>
#include <boost/simd/function/mask2logical.hpp>
#include <boost/simd/function/bitwise_ornot.hpp>
#include <boost/simd/function/genmask.hpp>
#include <boost/simd/meta/hierarchy/simd.hpp>
#include <boost/simd/meta/as_logical.hpp>

namespace boost { namespace simd { namespace ext
{
   namespace bd = boost::dispatch;
   namespace bs = boost::simd;
   BOOST_DISPATCH_OVERLOAD_IF(logical_ornot_
                             , (typename A0, typename A1, typename X)
                             , (detail::same_size<A0,A1>)
                             , bd::cpu_
                             , bs::pack_<bd::fundamental_<A0>, X>
                             , bs::pack_<bd::fundamental_<A1>, X>
                             )
   {
     BOOST_FORCEINLINE bs::as_logical_t<A0>  operator()( const A0& a0
                                                       , const  A1&  a1) const BOOST_NOEXCEPT
     {
       return mask2logical(bitwise_ornot(genmask(a0), genmask(a1)));
     }
   };
} } }

#endif
