/*
History
=======
2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


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

#include "proplist.h"

#define pf_endl '\n'
namespace Pol {
  namespace Core {

	PropertyList::PropertyList()
	{}

	PropertyList::PropertyList( const PropertyList& props )  //dave added 1/26/3
	{
	  copyprops( props );
	}

    size_t PropertyList::estimatedSize() const
    {
      size_t size = sizeof( PropertyList );
      size += properties.size( ) * ( sizeof(cprop_name)+sizeof(cprop_value)+( sizeof(void*)* 3 + 1 ) / 2 );
      return size;
    }

	bool PropertyList::getprop( const string& propname, string& propval ) const
	{
	  Properties::const_iterator itr = properties.find( cprop_name(propname) );
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
      properties[cprop_name( propname)] = propvalue;
	}

	void PropertyList::eraseprop( const std::string& propname )
	{
      properties.erase( cprop_name( propname) );
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
		const string& first = prop.first;
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
		const string& first = prop.first;
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
		const string& first = prop.first;
		if ( first[0] != '#' )
		{
		  sw() << "\t" << first << " " << prop.second.get() << pf_endl;
		}
	  }
	}

    void PropertyList::readProperties( Clib::ConfigElem& elem )
	{
	  string tempstr;
	  while ( elem.remove_prop( "StrProp", &tempstr ) )
	  {
		string propname;
		string propvalue;

        Clib::splitnamevalue( tempstr, propname, propvalue );

		setprop( propname, "s" + propvalue );
	  }
	  while ( elem.remove_prop( "CProp", &tempstr ) )
	  {
		string propname;
		string propvalue;

        Clib::splitnamevalue( tempstr, propname, propvalue );

		setprop( propname, propvalue );
	  }
	}

    void PropertyList::readRemainingPropertiesAsStrings( Clib::ConfigElem& elem )
	{
	  string propname, propvalue;
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
		  string propname = propname_str->value();
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
		  string propname = propname_str->value();
		  proplist.eraseprop( propname );
		  if ( propname[0] != '#' )
			changed = true;
		  return new BLong( 1 );
		}

		case MTH_PROPNAMES:
		{
		  vector<string> propnames;
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