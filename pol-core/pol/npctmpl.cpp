/*
History
=======
2010/02/03 Turley:    MethodScript support for mobiles


Notes
=======

*/

#include "../clib/stl_inc.h"


#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"

#include "npctmpl.h"
#include "../plib/pkg.h"
#include "item/weapon.h"
#include "syshookscript.h"
namespace Pol {
  namespace Core {
	struct TRANSLATION
	{
	  const char * name;
	  int value;
	};

	int translate( const std::string& name, TRANSLATION* table )
	{
	  for ( int i = 0; table[i].name; ++i )
	  {
		if ( table[i].name == name )
		  return table[i].value;
	  }
	  cerr << "Unable to translate value '" << name << "'" << endl;
	  cerr << "  Expected one of the following values:" << endl;
	  for ( int i = 0; table[i].name; ++i )
	  {
		cerr << "	" << table[i].name << endl;
	  }
	  throw runtime_error( "Unable to translate value" );
	  return 0;
	}

	TRANSLATION xlate_align[] =
	{
	  { "good", NpcTemplate::GOOD },
	  { "neutral", NpcTemplate::NEUTRAL },
	  { "evil", NpcTemplate::EVIL },
	  { 0, 0 }
	};


	NpcTemplate::NpcTemplate( const Clib::ConfigElem& elem, const Plib::Package* pkg ) :
	  intrinsic_weapon( Items::find_intrinsic_weapon( elem.rest() ) ),
	  pkg( pkg ),
	  // script( elem.read_string( "SCRIPT" ) ),
	  alignment( static_cast<ALIGNMENT>( translate( elem.read_string( "ALIGNMENT", "neutral" ), xlate_align ) ) ),
	  method_script( NULL )
	{
	  if ( pkg == NULL )
	  {
		name = elem.rest();
	  }
	  else
	  {
		if ( elem.rest()[0] == ':' )
		{
		  name = elem.rest();
		}
		else
		{
		  name = ":" + pkg->name() + ":" + elem.rest();
		}
	  }

	  if ( elem.has_prop( "MethodScript" ) )
	  {
		std::string temp = elem.read_string( "MethodScript" );
		if ( !temp.empty() )
		{
		  ExportScript* shs = new ExportScript( pkg, temp );
		  if ( shs->Initialize() )
			method_script = shs;
		  else
			delete shs;
		}
	  }
	}

	NpcTemplate::~NpcTemplate()
	{
	  if ( method_script != NULL )
	  {
		delete method_script;
		method_script = NULL;
	  }
	}

	NpcTemplates npc_templates;

	const NpcTemplate& create_npc_template( const Clib::ConfigElem& elem, const Plib::Package* pkg )
	{
	  NpcTemplate* tmpl = new NpcTemplate( elem, pkg );
	  npc_templates[tmpl->name] = tmpl;
	  return *tmpl;
	}

	void load_npc_templates()
	{
	  if ( Clib::FileExists( "config/npcdesc.cfg" ) )
	  {
        Clib::ConfigFile cf( "config/npcdesc.cfg" );
        Clib::ConfigElem elem;
		while ( cf.read( elem ) )
		{
		  create_npc_template( elem, NULL );
		}
	  }

      for ( Plib::Packages::iterator itr = Plib::packages.begin( ); itr != Plib::packages.end( ); ++itr )
	  {
        Plib::Package* pkg = ( *itr );
        string filename = Plib::GetPackageCfgPath( pkg, "npcdesc.cfg" );

        if ( Clib::FileExists( filename.c_str( ) ) )
		{
          Clib::ConfigFile cf( filename.c_str( ) );
          Clib::ConfigElem elem;
		  while ( cf.read( elem ) )
		  {
			create_npc_template( elem, pkg );
		  }
		}
	  }
	}

    const NpcTemplate& find_npc_template( const Clib::ConfigElem& elem )
	{
	  NpcTemplates::const_iterator itr = npc_templates.find( elem.rest() );
	  if ( itr != npc_templates.end() )
	  {
		return *( ( *itr ).second );
	  }
	  else
	  {
        const Plib::Package* pkg;
		string path;
        if ( Plib::pkgdef_split( elem.rest( ), NULL, &pkg, &path ) )
		{
		  return create_npc_template( elem, pkg );
		}
		else
		{
		  throw runtime_error( string( "Error reading NPC template name " ) + elem.rest() );
		}
	  }
	}

	//quick and nasty fix until npcdesc usage is rewritten
	void unload_npc_templates()
	{
	  NpcTemplates::iterator iter = npc_templates.begin();
	  for ( ; iter != npc_templates.end(); ++iter )
	  {
		if ( iter->second != NULL )
		  delete iter->second;
		iter->second = NULL;
	  }
	  npc_templates.clear();
	}
  }
}
