/** @file
 *
 * @par History
 * - 2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )
 * - 2016/01/31 Bodom:     added profiling support
 */


#ifndef PROPLIST_H
#define PROPLIST_H

#include <array>
#include <boost/flyweight.hpp>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "../clib/boostutils.h"
#include "../clib/rawtypes.h"
#include "../clib/spinlock.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Core
{
class PropertyList;

enum class UOBJ_CLASS : u8;

/**
 * Profiler for CProps: stores usage information and computes statistics
 *
 * @author Bodom
 */
class CPropProfiler
{
public:
  /**
   * Type of CProp, for profiling
   */
  enum class Type : u8
  {
    // main types
    ACCOUNT,
    GUILD,
    GLOBAL,
    ITEM,
    MOBILE,
    MULTI,
    PARTY,

    // ignored types
    DATAFILEELEMENT,
    REGION,

    /// unknown type, do not use (internally used when profiler is enabled after startup)
    UNKNOWN,
  };

  static Type class_to_type( UOBJ_CLASS oclass );

  /** Returns an instance of the profiler, instantiate it on first need */
  static CPropProfiler& instance();

  /** No copies allowed */
  CPropProfiler( const CPropProfiler& ) = delete;
  /** No copies allowed */
  CPropProfiler& operator=( const CPropProfiler& ) = delete;

  void registerProplist( const PropertyList* proplist, const Type type );
  void registerProplist( const PropertyList* proplist, const PropertyList* copiedFrom );
  void unregisterProplist( const PropertyList* proplist );

  void clear();
  void dumpProfile( std::ostream& os ) const;
  size_t estimateSize() const;

private:
  class HitsCounter
  {
  public:
    static const size_t READ = 0;
    static const size_t WRITE = 1;
    static const size_t ERASE = 2;

    HitsCounter();
    u64& operator[]( size_t idx );
    const u64& operator[]( size_t idx ) const;

  private:
    /// 0=read, 1=write, 2=erase
    std::array<u64, 3> hits;
  };
  typedef std::map<const PropertyList*, const Type> PropLists;
  typedef std::map<const std::string, HitsCounter> HitsEntries;
  typedef std::map<const Type, HitsEntries> Hits;

  CPropProfiler();

  Type getProplistType( const PropertyList* proplist ) const;
  bool isIgnored( Type type ) const;
  void cpropAction( const PropertyList* proplist, const std::string& name, const size_t key );

  std::unique_ptr<PropLists> _proplists;
  std::unique_ptr<Hits> _hits;
  mutable Clib::SpinLock _proplistsLock;
  mutable Clib::SpinLock _hitsLock;

public:
  void cpropRead( const PropertyList* proplist, const std::string& name );
  void cpropWrite( const PropertyList* proplist, const std::string& name );
  void cpropErase( const PropertyList* proplist, const std::string& name );
};


/**
 * Holds an object's CProps
 */
class PropertyList
{
public:
  PropertyList() = delete;
  PropertyList( CPropProfiler::Type type );
  PropertyList( CPropProfiler::Type type, bool force );
  PropertyList( const PropertyList& );  // dave added 1/26/3
  bool getprop( const std::string& propname, std::string& propvalue ) const;
  void setprop( const std::string& propname, const std::string& propvalue );
  void eraseprop( const std::string& propname );
  void copyprops( const PropertyList& proplist );
  void getpropnames( std::vector<std::string>& propnames ) const;
  void clear();
  size_t estimatedSize() const;

  void printProperties( Clib::StreamWriter& sw ) const;
  void printProperties( Clib::ConfigElem& elem ) const;
  void readProperties( Clib::ConfigElem& elem );

  void printPropertiesAsStrings( Clib::StreamWriter& sw ) const;
  void readRemainingPropertiesAsStrings( Clib::ConfigElem& elem );

  bool operator==( const PropertyList& ) const;
  PropertyList& operator-( const std::set<std::string>& );  // dave added 1/26/3
  void operator-=( const std::set<std::string>& );          // dave added 1/26/3
protected:
  typedef std::map<boost_utils::cprop_name_flystring, boost_utils::cprop_value_flystring>
      Properties;

  Properties properties;

private:
  // not implemented
  PropertyList& operator=( const PropertyList& ) = delete;
};

Bscript::BObjectImp* CallPropertyListMethod( PropertyList& proplist, const char* methodname,
                                             Bscript::Executor& ex, bool& changed );
Bscript::BObjectImp* CallPropertyListMethod_id( PropertyList& proplist, const int id,
                                                Bscript::Executor& ex, bool& changed );
}  // namespace Core
}  // namespace Pol
#endif
