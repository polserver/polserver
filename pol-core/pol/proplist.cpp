/** @file
 *
 * @par History
 * - 2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )
 */


#include "proplist.h"

#include <stddef.h>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmethods.h"
#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../clib/streamsaver.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "baseobject.h"

#define pf_endl '\n'
namespace Pol
{
namespace Core
{
CPropProfiler::HitsCounter::HitsCounter() : hits( std::array<u64, 3>{ { 0, 0, 0 } } ) {}

u64& CPropProfiler::HitsCounter::operator[]( size_t idx )
{
  return hits[idx];
}

const u64& CPropProfiler::HitsCounter::operator[]( size_t idx ) const
{
  return hits[idx];
}

/** Given an UOBJ_CLASS, returns the corresponding Type for profiling */
CPropProfiler::Type CPropProfiler::class_to_type( UOBJ_CLASS oclass )
{
  switch ( oclass )
  {
  case UOBJ_CLASS::CLASS_ITEM:
  case UOBJ_CLASS::CLASS_ARMOR:
  case UOBJ_CLASS::CLASS_CONTAINER:
  case UOBJ_CLASS::CLASS_WEAPON:
    return CPropProfiler::Type::ITEM;
  case UOBJ_CLASS::CLASS_CHARACTER:
  case UOBJ_CLASS::CLASS_NPC:
    return CPropProfiler::Type::MOBILE;
  case UOBJ_CLASS::CLASS_MULTI:
    return CPropProfiler::Type::MULTI;
  case UOBJ_CLASS::INVALID:
    return CPropProfiler::Type::UNKNOWN;
  }

  /// Must compute all cases, relying on GCC's -wSwitch option to check it
  /// but placing a safe fallback anyway.
  return CPropProfiler::Type::UNKNOWN;
}

CPropProfiler& CPropProfiler::instance()
{
  static CPropProfiler instance;
  return instance;
}

CPropProfiler::CPropProfiler() : _proplists( new PropLists() ), _hits( new Hits() ) {}

/**
 * Returns proplist type, internal usage
 */
CPropProfiler::Type CPropProfiler::getProplistType( const PropertyList* proplist ) const
{
  PropLists::iterator el;
  {
    Clib::SpinLockGuard lock( _proplistsLock );
    el = _proplists->find( proplist );
  }

  if ( el == _proplists->end() )
  {
    /// Unknown should happen only when the profiler has been disabled and
    /// then re-enabled (including when it was disabled at startup and enabled
    /// later). In any other case, it's a bug.
    return Type::UNKNOWN;
  }

  return el->second;
}

/**
 * Returns wether a given type should be ignored, intenal usage
 */
bool CPropProfiler::isIgnored( Type type ) const
{
  if ( type == Type::DATAFILEELEMENT || type == Type::REGION )
    return true;
  return false;
}

/**
 * Register a cprop read
 *
 * @param proplist Pointer to the registered list where this cprop resides
 * @param name Name of the cprop
 */
void CPropProfiler::cpropRead( const PropertyList* proplist, const std::string& name )
{
  cpropAction( proplist, name, HitsCounter::READ );
}
/**
 * Register a cprop write
 *
 * @param proplist Pointer to the registered list where this cprop resides
 * @param name Name of the cprop
 */
void CPropProfiler::cpropWrite( const PropertyList* proplist, const std::string& name )
{
  cpropAction( proplist, name, HitsCounter::WRITE );
}
/**
 * Register a cprop erase
 *
 * @param proplist Pointer to the registered list where this cprop resides
 * @param name Name of the cprop
 * @throws std::runtime_error When proplist is not registered
 */
void CPropProfiler::cpropErase( const PropertyList* proplist, const std::string& name )
{
  cpropAction( proplist, name, HitsCounter::ERASE );
}

/**
 * Registers a property list address
 *
 * @param proplist Pointer to the list to be registered
 * @param type The type of the list to be registered
 */
void CPropProfiler::registerProplist( const PropertyList* proplist, const CPropProfiler::Type type )
{
  Clib::SpinLockGuard lock( _proplistsLock );
  _proplists->insert( std::make_pair( proplist, type ) );
}

/**
 * Registers a property list with same type of an already registered one
 *
 * @param proplist Pointer to the list to be registered
 * @param copiedFrom Pointer to the already registered list to copy the type from
 */
void CPropProfiler::registerProplist( const PropertyList* proplist, const PropertyList* copiedFrom )
{
  registerProplist( proplist, getProplistType( copiedFrom ) );
}

/**
 * Unregisters a property list address
 */
void CPropProfiler::unregisterProplist( const PropertyList* proplist )
{
  Clib::SpinLockGuard lock( _proplistsLock );
  _proplists->erase( proplist );
}

/**
 * Register a cprop action
 *
 * @param proplist Pointer to the registered list where this cprop resides
 * @param name Name of the cprop
 * @param key Index of the array key to update
 */
void CPropProfiler::cpropAction( const PropertyList* proplist, const std::string& name,
                                 const size_t key )
{
  Type type = getProplistType( proplist );
  if ( isIgnored( type ) )
    return;

  {
    Clib::SpinLockGuard lock( _hitsLock );
    u64* cur = &( *_hits )[type][name][key];
    if ( *cur < std::numeric_limits<u64>::max() )
      ( *cur )++;
  }
}

/**
 * Discards all the gathered data
 */
void CPropProfiler::clear()
{
  Clib::SpinLockGuard plock( _proplistsLock );
  Clib::SpinLockGuard hlock( _hitsLock );

  _proplists->clear();
  _hits->clear();
}

/**
 * Dumps current status into a given stream
 *
 * @param os The output stream to write into
 */
void CPropProfiler::dumpProfile( std::ostream& os ) const
{
  // First generate the data

  // map<categoryname, map<typename, vector<lines> >>
  std::map<std::string, std::map<std::string, std::vector<std::string>>> outData;

  {
    Clib::SpinLockGuard lock( _hitsLock );

    for ( auto tIter = _hits->begin(); tIter != _hits->end(); ++tIter )
    {
      Type t = tIter->first;

      std::string typeName;
      switch ( t )
      {
      case Type::ACCOUNT:
        typeName = "Account";
        break;
      case Type::GUILD:
        typeName = "Guild";
        break;
      case Type::GLOBAL:
        typeName = "Global";
        break;
      case Type::ITEM:
        typeName = "Item";
        break;
      case Type::MOBILE:
        typeName = "Mobile";
        break;
      case Type::MULTI:
        typeName = "Multi";
        break;
      case Type::PARTY:
        typeName = "Party";
        break;
      case Type::UNKNOWN:
        typeName = "UNKNOWN";
        break;
      default:
        typeName = "ERROR " + std::to_string( static_cast<unsigned int>( t ) );
        break;
      }

      for ( auto pIter = tIter->second.begin(); pIter != tIter->second.end(); ++pIter )
      {
        std::ostringstream line;
        line << pIter->first << " ";
        line << pIter->second[HitsCounter::READ] << "/";
        line << pIter->second[HitsCounter::WRITE] << "/";
        line << pIter->second[HitsCounter::ERASE] << std::endl;

        if ( !pIter->second[HitsCounter::READ] )
          outData["WRITTEN BUT NEVER READ"][typeName].push_back( line.str() );
        else if ( !pIter->second[HitsCounter::WRITE] )
          outData["READ BUT NEVER WRITTEN"][typeName].push_back( line.str() );
        else
          outData["ALL THE REST"][typeName].push_back( line.str() );
      }
    }
  }

  // Then output it
  for ( auto it1 = outData.rbegin(); it1 != outData.rend(); ++it1 )
  {
    // 1st level header
    os << std::string( 15, '-' ) << " " << it1->first << " " << std::string( 15, '-' ) << std::endl;

    for ( auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 )
    {
      // 2nd level header
      os << it2->first << " CProps summary (read/write/erase):" << std::endl;

      std::sort( it2->second.begin(), it2->second.end() );
      for ( auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3 )
      {
        os << "- " << *it3;
      }
    }

    os << std::endl;
  }
}

/**
 * Returns the estimated in-memory size of the profiler
 */
size_t CPropProfiler::estimateSize() const
{
  /// Size of base empty containers
  size_t ret = sizeof( CPropProfiler );
  /// + size of proplists
  {
    Clib::SpinLockGuard lock( _proplistsLock );
    ret += Clib::memsize( *_proplists );
  }

  /// + size of hits
  {
    Clib::SpinLockGuard lock( _hitsLock );
    ret += Clib::memsize( *_hits, []( const auto& v ) { return Clib::memsize( v ); } );
  }

  return ret;
}

/**
 * Initialize and register this property list based on a given type
 * register only if the profile_cprops flag is set
 */
PropertyList::PropertyList( CPropProfiler::Type type ) : properties()
{
  if ( Plib::systemstate.config.profile_cprops )
    CPropProfiler::instance().registerProplist( this, type );
}

/**
 * Initialize and register this property list based on a given type,
 * always register if force flag is is true
 */
PropertyList::PropertyList( CPropProfiler::Type type, bool force ) : properties()
{
  if ( force || Plib::systemstate.config.profile_cprops )
    CPropProfiler::instance().registerProplist( this, type );
}

/**
 * Initialize by copying content and type from a given one
 */
PropertyList::PropertyList( const PropertyList& props ) : properties()
{
  copyprops( props );

  if ( Plib::systemstate.config.profile_cprops )
    CPropProfiler::instance().registerProplist( this, &props );
}

size_t PropertyList::estimatedSize() const
{
  size_t size = sizeof( PropertyList );
  size += Clib::memsize( properties );
  return size;
}

bool PropertyList::getprop( const std::string& propname, std::string& propval ) const
{
  if ( Plib::systemstate.config.profile_cprops )
    CPropProfiler::instance().cpropRead( this, propname );

  Properties::const_iterator itr = properties.find( boost_utils::cprop_name_flystring( propname ) );
  if ( itr == properties.end() )
  {
    return false;
  }
  else
  {
    propval = ( *itr ).second;
    return true;
  }
}
void PropertyList::setprop( const std::string& propname, const std::string& propvalue )
{
  if ( Plib::systemstate.config.profile_cprops )
    CPropProfiler::instance().cpropWrite( this, propname );

  properties[boost_utils::cprop_name_flystring( propname )] = propvalue;
}

void PropertyList::eraseprop( const std::string& propname )
{
  if ( Plib::systemstate.config.profile_cprops )
    CPropProfiler::instance().cpropErase( this, propname );

  properties.erase( boost_utils::cprop_name_flystring( propname ) );
}

void PropertyList::copyprops( const PropertyList& from )
{
  // dave 4/25/3 map insert won't overwrite with new values, so remove those first and then
  // reinsert.
  if ( !properties.empty() )
  {
    for ( const auto& prop : from.properties )
      properties.erase( prop.first );
  }

  properties.insert( from.properties.begin(), from.properties.end() );
}

void PropertyList::clear()
{
  properties.clear();
}

void PropertyList::getpropnames( std::vector<std::string>& propnames ) const
{
  for ( const auto& prop : properties )
  {
    propnames.push_back( prop.first );
  }
}

void PropertyList::printProperties( Clib::StreamWriter& sw ) const
{
  for ( const auto& prop : properties )
  {
    const std::string& first = prop.first;
    if ( first[0] != '#' )
    {
      sw.add( "CProp", fmt::format( "{} {}", first, prop.second.get() ) );
    }
  }
}
void PropertyList::printProperties( Clib::ConfigElem& elem ) const
{
  for ( const auto& prop : properties )
  {
    const std::string& first = prop.first;
    if ( first[0] != '#' )
    {
      elem.add_prop( "CProp", ( first + "\t" + prop.second.get() ) );
    }
  }
}

void PropertyList::printPropertiesAsStrings( Clib::StreamWriter& sw ) const
{
  for ( const auto& prop : properties )
  {
    const std::string& first = prop.first;
    if ( first[0] != '#' )
    {
      sw.add( first, prop.second.get() );
    }
  }
}

void PropertyList::readProperties( Clib::ConfigElem& elem )
{
  std::string tempstr;
  while ( elem.remove_prop( "StrProp", &tempstr ) )
  {
    std::string propname;
    std::string propvalue;

    Clib::splitnamevalue( tempstr, propname, propvalue );

    setprop( propname, "s" + propvalue );
  }
  while ( elem.remove_prop( "CProp", &tempstr ) )
  {
    std::string propname;
    std::string propvalue;

    Clib::splitnamevalue( tempstr, propname, propvalue );

    setprop( propname, propvalue );
  }
}

void PropertyList::readRemainingPropertiesAsStrings( Clib::ConfigElem& elem )
{
  std::string propname, propvalue;
  while ( elem.remove_first_prop( &propname, &propvalue ) )
  {
    setprop( propname, propvalue );
  }
}

bool PropertyList::operator==( const PropertyList& plist ) const
{
  return ( this->properties == plist.properties );
}

PropertyList& PropertyList::operator-(
    const std::set<std::string>& CPropNames )  // dave added 1/26/3
{
  for ( const auto& name : CPropNames )
  {
    eraseprop( name );
  }

  return *this;
}

void PropertyList::operator-=( const std::set<std::string>& CPropNames )  // dave added 1/26/3
{
  for ( const auto& name : CPropNames )
  {
    eraseprop( name );
  }
}

Bscript::BObjectImp* CallPropertyListMethod_id( PropertyList& proplist, const int id,
                                                Bscript::Executor& ex, bool& changed )
{
  using namespace Bscript;
  switch ( id )
  {
  case MTH_GETPROP:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* propname_str;
    if ( !ex.getStringParam( 0, propname_str ) )
      return new BError( "Invalid parameter type" );
    std::string val;
    if ( !proplist.getprop( propname_str->value(), val ) )
      return new BError( "Property not found" );

    return Bscript::BObjectImp::unpack( val.c_str() );
  }

  case MTH_SETPROP:
  {
    if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    const String* propname_str;
    if ( !ex.getStringParam( 0, propname_str ) )
      return new BError( "Invalid parameter type" );

    Bscript::BObjectImp* propval = ex.getParamImp( 1 );
    if ( propval->isa( Bscript::BObjectImp::OTError ) )
    {
      POLLOGLN( "wtf, setprop w/ an error '{}' PC:{}", ex.scriptname().c_str(), ex.PC );
    }
    std::string propname = propname_str->value();
    proplist.setprop( propname, propval->pack() );
    if ( propname[0] != '#' )
      changed = true;
    return new BLong( 1 );
  }

  case MTH_ERASEPROP:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* propname_str;
    if ( !ex.getStringParam( 0, propname_str ) )
      return new BError( "Invalid parameter type" );
    std::string propname = propname_str->value();
    proplist.eraseprop( propname );
    if ( propname[0] != '#' )
      changed = true;
    return new BLong( 1 );
  }

  case MTH_PROPNAMES:
  {
    std::vector<std::string> propnames;
    proplist.getpropnames( propnames );
    std::unique_ptr<ObjArray> arr( new ObjArray );
    for ( const auto& name : propnames )
    {
      arr->addElement( new String( name ) );
    }
    return arr.release();
  }

  default:
    return nullptr;
  }
}

Bscript::BObjectImp* CallPropertyListMethod( PropertyList& proplist, const char* methodname,
                                             Bscript::Executor& ex, bool& changed )
{
  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return CallPropertyListMethod_id( proplist, objmethod->id, ex, changed );
  else
    return nullptr;
}
}  // namespace Core
}  // namespace Pol
