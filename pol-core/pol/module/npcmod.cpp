/** @file
 *
 * @par History
 * - 2009/09/14 MuadDib:   CreateItem() now has slot support.
 */

#include "npcmod.h"
#include <iostream>
#include <stddef.h>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../../clib/clib.h"
#include "../../clib/logfacility.h"
#include "../../clib/random.h"
#include "../../clib/rawtypes.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../plib/poltype.h"
#include "../containr.h"
#include "../item/item.h"
#include "../mobile/boundbox.h"
#include "../mobile/charactr.h"
#include "../mobile/npc.h"
#include "../mobile/ufacing.h"
#include "../network/packethelper.h"
#include "../network/packets.h"
#include "../network/pktdef.h"
#include "../objtype.h"
#include "../polobject.h"
#include "../uoscrobj.h"
#include "../uworld.h"
#include "osmod.h"
#include "unimod.h"

#include <module_defs/npc.h>

namespace Pol
{
namespace Module
{
using namespace Bscript;
NPCExecutorModule::NPCExecutorModule( Executor& ex, Mobile::NPC& npc )
    : TmplExecutorModule<NPCExecutorModule, Core::PolModule>( ex ), npcref( &npc ), npc( npc )
{
  os_module = static_cast<OSExecutorModule*>( exec.findModule( "OS" ) );
  if ( os_module == nullptr )
    throw std::runtime_error( "NPCExecutorModule needs OS module!" );
}

NPCExecutorModule::~NPCExecutorModule()
{
  if ( npc.ex == &exec )
    npc.ex = nullptr;
}

BApplicObjType bounding_box_type;

class BoundingBoxObjImp final : public Core::PolApplicObj<Mobile::BoundingBox>
{
public:
  BoundingBoxObjImp() : PolApplicObj<Mobile::BoundingBox>( &bounding_box_type ) {}
  explicit BoundingBoxObjImp( const Mobile::BoundingBox& b )
      : PolApplicObj<Mobile::BoundingBox>( &bounding_box_type, b )
  {
  }
  virtual const char* typeOf() const override { return "BoundingBox"; }
  virtual u8 typeOfInt() const override { return OTBoundingBox; }
  virtual BObjectImp* copy() const override { return new BoundingBoxObjImp( value() ); }
};

/* IsLegalMove: parameters (move, bounding box)*/
BObjectImp* NPCExecutorModule::mf_IsLegalMove()
{
  String* facing_str = static_cast<String*>( exec.getParamImp( 0, BObjectImp::OTString ) );
  BApplicObjBase* appobj =
      static_cast<BApplicObjBase*>( exec.getParamImp( 1, BObjectImp::OTApplicObj ) );
  if ( facing_str == nullptr || appobj == nullptr || appobj->object_type() != &bounding_box_type )
  {
    return new BLong( 0 );
  }

  BApplicObj<Mobile::BoundingBox>* ao_bbox =
      static_cast<BApplicObj<Mobile::BoundingBox>*>( appobj );
  const Mobile::BoundingBox& bbox = ao_bbox->value();

  Core::UFACING facing;
  if ( Mobile::DecodeFacing( facing_str->value().c_str(), facing ) == false )
    return new BLong( 0 );

  auto pos = npc.pos().move( facing );

  return new BLong( bbox.contains( pos.xy() ) );
}

/* CanMove: parameters (facing)*/
BObjectImp* NPCExecutorModule::mf_CanMove()
{
  if ( exec.fparams.size() == 1 )
  {
    BObjectImp* param0 = exec.getParamImp( 0 );

    if ( param0->isa( BObjectImp::OTString ) )
    {
      const char* dir = exec.paramAsString( 0 );
      Core::UFACING facing;

      if ( Mobile::DecodeFacing( dir, facing ) == false )
      {
        DEBUGLOGLN(
            "Script Error in '{}' PC={}: \n"
            "\tCall to function npc::canmove():\n"
            "\tParameter 0: Expected direction: N S E W NW NE SW SE, got {}",
            scriptname(), exec.PC, dir );
        return new BError( "Invalid facing value" );
      }

      return new BLong( npc.could_move( facing ) ? 1 : 0 );
    }
    else if ( param0->isa( BObjectImp::OTLong ) )
    {
      BLong* blong = static_cast<BLong*>( param0 );
      Core::UFACING facing = static_cast<Core::UFACING>( blong->value() & PKTIN_02_FACING_MASK );
      return new BLong( npc.could_move( facing ) ? 1 : 0 );
    }
    else
    {
      DEBUGLOGLN(
          "Script Error in '{}' PC={}: \n"
          "\tCall to function npc::canmove():\n"
          "\tParameter 0: Expected direction, got datatype {}",
          scriptname(), exec.PC, BObjectImp::typestr( param0->type() ) );
      return new BError( "Invalid parameter type" );
    }
  }
  else
    return new BError( "Invalid parameter count" );
}

BObjectImp* NPCExecutorModule::mf_Self()
{
  return new ECharacterRefObjImp( &npc );
}

BObjectImp* NPCExecutorModule::mf_SetAnchor()
{
  Core::Pos2d pos;
  int dstart, psub;
  if ( getPos2dParam( 0, 1, &pos, npc.realm() ) && getParam( 2, dstart ) && getParam( 3, psub ) )
  {
    if ( dstart )
    {
      npc.anchor.enabled = true;
      npc.anchor.pos = pos;
      npc.anchor.dstart = static_cast<unsigned short>( dstart );
      npc.anchor.psub = static_cast<unsigned short>( psub );
      return new BLong( 1 );
    }
    else
    {
      npc.anchor.enabled = false;
      return new BLong( 1 );
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}


bool NPCExecutorModule::_internal_move( Core::UFACING facing, int run )
{
  bool success = false;
  int dir = facing;
  if ( run )
    dir |= 0x80;  // FIXME HARDCODE

  if ( npc.could_move( facing ) )
  {
    if ( npc.move(
             static_cast<unsigned char>( dir ) ) )  // this could still fail, if paralyzed or frozen
    {
      npc.tellmove();
      // move was successful
      success = true;
    }
    // else, npc could move, but move failed.
  }
  // else npc could not move

  return success;
}

BObjectImp* NPCExecutorModule::move_self( Core::UFACING facing, bool run, bool adjust_ok )
{
  bool success = false;
  int dir = facing;

  if ( run )
    dir |= 0x80;  // FIXME HARDCODE

  if ( adjust_ok )
  {
    if ( npc.use_adjustments() )
    {
      for ( int adjust : Core::adjustments )
      {
        facing = static_cast<Core::UFACING>( ( dir + adjust ) & 7 );

        success = _internal_move( facing, run );
        if ( success == true )
          break;
      }
    }
    else
    {
      success = _internal_move( facing, run );
    }
  }
  else
  {
    if ( npc.anchor_allows_move( facing ) && npc.move( static_cast<unsigned char>( dir ) ) )
    {
      npc.tellmove();
      success = true;
    }
  }

  // int delay = 1000 - npc.dexterity() * 3;
  int delay = std::max<int>( 1000 - npc.run_speed * 3,
                             Core::settingsManager.ssopt.npc_minimum_movement_delay );
  u32 sleep = static_cast<u32>( delay );
  os_module->SleepForMs( run ? ( sleep / 2 ) : sleep );

  // return new String( FacingStr(facing) );
  return new BLong( success ? 1 : 0 );
}

BObjectImp* NPCExecutorModule::mf_Wander()
{
  u8 newfacing = 0;
  bool adjust_ok = true;
  switch ( Clib::random_int( 7 ) )
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
    newfacing = npc.facing;
    break;
  case 6:
    newfacing = ( static_cast<int>( npc.facing ) - 1 ) & PKTIN_02_FACING_MASK;
    adjust_ok = false;
    break;
  case 7:
    newfacing = ( static_cast<int>( npc.facing ) + 1 ) & PKTIN_02_FACING_MASK;
    adjust_ok = false;
    break;
  }
  return move_self( static_cast<Core::UFACING>( newfacing ), false, adjust_ok );
}

BObjectImp* NPCExecutorModule::mf_Face()
{
  BObjectImp* param0 = exec.getParamImp( 0 );
  int flags;

  if ( param0 == nullptr || !exec.getParam( 1, flags ) )
    return new BError( "Invalid parameter type." );

  Core::UFACING i_facing;

  if ( param0->isa( BObjectImp::OTString ) )
  {
    const char* dir = exec.paramAsString( 0 );

    if ( Mobile::DecodeFacing( dir, i_facing ) == false )
    {
      DEBUGLOGLN(
          "Script Error in '{}' PC={}: \n"
          "\tCall to function npc::face():\n"
          "\tParameter 0: Expected direction: N S E W NW NE SW SE, got {}",
          scriptname(), exec.PC, dir );
      return nullptr;
    }
  }
  else if ( param0->isa( BObjectImp::OTLong ) )
  {
    BLong* blong = static_cast<BLong*>( param0 );
    i_facing = static_cast<Core::UFACING>( blong->value() & PKTIN_02_FACING_MASK );
  }
  else
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tCall to function npc::face():\n"
        "\tParameter 0: Expected direction, , got datatype {}",
        scriptname(), exec.PC, BObjectImp::typestr( param0->type() ) );

    return nullptr;
  }

