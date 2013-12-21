/*
History
=======
2005/05/25 Shinigami: added PropertyList::printProperties( ConfigElem& elem )

Notes
=======

*/

#ifndef PROPLIST_H
#define PROPLIST_H

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "../clib/streamsaver.h"
#include "../clib/cfgelem.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../../lib/format/format.h"
namespace Pol {
  namespace Core {

	class PropertyList
	{
	public:
	  PropertyList();
	  PropertyList( const PropertyList& );  //dave added 1/26/3
	  bool getprop( const std::string& propname, std::string& propvalue ) const;
	  void setprop( const std::string& propname, const std::string& propvalue );
	  void eraseprop( const std::string& propname );
	  void copyprops( const PropertyList& proplist );
	  void getpropnames( std::vector< std::string >& propnames ) const;
	  void clear();

	  void printProperties( Clib::StreamWriter& sw ) const;
	  void printProperties( Clib::ConfigElem& elem ) const;
	  void readProperties( Clib::ConfigElem& elem );

	  void printPropertiesAsStrings( Clib::StreamWriter& sw ) const;
	  void readRemainingPropertiesAsStrings( Clib::ConfigElem& elem );

	  bool operator==( const PropertyList& ) const;
	  PropertyList& operator-( const std::set<std::string>& );  //dave added 1/26/3
	  void operator-=( const std::set<std::string>& );  //dave added 1/26/3
	protected:
	  typedef std::map<std::string, std::string> Properties;
	  Properties properties;

	private:
	  friend class UObjectHelper;

	  // not implemented
	  PropertyList& operator=( const PropertyList& );
	};

	Bscript::BObjectImp* CallPropertyListMethod( PropertyList& proplist,
										const char* methodname,
										Bscript::Executor& ex,
										bool& changed );
	Bscript::BObjectImp* CallPropertyListMethod_id( PropertyList& proplist,
										   const int id,
										   Bscript::Executor& ex,
										   bool& changed );
  }
}
#endif
