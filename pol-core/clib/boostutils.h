// Debug Flyweight container
// hash factory with public estimateSize method
// each factory registers in boost_utils::debug_flyweight_queries


#ifndef BOOSTUTILS_H
#define BOOSTUTILS_H

#include "pol_global_config.h"

#ifdef _MSC_VER
#pragma warning( disable : 4503 )  // decorated name length exceeded
#endif

#ifdef ENABLE_FLYWEIGHT_REPORT
#include <boost/config.hpp> /* keep it first to prevent nasty warns in MSVC */
#include <boost/flyweight/factory_tag.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>
#include <boost/mpl/if.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index_container.hpp>

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
#include <utility>
#endif
#include <string>
#include <vector>
#endif
#include <boost/flyweight.hpp>

namespace Pol
{
namespace boost_utils
{
#ifdef ENABLE_FLYWEIGHT_REPORT
template <typename Entry, typename Key, typename Hash = boost::mpl::na,
          typename Pred = boost::mpl::na, typename Allocator = boost::mpl::na>
class accessible_hashed_factory_class;

template <typename Hash = boost::mpl::na, typename Pred = boost::mpl::na,
          typename Allocator = boost::mpl::na BOOST_FLYWEIGHT_NOT_A_PLACEHOLDER_EXPRESSION>
struct accessible_hashed_factory;

class bucket_query
{
public:
  bucket_query();  // register instance in Query
  typedef std::size_t size_type;
  virtual ~bucket_query() = default;
  virtual size_type bucket_count() const = 0;
  virtual size_type max_bucket_count() const = 0;
  virtual size_type bucket_size( size_type n ) const = 0;
  virtual size_type estimateSize() const = 0;
};

struct Query
{
  static std::vector<std::pair<size_t, size_t>> getCountAndSize();
  static void add( bucket_query* b );
  static std::vector<bucket_query*>* debug_flyweight_queries;
};

template <typename Entry, typename Key, typename Hash, typename Pred, typename Allocator>
class accessible_hashed_factory_class : public boost::flyweights::factory_marker,
                                        public bucket_query
{
  struct index_list
      : boost::mpl::vector1<boost::multi_index::hashed_unique<
            boost::multi_index::identity<Entry>,
            typename boost::mpl::if_<boost::mpl::is_na<Hash>, boost::hash<Key>, Hash>::type,
            typename boost::mpl::if_<boost::mpl::is_na<Pred>, std::equal_to<Key>, Pred>::type>>
  {
  };

  typedef boost::multi_index::multi_index_container<
      Entry, index_list,
      typename boost::mpl::if_<boost::mpl::is_na<Allocator>, std::allocator<Entry>,
                               Allocator>::type>
      container_type;

public:
  accessible_hashed_factory_class() : boost::flyweights::factory_marker(), bucket_query() {}
  typedef const Entry* handle_type;

  handle_type insert( const Entry& x ) { return &*cont.insert( x ).first; }

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )
  handle_type insert( Entry&& x ) { return &*cont.insert( std::move( x ) ).first; }
#endif

  void erase( handle_type h ) { cont.erase( cont.iterator_to( *h ) ); }

  static const Entry& entry( handle_type h ) { return *h; }

private:
  container_type cont;

public:
  typedef accessible_hashed_factory_class type;
  BOOST_MPL_AUX_LAMBDA_SUPPORT( 5, accessible_hashed_factory_class,
                                ( Entry, Key, Hash, Pred, Allocator ) )

  virtual size_type bucket_count() const override { return cont.bucket_count(); }
  virtual size_type max_bucket_count() const override { return cont.max_bucket_count(); }
  virtual size_type bucket_size( size_type n ) const override { return cont.bucket_size( n ); }
  virtual size_type estimateSize() const override
  {
    size_type size = 0;
    for ( const Entry& e : cont )
    {
      size += (int)( 4.5 * sizeof( long ) );  // ~overhead
      size += ( (std::string)e ).capacity();
    }
    return size;
  }
};
/* hashed_factory_class specifier */

template <typename Hash, typename Pred,
          typename Allocator BOOST_FLYWEIGHT_NOT_A_PLACEHOLDER_EXPRESSION_DEF>
struct accessible_hashed_factory : boost::flyweights::factory_marker
{
  template <typename Entry, typename Key>
  struct apply
      : boost::mpl::apply2<
            accessible_hashed_factory_class<boost::mpl::_1, boost::mpl::_2, Hash, Pred, Allocator>,
            Entry, Key>
  {
  };
};

#define FLYWEIGHT_HASH_FACTORY boost_utils::accessible_hashed_factory<>
#else
#define FLYWEIGHT_HASH_FACTORY boost::flyweights::hashed_factory<>
#endif

// definition of the different Flyweight factories
// WARNING: do not forget to add new types to struct flyweight_initializers
struct cprop_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<cprop_name_tag>,
                         FLYWEIGHT_HASH_FACTORY>
    cprop_name_flystring;
struct cprop_value_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<cprop_value_tag>,
                         FLYWEIGHT_HASH_FACTORY>
    cprop_value_flystring;
struct cfg_key_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<cfg_key_tag>, FLYWEIGHT_HASH_FACTORY>
    cfg_key_flystring;
struct object_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<object_name_tag>,
                         FLYWEIGHT_HASH_FACTORY>
    object_name_flystring;
struct script_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<script_name_tag>,
                         FLYWEIGHT_HASH_FACTORY>
    script_name_flystring;
struct npctemplate_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<npctemplate_name_tag>,
                         FLYWEIGHT_HASH_FACTORY>
    npctemplate_name_flystring;
struct function_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<function_name_tag>,
                         FLYWEIGHT_HASH_FACTORY>
    function_name_flystring;

/**
 * These types must be initialized before any static objects using them
 * are initialized. Prevents a crash on exit after last destructor is called
 * because boost::flyweights initialization is not thread safe.
 */
struct flyweight_initializers
{
  cprop_name_flystring::initializer fwInit_cprop_name;
  cprop_value_flystring::initializer fwInit_cprop_value;
  cfg_key_flystring::initializer fwInit_cfg_key;
  object_name_flystring::initializer fwInit_obj_name;
  script_name_flystring::initializer fwInit_script_name;
  npctemplate_name_flystring::initializer fwInit_npctemplate_name;
  function_name_flystring::initializer fwInit_func_name;

  ~flyweight_initializers();
};
inline auto format_as( const cprop_name_flystring& s )
{
  return s.get();
}
inline auto format_as( const cprop_value_flystring& s )
{
  return s.get();
}
inline auto format_as( const cfg_key_flystring& s )
{
  return s.get();
}
inline auto format_as( const object_name_flystring& s )
{
  return s.get();
}
inline auto format_as( const script_name_flystring& s )
{
  return s.get();
}
inline auto format_as( const npctemplate_name_flystring& s )
{
  return s.get();
}
inline auto format_as( const function_name_flystring& s )
{
  return s.get();
}
}  // namespace boost_utils
}  // namespace Pol

#endif