  if ( !npc.face( i_facing, flags ) )
    return new BLong( 0 );

  npc.on_facing_changed();
  return new BLong( i_facing );
}

BObjectImp* NPCExecutorModule::mf_Move()
{
  BObjectImp* param0 = exec.getParamImp( 0 );

  if ( param0->isa( BObjectImp::OTString ) )
  {
    const char* dir = exec.paramAsString( 0 );
    Core::UFACING facing;

    if ( Mobile::DecodeFacing( dir, facing ) == false )
    {
      DEBUGLOGLN(
          "Script Error in '{}' PC={}: \n"
          "\tCall to function npc::move():\n"
          "\tParameter 0: Expected direction: N S E W NW NE SW SE, got {}",
          scriptname(), exec.PC, dir );
      return nullptr;
    }

    return move_self( facing, false );
  }
  else if ( param0->isa( BObjectImp::OTLong ) )
  {
    BLong* blong = static_cast<BLong*>( param0 );
    Core::UFACING facing = static_cast<Core::UFACING>( blong->value() & PKTIN_02_FACING_MASK );
    return move_self( facing, false );
  }
  else if ( param0->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* appobj = static_cast<BApplicObjBase*>( param0 );
    if ( appobj->object_type() == &bounding_box_type )
    {
      BApplicObj<Mobile::BoundingBox>* ao_bbox =
          static_cast<BApplicObj<Mobile::BoundingBox>*>( appobj );
      const Mobile::BoundingBox& bbox = ao_bbox->value();
      Core::UFACING facing = Mobile::GetRandomFacing();

      auto pos = npc.pos().move( facing );
      if ( bbox.contains( pos.xy() ) || !bbox.contains( npc.pos2d() ) )
      {
        npc.move( static_cast<unsigned char>( facing ) );
        npc.tellmove();
        os_module->SleepFor( 1 );
        return new String( Mobile::FacingStr( facing ) );
      }
      else
      {
        return new String( "" );
      }
    }
    else
    {
      DEBUGLOGLN(
          "Script Error in '{}' PC={}: \n"
          "\tCall to function npc::move():\n"
          "\tParameter 0: Expected direction or bounding box, , got datatype {}",
          scriptname(), exec.PC, BObjectImp::typestr( param0->type() ) );
      return nullptr;
    }
  }
  else
  {
    DEBUGLOGLN(
        "Script Error in '{}' PC={}: \n"
        "\tCall to function npc::move():\n"
        "\tParameter 0: Expected direction or bounding box, , got datatype {}",
        scriptname(), exec.PC, BObjectImp::typestr( param0->type() ) );

    return nullptr;
  }
}

BObjectImp* NPCExecutorModule::mf_WalkToward()
{
  Core::UObject* obj;
  if ( getUObjectParam( 0, obj ) )
  {
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !npc.is_visible_to_me( chr, /*check_range*/ false ) )
        return new BError( "Mobile specified cannot be seen" );
    }
    Core::UFACING fac = npc.direction_toward( obj );
    return move_self( fac, false, true );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}


