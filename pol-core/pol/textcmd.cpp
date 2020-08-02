/** @file
 *
 * @par History
 * - 2006/06/15 Austin:    Removed .set .priv and .priv
 * - 2006/05/30 Shinigami: fixed a smaller mistype in textcmd_startlog()
 *                         set correct time stamp in textcmd_startlog() and textcmd_stoplog()
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 * - 2009/09/10 MuadDib:   Cleanup of depreciated internal text commands.
 * - 2009/10/17 Turley:    check for priv "plogany" enabled instead of existence - Tomi
 */


#include "textcmd.h"

#include <cstddef>
#include <ctype.h>
#include <iosfwd>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>

#include "../bscript/berror.h"
#include "../bscript/impstr.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/clib_endian.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/opnew.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/spinlock.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"
#include "../plib/clidata.h"
#include "../plib/pkg.h"
#include "../plib/systemstate.h"
#include "../plib/uconst.h"
#include "accounts/account.h"
#include "allocd.h"
#include "globals/network.h"
#include "globals/state.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "mobile/charactr.h"
#include "module/uomod.h"
#include "network/client.h"
#include "network/pktboth.h"
#include "polclock.h"
#include "repsys.h"
#include "scrdef.h"
#include "scrsched.h"
#include "scrstore.h"
#include "ufunc.h"
#include "ufuncstd.h"
#include "uoexec.h"
#include "uoscrobj.h"
#include "utype.h"
#include "uworld.h"

