/** @file
 *
 * @par History
 * - 2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )
 */


#include "proplist.h"

#include "../plib/systemstate.h"

#include "../clib/cfgelem.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/logfacility.h"
#include "../clib/streamsaver.h"

#include "../bscript/berror.h"
#include "../bscript/execmodl.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmethods.h"

#include <memory>

#define pf_endl '\n'
namespace Pol {
  namespace Core {

    /**
     * Registers a property list address
     *
     * @param proplist Pointer to the list to be registered
     * @param type The type of the list to be registered
     */
    void CPropProfiler::registerProplist(const PropertyList* proplist, const CPropProfiler::Type type)
    {
      _proplistsMutex.lock();
      _proplists->insert(std::make_pair(proplist, type));
      _proplistsMutex.unlock();
    }

    /**
     * Registers a property list with same type of an already registered one
     *
     * @param proplist Pointer to the list to be registered
     * @param copiedFrom Pointer to the already registered list to copy the type from
     */
    void CPropProfiler::registerProplist(const PropertyList* proplist, const PropertyList* copiedFrom)
    {
      registerProplist(proplist, getProplistType(copiedFrom));
    }

    /**
     * Unregisters a property list address
     */
    void CPropProfiler::unregisterProplist(const PropertyList* proplist) {
      _proplistsMutex.lock();
      _proplists->erase(proplist);
      _proplistsMutex.unlock();
    }

    /**
    * Register a cprop action
    *
    * @param proplist Pointer to the registered list where this cprop resides
    * @param name Name of the cprop
    * @param key Index of the array key to update
    */
    void CPropProfiler::cpropAction(const PropertyList* proplist, const std::string& name, const size_t key )
    {
      Type type = getProplistType(proplist);
      if( isIgnored(type) )
        return;

      _hitsMutex.lock();
      if( (*_hits)[type][name][key] < std::numeric_limits<u64>::max() )
        (*_hits)[type][name][key]++;
      _hitsMutex.unlock();
    }

    /**
     * Discards all the gathered data
     */
    void CPropProfiler::clear()
    {
      _proplistsMutex.lock();
      _hitsMutex.lock();

      _proplists->clear();
      _hits->clear();

      _hitsMutex.unlock();
      _proplistsMutex.unlock();
    }

    /**
     * Dumps current status into a given stream
     *
     * @param os The output stream to write into
     */
    void CPropProfiler::dumpProfile(std::ostream& os)
    {
      // First generate the data

      // map<categoryname, map<typename, vector<lines> >>
      std::map<std::string, std::map<std::string, std::vector<std::string>>> outData;

      _hitsMutex.lock();
      for( auto tIter = _hits->begin(); tIter != _hits->end(); ++tIter ) {
        Type t = tIter->first;

        std::string typeName;
        switch( t ) {
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
          typeName = "ERROR " + std::to_string(static_cast<unsigned int>(t));
          break;
        }

        for( auto pIter = tIter->second.begin(); pIter != tIter->second.end(); ++pIter ) {
          std::ostringstream line;
          line << pIter->first << " ";
          line << pIter->second[HitsCounter::READ] << "/";
          line << pIter->second[HitsCounter::WRITE] << "/";
          line << pIter->second[HitsCounter::ERASE] << std::endl;

          if( ! pIter->second[HitsCounter::READ] )
            outData["WRITTEN BUT NEVER READ"][typeName].push_back(line.str());
          else if( ! pIter->second[HitsCounter::WRITE] )
            outData["READ BUT NEVER WRITTEN"][typeName].push_back(line.str());
          else
            outData["ALL THE REST"][typeName].push_back(line.str());
        }
      }
      _hitsMutex.unlock();

      // Then output it
      for( auto it1 = outData.rbegin(); it1 != outData.rend(); ++it1 ) {
        // 1st level header
        os << std::string(15, '-') << " " << it1->first << " " << std::string(15, '-') << std::endl;

        for( auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ) {
          // 2nd level header
          os << it2->first << " CProps summary (read/write/erase):" << std::endl;

          std::sort( it2->second.begin(), it2->second.end() );
          for( auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3 ) {
            os << "- " << *it3;
          }
        }

        os << std::endl;
      }
    }

    /**
     * Returns the estimated in-memory size of the profiler
     */
    size_t CPropProfiler::estimateSize()
    {
      /// Size of base empty containers
      size_t ret = sizeof(_hitsMutex) + sizeof(_proplistsMutex) +
                   sizeof(_hits) + sizeof(_proplists) +
                   sizeof(void*) * 2;

      /// + size of proplists
      ret += ( sizeof(PropertyList*) + sizeof(Type) ) * _proplists->size();

      /// + size of hits
      _hitsMutex.lock();
      for( auto itr1 = _hits->begin(); itr1 != _hits->end(); ++itr1 ) {
        ret += sizeof(Type) + sizeof(HitsEntries);
        for( auto itr2 = itr1->second.begin(); itr2 != itr1->second.end(); ++itr2 ) {
          ret += itr2->first.size() + itr2->second.sizeEstimate();
        }
      }
      _hitsMutex.unlock();

      return ret;
    }

