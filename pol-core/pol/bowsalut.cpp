/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2009/07/27 MuadDib:   Pakcet struct refactoring

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/endian.h"
#include "../clib/fileutil.h"
#include "../clib/strutil.h"
#include "../clib/stlutil.h"
#include "../clib/logfacility.h"

#include "mobile/charactr.h"
#include "network/client.h"
#include "network/packets.h"
#include "network/packetdefs.h"
#include "extcmd.h"
#include "pktin.h"
#include "pktdef.h"
#include "uobject.h"
#include "uvars.h"
#include "uworld.h"
#include "ufunc.h"
#include "sockio.h"

#include "action.h"
namespace Pol {
  namespace Core {
	UACTION mount_action_xlate[ACTION__HIGHEST + 1];
    struct MobileTranslate
    {
      struct OldAnimDef
      {
        bool valid;
        u16 action;
        u16 framecount;
        u16 repeatcount;
        u8 backward;
        u8 repeatflag;
        u8 delay;
        OldAnimDef() :valid( false ), action( 0 ), framecount( 0 ), repeatcount( 0 ), backward( 0 ), repeatflag( 0 ), delay( 0 )
        {};
      };
      struct NewAnimDef
      {
        bool valid;
        u16 anim;
        u16 action;
        u8 subaction;
        NewAnimDef( ) :valid( false ), anim( 0 ), action( 0 ), subaction( 0 )
        {};
      };
      std::vector<u16> graphics;
      OldAnimDef old_anim[ACTION__HIGHEST + 1];
      NewAnimDef new_anim[ACTION__HIGHEST + 1];
      OldAnimDef old_fallback;
      NewAnimDef new_fallback;
      bool supports_mount;
	  MobileTranslate() : graphics(), old_fallback(), new_fallback(), supports_mount(false)
      {
		memset( &old_anim, 0, sizeof( old_anim ) );
		memset( &new_anim, 0, sizeof( new_anim ) );
      }
      bool has_graphic( u16 graphic ) const
      {
        return std::find( graphics.begin(), graphics.end(), graphic ) != graphics.end();
      }
    };
    std::map<std::string, MobileTranslate> animation_translates;



	UACTION str_to_action( Clib::ConfigElem& elem, const string& str )
	{
	  unsigned short tmp = static_cast<unsigned short>( strtoul( str.c_str(), NULL, 0 ) );

	  if ( UACTION_IS_VALID( tmp ) )
	  {
		return static_cast<UACTION>( tmp );
	  }
	  else
	  {
		elem.throw_error( "Animation value of " + str + " is out of range" );
		return ACTION__LOWEST;
	  }
	}

