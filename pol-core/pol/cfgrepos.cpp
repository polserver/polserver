/*
History
=======
2005/07/01 Shinigami: added StoredConfigFile::modified_ (stat.st_mtime) to detect cfg file modification
                      added oldcfgfiles - holds all references to "removed" cfg files
                      UnloadConfigFile will reload file if modified only
2005/07/04 Shinigami: added StoredConfigFile::reload to check for file modification
                      moved file modification check from UnloadConfigFile to FindConfigFile
                      added ConfigFiles_log_stuff for memory logging
2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format too (leak.log)

Notes
=======
Configuration File Repository

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include <sys/stat.h>

#include "../bscript/bobject.h"
#include "../bscript/escrutil.h"
#include "../bscript/impstr.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/mlog.h"
#include "../clib/refptr.h"
#include "../clib/strutil.h"

#include "cfgrepos.h"
#include "polcfg.h"
#include "../plib/pkg.h"
namespace Pol {
  namespace Core {
	StoredConfigElem::StoredConfigElem( Clib::ConfigElem& elem )
	{
	  string propname, propval;

	  while ( elem.remove_first_prop( &propname, &propval ) )
	  {
		Bscript::BObjectImp* newimp = Bscript::bobject_from_string( propval );

		addprop( propname, newimp );
	  }
	}

	StoredConfigElem::StoredConfigElem()
	{}

	// ToDo: we have to think over... it's a problem with script-inside references
	StoredConfigElem::~StoredConfigElem()
	{
	  //	while (!propimps_.empty())
	  //	{
	  //		delete ((*propimps_.begin()).second);
	  //		propimps_.erase( propimps_.begin() );
	  //	}
	}

	void StoredConfigElem::addprop( const std::string& propname, Bscript::BObjectImp* imp )
	{
	  propimps_.insert( PropImpList::value_type( propname.c_str(), ref_ptr<class Bscript::BObjectImp>( imp ) ) );
	}

	Bscript::BObjectImp* StoredConfigElem::getimp( const std::string& propname ) const
	{
	  PropImpList::const_iterator itr = propimps_.find( propname );
	  if ( itr == propimps_.end() )
		return NULL;
	  else
		return ( *itr ).second.get();

	}

	Bscript::BObjectImp* StoredConfigElem::listprops() const
	{
	  Bscript::ObjArray* objarr = new Bscript::ObjArray;
	  Bscript::String propname;
	  PropImpList::const_iterator itr;
	  for ( itr = propimps_.begin(); itr != propimps_.end(); ++itr )
	  {
		propname = ( *itr ).first.c_str();
		if ( !objarr->contains( propname ) )
		  objarr->addElement( propname.copy() );
	  }
	  return objarr;
	}

	std::pair<StoredConfigElem::const_iterator, StoredConfigElem::const_iterator> StoredConfigElem::equal_range( const std::string& propname ) const
	{
	  return propimps_.equal_range( propname );
	}

	StoredConfigFile::StoredConfigFile() :
	  reload( false ),
	  modified_( 0 )
	{}

	// ToDo: we have to think over... it's a problem with script-inside references
	//StoredConfigFile::~StoredConfigFile( )
	//{
	//	while (!elements_bynum_.empty())
	//	{
	//		delete ((*elements_bynum_.begin()).second);
	//		elements_bynum_.erase( elements_bynum_.begin() );
	//	}
	//   
	//	while (!elements_byname_.empty())
	//	{
	//		delete ((*elements_byname_.begin()).second);
	//		elements_byname_.erase( elements_byname_.begin() );
	//	}
	//}

	void StoredConfigFile::load( Clib::ConfigFile& cf )
	{
	  reload = false;
	  modified_ = cf.modified();

	  Clib::ConfigElem elem;
	  while ( cf.read( elem ) )
	  {
		ElemRef elemref( new StoredConfigElem( elem ) );

		if ( isdigit( elem.rest()[0] ) )
		{
		  unsigned int key = strtoul( elem.rest(), NULL, 0 );
		  elements_bynum_.insert( ElementsByNum::value_type( key, elemref ) );
		}

		string key( elem.rest() );
		elements_byname_.insert( ElementsByName::value_type( key, elemref ) );
	  }
	}

	StoredConfigFile::ElemRef StoredConfigFile::findelem( int key )
	{
	  ElementsByNum::const_iterator itr = elements_bynum_.find( key );
	  if ( itr == elements_bynum_.end() )
		return ElemRef( 0 );
	  else
		return ( *itr ).second;
	}

	StoredConfigFile::ElemRef StoredConfigFile::findelem( const string& key )
	{
	  ElementsByName::const_iterator itr = elements_byname_.find( key );
	  if ( itr == elements_byname_.end() )
		return ElemRef( 0 );
	  else
		return ( *itr ).second;
	}

	int StoredConfigFile::maxintkey() const
	{
	  if ( elements_bynum_.empty() )
	  {
		return 0;
	  }
	  else
	  {
		ElementsByNum::const_iterator itr = elements_bynum_.end();
		--itr;
		return ( *itr ).first;
	  }
	}

	const time_t StoredConfigFile::modified() const
	{
	  return modified_;
	}

	//  From "[some stuff]" return "some stuff"
	string extractkey( const string& istr )
	{
	  string::size_type vstart = istr.find_first_not_of( " [" );
	  string::size_type vend = istr.find_last_not_of( "] " );
	  return istr.substr( vstart, vend );
	}

	void StoredConfigFile::load_tus_scp( const std::string& filename )
	{
	  ifstream ifs( filename.c_str() );

	  int count = 0;
	  ElemRef elemref( new StoredConfigElem() );
	  elements_bynum_.insert( ElementsByNum::value_type( count++, elemref ) );

	  string strbuf;
	  while ( getline( ifs, strbuf ) )
	  {
		if ( strbuf[0] == '[' )
		{
		  elemref.set( new StoredConfigElem() );
		  elements_bynum_.insert( ElementsByNum::value_type( count++, elemref ) );
		  strbuf = extractkey( strbuf );
		  elemref->addprop( "_key", Bscript::bobject_from_string( strbuf, 16 ) );
		}
		// FIXME: skip empty lines and comment lines (duh)
		string propname, propvalue;
		Clib::splitnamevalue( strbuf, propname, propvalue );

		if ( propname == "" || propname.substr( 0, 2 ) == "//" )
		  continue;

		Bscript::BObjectImp* newimp = Bscript::bobject_from_string( propvalue, 16 );

		elemref->addprop( propname, newimp );
	  }
	}


	typedef map<string, ConfigFileRef> CfgFiles;
	typedef vector<string> OldCfgFiles; // we've multiple older instances
	CfgFiles cfgfiles;
	OldCfgFiles oldcfgfiles;


	ConfigFileRef FindConfigFile( const string& filename, const string& allpkgbase )
	{
	  CfgFiles::iterator itr = cfgfiles.find( filename );
	  if ( itr != cfgfiles.end() )
	  {
		if ( ( *itr ).second->reload ) // check cfg file modification?
		{
		  struct stat newcfgstat;
		  stat( filename.c_str(), &newcfgstat );
		  if ( ( *itr ).second->modified() != newcfgstat.st_mtime )
		  {
			oldcfgfiles.push_back( ( *itr ).first );
			cfgfiles.erase( itr );
		  }
		  else
			return ( *itr ).second;
		}
		else
		  return ( *itr ).second;
	  }

	  try
	  {
		if ( !allpkgbase.empty() )
		{
		  bool any = false;
		  ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
		  string main_cfg = "config/" + allpkgbase + ".cfg";
		  if ( Clib::FileExists( main_cfg.c_str() ) )
		  {
			Clib::ConfigFile cf_main( main_cfg.c_str() );
			scfg->load( cf_main );
			any = true;
		  }
		  for ( Plib::Packages::iterator pitr = Plib::packages.begin( ), pitrend = Plib::packages.end( ); pitr != pitrend; ++pitr )
		  {
			Plib::Package* pkg = ( *pitr );
			//string pkgfilename = pkg->dir() + allpkgbase + ".cfg";
			string pkgfilename = GetPackageCfgPath( pkg, allpkgbase + ".cfg" );
			if ( Clib::FileExists( pkgfilename.c_str() ) )
			{
			  Clib::ConfigFile cf( pkgfilename.c_str() );
			  scfg->load( cf );
			  any = true;
			}
		  }
		  if ( !any )
			return ConfigFileRef( 0 );
		  cfgfiles.insert( CfgFiles::value_type( filename, scfg ) );
		  return scfg;
		}
		else
		{
		  if ( !Clib::FileExists( filename.c_str() ) )
		  {
			if ( config.report_missing_configs )
			{
			  if ( Clib::mlog.is_open() )
				Clib::mlog << "Config File " << filename << " does not exist." << endl;
			}
			return ConfigFileRef( 0 );
		  }

		  Clib::ConfigFile cf( filename.c_str() );

		  ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
		  scfg->load( cf );
		  cfgfiles.insert( CfgFiles::value_type( filename, scfg ) );
		  return scfg;
		}
	  }
	  catch ( exception& ex )
	  {
		// There was some weird problem reading the config file.
		if ( Clib::mlog.is_open() )
		  Clib::mlog << "An exception was encountered while reading " << filename << ": " << ex.what() << endl;
		return ConfigFileRef( 0 );
	  }
	}

	ConfigFileRef LoadTusScpFile( const string& filename )
	{
	  if ( !Clib::FileExists( filename.c_str() ) )
	  {
		return ConfigFileRef( 0 );
	  }

	  ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
	  scfg->load_tus_scp( filename );
	  return scfg;
	}

	void CreateEmptyStoredConfigFile( const string& filename )
	{
	  ref_ptr<StoredConfigFile> scfg( new StoredConfigFile() );
	  cfgfiles.insert( CfgFiles::value_type( filename, scfg ) );
	}

	int UnloadConfigFile( const string& filename )
	{
	  CfgFiles::iterator itr = cfgfiles.find( filename );
	  if ( itr != cfgfiles.end() )
	  {
		( *itr ).second->reload = true; // check cfg file modification on FindConfigFile

		return ( *itr ).second->count() - 1;
	  }
	  else
	  {
		return -1;
	  }
	}

#ifdef MEMORYLEAK
	void ConfigFiles_log_stuff()
	{
	  if (mlog.is_open())
		mlog << "ConfigFiles: " << cfgfiles.size() << " files loaded and "
		<< oldcfgfiles.size() << " files 'removed'" << endl;

	  if (llog.is_open())
		llog << cfgfiles.size() << ";" << oldcfgfiles.size() << ";";
	}
#endif

	// ToDo: we have to think over... it's a problem with script-inside references
	void UnloadAllConfigFiles()
	{
	  oldcfgfiles.clear();
	  cfgfiles.clear();
	}
  }
}