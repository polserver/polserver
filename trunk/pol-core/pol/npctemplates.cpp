/*
History
=======

2006/05/10 MuadDib:   Removed unload_npc_templates for non use.
2009/08/25 Shinigami: STLport-5.2.1 fix: elem->type() check will use strlen() now

Notes
=======

*/


#include "clib/stl_inc.h"

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/fileutil.h"
#include "clib/stlutil.h"

#include "plib/pkg.h"

class NpcTemplateConfigSource : public ConfigSource
{
public:
    NpcTemplateConfigSource();
    NpcTemplateConfigSource( const ConfigFile& cf );
    virtual void display_error( const std::string& msg, 
                        bool show_curline = true,
                        const ConfigElemBase* elem = NULL,
                        bool error = true ) const;
private:
    string _filename;
    unsigned _fileline;
};
NpcTemplateConfigSource::NpcTemplateConfigSource( const ConfigFile& cf ) :
    _filename( cf.filename() ),
    _fileline( cf.element_line_start() )
{
};
NpcTemplateConfigSource::NpcTemplateConfigSource()
{
}
void NpcTemplateConfigSource::display_error( const std::string& msg, 
                        bool show_curline,
                        const ConfigElemBase* elem,
                        bool error ) const
{
    bool showed_elem_line = false;

    cerr << (error?"Error":"Warning")
         << " reading configuration file " << _filename << ":" << endl;

    cerr << "\t" << msg << endl;
    
    if (elem != NULL)
    {
        if (strlen(elem->type()) > 0)
        {
            cerr << "\tElement: " << elem->type() << " " << elem->rest();
            if (_fileline)
                cerr << ", found on line " << _fileline;
            cerr << endl;
            showed_elem_line = true;
        }
    }

    //if (show_curline)
    //    cerr << "\tNear line: " << _cur_line << endl;
    if (_fileline && !showed_elem_line)
        cerr << "\tElement started on line: " << _fileline << endl;
}


class NpcTemplateElem
{
public:
    NpcTemplateElem();
    NpcTemplateElem( const ConfigFile& cf, const ConfigElem& elem );
    
    void copyto( ConfigElem& elem ) const;
private:
    NpcTemplateConfigSource _source;
    ConfigElem _elem;
};
NpcTemplateElem::NpcTemplateElem()
{
}
NpcTemplateElem::NpcTemplateElem( const ConfigFile& cf, const ConfigElem& elem ) :
    _source( cf ),
    _elem( elem )
{
}

void NpcTemplateElem::copyto( ConfigElem& elem ) const
{
    elem = _elem;
    elem.set_source( &_source );
}

typedef map< string, NpcTemplateElem, ci_cmp_pred > NpcTemplates;
static NpcTemplates npc_templates;

bool FindNpcTemplate( const char* template_name, ConfigElem& elem )
{
    NpcTemplates::const_iterator itr = npc_templates.find( template_name );
    if (itr != npc_templates.end())
    {
        const NpcTemplateElem& tm = (*itr).second;
        tm.copyto( elem );
        return true;
    }
    else
    {
        return false;
    }
}

// FIXME inefficient.  Templates should be read in once, and reused.
bool FindNpcTemplate( const char *template_name, ConfigFile& cf, ConfigElem& elem )
{
	try {
		const Package* pkg;
		string npctemplate;
		if ( !pkgdef_split(template_name, NULL, &pkg, &npctemplate) )
			return false;

		string filename = GetPackageCfgPath(const_cast<Package*>(pkg), "npcdesc.cfg");

		cf.open( filename.c_str() );
		while (cf.read( elem ))
		{
			if (!elem.type_is( "NpcTemplate" ))
				continue;

			string orig_rest = elem.rest();
			if (pkg != NULL)
			{
				string newrest = ":" + pkg->name() + ":" + npctemplate;
				elem.set_rest( newrest.c_str() );
			}
			const char* rest = elem.rest();
			if (rest != NULL && *rest != '\0')
			{
				if (stricmp( orig_rest.c_str(), npctemplate.c_str() ) == 0)
					return true;
			}
			else
			{
				string tname = elem.remove_string( "TemplateName" );
				if (stricmp( tname.c_str(), npctemplate.c_str() ) == 0)
					return true;
			}
		}
		return false;
	}
	catch( const char *msg )
	{
		cerr << "NPC Creation (" << template_name << ") Failed: " << msg << endl;
	}
	catch( string& str )
	{
		cerr << "NPC Creation (" << template_name << ") Failed: " << str << endl;
	}	   // egcs has some trouble realizing 'exception' should catch
	catch( runtime_error& re )   // runtime_errors, so...
	{
		cerr << "NPC Creation (" << template_name << ") Failed: " << re.what() << endl;
	}
	catch( exception& ex )
	{
		cerr << "NPC Creation (" << template_name << ") Failed: " << ex.what() << endl;
	}
#ifndef WIN32
	catch(...)
	{
	}
#endif
	return false;
}

void read_npc_templates( Package* pkg )
{
    string filename = GetPackageCfgPath(pkg, "npcdesc.cfg");
	if ( !FileExists(filename) )
        return;

    ConfigFile cf( filename.c_str() );
    ConfigElem elem;
    while (cf.read( elem ))
    {
        if (elem.type_is( "NpcTemplate" ))
        {
            // first determine the NPC template name.
            string namebase;
            const char* rest = elem.rest();
            if (rest != NULL && *rest != '\0')
            {
                namebase = rest;
            }
            else
            {
                namebase = elem.remove_string( "TemplateName" );
            }
            string descname;
            if (pkg != NULL)
            {
                descname = ":" + pkg->name() + ":" + namebase;
                elem.set_rest( descname.c_str() );
            }
            else
                descname = namebase;

            npc_templates[ descname ] = NpcTemplateElem( cf, elem );
        }
    }
}

void read_npc_templates()
{
    npc_templates.clear();
    read_npc_templates( NULL );
    ForEach( packages, read_npc_templates );
}
