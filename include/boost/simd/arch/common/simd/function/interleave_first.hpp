//==================================================================================================
/**
  Copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
**/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_INTERLEAVE_FIRST_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_SIMD_FUNCTION_INTERLEAVE_FIRST_HPP_INCLUDED

#include <boost/simd/detail/overload.hpp>
#include <boost/simd/function/extract.hpp>
#include <boost/simd/function/combine.hpp>
#include <boost/simd/function/slide.hpp>
#include <boost/simd/function/interleave_second.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bd = boost::dispatch;
  namespace bs = boost::simd;
  namespace br = brigand;

  BOOST_DISPATCH_OVERLOAD ( interleave_first_
                          , (typename T, typename X)
                          , bd::cpu_
                          , bs::pack_< bd::unspecified_<T>, X >
                          , bs::pack_< bd::unspecified_<T>, X >
                          )
  {
    template<typename N, typename V>
    static BOOST_FORCEINLINE
    typename V::value_type value(V const& x, V const&, std::true_type const&)
    {
      return bs::extract<N::value/2>(x);
    }

    template<typename N, typename V>
    static BOOST_FORCEINLINE
    typename V::value_type value(V const&, V const& y, std::false_type const&)
    {
      return bs::extract<N::value/2>(y);
    }

    template<typename K, typename... N> static BOOST_FORCEINLINE
    T do_( T const& x, T const& y, K const&, br::list<N...> const&) BOOST_NOEXCEPT
    {
      return T( value<N>(x,y, brigand::bool_<N::value%2==0>{})... );
    }

    template<typename K, typename N> static BOOST_FORCEINLINE
    T do_( T const& x, T const&, K const&, br::list<N> const&) BOOST_NOEXCEPT
    {
      return x;
    }

    template<typename... N> static BOOST_FORCEINLINE
    T do_( T const& x, T const& y, aggregate_storage const&, br::list<N...> const&) BOOST_NOEXCEPT
    {
      auto const& x0 = x.storage()[0];
      auto const& y0 = y.storage()[0];

      return  combine ( interleave_first(x0, y0)
                      , interleave_second(x0, y0)
                      );
    }

    BOOST_FORCEINLINE T operator()(T const& x, T const& y) const BOOST_NOEXCEPT
    {
      return do_(x,y, typename T::traits::storage_kind{}
                    , br::range<std::size_t, 0, T::static_size>{}
                );
    }
  };
} } }

#endif
