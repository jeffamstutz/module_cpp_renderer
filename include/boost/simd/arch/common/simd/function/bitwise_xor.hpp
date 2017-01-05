//==================================================================================================
/**
  Copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
**/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_BITWISE_XOR_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_BITWISE_XOR_HPP_INCLUDED

#include <boost/simd/function/bitwise_cast.hpp>
#include <boost/simd/detail/overload.hpp>
#include <boost/simd/detail/traits.hpp>
#include <boost/simd/detail/brigand.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bs = boost::simd;
  namespace bd = boost::dispatch;

  BOOST_DISPATCH_OVERLOAD_IF( bitwise_xor_
                            , (typename A0,typename A1,typename X, typename Y)
                            , (brigand::and_< brigand::not_ < std::is_same<A0,A1> >
                                            , brigand::and_ < detail::is_native<X>
                                                            , detail::is_native<Y>
                                                            >
                                            >
                              )
                            , bs::simd_
                            , bs::pack_<bd::arithmetic_<A0>,X>
                            , bs::pack_<bd::arithmetic_<A1>,Y>
                            )
  {
    BOOST_FORCEINLINE A0 operator()(const A0& a0, const A1& a1) const BOOST_NOEXCEPT
    {
      return bitwise_xor(a0, bitwise_cast<A0>(a1));
    }
  };
} } }

#endif