    /**
     * Initialize and register this property list based on a given type
     * register only if the profile_cprops flag is set
     */
    PropertyList::PropertyList( const CPropProfiler::Type& type )
    {
      if( Plib::systemstate.config.profile_cprops )
        CPropProfiler::instance().registerProplist(this, type);
    }

    /**
     * Initialize and register this property list based on a given type,
     * always register if force flag is is true
     */
    PropertyList::PropertyList( const CPropProfiler::Type& type, bool force )
    {
      if( force || Plib::systemstate.config.profile_cprops )
        CPropProfiler::instance().registerProplist(this, type);
    }

    /**
     * Initialize by copying content and type from a given one
     */
	PropertyList::PropertyList( const PropertyList& props )  //dave added 1/26/3
	{
	  copyprops( props );

      if( Plib::systemstate.config.profile_cprops )
        CPropProfiler::instance().registerProplist(this, &props);
	}

    size_t PropertyList::estimatedSize() const
    {
      size_t size = sizeof( PropertyList );
      size += properties.size( ) * ( sizeof( boost_utils::cprop_name_flystring ) + sizeof( boost_utils::cprop_value_flystring ) + ( sizeof(void*)* 3 + 1 ) / 2 );
      return size;
    }

	bool PropertyList::getprop( const std::string& propname, std::string& propval ) const
	{
      if( Plib::systemstate.config.profile_cprops )
        CPropProfiler::instance().cpropRead(this, propname);

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
      if( Plib::systemstate.config.profile_cprops )
        CPropProfiler::instance().cpropWrite(this, propname);

      properties[boost_utils::cprop_name_flystring( propname )] = propvalue;
	}

	void PropertyList::eraseprop( const std::string& propname )
	{
      if( Plib::systemstate.config.profile_cprops )
        CPropProfiler::instance().cpropErase(this, propname);

      properties.erase( boost_utils::cprop_name_flystring( propname ) );
	}

	void PropertyList::copyprops( const PropertyList& from )
	{
	  //dave 4/25/3 map insert won't overwrite with new values, so remove those first and then reinsert.
	  Properties::const_iterator itr;
	  for ( const auto& prop : from.properties)
        properties.erase( prop.first );

	  properties.insert( from.properties.begin(), from.properties.end() );
	}

	void PropertyList::clear()
	{
	  properties.clear();
	}

	void PropertyList::getpropnames( std::vector< std::string >& propnames ) const
	{
	  for ( const auto &prop : properties )
	  {
		propnames.push_back( prop.first );
	  }
	}

	void PropertyList::printProperties( Clib::StreamWriter& sw ) const
	{
	  for ( const auto &prop : properties )
	  {
		const std::string& first = prop.first;
		if ( first[0] != '#' )
		{
		  sw() << "\tCProp\t" << first << " " << prop.second.get() << pf_endl;
		}
	  }
	}
    void PropertyList::printProperties( Clib::ConfigElem& elem ) const
	{
	  for ( const auto &prop : properties )
	  {
		const std::string& first = prop.first;
		if ( first[0] != '#' )
		{
		  elem.add_prop( "CProp", ( first + "\t" + prop.second.get() ).c_str() );
		}
	  }
	}

    void PropertyList::printPropertiesAsStrings( Clib::StreamWriter& sw ) const
	{
	  for ( const auto &prop : properties )
	  {
		const std::string& first = prop.first;
		if ( first[0] != '#' )
		{
		  sw() << "\t" << first << " " << prop.second.get() << pf_endl;
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
	 std:: string propname, propvalue;
	  while ( elem.remove_first_prop( &propname, &propvalue ) )
	  {
		setprop( propname, propvalue );
	  }
	}

	bool PropertyList::operator==( const PropertyList& plist ) const
	{
	  return ( this->properties == plist.properties );
	}

	PropertyList& PropertyList::operator-( const std::set<std::string>& CPropNames ) //dave added 1/26/3
	{
	  for ( const auto& name : CPropNames)
	  {
        eraseprop( name );
	  }

	  return *this;
	}

	void PropertyList::operator-=( const std::set<std::string>& CPropNames )  //dave added 1/26/3
	{
      for ( const auto& name : CPropNames )
	  {
        eraseprop( name );
	  }
	}

	Bscript::BObjectImp* CallPropertyListMethod_id( PropertyList& proplist, const int id, Bscript::Executor& ex, bool& changed )
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
            POLLOG.Format( "wtf, setprop w/ an error '{}' PC:{}\n" ) << ex.scriptname().c_str() << ex.PC;
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
		  for ( const auto& name : propnames)
		  {
            arr->addElement( new String( name ) );
		  }
		  return arr.release();
		}

		default:
		  return NULL;
	  }
	}

	Bscript::BObjectImp* CallPropertyListMethod( PropertyList& proplist, const char* methodname, Bscript::Executor& ex, bool& changed )
	{
      Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
	  if ( objmethod != NULL )
		return CallPropertyListMethod_id( proplist, objmethod->id, ex, changed );
	  else
		return NULL;
	}

  }
}