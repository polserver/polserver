/*
History
=======
2005/07/26 Shinigami: you can use "commands/cmdlevel" to hold textcmds in packages too

Notes
=======

*/

#include <string>

#include "../clib/stl_inc.h"
#include "../clib/cfgfile.h"
#include "../clib/cfgelem.h"
#include "../clib/dirlist.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "../bscript/impstr.h"
#include "../bscript/bstruct.h"

#include "../plib/pkg.h"

#include "cmdlevel.h"


CmdLevel::CmdLevel( ConfigElem& elem, int cmdlevelnum ) :
	name( elem.rest() ),
	cmdlevel( static_cast<unsigned char>(cmdlevelnum) )
{
	mklower(name);
	string tmp;
	while (elem.remove_prop( "DIR", &tmp ))
	{
		mklower(tmp);
		add_searchdir( NULL, normalized_dir_form(tmp) );
	}
	while (elem.remove_prop( "ALIAS", &tmp ))
	{
		mklower(tmp);
		aliases.push_back( tmp );
	}
}

bool CmdLevel::matches( const string& i_name ) const
{
	if (stringicmp( i_name, name ) == 0)
		return true;
	for( Aliases::const_iterator itr = aliases.begin(); itr != aliases.end(); ++itr )
	{
		if (stringicmp( i_name, *itr ) == 0)
			return true;
	}
	return false;
}
void CmdLevel::add_searchdir( Package* pkg, const std::string& dir )
{
	SearchDir sdir;
	sdir.pkg = pkg;
	sdir.dir = dir;
	searchlist.push_back( sdir );
}
void CmdLevel::add_searchdir_front( Package* pkg, const std::string& dir )
{
	SearchDir sdir;
	sdir.pkg = pkg;
	sdir.dir = dir;
	searchlist.insert( searchlist.begin(), sdir );
}

CmdLevels cmdlevels2;

CmdLevel* find_cmdlevel( const char* name )
{
	for( unsigned i = 0; i < cmdlevels2.size(); ++i )
	{
		if (stricmp( name, cmdlevels2[i].name.c_str() ) == 0)
			return &cmdlevels2[i];
	}
	return NULL;
}

CmdLevel* FindCmdLevelByAlias( const std::string& str )
{
    for( unsigned i = 0; i < cmdlevels2.size(); ++i )
	{
		CmdLevel* cmdlvl = &cmdlevels2[i];
		if ( cmdlvl->matches(str) )
			return cmdlvl;
	}
	return NULL;
}

ObjArray* GetCommandsInPackage(Package* m_pkg, int cmdlvl_num)
{
	if ( cmdlvl_num >= int(cmdlevels2.size()) )
			cmdlvl_num = cmdlevels2.size()-1;

	CmdLevel& cmdlevel = cmdlevels2[int(cmdlvl_num)];
	
	auto_ptr<ObjArray> script_names(new ObjArray);
	
	for( unsigned diridx = 0; diridx < cmdlevel.searchlist.size(); ++diridx )
	{	
		Package* pkg = cmdlevel.searchlist[diridx].pkg;
		string dir_name = cmdlevel.searchlist[diridx].dir;
		if ( (!pkg && m_pkg) || (pkg && !m_pkg) )
			continue;
		if ( pkg && m_pkg )
		{	
			if ( pkg != m_pkg )
				continue;
			dir_name = string(pkg->dir().c_str())+dir_name;
		}
		
		for( DirList dl(dir_name.c_str()); !dl.at_end(); dl.next() )
		{
			string name = dl.name(), ext;
			if ( name[0] == '.' ) continue;
			
			string::size_type pos = name.rfind(".");
			if ( pos != string::npos )
				ext = name.substr(pos);
		
			if ( pos != string::npos && (!ext.compare(".ecl")) )
			{
				auto_ptr<BStruct> cmdinfo (new BStruct());
				cmdinfo->addMember("dir", new String(cmdlevel.searchlist[diridx].dir));
				cmdinfo->addMember("script", new String(name.c_str()));
				script_names->addElement(cmdinfo.release());
			}
		}
	}
	if (script_names->ref_arr.size() > 0)
		return script_names.release();
	else
		return NULL;
}

void load_cmdlevels()
{
	ConfigFile cf( "config/cmds.cfg", "CmdLevel" );
	ConfigElem elem;
	
	while (cf.read( elem ))
	{
		CmdLevel cmdlevel( elem, cmdlevels2.size() );
		cmdlevels2.push_back( cmdlevel );
	}

}

void process_package_cmds_cfg( Package* pkg )
{
	//ConfigFile cf( (pkg->dir() + "cmds.cfg").c_str(), "Commands" );
	ConfigFile cf(GetPackageCfgPath(pkg, "cmds.cfg").c_str(), "Commands");
	ConfigElem elem;
	while (cf.read( elem ))
	{
		CmdLevel* cmdlevel = find_cmdlevel( elem.rest() );
		if (!cmdlevel)
		{
			elem.throw_error( string("Command Level ") + elem.rest() + " not found." );
		}

		string tmp;
		while (elem.remove_prop( "DIR", &tmp ))
		{
			mklower(tmp);
			cmdlevel->add_searchdir_front( pkg, normalized_dir_form(pkg->dir() + tmp) );
		}
	}
}

// look for a "textcmd/cmdlevel" or "commands/cmdlevel" directory for each name and alias
void implicit_package_cmds_cfg( Package* pkg )
{
	for( unsigned i = 0; i < cmdlevels2.size(); ++i )
	{
		CmdLevel& cmdlevel = cmdlevels2[i];
		std::string dir, part;
		
		// first check for the package name
		part = "textcmd/" + cmdlevel.name + "/";
		dir = pkg->dir() + part;
		if (FileExists( dir.c_str() ))
			cmdlevel.add_searchdir_front( pkg, part );

		part = "commands/" + cmdlevel.name + "/";
		dir = pkg->dir() + part;
		if (FileExists( dir.c_str() ))
			cmdlevel.add_searchdir_front( pkg, part );

		// then each alias
		for( CmdLevel::Aliases::iterator itr = cmdlevel.aliases.begin();
			 itr != cmdlevel.aliases.end();
			 ++itr)
		{
			part = "textcmd/" + *itr + "/";
			dir = pkg->dir() + part;
			if (FileExists( dir.c_str() ))
				cmdlevel.add_searchdir_front( pkg, part );

			part = "commands/" + *itr + "/";
			dir = pkg->dir() + part;
			if (FileExists( dir.c_str() ))
				cmdlevel.add_searchdir_front( pkg, part );
		}
	}
}

void load_package_cmdlevels()
{
	for( Packages::iterator itr = packages.begin(); itr != packages.end(); ++itr )
	{
		Package* pkg = (*itr);
		string filename = GetPackageCfgPath(pkg, "cmds.cfg");
		if ( FileExists(filename.c_str()) )
		{
			process_package_cmds_cfg( pkg );
		}
		else
		{
			implicit_package_cmds_cfg( pkg );
		}
	}
}

