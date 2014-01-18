/*
History
=======

2006/05/10 MuadDib:   Removed unload_npc_templates for non use.
2009/08/25 Shinigami: STLport-5.2.1 fix: elem->type() check will use strlen() now

Notes
=======

*/


#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"

#include "../plib/pkg.h"
namespace Pol {
  namespace Core {
	class NpcTemplateConfigSource : public Clib::ConfigSource
	{
	public:
	  NpcTemplateConfigSource();
      NpcTemplateConfigSource( const Clib::ConfigFile& cf );
	  virtual void display_error( const std::string& msg,
								  bool show_curline = true,
                                  const Clib::ConfigElemBase* elem = NULL,
								  bool error = true ) const;
	private:
	  string _filename;
	  unsigned _fileline;
	};
    NpcTemplateConfigSource::NpcTemplateConfigSource( const Clib::ConfigFile& cf ) :
	  _filename( cf.filename() ),
	  _fileline( cf.element_line_start() )
	{};
	NpcTemplateConfigSource::NpcTemplateConfigSource()
	{}
	void NpcTemplateConfigSource::display_error( const std::string& msg,
												 bool show_curline,
                                                 const Clib::ConfigElemBase* elem,
												 bool error ) const
	{
	  bool showed_elem_line = false;

      fmt::Writer tmp;
      tmp << ( error ? "Error" : "Warning" )
        << " reading configuration file " << _filename << ":\n";

	  tmp << "\t" << msg << "\n";

	  if ( elem != NULL )
	  {
		if ( strlen( elem->type() ) > 0 )
		{
		  tmp << "\tElement: " << elem->type() << " " << elem->rest();
		  if ( _fileline )
			tmp << ", found on line " << _fileline;
		  tmp << "\n";
		  showed_elem_line = true;
		}
	  }

	  //if (show_curline)
	  //    cerr << "\tNear line: " << _cur_line << endl;
	  if ( _fileline && !showed_elem_line )
		tmp << "\tElement started on line: " << _fileline << "\n";
      ERROR_PRINT << tmp.c_str();
	}


	class NpcTemplateElem
	{
	public:
	  NpcTemplateElem();
      NpcTemplateElem( const Clib::ConfigFile& cf, const Clib::ConfigElem& elem );

      void copyto( Clib::ConfigElem& elem ) const;
	private:
	  NpcTemplateConfigSource _source;
      Clib::ConfigElem _elem;
	};
	NpcTemplateElem::NpcTemplateElem()
	{}
    NpcTemplateElem::NpcTemplateElem( const Clib::ConfigFile& cf, const Clib::ConfigElem& elem ) :
	  _source( cf ),
	  _elem( elem )
	{}

    void NpcTemplateElem::copyto( Clib::ConfigElem& elem ) const
	{
	  elem = _elem;
	  elem.set_source( &_source );
	}

    typedef map< string, NpcTemplateElem, Clib::ci_cmp_pred > NpcTemplates;
	static NpcTemplates npc_templates;

    bool FindNpcTemplate( const char* template_name, Clib::ConfigElem& elem )
	{
	  NpcTemplates::const_iterator itr = npc_templates.find( template_name );
	  if ( itr != npc_templates.end() )
	  {
		const NpcTemplateElem& tm = ( *itr ).second;
		tm.copyto( elem );
		return true;
	  }
	  else
	  {
		return false;
	  }
	}

	// FIXME inefficient.  Templates should be read in once, and reused.
    bool FindNpcTemplate( const char *template_name, Clib::ConfigFile& cf, Clib::ConfigElem& elem )
	{
	  try
	  {
		const Plib::Package* pkg;
		string npctemplate;
        if ( !Plib::pkgdef_split( template_name, NULL, &pkg, &npctemplate ) )
		  return false;

        string filename = Plib::GetPackageCfgPath( const_cast<Plib::Package*>( pkg ), "npcdesc.cfg" );

		cf.open( filename.c_str() );
		while ( cf.read( elem ) )
		{
		  if ( !elem.type_is( "NpcTemplate" ) )
			continue;

		  string orig_rest = elem.rest();
		  if ( pkg != NULL )
		  {
			string newrest = ":" + pkg->name() + ":" + npctemplate;
			elem.set_rest( newrest.c_str() );
		  }
		  const char* rest = elem.rest();
		  if ( rest != NULL && *rest != '\0' )
		  {
			if ( stricmp( orig_rest.c_str(), npctemplate.c_str() ) == 0 )
			  return true;
		  }
		  else
		  {
			string tname = elem.remove_string( "TemplateName" );
			if ( stricmp( tname.c_str(), npctemplate.c_str() ) == 0 )
			  return true;
		  }
		}
		return false;
	  }
	  catch ( const char *msg )
	  {
        ERROR_PRINT << "NPC Creation (" << template_name << ") Failed: " << msg << "\n";
	  }
	  catch ( string& str )
	  {
        ERROR_PRINT << "NPC Creation (" << template_name << ") Failed: " << str << "\n";
	  }	   // egcs has some trouble realizing 'exception' should catch
	  catch ( runtime_error& re )   // runtime_errors, so...
	  {
        ERROR_PRINT << "NPC Creation (" << template_name << ") Failed: " << re.what( ) << "\n";
	  }
	  catch ( exception& ex )
	  {
        ERROR_PRINT << "NPC Creation (" << template_name << ") Failed: " << ex.what() << "\n";
	  }
#ifndef WIN32
	  catch ( ... )
	  {
	  }
#endif
	  return false;
	}

    void read_npc_templates( Plib::Package* pkg )
	{
	  string filename = GetPackageCfgPath( pkg, "npcdesc.cfg" );
	  if ( !Clib::FileExists( filename ) )
		return;

      Clib::ConfigFile cf( filename.c_str( ) );
      Clib::ConfigElem elem;
	  while ( cf.read( elem ) )
	  {
		if ( elem.type_is( "NpcTemplate" ) )
		{
		  // first determine the NPC template name.
		  string namebase;
		  const char* rest = elem.rest();
		  if ( rest != NULL && *rest != '\0' )
		  {
			namebase = rest;
		  }
		  else
		  {
			namebase = elem.remove_string( "TemplateName" );
		  }
		  string descname;
		  if ( pkg != NULL )
		  {
			descname = ":" + pkg->name() + ":" + namebase;
			elem.set_rest( descname.c_str() );
		  }
		  else
			descname = namebase;

		  npc_templates[descname] = NpcTemplateElem( cf, elem );
		}
	  }
	}

	void read_npc_templates()
	{
	  npc_templates.clear();
	  read_npc_templates( NULL );
      Clib::ForEach( Plib::packages, read_npc_templates );
	}
  }
}