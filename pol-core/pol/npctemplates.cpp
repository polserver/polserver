/*
History
=======

2006/05/10 MuadDib:   Removed unload_npc_templates for non use.
2009/08/25 Shinigami: STLport-5.2.1 fix: elem->type() check will use strlen() now

Notes
=======

*/

#include "npctmpl.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"

#include "../plib/pkg.h"
#include "../plib/systemstate.h"

#include "globals/uvars.h"

#include <stdexcept>
#include <map>
#include <string>

#ifdef _MSC_VER
#pragma warning(disable:4996) // deprecation warning for stricmp
#endif


namespace Pol {
  namespace Core {
	
    NpcTemplateConfigSource::NpcTemplateConfigSource( const Clib::ConfigFile& cf ) :
	  _filename( cf.filename() ),
	  _fileline( cf.element_line_start() )
	{};
	NpcTemplateConfigSource::NpcTemplateConfigSource() : _filename(""), _fileline(0)
	{}
    size_t NpcTemplateConfigSource::estimateSize() const
    {
      return _filename.capacity() + sizeof(unsigned int);
    }
	void NpcTemplateConfigSource::display_error( const std::string& msg,
												 bool /*show_curline*/,
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
      ERROR_PRINT << tmp.str();
	}


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

    size_t NpcTemplateElem::estimateSize() const
    {
      size_t size = sizeof(NpcTemplateElem)
        + _source.estimateSize()
        + _elem.estimateSize();
      return size;
    }

	  

    bool FindNpcTemplate( const char* template_name, Clib::ConfigElem& elem )
	{
	  auto itr = gamestate.npc_template_elems.find( template_name );
	  if ( itr != gamestate.npc_template_elems.end() )
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
        std::string npctemplate;
        if ( !Plib::pkgdef_split( template_name, NULL, &pkg, &npctemplate ) )
		  return false;

        std::string filename = Plib::GetPackageCfgPath(const_cast<Plib::Package*>(pkg), "npcdesc.cfg");

		cf.open( filename.c_str() );
		while ( cf.read( elem ) )
		{
		  if ( !elem.type_is( "NpcTemplate" ) )
			continue;

          std::string orig_rest = elem.rest();
		  if ( pkg != NULL )
		  {
            std::string newrest = ":" + pkg->name() + ":" + npctemplate;
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
              std::string tname = elem.remove_string("TemplateName");
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
      catch (std::string& str)
	  {
        ERROR_PRINT << "NPC Creation (" << template_name << ") Failed: " << str << "\n";
	  }	   // egcs has some trouble realizing 'exception' should catch
      catch (std::runtime_error& re)   // runtime_errors, so...
	  {
        ERROR_PRINT << "NPC Creation (" << template_name << ") Failed: " << re.what( ) << "\n";
	  }
      catch (std::exception& ex)
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
        std::string filename = GetPackageCfgPath(pkg, "npcdesc.cfg");
	  if ( !Clib::FileExists( filename ) )
		return;

      Clib::ConfigFile cf( filename.c_str( ) );
      Clib::ConfigElem elem;
	  while ( cf.read( elem ) )
	  {
		if ( elem.type_is( "NpcTemplate" ) )
		{
		  // first determine the NPC template name.
            std::string namebase;
		  const char* rest = elem.rest();
		  if ( rest != NULL && *rest != '\0' )
		  {
			namebase = rest;
		  }
		  else
		  {
			namebase = elem.remove_string( "TemplateName" );
		  }
          std::string descname;
		  if ( pkg != NULL )
		  {
			descname = ":" + pkg->name() + ":" + namebase;
			elem.set_rest( descname.c_str() );
		  }
		  else
			descname = namebase;

		  gamestate.npc_template_elems[descname] = NpcTemplateElem( cf, elem );
		}
	  }
	}

	void read_npc_templates()
	{
	  gamestate.npc_template_elems.clear();
	  read_npc_templates( NULL );
      for (auto &pkg : Plib::systemstate.packages )
      {
        read_npc_templates( pkg );
      }
	}
  }
}