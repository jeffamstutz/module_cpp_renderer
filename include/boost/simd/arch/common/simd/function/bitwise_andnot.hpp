//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_BITWISE_ANDNOT_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_BITWISE_ANDNOT_HPP_INCLUDED
#include <boost/simd/detail/overload.hpp>

#include <boost/simd/meta/hierarchy/simd.hpp>
#include <boost/simd/function/bitwise_and.hpp>
#include <boost/simd/function/bitwise_cast.hpp>
#include <boost/simd/function/complement.hpp>
#include <boost/simd/detail/brigand.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <type_traits>

namespace boost { namespace simd { namespace ext
{
   namespace bd = boost::dispatch;
   namespace bs = boost::simd;
  BOOST_DISPATCH_OVERLOAD_IF(bitwise_andnot_
                            , (typename A0,typename A1, typename X, typename Y)
                            , (brigand::not_<std::is_same<A0,A1>>)
                            , bs::simd_
                            , bs::pack_<bd::arithmetic_<A0>,X>
                            , bs::pack_<bd::arithmetic_<A1>,Y>
                            )
  {
     BOOST_FORCEINLINE A0 operator()( const A0& a0, const A1& a1) const BOOST_NOEXCEPT
     {
       return bitwise_and(a0, complement(a1));
     }
   };
} } }

#endif

