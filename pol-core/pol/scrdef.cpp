/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../bscript/eprog.h"
#include "../bscript/escrutil.h"

#include "../clib/fileutil.h"
#include "../clib/refptr.h"

#include "../plib/pkg.h"
#include "scrdef.h"
#include "scrstore.h"
namespace Pol {
  namespace Core {
	string full_scriptname( const string& spec, const Plib::Package* pkg, const char* mainpfx )
	{
	  if ( spec.empty() )
		return spec;

	  if ( pkg != NULL )
        return Bscript::normalize_ecl_filename( pkg->dir( ) + spec );

	  if ( spec.find( '/' ) == string::npos )
        return Bscript::normalize_ecl_filename( mainpfx + spec );
	  else
        return Bscript::normalize_ecl_filename( "scripts/" + spec );

	}

    ScriptDef::ScriptDef( const std::string& iname, const Plib::Package* ipkg, const char* mainpfx )
	{
	  config( iname, ipkg, mainpfx, true );
	}

    ScriptDef::ScriptDef( const std::string& iname, const Plib::Package* ipkg )
	{
	  config( iname, ipkg, "", true );
	}

	bool ScriptDef::operator==( const ScriptDef& other ) const
	{
	  if ( empty() && other.empty() )
		return true;

	  return pkg_ == other.pkg_ &&
		name_ == other.name_;
	}

	void ScriptDef::config( const std::string& iname,
                            const Plib::Package* ipkg,
							const char* mainpfx,
							bool warn_if_not_found )
	{
	  string path;
      const Plib::Package* npkg;
      if ( !Plib::pkgdef_split( iname, ipkg, &npkg, &path ) )
	  {
		cerr << "Error reading script descriptor '" << iname << "'" << endl;
		throw runtime_error( "Error reading script descriptor" );
	  }

	  localname_ = path;
	  name_ = full_scriptname( path, npkg, mainpfx );
	  pkg_ = npkg;

	  if ( warn_if_not_found )
	  {
		if ( !empty() && !exists() )
		{
		  cerr << "Warning! " << name_ << " does not exist!" << endl;
		}
	  }
	}

	void ScriptDef::config( const std::string& iname,
                            const Plib::Package* ipkg )
	{
	  config( iname, ipkg, "", true );
	}

	bool ScriptDef::config_nodie( const std::string& iname,
                                  const Plib::Package* ipkg,
								  const char* mainpfx )
	{
	  string path;
      const Plib::Package* npkg;
      if ( !Plib::pkgdef_split( iname, ipkg, &npkg, &path ) )
	  {
		cerr << "Error reading script descriptor '" << iname << "'" << endl;
		return false;
	  }

	  localname_ = iname;
	  name_ = full_scriptname( path, npkg, mainpfx );
	  pkg_ = npkg;

	  return true;
	}

    string ScriptDef::relativename( const Plib::Package* pkg ) const
	{
	  if ( name_ == "" )
		return "";
	  else if ( pkg == pkg_ )
		return localname_;
	  else
		return ":" + ( pkg_ ? pkg_->name() : "" ) + ":" + localname_;
	}

    void ScriptDef::quickconfig( const Plib::Package* pkg,
								 const std::string& name_ecl )
	{
	  localname_ = "unknown";
	  name_ = pkg->dir() + name_ecl;
	  pkg_ = pkg;
	}

	void ScriptDef::quickconfig( const std::string& name_ecl )
	{
	  localname_ = "unknown";
	  name_ = name_ecl;
	  pkg_ = NULL;
	}

	bool ScriptDef::exists() const
	{
	  return !empty() && Clib::FileExists( c_str() );
	  //ref_ptr<EScriptProgram> prog = find_script2( *this, false, true );
	  //return (prog.get() != NULL);
	}

	void ScriptDef::clear()
	{
	  localname_ = "";
	  name_ = "";
	  pkg_ = NULL;
	}

  }
}