// Debug Flyweight container
// hash factory with public estimateSize method
// each factory registers in boost_utils::debug_flyweight_queries


#ifndef BOOSTUTILS_H
#define BOOSTUTILS_H

#ifdef WINDOWS
#include "pol_global_config_win.h"
#else
#include "pol_global_config.h"
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4503 )  // decorated name length exceeded
#endif

#ifdef DEBUG_FLYWEIGHT
#include <vector>
#include <string>

#include <boost/flyweight/factory_tag.hpp>
#include <boost/flyweight/hashed_factory_fwd.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>
#include <boost/mpl/if.hpp>
#include <boost/foreach.hpp>

#include "compilerspecifics.h"
#endif
#include <boost/flyweight.hpp>

namespace Pol
{
namespace boost_utils
{
#ifdef DEBUG_FLYWEIGHT
class bucket_query
{
public:
  typedef std::size_t size_type;
  virtual ~bucket_query(){};
  virtual size_type bucket_count() const = 0;
  virtual size_type max_bucket_count() const = 0;
  virtual size_type bucket_size( size_type n ) const = 0;
  virtual size_type estimateSize() const = 0;
};

extern std::vector<bucket_query*> debug_flyweight_queries;

template <typename Entry, typename Key, typename Hash = boost::mpl::na,
          typename Pred = boost::mpl::na, typename Allocator = boost::mpl::na>
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
      Entry, index_list, typename boost::mpl::if_<boost::mpl::is_na<Allocator>,
                                                  std::allocator<Entry>, Allocator>::type>
      container_type;

public:
  typedef const Entry* handle_type;

  accessible_hashed_factory_class() { debug_flyweight_queries.push_back( this ); }
  handle_type insert( const Entry& x ) { return &*cont.insert( x ).first; }
  void erase( handle_type h ) { cont.erase( cont.iterator_to( *h ) ); }
  static const Entry& entry( handle_type h ) { return *h; }
  typedef std::size_t size_type;

  virtual size_type bucket_count() const POL_OVERRIDE { return cont.bucket_count(); }
  virtual size_type max_bucket_count() const POL_OVERRIDE { return cont.max_bucket_count(); }
  virtual size_type bucket_size( size_type n ) const POL_OVERRIDE { return cont.bucket_size( n ); }
  virtual size_type estimateSize() const POL_OVERRIDE
  {
    size_type size = 0;
    BOOST_FOREACH ( const Entry& e, cont )
    {
      size += (int)( 4.5 * sizeof( long ) );  // ~overhead
      size += ( (std::string)e ).capacity();
    }
    return size;
  }

private:
  container_type cont;

public:
  typedef accessible_hashed_factory_class type;
  BOOST_MPL_AUX_LAMBDA_SUPPORT( 5, accessible_hashed_factory_class,
                                ( Entry, Key, Hash, Pred, Allocator ) )
};

template <typename Hash = boost::mpl::na, typename Pred = boost::mpl::na,
          typename Allocator = boost::mpl::na BOOST_FLYWEIGHT_NOT_A_PLACEHOLDER_EXPRESSION>
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
                         FLYWEIGHT_HASH_FACTORY> cprop_name_flystring;
struct cprop_value_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<cprop_value_tag>,
                         FLYWEIGHT_HASH_FACTORY> cprop_value_flystring;
struct cfg_key_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<cfg_key_tag>, FLYWEIGHT_HASH_FACTORY>
    cfg_key_flystring;
struct object_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<object_name_tag>,
                         FLYWEIGHT_HASH_FACTORY> object_name_flystring;
struct script_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<script_name_tag>,
                         FLYWEIGHT_HASH_FACTORY> script_name_flystring;
struct npctemplate_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<npctemplate_name_tag>,
                         FLYWEIGHT_HASH_FACTORY> npctemplate_name_flystring;
struct function_name_tag
{
};
typedef boost::flyweight<std::string, boost::flyweights::tag<function_name_tag>,
                         FLYWEIGHT_HASH_FACTORY> function_name_flystring;

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
};
}
}
#endif
