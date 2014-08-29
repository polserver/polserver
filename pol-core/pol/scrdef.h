/*
History
=======


Notes
=======

*/

#ifndef SCRDEF_H
#define SCRDEF_H

#include <string>
#include "../clib/boostutils.h"
namespace Pol {
  namespace Plib {
	class Package;
  }
  namespace Core {

	class ScriptDef
	{
	public:
	  ScriptDef() : name_( "" ), pkg_( NULL ) {}
	  ScriptDef( const std::string& name, const Plib::Package* pkg, const char* mainpfx );
	  ScriptDef( const std::string& name, const Plib::Package* pkg );
	  // ScriptDef( const ScriptDef& ); default copy ctor is okay
	  // ScriptDef& operator=( const ScriptDef& ); default operator= is okay

	  bool operator==( const ScriptDef& other ) const;
	  bool operator!=(const ScriptDef& other) const;
	  void config( const std::string& name,
				   const Plib::Package* pkg,
				   const char* mainpfx,
				   bool warn_if_not_found = true );
	  void config( const std::string& name,
				   const Plib::Package* pkg );
	  bool config_nodie( const std::string& name,
						 const Plib::Package* pkg,
						 const char* mainpfx );
	  void quickconfig( const Plib::Package* pkg, const std::string& name_ecl );
	  void quickconfig( const std::string& name_ecl );

	  bool empty() const { return name_.get().empty(); }
	  const char* c_str() const { return name_.get().c_str(); }
	  void clear();

	  const std::string& name() const { return name_; }
	  const Plib::Package* pkg( ) const { return pkg_; }

	  bool exists() const;

	  std::string relativename( const Plib::Package* pkg = NULL ) const;
      size_t estimatedSize() const;
	private:
      boost_utils::script_name_flystring localname_;
      boost_utils::script_name_flystring name_;
	  const Plib::Package* pkg_;

	private: // not implemented
	};
  }
}

#endif
