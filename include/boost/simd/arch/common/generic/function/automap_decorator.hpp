//==================================================================================================
/**
  Copyright 2016 NumScale SAS

  Distributed under the Boost Software License, Version 1.0.
  (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
**/
//==================================================================================================
#ifndef BOOST_SIMD_ARCH_COMMON_GENERIC_FUNCTION_AUTOMAP_DECORATOR_HPP_INCLUDED
#define BOOST_SIMD_ARCH_COMMON_GENERIC_FUNCTION_AUTOMAP_DECORATOR_HPP_INCLUDED

#include <boost/simd/detail/overload.hpp>
#include <boost/simd/detail/decorator.hpp>
#include <boost/simd/detail/dispatch/hierarchy/functions.hpp>
#include <boost/simd/arch/common/tags.hpp>
#include <boost/simd/detail/diagnostic.hpp>

namespace boost { namespace simd { namespace ext
{
  namespace bs = boost::simd;
  namespace bd = boost::dispatch;
  namespace br = brigand;
  // -----------------------------------------------------------------------------------------------
  // automap for functions with decorators
  BOOST_DISPATCH_OVERLOAD_FALLBACK ( ( typename F
                                     , typename Decorator, typename... Pn, typename... En
                                     )
                                   , bd::elementwise_<F>
                                   , bd::cpu_
                                   , bs::decorator_<Decorator>
                                   , bs::pack_<bd::unspecified_<Pn>, En>...
                                   )
  {
    // The functor
    using functor             = decltype(detail::decorator<Decorator>()(bd::functor<F>()));

    // Use the very first pack as reference
    using pack                = br::front<br::list<Pn...>>;
    using traits              = typename pack::traits;

    // We build the return pack as a pack of same cardinal but which type is the scalar return
    // type of functor applied to the input parameter value type.
    using return_type         = decltype(functor()(std::declval<typename Pn::value_type>()...));
    using result_type         = typename pack::template rebind<return_type>;
    using result_storage_type = typename result_type::storage_type;
    using result_traits       = typename result_type::traits;

    // We need to iterate over output type cardinal to fill it
    using element_range       = br::range<std::size_t, 0, pack::static_size>;

    // Constructing the output depends on the storage_kind of all the inputs
    struct storage_checker
    {
      template <typename S, typename E>
      struct apply : br::bool_<  S::value
                            && std::is_same<typename E::storage_kind, aggregate_storage>::value
                            >
      {};
    };

    using storage_kind  = br::fold< br::list<typename Pn::traits...>
                                  , br::bool_<true>
                                  , storage_checker
                                  >;

    // Constructing the output also depends on the storage_kind the output
    using result_storage_kind = typename result_traits::storage_kind;

    // CHecks some basic assertions
    struct traits_checker
    {
      template <typename S, typename E>
      struct apply : br::bool_<  S::value
                            && std::size_t(E::static_size) == std::size_t(traits::static_size)
                            >
      {};
    };

    using traits_info = br::fold< br::list<typename Pn::traits...>
                                , br::bool_<true>
                                , traits_checker
                                >;

    enum {
      same_static_size = traits_info::value,
      is_noexcept = BOOST_NOEXCEPT_EXPR(std::declval<functor>()(std::declval<Pn>()[0]...))
    };

    static_assert( same_static_size
                 , "automap fallback: all packs must have the same size"
                 );

    // ---------------------------------------------------------------------------------------------
    // (P)
    template <typename P0, typename SKI, typename SKO, typename... N>
    BOOST_FORCEINLINE static result_type map_ ( SKI const&, SKO const&
                                              , br::list<N...> const&
                                              , P0 const& p0
                                              )
    BOOST_NOEXCEPT_IF(is_noexcept)
    {
      functor f;
      return result_type{ f(p0[N::value])... };
    }

    // ---------------------------------------------------------------------------------------------
    // (P, P)
    template <typename P0, typename P1, typename SKI, typename SKO, typename... N>
    BOOST_FORCEINLINE static result_type map_ ( SKI const&, SKO const&
                                              , br::list<N...> const&
                                              , P0 const& p0, P1 const& p1
                                              )
    BOOST_NOEXCEPT_IF(is_noexcept)
    {
      functor f;
      return result_type{ f(p0[N::value], p1[N::value])...  };
    }

    // ---------------------------------------------------------------------------------------------
    // (P, P, P)
    template< typename P0, typename P1, typename P2
            , typename SKI, typename SKO, typename... N
            >
    BOOST_FORCEINLINE static result_type map_ ( SKI const&, SKO const&
                                              , br::list<N...> const&
                                              , P0 const& p0, P1 const& p1, P2 const& p2
                                              )
    BOOST_NOEXCEPT_IF(is_noexcept)
    {
      functor f;
      return result_type{ f(p0[N::value], p1[N::value], p2[N::value])... };
    }

    // ---------------------------------------------------------------------------------------------
    // (P, P, P, P)
    template< typename P0, typename P1, typename P2, typename P3
            , typename SKI, typename SKO, typename... N
            >
    BOOST_FORCEINLINE static result_type map_ ( SKI const&, SKO const&
                                              , br::list<N...> const&
                                              , P0 const& p0, P1 const& p1
                                              , P2 const& p2, P3 const& p3
                                              )
    BOOST_NOEXCEPT_IF(is_noexcept)
    {
      functor f;
      return result_type{ f(p0[N::value], p1[N::value], p2[N::value], p3[N::value])... };
    }

    // ---------------------------------------------------------------------------------------------
    // Map dispatcher when both input and output are aggregates
    BOOST_FORCEINLINE static result_type map_ ( Pn const&... p ) BOOST_NOEXCEPT_IF(is_noexcept)
    {
      functor f;
      return result_storage_type{{ f( p.storage()[0]...), f( p.storage()[1]...) }};
    }

    template <typename... N>
    BOOST_FORCEINLINE static result_type map_ ( brigand::bool_<true> const&
                                              , ::boost::simd::aggregate_storage const&
                                              , br::list<N...> const&
                                              , Pn const&... pn
                                              )
    BOOST_NOEXCEPT_IF(is_noexcept)
    {
      return map_( pn... );
    }

    // ---------------------------------------------------------------------------------------------
    // Map operator()
    BOOST_FORCEINLINE result_type operator()(Decorator const&, Pn const&... pn) const
    BOOST_NOEXCEPT_IF(is_noexcept)
    {
      BOOST_SIMD_DIAG("automap for: " << *this << " decorated by " << Decorator{} );
      return map_( storage_kind{}, result_storage_kind{}, element_range{}, pn... );
    }
  };
} } }

#endif
