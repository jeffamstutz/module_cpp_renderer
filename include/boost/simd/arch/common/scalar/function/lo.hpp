//==================================================================================================
/*!
  @file

  @copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
*/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_LO_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SCALAR_FUNCTION_LO_HPP_INCLUDED

#include <boost/simd/function/bitwise_and.hpp>
#include <boost/simd/constant/ratio.hpp>
#include <boost/simd/detail/dispatch/function/overload.hpp>
#include <boost/simd/detail/dispatch/meta/as_integer.hpp>
#include <boost/config.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  BOOST_DISPATCH_OVERLOAD ( lo_
                          , (typename A0)
                          , bd::cpu_
                          , bd::scalar_< bd::arithmetic_<A0> >
                          )
  {
    using result = bd::as_integer_t<A0,unsigned>;

    BOOST_FORCEINLINE result operator() ( A0 const& a0) const BOOST_NOEXCEPT
    {
      result pattern((result(1) << sizeof(result)*(CHAR_BIT/2)) - 1);
      return bitwise_and(pattern, a0);
    }
  };
} } }


#endif
