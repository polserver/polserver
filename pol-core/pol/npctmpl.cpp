/*
History
=======


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

struct TRANSLATION
{
	const char * name;
	int value;
};

int translate( const std::string& name, TRANSLATION* table )
{
	for( int i = 0; table[i].name; ++i )
	{
		if (table[i].name == name)
			return table[i].value;
	}
	cerr << "Unable to translate value '" << name << "'" << endl;
	cerr << "  Expected one of the following values:" << endl;
	for( int i = 0; table[i].name; ++i )
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


NpcTemplate::NpcTemplate( const ConfigElem& elem, const Package* pkg ) :
	intrinsic_weapon( find_intrinsic_weapon(elem.rest()) ),
	pkg( pkg ),
	// script( elem.read_string( "SCRIPT" ) ),
	alignment( static_cast<ALIGNMENT>(translate(elem.read_string( "ALIGNMENT", "neutral" ), xlate_align )) )
{
	if (pkg == NULL)
	{
		name = elem.rest();
	}
	else
	{
		if (elem.rest()[0] == ':')
		{
			name = elem.rest();
		}
		else
		{
			name = ":" + pkg->name() + ":" + elem.rest();
		}
		
	}
}

NpcTemplates npc_templates;

const NpcTemplate& create_npc_template( const ConfigElem& elem, const Package* pkg )
{
	NpcTemplate* tmpl = new NpcTemplate( elem, pkg );
	npc_templates[ tmpl->name ] = tmpl;
	return *tmpl;
}

void load_npc_templates()
{
	if ( FileExists("config/npcdesc.cfg") )
	{
		ConfigFile cf("config/npcdesc.cfg");
		ConfigElem elem;
		while ( cf.read(elem) )
		{
			create_npc_template(elem, NULL);
		}
	}

	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		string filename = GetPackageCfgPath(pkg, "npcdesc.cfg");
		
		if ( FileExists(filename.c_str()) )
		{
			ConfigFile cf(filename.c_str());
			ConfigElem elem;
			while ( cf.read(elem) )
			{
				create_npc_template(elem, pkg);
			}
		}
	}
}

const NpcTemplate& find_npc_template( const ConfigElem& elem )
{
	NpcTemplates::const_iterator itr = npc_templates.find( elem.rest() );
	if (itr != npc_templates.end())
	{
		return *((*itr).second);
	}
	else
	{
		const Package* pkg;
		string path;
		if (pkgdef_split( elem.rest(), NULL, &pkg, &path ))
		{
			return create_npc_template( elem, pkg );
		}
		else
		{
			throw runtime_error( string("Error reading NPC template name ") + elem.rest() );
		}
	}
}

//quick and nasty fix until npcdesc usage is rewritten
void unload_npc_templates()
{
    NpcTemplates::iterator iter = npc_templates.begin();
	for ( ; iter != npc_templates.end(); iter++) {
		if (iter->second != NULL)
			delete iter->second;
		iter->second = NULL;
	}
	npc_templates.clear();
}
