/** @file
 *
 * @par History
 * - 2005/07/26 Shinigami: you can use "commands/cmdlevel" to hold textcmds in packages too
 */


#include "cmdlevel.h"

#include <memory>
#include <stddef.h>
#include <string>

#include "../bscript/bstruct.h"
#include "../bscript/impstr.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/dirlist.h"
#include "../clib/fileutil.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "bscript/dict.h"
#include "globals/uvars.h"
namespace Pol
{
namespace Core
{
CmdLevel::CmdLevel( Clib::ConfigElem& elem, int cmdlevelnum )
    : name( elem.rest() ), cmdlevel( static_cast<unsigned char>( cmdlevelnum ) )
{
  Clib::mklowerASCII( name );
  std::string tmp;
  while ( elem.remove_prop( "DIR", &tmp ) )
  {
    Clib::mklowerASCII( tmp );
    add_searchdir( nullptr, Clib::normalized_dir_form( tmp ) );
  }
  while ( elem.remove_prop( "ALIAS", &tmp ) )
  {
    Clib::mklowerASCII( tmp );
    aliases.push_back( tmp );
  }
}

bool CmdLevel::matches( const std::string& i_name ) const
{
  if ( Clib::stringicmp( i_name, name ) == 0 )
    return true;
  for ( Aliases::const_iterator itr = aliases.begin(); itr != aliases.end(); ++itr )
  {
    if ( Clib::stringicmp( i_name, *itr ) == 0 )
      return true;
  }
  return false;
}
void CmdLevel::add_searchdir( Plib::Package* pkg, const std::string& dir )
{
  searchlist.emplace_back( SearchDir{pkg, dir} );
}
void CmdLevel::add_searchdir_front( Plib::Package* pkg, const std::string& dir )
{
  searchlist.insert( searchlist.begin(), SearchDir{pkg, dir} );
}

size_t CmdLevel::estimateSize() const
{
  size_t size = name.capacity() + sizeof( unsigned char ); /*cmdlevel*/

  size += 3 * sizeof( SearchDir* );
  for ( const auto& ele : searchlist )
  {
    size += sizeof( Plib::Package* ) + ele.dir.capacity();
  }
  size += 3 * sizeof( std::string* );
  for ( const auto& ele : aliases )
  {
    size += ele.capacity();
  }
  return size;
}


CmdLevel* find_cmdlevel( const char* name )
{
  for ( auto& cmdlvl : gamestate.cmdlevels )
  {
    if ( stricmp( name, cmdlvl.name.c_str() ) == 0 )
      return &cmdlvl;
  }
  return nullptr;
}

CmdLevel* FindCmdLevelByAlias( const std::string& str )
{
  for ( auto& cmdlvl : gamestate.cmdlevels )
  {
    if ( cmdlvl.matches( str ) )
      return &cmdlvl;
  }
  return nullptr;
}

std::unique_ptr<Bscript::BDictionary> ListAllCommandsInPackage( Plib::Package* m_pkg,
                                                                int max_cmdlevel /*= -1*/ )
{
  auto cmd_lvl_list = std::make_unique<Bscript::BDictionary>();

  if ( max_cmdlevel < 0 )
    max_cmdlevel = static_cast<int>( Core::gamestate.cmdlevels.size() - 1 );

  for ( unsigned num = 0; num <= max_cmdlevel; ++num )
  {
    auto script_list = Core::ListCommandsInPackageAtCmdlevel( m_pkg, num );
    if ( script_list->ref_arr.empty() )
      continue;
    else
      cmd_lvl_list->addMember( new Bscript::BLong( num ), script_list.release() );
  }
  return std::move( cmd_lvl_list );
}


std::unique_ptr<Bscript::ObjArray> ListCommandsInPackageAtCmdlevel( Plib::Package* m_pkg, int cmdlvl_num )
{
  auto script_names = std::make_unique<Bscript::ObjArray>();

  if ( cmdlvl_num >= static_cast<int>( gamestate.cmdlevels.size() ) )
    cmdlvl_num = static_cast<int>( gamestate.cmdlevels.size() - 1 );

  CmdLevel& cmdlevel = gamestate.cmdlevels[cmdlvl_num];

  for ( unsigned diridx = 0; diridx < cmdlevel.searchlist.size(); ++diridx )
  {
    CmdLevel::SearchDir* search_dir = &cmdlevel.searchlist[diridx];
    Plib::Package* pkg = search_dir->pkg;
    std::string dir_name = search_dir->dir;
    if ( ( !pkg && m_pkg ) || ( pkg && !m_pkg ) )
      continue;
    if ( pkg && m_pkg )
    {
      if ( pkg != m_pkg )
        continue;
      dir_name = std::string( pkg->dir().c_str() ) + dir_name;
    }

    for ( Clib::DirList dl( dir_name.c_str() ); !dl.at_end(); dl.next() )
    {
      std::string name = dl.name(), ext;
      if ( name[0] == '.' )
        continue;

      std::string::size_type pos = name.rfind( '.' );
      if ( pos != std::string::npos )
        ext = name.substr( pos );

      if ( pos != std::string::npos && ( !ext.compare( ".ecl" ) ) )
      {
        std::unique_ptr<Bscript::BStruct> cmdinfo( new Bscript::BStruct );
        cmdinfo->addMember( "dir", new Bscript::String( search_dir->dir ) );
        cmdinfo->addMember( "script", new Bscript::String( name.c_str() ) );
        script_names->addElement( cmdinfo.release() );
      }
    }
  }

  return std::move( script_names );
}

void load_cmdlevels()
{
  Clib::ConfigFile cf( "config/cmds.cfg", "CmdLevel" );
  Clib::ConfigElem elem;

  while ( cf.read( elem ) )
  {
    CmdLevel cmdlevel( elem, static_cast<int>( gamestate.cmdlevels.size() ) );
    gamestate.cmdlevels.push_back( cmdlevel );
  }
}

void process_package_cmds_cfg( Plib::Package* pkg )
{
  // ConfigFile cf( (pkg->dir() + "cmds.cfg").c_str(), "Commands" );
  Clib::ConfigFile cf( GetPackageCfgPath( pkg, "cmds.cfg" ).c_str(), "Commands" );
  Clib::ConfigElem elem;
  while ( cf.read( elem ) )
  {
    CmdLevel* cmdlevel = find_cmdlevel( elem.rest() );
    if ( !cmdlevel )
    {
      elem.throw_error( std::string( "Command Level " ) + elem.rest() + " not found." );
    }

    std::string tmp;
    while ( elem.remove_prop( "DIR", &tmp ) )
    {
      Clib::mklowerASCII( tmp );
      cmdlevel->add_searchdir_front( pkg, Clib::normalized_dir_form( pkg->dir() + tmp ) );
    }
  }
}

// look for a "textcmd/cmdlevel" or "commands/cmdlevel" directory for each name and alias
void implicit_package_cmds_cfg( Plib::Package* pkg )
{
  for ( unsigned i = 0; i < gamestate.cmdlevels.size(); ++i )
  {
    CmdLevel& cmdlevel = gamestate.cmdlevels[i];
    std::string dir, part;

    // first check for the package name
    part = "textcmd/" + cmdlevel.name + "/";
    dir = pkg->dir() + part;
    if ( Clib::FileExists( dir.c_str() ) )
      cmdlevel.add_searchdir_front( pkg, part );

    part = "commands/" + cmdlevel.name + "/";
    dir = pkg->dir() + part;
    if ( Clib::FileExists( dir.c_str() ) )
      cmdlevel.add_searchdir_front( pkg, part );

    // then each alias
    for ( CmdLevel::Aliases::iterator itr = cmdlevel.aliases.begin(); itr != cmdlevel.aliases.end();
          ++itr )
    {
      part = "textcmd/" + *itr + "/";
      dir = pkg->dir() + part;
      if ( Clib::FileExists( dir.c_str() ) )
        cmdlevel.add_searchdir_front( pkg, part );

      part = "commands/" + *itr + "/";
      dir = pkg->dir() + part;
      if ( Clib::FileExists( dir.c_str() ) )
        cmdlevel.add_searchdir_front( pkg, part );
    }
  }
}

void load_package_cmdlevels()
{
  for ( Plib::Packages::iterator itr = Plib::systemstate.packages.begin();
        itr != Plib::systemstate.packages.end(); ++itr )
  {
    Plib::Package* pkg = ( *itr );
    std::string filename = Plib::GetPackageCfgPath( pkg, "cmds.cfg" );
    if ( Clib::FileExists( filename.c_str() ) )
    {
      process_package_cmds_cfg( pkg );
    }
    else
    {
      implicit_package_cmds_cfg( pkg );
    }
  }
}
}  // namespace Core
}  // namespace Pol
