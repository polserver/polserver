/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2009/07/27 MuadDib:   Pakcet struct refactoring
 */


#include <string.h>
#include <string>

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/clib.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"
#include "action.h"
#include "globals/uvars.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packetdefs.h"
#include "network/pktin.h"
#include "uworld.h"


namespace Pol::Core
{
MobileTranslate::OldAnimDef::OldAnimDef()
    : valid( false ),
      action( 0 ),
      framecount( 0 ),
      repeatcount( 0 ),
      backward( 0 ),
      repeatflag( 0 ),
      delay( 0 )
{
}
MobileTranslate::NewAnimDef::NewAnimDef() : valid( false ), anim( 0 ), action( 0 ), subaction( 0 )
{
}
MobileTranslate::MobileTranslate() : graphics(), supports_mount( false )
{
  memset( &old_anim, 0, sizeof( old_anim ) );
  memset( &new_anim, 0, sizeof( new_anim ) );
}
bool MobileTranslate::has_graphic( u16 graphic ) const
{
  return std::find( graphics.begin(), graphics.end(), graphic ) != graphics.end();
}

size_t MobileTranslate::estimateSize() const
{
  return sizeof( MobileTranslate ) + Clib::memsize( graphics );
}


UACTION str_to_action( Clib::ConfigElem& elem, const std::string& str )
{
  unsigned short tmp = static_cast<unsigned short>( strtoul( str.c_str(), nullptr, 0 ) );

  if ( UACTION_IS_VALID( tmp ) )
  {
    return static_cast<UACTION>( tmp );
  }
  else
  {
    elem.throw_error( "Animation value of " + str + " is out of range" );
  }
}

void load_anim_xlate_cfg( bool /*reload*/ )
{
  memset( &Core::gamestate.mount_action_xlate, 0, sizeof( Core::gamestate.mount_action_xlate ) );
  Core::gamestate.animation_translates.clear();

  if ( Clib::FileExists( "config/animxlate.cfg" ) )
  {
    Clib::ConfigFile cf( "config/animxlate.cfg", "OnMount MobileType" );
    Clib::ConfigElem elem;
    while ( cf.read( elem ) )
    {
      if ( elem.type_is( "OnMount" ) )
      {
        std::string from_str, to_str;
        while ( elem.remove_first_prop( &from_str, &to_str ) )
        {
          UACTION from = str_to_action( elem, from_str );
          UACTION to = str_to_action( elem, to_str );
          Core::gamestate.mount_action_xlate[from] = to;
        }
      }
      else if ( elem.type_is( "MobileType" ) )
      {
        MobileTranslate mobiletype;
        std::string key, value;
        while ( elem.has_prop( "Graphic" ) )
        {
          mobiletype.graphics.push_back( elem.remove_ushort( "Graphic" ) );
        }
        std::sort( mobiletype.graphics.begin(), mobiletype.graphics.end() );
        mobiletype.supports_mount = elem.remove_bool( "MountTranslation", false );

        auto split_str = []( const std::string& source ) -> std::vector<std::string>
        {
          ISTRINGSTREAM is( source );
          std::string tmp;
          std::vector<std::string> result;
          while ( is >> tmp )
          {
            if ( tmp.empty() )
              continue;
            if ( tmp.at( 0 ) == '#' )
              break;
            result.push_back( tmp );
            tmp.clear();
          }
          return result;
        };
        for ( int id = 0; id <= ACTION__HIGHEST; ++id )
        {
          std::string entry( "OldAnim" + std::to_string( id ) );
          if ( elem.has_prop( entry.c_str() ) )
          {
            std::vector<std::string> values = split_str( elem.remove_string( entry.c_str() ) );
            if ( !values.empty() )
            {
              mobiletype.old_anim[id].action =
                  static_cast<u16>( strtoul( values[0].c_str(), nullptr, 0 ) );
              mobiletype.old_anim[id].valid = true;
              mobiletype.old_anim[id].framecount = 5;
              mobiletype.old_anim[id].repeatcount = 1;
              mobiletype.old_anim[id].backward = 0;
              mobiletype.old_anim[id].repeatflag = 0;
              mobiletype.old_anim[id].delay = 1;
            }
            if ( values.size() > 1 )
              mobiletype.old_anim[id].framecount =
                  static_cast<u16>( strtoul( values[1].c_str(), nullptr, 0 ) );
            if ( values.size() > 2 )
              mobiletype.old_anim[id].repeatcount =
                  static_cast<u16>( strtoul( values[2].c_str(), nullptr, 0 ) );
            if ( values.size() > 3 )
              mobiletype.old_anim[id].backward =
                  static_cast<u8>( strtoul( values[3].c_str(), nullptr, 0 ) );
            if ( values.size() > 4 )
              mobiletype.old_anim[id].repeatflag =
                  static_cast<u8>( strtoul( values[4].c_str(), nullptr, 0 ) );
            if ( values.size() > 5 )
              mobiletype.old_anim[id].delay =
                  static_cast<u8>( strtoul( values[5].c_str(), nullptr, 0 ) );
          }
          entry = "NewAnim" + std::to_string( id );
          if ( elem.has_prop( entry.c_str() ) )
          {
            std::vector<std::string> values = split_str( elem.remove_string( entry.c_str() ) );
            if ( !values.empty() )
            {
              mobiletype.new_anim[id].anim =
                  static_cast<u16>( strtoul( values[0].c_str(), nullptr, 0 ) );
              mobiletype.new_anim[id].valid = true;
              mobiletype.new_anim[id].action = 0;
              mobiletype.new_anim[id].subaction = 0;
            }
            if ( values.size() > 1 )
              mobiletype.new_anim[id].action =
                  static_cast<u16>( strtoul( values[1].c_str(), nullptr, 0 ) );
            if ( values.size() > 2 )
              mobiletype.new_anim[id].subaction =
                  static_cast<u8>( strtoul( values[2].c_str(), nullptr, 0 ) );
          }
        }
        Core::gamestate.animation_translates[elem.rest()] = mobiletype;
      }
    }
  }
}

void send_action_to_inrange( const Mobile::Character* obj, UACTION action,
                             unsigned short framecount /*=0x05*/,
                             unsigned short repeatcount /*=0x01*/,
                             DIRECTION_FLAG_OLD backward /*=PKTOUT_6E::FORWARD*/,
                             REPEAT_FLAG_OLD repeatflag /*=PKTOUT_6E::NOREPEAT*/,
                             unsigned char delay /*=0x01*/ )
{
  if ( !UACTION_IS_VALID( static_cast<u16>( action ) ) )
  {
    ERROR_PRINTLN( "Warning: attempt to send invalid action {:#x} to character {:#x}", action,
                   obj->serial );
    return;
  }

  bool build = false;
  Network::MobileAnimationMsg msg( obj->serial_ext );
  WorldIterator<OnlinePlayerFilter>::InMaxVisualRange(
      obj,
      [&]( Mobile::Character* zonechr )
      {
        if ( !zonechr->in_visual_range( obj ) )
          return;
        if ( !build )  // only build if client in range
        {
          MobileTranslate::OldAnimDef oldanim;
          oldanim.valid = true;
          oldanim.action = static_cast<u16>( action );
          oldanim.framecount = framecount;
          oldanim.repeatcount = repeatcount;
          oldanim.backward = static_cast<u8>( backward );
          oldanim.repeatflag = static_cast<u8>( repeatflag );
          oldanim.delay = delay;
          MobileTranslate::NewAnimDef newanim;
          bool supports_mount = true;
          build = true;
          MobileTranslate const* translate = nullptr;
          for ( const auto& translates : Core::gamestate.animation_translates )
          {
            if ( translates.second.has_graphic( obj->graphic ) )
            {
              translate = &translates.second;
              supports_mount = translate->supports_mount;
              break;
            }
          }

          if ( obj->on_mount() && supports_mount )
          {
            if ( action < ACTION_RIDINGHORSE1 || action > ACTION_RIDINGHORSE7 )
            {
              UACTION new_action = Core::gamestate.mount_action_xlate[action];
              if ( new_action == 0 )
                return;
              action = new_action;
              oldanim.action = static_cast<u16>( new_action );
            }
          }
          if ( translate != nullptr )
          {
            oldanim = translate->old_anim[action];
            newanim = translate->new_anim[action];
          }
          else
          {
            ERROR_PRINTLN( "Warning: undefined animXlate.cfg entry for graphic {:#x}",
                           obj->graphic );
          }

          msg.update( newanim.anim, newanim.action, newanim.subaction, oldanim.action,
                      oldanim.framecount, oldanim.repeatcount,
                      static_cast<DIRECTION_FLAG_OLD>( oldanim.backward ),
                      static_cast<REPEAT_FLAG_OLD>( oldanim.repeatflag ), oldanim.delay,
                      oldanim.valid, newanim.valid );
        }

        msg.Send( zonechr->client );
      } );
}

void handle_action( Network::Client* client, PKTIN_12* cmd )
{
  if ( stricmp( (const char*)cmd->data, "bow" ) == 0 )
    send_action_to_inrange( client->chr, ACTION_BOW );
  else if ( stricmp( (const char*)cmd->data, "salute" ) == 0 )
    send_action_to_inrange( client->chr, ACTION_SALUTE );
}
}  // namespace Pol::Core