	void load_anim_xlate_cfg( bool reload )
	{
	  memset( &mount_action_xlate, 0, sizeof( mount_action_xlate ) );
      animation_translates.clear();

	  if ( Clib::FileExists( "config/animxlate.cfg" ) )
	  {
        std::map<string, u16> animation_names;
        {
          Clib::ConfigFile cf( "config/animxlate.cfg", "OnMount MobileType AnimationNames" );
          Clib::ConfigElem elem;
          while ( cf.read( elem ) ) // first read the names
          {
            if ( elem.type_is( "AnimationNames" ) )
            {
              string name, id;
              while ( elem.remove_first_prop( &name, &id ) )
              {
                animation_names[Clib::strlower( name )] = static_cast<unsigned short>( strtoul( id.c_str(), NULL, 0 ) );
              }
            }
          }
        }
        Clib::ConfigFile cf( "config/animxlate.cfg", "OnMount MobileType AnimationNames" );
        Clib::ConfigElem elem;
        while ( cf.read( elem ) )
		{
          if ( elem.type_is("OnMount") )
          {
            string from_str, to_str;
            while ( elem.remove_first_prop( &from_str, &to_str ) )
            {
              UACTION from = str_to_action( elem, from_str );
              UACTION to = str_to_action( elem, to_str );
              mount_action_xlate[from] = to;
            }
          }
          else if ( elem.type_is( "MobileType" ) )
          {
            MobileTranslate mobiletype;
            string key, value;
            while ( elem.has_prop( "Graphic" ) )
            {
              mobiletype.graphics.push_back( elem.remove_ushort( "Graphic" ) );
            }
            std::sort( mobiletype.graphics.begin(), mobiletype.graphics.end() );
            mobiletype.supports_mount = elem.remove_bool( "MountTranslation", false );

            auto split_str = [](const string &source) -> vector<string>
            {
              ISTRINGSTREAM is( source );
              string tmp;
              vector<string> result;
              while ( is >> tmp )
              {
                result.push_back( tmp );
                tmp.clear();
              }
              return result;
            };

            while ( elem.has_prop( "OldAnim" ) )
            {
              vector<string> values = split_str(elem.remove_string( "OldAnim" ));
              if ( !values.empty() )
              {
                auto itr = animation_names.find( Clib::strlower( values[0] ) );
                if ( itr != animation_names.end() )
                {
                  u16 id = itr->second;
                  if ( values.size() > 1 )
                  {
                    mobiletype.old_anim[id].action = static_cast<u16>( strtoul( values[1].c_str(), NULL, 0 ) );
                    mobiletype.old_anim[id].valid = true;
                    mobiletype.old_anim[id].framecount = 5;
                    mobiletype.old_anim[id].repeatcount = 1;
                    mobiletype.old_anim[id].backward = 0;
                    mobiletype.old_anim[id].repeatflag = 0;
                    mobiletype.old_anim[id].delay = 1;
                  }
                  if ( values.size() > 2 )
                    mobiletype.old_anim[id].framecount = static_cast<u16>( strtoul( values[2].c_str(), NULL, 0 ) );
                  if ( values.size() > 3 )
                    mobiletype.old_anim[id].repeatcount = static_cast<u16>( strtoul( values[3].c_str(), NULL, 0 ) );
                  if ( values.size() > 4 )
                    mobiletype.old_anim[id].backward = static_cast<u8>( strtoul( values[4].c_str(), NULL, 0 ) );
                  if ( values.size() > 5 )
                    mobiletype.old_anim[id].repeatflag = static_cast<u8>( strtoul( values[5].c_str(), NULL, 0 ) );
                  if ( values.size() > 6 )
                    mobiletype.old_anim[id].delay = static_cast<u8>( strtoul( values[6].c_str(), NULL, 0 ) );
                }
              }
            }
            while ( elem.has_prop( "NewAnim" ) )
            {
              vector<string> values = split_str( elem.remove_string( "NewAnim" ) );
              if ( !values.empty( ) )
              {
                auto itr = animation_names.find( Clib::strlower( values[0] ) );
                if ( itr != animation_names.end() )
                {
                  u16 id = itr->second;
                  if ( values.size() > 1 )
                  {
                    mobiletype.new_anim[id].anim = static_cast<u16>( strtoul( values[1].c_str(), NULL, 0 ) );
                    mobiletype.new_anim[id].valid = true;
                    mobiletype.new_anim[id].action = 0;
                    mobiletype.new_anim[id].subaction = 0;
                  }
                  if ( values.size() > 2 )
                    mobiletype.new_anim[id].action = static_cast<u16>( strtoul( values[2].c_str(), NULL, 0 ) );
                  if ( values.size() > 3 )
                    mobiletype.new_anim[id].subaction = static_cast<u8>( strtoul( values[3].c_str(), NULL, 0 ) );
                }
              }
            }
            animation_translates[elem.rest()] = mobiletype;
          }
		}
	  }
	}

	void send_action_to_inrange( const Mobile::Character* obj, UACTION action,
								 unsigned short framecount /*=0x05*/,
								 unsigned short repeatcount /*=0x01*/,
                                 Network::MobileAnimationMsg::DIRECTION_FLAG_OLD backward /*=PKTOUT_6E::FORWARD*/,
                                 Network::MobileAnimationMsg::REPEAT_FLAG_OLD repeatflag /*=PKTOUT_6E::NOREPEAT*/,
								 unsigned char delay /*=0x01*/ )
	{
      bool build = false;
      Network::MobileAnimationMsg msg( obj->serial_ext );
      WorldIterator<PlayerFilter>::InVisualRange( obj, [&]( Mobile::Character *zonechr )
      {
        if ( !zonechr->has_active_client( ) )
          return;
        if ( !build ) // only build if client in range
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
          for ( const auto& translates : animation_translates )
          {
            if ( translates.second.has_graphic( obj->graphic ) )
            {
              translate = &translates.second;
              supports_mount = translate->supports_mount;
              break;
            }
          }

          if ( obj->on_mount( ) && supports_mount )
          {
            if ( action < ACTION_RIDINGHORSE1 || action > ACTION_RIDINGHORSE7 )
            {
              UACTION new_action = mount_action_xlate[action];
              if ( new_action == 0 )
                return;
              action = new_action;
	      oldanim.action = new_action;
            }
          }
          if ( translate != nullptr )
          {
            oldanim = translate->old_anim[action];
            newanim = translate->new_anim[action];
          }

          msg.update(
            newanim.anim, newanim.action, newanim.subaction,
            oldanim.action, oldanim.framecount, oldanim.repeatcount,
            static_cast<Network::MobileAnimationMsg::DIRECTION_FLAG_OLD>( oldanim.backward ),
            static_cast<Network::MobileAnimationMsg::REPEAT_FLAG_OLD>( oldanim.repeatflag ), oldanim.delay,
            oldanim.valid, newanim.valid );
        }

        msg.Send( zonechr->client );
      } );
	}

	void handle_action( Network::Client *client, PKTIN_12 *cmd )
	{
	  if ( stricmp( (const char *)cmd->data, "bow" ) == 0 )
		send_action_to_inrange( client->chr, ACTION_BOW );
	  else if ( stricmp( (const char *)cmd->data, "salute" ) == 0 )
		send_action_to_inrange( client->chr, ACTION_SALUTE );
	}

	ExtendedMessageHandler action_handler( EXTMSGID_ACTION, handle_action );
  }
}