namespace Pol
{
namespace Core
{
bool wordicmp::operator()( const std::string& lhs, const std::string& rhs ) const
{
  size_t len = std::min( lhs.size(), rhs.size() );

  return ( strnicmp( lhs.c_str(), rhs.c_str(), len ) < 0 );
}


void register_command( const std::string& cmd, TextCmdFunc f )
{
  gamestate.textcmds.insert( TextCmds::value_type( cmd, f ) );
}
void register_command( const std::string& cmd, ParamTextCmdFunc f )
{
  gamestate.paramtextcmds.insert( ParamTextCmds::value_type( cmd, f ) );
}

bool FindEquipTemplate( const std::string& template_name, Clib::ConfigElem& elem )
{
  try
  {
    Clib::ConfigFile cf( "config/equip.cfg" );
    while ( cf.read( elem ) )
    {
      if ( !elem.type_is( "Equipment" ) )
        continue;
      const char* rest = elem.rest();
      if ( rest == nullptr || *rest == '\0' )
        continue;
      if ( stricmp( rest, template_name.c_str() ) == 0 )
        return true;
    }
    return false;
  }
  catch ( ... )
  {
    return false;
  }
}

Bscript::BObjectImp* equip_from_template( Mobile::Character* chr, const std::string& template_name )
{
  Clib::ConfigElem elem;
  if ( !FindEquipTemplate( template_name, elem ) )
  {
    return new Bscript::BError( "Equipment template not found" );
  }

  std::string tmp;
  while ( elem.remove_prop( "Equip", &tmp ) || elem.remove_prop( "Weapon", &tmp ) ||
          elem.remove_prop( "Armor", &tmp ) )
  {
    ISTRINGSTREAM is( tmp );
    std::string objtype_str;
    if ( is >> objtype_str )
    {
      unsigned int objtype;
      const char* ot_str = objtype_str.c_str();
      if ( isdigit( *ot_str ) )
      {
        objtype = static_cast<u32>( strtoul( ot_str, nullptr, 0 ) );
      }
      else
      {
        objtype = Items::get_objtype_byname( ot_str );
        if ( !objtype )
        {
          ERROR_PRINT << "Blech! Can't find '" << ot_str << "' named in equip.cfg\n";
          continue;
        }
      }
      std::string color_str;
      unsigned short color = 0;
      if ( is >> color_str )
      {
        color = static_cast<unsigned short>( strtoul( color_str.c_str(), nullptr, 0 ) );
      }
      Items::Item* it = Items::Item::create( objtype );
      if ( it != nullptr )
      {
        color &= Plib::VALID_ITEM_COLOR_MASK;
        it->color = color;
        it->layer = Plib::tilelayer( it->graphic );
        it->realm = chr->realm;
        // FIXME equip scripts, equiptest scripts
        if ( chr->equippable( it ) )
        {
          chr->equip( it );
          update_item_to_inrange( it );
        }
        else
        {
          it->destroy();
        }
      }
    }
  }
  return new Bscript::BLong( 1 );
}

// FIXME should do a command handler table like for messages received.
void send_client_char_data( Mobile::Character* chr, Network::Client* client );

void send_move_if_inrange2( Mobile::Character* chr, Network::Client* client )
{
  if ( client->ready && client->chr && client->chr != chr && inrange( client->chr, chr ) )
  {
    send_move( client, chr );
  }
}

void textcmd_resendchars( Network::Client* client )
{
  Core::WorldIterator<Core::MobileFilter>::InVisualRange(
      client->chr,
      [&]( Mobile::Character* zonechr ) { send_client_char_data( zonechr, client ); } );
}

void textcmd_shutdown( Network::Client* /*client*/ )
{
  Clib::signal_exit();
}

void handle_ident_cursor( Mobile::Character* chr, PKTBI_6C* msgin )
{
  if ( chr->client != nullptr )
  {
    char s[80];
    sprintf( s, "Serial: 0x%8.08lX, %ld, ObjType 0x%4.04X",
             static_cast<unsigned long>( cfBEu32( msgin->selected_serial ) ),
             static_cast<signed long>( cfBEu32( msgin->selected_serial ) ),
             cfBEu16( msgin->graphic ) );
    send_sysmessage( chr->client, s );
  }
}

void textcmd_ident( Network::Client* client )
{
  send_sysmessage( client, "Select something to identify." );
  gamestate.target_cursors.ident_cursor.send_object_cursor( client );
}

void textcmd_listarmor( Network::Client* client )
{
  client->chr->showarmor();
}


std::string timeoutstr( polclock_t at )
{
  polclock_t ticks = at - polclock();
  int seconds = ticks / POLCLOCKS_PER_SEC;
  return Clib::tostring( seconds ) + " seconds";
}

///
/// Internal Command: .i_repdata
/// Show Reputation System Data for a Targetted Mobile
/// Displays:
///  Murderer status
///  Criminal status and timeout
///  LawfullyDamaged status and timeouts
///  AggressorTo status and timeouts
///  ToBeReportable list
///  Reportable list
///  RepSystem Task status
///
void RepSystem::show_repdata( Network::Client* client, Mobile::Character* mob )
{
  if ( mob->is_murderer() )
  {
    send_sysmessage( client, "Mobile is a murderer." );
  }
  else if ( mob->is_criminal() )
  {
    send_sysmessage( client, "Mobile is criminal for " + timeoutstr( mob->criminal_until_ ) + " [" +
                                 Clib::tostring( mob->criminal_until_ ) + "]" );
  }

  for ( Mobile::Character::MobileCont::const_iterator itr = mob->aggressor_to_.begin();
        itr != mob->aggressor_to_.end(); ++itr )
  {
    send_sysmessage( client, "Aggressor to " + ( *itr ).first->name() + " for " +
                                 timeoutstr( ( *itr ).second ) + " [" +
                                 Clib::tostring( ( *itr ).second ) + "]" );
  }

  for ( Mobile::Character::MobileCont::const_iterator itr = mob->lawfully_damaged_.begin();
        itr != mob->lawfully_damaged_.end(); ++itr )
  {
    send_sysmessage( client, "Lawfully Damaged " + ( *itr ).first->name() + " for " +
                                 timeoutstr( ( *itr ).second ) + " [" +
                                 Clib::tostring( ( *itr ).second ) + "]" );
  }

  for ( Mobile::Character::ToBeReportableList::const_iterator itr = mob->to_be_reportable_.begin();
        itr != mob->to_be_reportable_.end(); ++itr )
  {
    USERIAL serial = ( *itr );
    send_sysmessage( client, "ToBeReportable: " + Clib::hexint( serial ) );
  }

  for ( Mobile::Character::ReportableList::const_iterator itr = mob->reportable_.begin();
        itr != mob->reportable_.end(); ++itr )
  {
    const Mobile::reportable_t& rt = ( *itr );
    send_sysmessage( client, "Reportable: " + Clib::hexint( rt.serial ) + " at " +
                                 Clib::tostring( rt.polclock ) );
  }

  if ( mob->repsys_task_ != nullptr )
    send_sysmessage( client, "Repsys task is active, runs in " +
                                 timeoutstr( mob->repsys_task_->next_run_clock() ) + " [" +
                                 Clib::tostring( mob->repsys_task_->next_run_clock() ) + "]" );
}

void show_repdata( Mobile::Character* looker, Mobile::Character* mob )
{
  RepSystem::show_repdata( looker->client, mob );
}


void textcmd_repdata( Network::Client* client )
{
  send_sysmessage( client, "Please target a mobile to display repdata for." );
  gamestate.target_cursors.repdata_cursor.send_object_cursor( client );
}

void start_packetlog( Mobile::Character* looker, Mobile::Character* mob )
{
  if ( mob->connected() )  // gotta be connected to get packets right?
  {
    Clib::SpinLockGuard guard( mob->client->_fpLog_lock );
    if ( mob->client->fpLog.empty() )
    {
      std::string filename = "log/";
      filename += mob->client->acct->name();
      filename += ".log";
      mob->client->fpLog = OPEN_FLEXLOG( filename, true );
      if ( !mob->client->fpLog.empty() )
      {
        send_sysmessage( looker->client, "I/O log file opened for " + mob->name() );
      }
      else
      {
        send_sysmessage( looker->client, "Unable to open I/O log file for " + mob->name() );
      }
    }
  }
}

void textcmd_startlog( Network::Client* client )
{
  if ( client->chr->can_plogany() )
  {
    send_sysmessage( client, "Please target a player to start packet logging for." );
    gamestate.target_cursors.startlog_cursor.send_object_cursor( client );
  }
  else
  {
    Clib::SpinLockGuard guard( client->_fpLog_lock );
    if ( client->fpLog.empty() )
    {
      std::string filename = "log/";
      filename += client->acct->name();
      filename += ".log";
      client->fpLog = OPEN_FLEXLOG( filename, true );
      if ( !client->fpLog.empty() )
      {
        send_sysmessage( client, "I/O log file opened." );
      }
      else
      {
        send_sysmessage( client, "Unable to open I/O log file." );
      }
    }
  }
}

void stop_packetlog( Mobile::Character* looker, Mobile::Character* mob )
{
  if ( mob->connected() )  // gotta be connected to already have packets right?
  {
    Clib::SpinLockGuard guard( mob->client->_fpLog_lock );
    if ( !mob->client->fpLog.empty() )
    {
      auto time_tm = Clib::localtime( time( nullptr ) );
      FLEXLOG( mob->client->fpLog ) << "Log closed at %s" << asctime( &time_tm ) << "\n";
      CLOSE_FLEXLOG( mob->client->fpLog );
      mob->client->fpLog.clear();
      send_sysmessage( looker->client, "I/O log file closed for " + mob->name() );
    }
    else
    {
      send_sysmessage( looker->client, "Packet Logging not enabled for " + mob->name() );
    }
  }
}

void textcmd_stoplog( Network::Client* client )
{
  if ( client->chr->can_plogany() )
  {
    send_sysmessage( client, "Please target a player to stop packet logging for." );
    gamestate.target_cursors.stoplog_cursor.send_object_cursor( client );
  }
  else
  {
    Clib::SpinLockGuard guard( client->_fpLog_lock );
    if ( !client->fpLog.empty() )
    {
      auto time_tm = Clib::localtime( time( nullptr ) );
      FLEXLOG( client->fpLog ) << "Log closed at %s" << asctime( &time_tm ) << "\n";
      CLOSE_FLEXLOG( client->fpLog );
      client->fpLog.clear();
      send_sysmessage( client, "I/O log file closed." );
    }
    else
    {
      send_sysmessage( client, "Packet Logging not enabled." );
    }
  }
}

void textcmd_orphans( Network::Client* client )
{
  OSTRINGSTREAM os;
  os << "Unreaped orphans: " << stateManager.uobjcount.unreaped_orphans;

  send_sysmessage( client, OSTRINGSTREAM_STR( os ) );

  OSTRINGSTREAM os2;
  os2 << "EChrRef count: " << stateManager.uobjcount.uobj_count_echrref;
  send_sysmessage( client, OSTRINGSTREAM_STR( os2 ) );
}

void list_scripts();
void textcmd_list_scripts( Network::Client* )
{
  list_scripts();
}
void list_crit_scripts();
void textcmd_list_crit_scripts( Network::Client* )
{
  list_crit_scripts();
}
void textcmd_procs( Network::Client* client )
{
  send_sysmessage( client, "Process Information:" );

  send_sysmessage(
      client,
      "Running: " + Clib::tostring( (unsigned int)( scriptScheduler.getRunlist().size() ) ) );
  send_sysmessage(
      client,
      "Blocked: " + Clib::tostring( (unsigned int)( scriptScheduler.getHoldlist().size() ) ) );
}

void textcmd_log_profile( Network::Client* client )
{
  log_all_script_cycle_counts( false );
  send_sysmessage( client, "Script profile written to logfile" );
}

void textcmd_log_profile_clear( Network::Client* client )
{
  log_all_script_cycle_counts( true );
  send_sysmessage( client, "Script profile written to logfile and cleared" );
}

void textcmd_heapcheck( Network::Client* /*client*/ )
{
  PrintAllocationData();
  Clib::PrintHeapData();
}

void textcmd_threads( Network::Client* client )
{
  std::string s = "Child threads: " + Clib::tostring( threadhelp::child_threads );
  send_sysmessage( client, s );
}

void textcmd_constat( Network::Client* client )
{
  int i = 0;
  send_sysmessage( client, "Connection statuses:" );
  for ( Clients::const_iterator itr = networkManager.clients.begin(),
                                end = networkManager.clients.end();
        itr != end; ++itr )
  {
    OSTRINGSTREAM os;
    os << i << ": " << ( *itr )->status() << " ";
    send_sysmessage( client, OSTRINGSTREAM_STR( os ) );
    ++i;
  }
}


bool check_single_zone_item_integrity( int, int, Realms::Realm* );
void textcmd_singlezone_integ_item( Network::Client* client )
{
  unsigned short wx, wy;
  zone_convert( client->chr->x, client->chr->y, &wx, &wy, client->chr->realm );
  bool ok = check_single_zone_item_integrity( wx, wy, client->chr->realm );
  if ( ok )
    send_sysmessage( client, "Item integrity checks out OK!" );
  else
    send_sysmessage( client, "Item integrity problems detected. " );
}

bool check_item_integrity();
void textcmd_integ_item( Network::Client* client )
{
  bool ok = check_item_integrity();
  if ( ok )
    send_sysmessage( client, "Item integrity checks out OK!" );
  else
    send_sysmessage( client, "Item integrity problems detected.  Check logfile" );
}
void check_character_integrity();
void textcmd_integ_chr( Network::Client* /*client*/ )
{
  check_character_integrity();
}

std::string get_textcmd_help( Mobile::Character* chr, const std::string& cmd )
{
  /* const char* t = cmd;
   while ( *t )
   {
     if ( !isalpha( *t ) && ( *t != '_' ) )
     {
       // cout << "illegal command char: as int = " << int(*t) << ", as char = " << *t << endl;
       return "";
     }
     ++t;
   }
 */
  std::string upp( cmd );
  Clib::mkupperASCII( upp );
  if ( upp == "AUX" || upp == "CON" || upp == "PRN" || upp == "NUL" )
    return "";

  for ( int i = chr->cmdlevel(); i >= 0; --i )
  {
    CmdLevel& cmdlevel = gamestate.cmdlevels[i];
    for ( unsigned diridx = 0; diridx < cmdlevel.searchlist.size(); ++diridx )
    {
      std::string filename;

      Plib::Package* pkg = cmdlevel.searchlist[diridx].pkg;
      filename = cmdlevel.searchlist[diridx].dir + cmd + std::string( ".txt" );
      if ( pkg )
        filename = pkg->dir() + filename;

      if ( Clib::FileExists( filename.c_str() ) )
      {
        std::string result;
        std::ifstream ifs( filename.c_str(), std::ios::binary );
        char temp[64];
        do
        {
          ifs.read( temp, sizeof temp );
          if ( ifs.gcount() )
            result.append( temp, static_cast<size_t>( ifs.gcount() ) );
        } while ( !ifs.eof() );
        return result;
      }
    }
  }
  return "";
}

bool start_textcmd_script( Network::Client* client, const std::string& text,
                           const std::string& lang )
{
  std::string scriptname;
  std::string params;
  size_t pos = text.find_first_of( ' ' );
  if ( pos != std::string::npos )
  {
    scriptname = std::string( text, 0, pos );
    params = text.substr( pos + 1 );
  }
  else
  {
    scriptname = text;
    params = "";
  }

  // early out test for invalid scriptnames
  if ( scriptname.find_first_of( "*\"':;!?#&-+()/\\=" ) != std::string::npos )
    return false;

  std::string upp( scriptname );
  Clib::mkupperASCII( upp );
  if ( upp == "AUX" || upp == "CON" || upp == "PRN" || upp == "NUL" )
    return false;

  for ( int i = client->chr->cmdlevel(); i >= 0; --i )
  {
    CmdLevel& cmdlevel = gamestate.cmdlevels[i];
    for ( unsigned diridx = 0; diridx < cmdlevel.searchlist.size(); ++diridx )
    {
      ScriptDef sd;
      Plib::Package* pkg = cmdlevel.searchlist[diridx].pkg;
      if ( pkg )
        sd.quickconfig( pkg, cmdlevel.searchlist[diridx].dir + scriptname + ".ecl" );
      else
        sd.quickconfig( cmdlevel.searchlist[diridx].dir + scriptname + ".ecl" );
      if ( !sd.exists() )
        continue;

      ref_ptr<Bscript::EScriptProgram> prog =
          find_script2( sd,
                        false,  // don't complain if not found
                        Plib::systemstate.config.cache_interactive_scripts );
      if ( prog.get() != nullptr )
      {
        std::unique_ptr<UOExecutor> ex( create_script_executor() );
        if ( prog->haveProgram )
        {
          if ( !lang.empty() )
            ex->pushArg( new Bscript::String( lang ) );
          ex->pushArg( new Bscript::String( params ) );
          ex->pushArg( new Module::ECharacterRefObjImp( client->chr ) );
        }

        Module::UOExecutorModule* uoemod = new Module::UOExecutorModule( *ex );
        ex->addModule( uoemod );
        ex->priority( 100 );

        if ( ex->setProgram( prog.get() ) )
        {
          uoemod->controller_.set( client->chr );  // DAVE added 12/04, let character issuing
                                                   // textcmd be the script controller
          schedule_executor( ex.release() );
          return true;
        }
        else
        {
          ERROR_PRINT << "script " << scriptname << ": setProgram failed\n";
          // TODO: it seems to keep looking until it finds one it can use..guess this is okay?
        }
      }
    }
  }
  return false;
}

bool process_command( Network::Client* client, const std::string& text,
                      const std::string& lang /*""*/ )
{
  static int init;
  if ( !init )
  {
    init = 1;
    register_command( "armor", textcmd_listarmor );
    register_command( "constat", textcmd_constat );
    register_command( "heapcheck", &textcmd_heapcheck );
    register_command( "i_repdata", textcmd_repdata );
    register_command( "t_ident", textcmd_ident );
    register_command( "integ_item", textcmd_integ_item );
    register_command( "integ_chr", textcmd_integ_chr );
    register_command( "i_s_item_integ", textcmd_singlezone_integ_item );  // davedebug
    register_command( "list_crit", &textcmd_list_crit_scripts );
    register_command( "list_scripts", &textcmd_list_scripts );
    register_command( "log_profile", &textcmd_log_profile );
    register_command( "log_profile_clear", &textcmd_log_profile_clear );
    register_command( "orphans", &textcmd_orphans );
    register_command( "procs", &textcmd_procs );
    register_command( "resendchars", &textcmd_resendchars );
    register_command( "shutdown", &textcmd_shutdown );
    register_command( "startlog", &textcmd_startlog );
    register_command( "stoplog", &textcmd_stoplog );
    register_command( "threads", &textcmd_threads );
  }

  std::string cmd = text.substr( 1 );  // skip the "/" or "."

  if ( start_textcmd_script( client, cmd, lang ) )
    return true;

  if ( client->chr->cmdlevel() >= gamestate.cmdlevels.size() - 2 )
  {
    TextCmds::iterator itr2 = gamestate.textcmds.find( cmd );
    if ( itr2 != gamestate.textcmds.end() )
    {
      TextCmdFunc f = ( *itr2 ).second;
      ( *f )( client );
      return true;
    }

    ParamTextCmds::iterator itr1 = gamestate.paramtextcmds.find( cmd );
    if ( itr1 != gamestate.paramtextcmds.end() )
    {
      ParamTextCmdFunc f = ( *itr1 ).second;

      ( *f )( client, cmd.substr( ( *itr1 ).first.size() + 1 ) );
      return true;
    }
  }

  return false;
}
}  // namespace Core
}  // namespace Pol