BObjectImp* NPCExecutorModule::mf_RunToward()
{
  Core::UObject* obj;
  if ( getUObjectParam( 0, obj ) )
  {
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !npc.is_visible_to_me( chr, /*check_range*/ false ) )
        return new BError( "Mobile specified cannot be seen" );
    }
    return move_self( npc.direction_toward( obj ), true, true );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* NPCExecutorModule::mf_WalkAwayFrom()
{
  Core::UObject* obj;
  if ( getUObjectParam( 0, obj ) )
  {
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !npc.is_visible_to_me( chr, /*check_range*/ false ) )
        return new BError( "Mobile specified cannot be seen" );
    }
    return move_self( npc.direction_away( obj ),

                      false, true );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* NPCExecutorModule::mf_RunAwayFrom()
{
  Core::UObject* obj;
  if ( getUObjectParam( 0, obj ) )
  {
    if ( obj->ismobile() )
    {
      Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
      if ( !npc.is_visible_to_me( chr, /*check_range*/ false ) )
        return new BError( "Mobile specified cannot be seen" );
    }
    return move_self( npc.direction_away( obj ),

                      true, true );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* NPCExecutorModule::mf_TurnToward()
{
  Core::UObject* obj;
  int flags;

  if ( !getUObjectParam( 0, obj ) || !exec.getParam( 1, flags ) )
  {
    return new BError( "Invalid parameter type" );
  }

  if ( obj->ismobile() )
  {
    Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
    if ( !npc.is_visible_to_me( chr, /*check_range*/ false ) )
      return new BError( "Mobile specified cannot be seen" );
  }

  Core::UFACING facing = npc.direction_toward( obj );
  if ( facing == npc.facing )
    return new BLong( 0 );  // nothing to do here, I'm already facing that direction

  if ( !npc.face( facing, flags ) )
    return new BLong( 0 );  // Uh-oh, seems that I can't move to face that

  npc.on_facing_changed();
  return new BLong( 1 );
}

BObjectImp* NPCExecutorModule::mf_TurnAwayFrom()
{
  Core::UObject* obj;
  int flags;

  if ( !getUObjectParam( 0, obj ) || !exec.getParam( 1, flags ) )
  {
    return new BError( "Invalid parameter type" );
  }


  if ( obj->ismobile() )
  {
    Mobile::Character* chr = static_cast<Mobile::Character*>( obj );
    if ( !npc.is_visible_to_me( chr, /*check_range*/ false ) )
      return new BError( "Mobile specified cannot be seen" );
  }

  Core::UFACING facing = npc.direction_away( obj );
  if ( facing == npc.facing )
    return new BLong( 0 );  // nothing to do here

  if ( !npc.face( facing, flags ) )
    return new BLong( 0 );  // couldn't move for some reason

  npc.on_facing_changed();
  return new BLong( 1 );
}

BObjectImp* NPCExecutorModule::mf_WalkTowardLocation()
{
  Core::Pos2d pos;
  if ( getPos2dParam( 0, 1, &pos, npc.realm() ) )
  {
    Core::UFACING fac = npc.direction_toward( pos );
    return move_self( fac, false, true );
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* NPCExecutorModule::mf_RunTowardLocation()
{
  Core::Pos2d pos;
  if ( getPos2dParam( 0, 1, &pos, npc.realm() ) )
  {
    Core::UFACING fac = npc.direction_toward( pos );
    return move_self( fac, true, true );
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* NPCExecutorModule::mf_WalkAwayFromLocation()
{
  Core::Pos2d pos;
  if ( getPos2dParam( 0, 1, &pos, npc.realm() ) )
  {
    Core::UFACING fac = npc.direction_away( pos );
    return move_self( fac, false, true );
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* NPCExecutorModule::mf_RunAwayFromLocation()
{
  Core::Pos2d pos;
  if ( getPos2dParam( 0, 1, &pos, npc.realm() ) )
  {
    Core::UFACING fac = npc.direction_away( pos );
    return move_self( fac, true, true );
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* NPCExecutorModule::mf_TurnTowardLocation()
{
  Core::Pos2d pos;
  int flags;

  if ( !getPos2dParam( 0, 1, &pos, npc.realm() ) || !exec.getParam( 2, flags ) )
  {
    return new BError( "Invalid parameter type" );
  }

  Core::UFACING fac = npc.direction_toward( pos );
  if ( npc.facing == fac )
    return new BLong( 0 );  // nothing to do here

  if ( !npc.face( fac, flags ) )
    return new BLong( 0 );  // I couldn't move!

  npc.on_facing_changed();
  return new BLong( 1 );
}

BObjectImp* NPCExecutorModule::mf_TurnAwayFromLocation()
{
  Core::Pos2d pos;
  int flags;

  if ( !getPos2dParam( 0, 1, &pos, npc.realm() ) || !exec.getParam( 2, flags ) )
  {
    return new BError( "Invalid parameter type" );
  }

  Core::UFACING fac = npc.direction_away( pos );
  if ( npc.facing == fac )
    return new BLong( 0 );  // nothing to do here

  if ( !npc.face( fac, flags ) )
    return new BLong( 0 );  // I couldn't move!

  npc.on_facing_changed();
  return new BLong( 1 );
}


BObjectImp* NPCExecutorModule::mf_Say()
{
  if ( npc.squelched() )
    return new BError( "NPC is squelched" );
  else if ( npc.hidden() )
    npc.unhide();

  const char* text = exec.paramAsString( 0 );
  std::string texttype_str = Clib::strlowerASCII( exec.paramAsString( 1 ) );
  int doevent;
  exec.getParam( 2, doevent );
  u8 texttype;
  if ( texttype_str == "default" )
    texttype = Plib::TEXTTYPE_NORMAL;
  else if ( texttype_str == "whisper" )
    texttype = Plib::TEXTTYPE_WHISPER;
  else if ( texttype_str == "yell" )
    texttype = Plib::TEXTTYPE_YELL;
  else
    return new BError( "texttype string param must be either 'default', 'whisper', or 'yell'" );


  Network::PktHelper::PacketOut<Network::PktOut_1C> msg;
  Network::PktHelper::PacketOut<Network::PktOut_AE> ucmsg;
  u16 len = 0;
  u16 uclen = 0;
  // switch to other pkt if utf8 found
  if ( !Bscript::String::hasUTF8Characters( text ) )
  {
    msg->offset += 2;
    msg->Write<u32>( npc.serial_ext );
    msg->WriteFlipped<u16>( npc.graphic );
    msg->Write<u8>( texttype );
    msg->WriteFlipped<u16>( npc.speech_color() );
    msg->WriteFlipped<u16>( npc.speech_font() );
    msg->Write( Clib::strUtf8ToCp1252( npc.name() ).c_str(), 30 );
    msg->Write( text, ( strlen( text ) > SPEECH_MAX_LEN + 1 )
                          ? SPEECH_MAX_LEN + 1
                          : static_cast<u16>( strlen( text ) + 1 ) );
    len = msg->offset;
    msg->offset = 1;
    msg->WriteFlipped<u16>( len );
  }
  else
  {
    std::vector<u16> utf16 = Bscript::String::toUTF16( text );
    if ( utf16.size() > SPEECH_MAX_LEN )
      utf16.resize( SPEECH_MAX_LEN );
    ucmsg->offset += 2;
    ucmsg->Write<u32>( npc.serial_ext );
    ucmsg->WriteFlipped<u16>( npc.graphic );
    ucmsg->Write<u8>( texttype );
    ucmsg->WriteFlipped<u16>( npc.speech_color() );
    ucmsg->WriteFlipped<u16>( npc.speech_font() );
    ucmsg->Write( "ENU", 4 );
    ucmsg->Write( Clib::strUtf8ToCp1252( npc.description() ).c_str(), 30 );
    ucmsg->WriteFlipped( utf16, true );
    uclen = ucmsg->offset;
    ucmsg->offset = 1;
    ucmsg->WriteFlipped<u16>( uclen );
  }

  // send to those nearby
  u16 range;
  if ( texttype == Plib::TEXTTYPE_WHISPER )
    range = Core::settingsManager.ssopt.whisper_range;
  else if ( texttype == Plib::TEXTTYPE_YELL )
    range = Core::settingsManager.ssopt.yell_range;
  else
    range = Core::settingsManager.ssopt.speech_range;
  Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
      &npc, range,
      [&]( Mobile::Character* chr )
      {
        if ( !chr->is_visible_to_me( &npc, /*check_range*/ false ) )
          return;
        if ( !uclen )
          msg.Send( chr->client, len );
        else
          ucmsg.Send( chr->client, uclen );
      } );

  if ( doevent >= 1 )
  {
    Core::WorldIterator<Core::NPCFilter>::InRange(
        &npc, range,
        [&]( Mobile::Character* chr )
        {
          Mobile::NPC* othernpc = static_cast<Mobile::NPC*>( chr );
          if ( chr != &npc )
            othernpc->on_pc_spoke( &npc, text, texttype );
        } );
  }

  return nullptr;
}

BObjectImp* NPCExecutorModule::mf_SayUC()
{
  if ( npc.squelched() )
    return new BError( "NPC is squelched" );
  else if ( npc.hidden() )
    npc.unhide();

  const String* text;
  const String* lang;
  int doevent;

  if ( getUnicodeStringParam( 0, text ) && getStringParam( 2, lang ) && getParam( 3, doevent ) )
  {
    std::string texttype_str = Clib::strlowerASCII( exec.paramAsString( 1 ) );
    if ( texttype_str != "default" && texttype_str != "whisper" && texttype_str != "yell" )
    {
      return new BError( "texttype string param must be either 'default', 'whisper', or 'yell'" );
    }

    if ( text->length() > SPEECH_MAX_LEN )
      return new BError( "Text exceeds maximum size." );
    if ( lang->length() != 3 )
      return new BError( "langcode must be a 3-character code." );

    std::vector<u16> utf16 = text->toUTF16();
    if ( utf16.size() > SPEECH_MAX_LEN )
      utf16.resize( SPEECH_MAX_LEN );
    std::string languc = Clib::strupperASCII( lang->value() );

    u8 texttype;
    if ( texttype_str == "whisper" )
      texttype = Plib::TEXTTYPE_WHISPER;
    else if ( texttype_str == "yell" )
      texttype = Plib::TEXTTYPE_YELL;
    else
      texttype = Plib::TEXTTYPE_NORMAL;

    Network::PktHelper::PacketOut<Network::PktOut_AE> talkmsg;
    talkmsg->offset += 2;
    talkmsg->Write<u32>( npc.serial_ext );
    talkmsg->WriteFlipped<u16>( npc.graphic );
    talkmsg->Write<u8>( texttype );
    talkmsg->WriteFlipped<u16>( npc.speech_color() );
    talkmsg->WriteFlipped<u16>( npc.speech_font() );
    talkmsg->Write( languc.c_str(), 4 );
    talkmsg->Write( Clib::strUtf8ToCp1252( npc.description() ).c_str(), 30 );
    talkmsg->WriteFlipped( utf16, true );
    u16 len = talkmsg->offset;
    talkmsg->offset = 1;
    talkmsg->WriteFlipped<u16>( len );

    u16 range;
    if ( texttype == Plib::TEXTTYPE_WHISPER )
      range = Core::settingsManager.ssopt.whisper_range;
    else if ( texttype == Plib::TEXTTYPE_YELL )
      range = Core::settingsManager.ssopt.yell_range;
    else
      range = Core::settingsManager.ssopt.speech_range;
    Core::WorldIterator<Core::OnlinePlayerFilter>::InRange(
        &npc, range,
        [&]( Mobile::Character* chr )
        {
          if ( !chr->is_visible_to_me( &npc, /*check_range*/ false ) )
            return;
          talkmsg.Send( chr->client, len );
        } );

    if ( doevent >= 1 )
    {
      Core::WorldIterator<Core::NPCFilter>::InRange(
          &npc, range,
          [&]( Mobile::Character* chr )
          {
            Mobile::NPC* othernpc = static_cast<Mobile::NPC*>( chr );
            if ( othernpc != &npc )
              othernpc->on_pc_spoke( &npc, text->value(), texttype, languc );
          } );
    }
  }
  else
  {
    return new BError( "A parameter was invalid" );
  }
  return nullptr;
}

BObjectImp* NPCExecutorModule::mf_position()
{
  std::unique_ptr<BStruct> oa( new BStruct );

  oa->addMember( "x", new BLong( npc.x() ) );
  oa->addMember( "y", new BLong( npc.y() ) );
  oa->addMember( "z", new BLong( npc.z() ) );

  return oa.release();
}

BObjectImp* NPCExecutorModule::mf_Facing()
{
  return new String( Mobile::FacingStr( static_cast<Core::UFACING>( npc.facing ) ) );
}

BObjectImp* NPCExecutorModule::mf_GetProperty()
{
  const String* propname_str;
  if ( exec.getStringParam( 0, propname_str ) )
  {
    std::string val;
    if ( npc.getprop( propname_str->value(), val ) )
    {
      return BObjectImp::unpack( val.c_str() );
    }
    else
    {
      return new BError( "Property not found" );
    }
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* NPCExecutorModule::mf_SetProperty()
{
  const String* propname_str;
  if ( exec.getStringParam( 0, propname_str ) )
  {
    BObjectImp* propval = getParamImp( 1 );
    npc.setprop( propname_str->value(), propval->pack() );
    return new BLong( 1 );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

BObjectImp* NPCExecutorModule::mf_CreateBackpack()
{
  // UNTESTED
  if ( !npc.layer_is_equipped( Core::LAYER_BACKPACK ) )
  {
    Items::Item* i = Items::Item::create( UOBJ_BACKPACK );
    i->setposition( npc.pos() );
    std::unique_ptr<Items::Item> item( i );
    item->layer = Core::LAYER_BACKPACK;
    if ( npc.equippable( item.get() ) )
    {
      npc.equip( item.release() );
    }
  }
  return new BLong( 1 );
}

BObjectImp* NPCExecutorModule::mf_CreateItem()
{
  // UNTESTED
  const BLong* objtype = exec.getLongParam( 0 );
  if ( objtype == nullptr )
    return new BLong( 0 );

  Core::UContainer* backpack = npc.backpack();
  if ( backpack == nullptr )
    return new BLong( 0 );

  Items::Item* i = Items::Item::create( static_cast<unsigned int>( objtype->value() ) );
  i->setposition( npc.pos() );
  std::unique_ptr<Items::Item> item( i );
  if ( item.get() == nullptr )
    return new BLong( 0 );

  if ( !backpack->can_add( *item ) )
    return new BLong( 0 );

  u8 slotIndex = item->slot_index();
  if ( !backpack->can_add_to_slot( slotIndex ) )
    return new BLong( 0 );

  if ( !item->slot_index( slotIndex ) )
    return new BLong( 0 );

  u32 serial = item->serial;

  backpack->add( item.release() );

  return new BLong( serial );
}

BObjectImp* NPCExecutorModule::mf_MakeBoundingBox( /* areastring */ )
{
  auto arealist = static_cast<String*>( getParamImp( 0, BObjectImp::OTString ) );
  if ( arealist == nullptr )
    return new String( "" );

  BoundingBoxObjImp* bbox = new BoundingBoxObjImp;
  std::unique_ptr<BoundingBoxObjImp> bbox_owner( bbox );

  ISTRINGSTREAM is( arealist->value() );

  u16 tlx, tly, brx, bry;
  // FIXME this is a terrible data format.
  while ( is >> tlx >> tly >> brx >> bry )
  {
    ( *bbox )->addarea(
        Core::Range2d( Core::Pos2d{ tlx, tly }, Core::Pos2d{ brx, bry }, npc.realm() ) );
  }

  return bbox_owner.release();
}

BObjectImp* NPCExecutorModule::mf_SetOpponent()
{
  Mobile::Character* chr;
  if ( getCharacterParam( 0, chr ) && chr != &npc )
  {
    npc.set_opponent( chr );
    return new BLong( 1 );
  }
  else
  {
    npc.set_opponent( nullptr );
    return new BLong( 0 );
  }
}

BObjectImp* NPCExecutorModule::mf_SetWarMode()
{
  int warmode;
  if ( exec.getParam( 0, warmode ) )
  {
    npc.set_warmode( warmode != 0 );
    return new BLong( 1 );
  }
  else
  {
    return new BLong( 0 );
  }
}

size_t NPCExecutorModule::sizeEstimate() const
{
  return sizeof( *this );
}
}  // namespace Module
}  // namespace Pol
