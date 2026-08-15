/** @file
 *
 * @par History
 * - 2006/05/16 Shinigami: ClientCreateChar() updated to support Elfs
 * - 2006/05/23 Shinigami: added Elf Hair Style to validhair() & comments added
 *                         validbeard() rewritten & comments added
 * - 2009/12/02 Turley:    added gargoyle support, 0x8D char create, face support
 * - 2010/01/14 Turley:    more error checks, Tomi's startequip patch
 * - 2011/10/26 Tomi:      added 0xF8 char create for clients >= 7.0.16.0
 */


#include <stdlib.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include "bscript/barray.h"
#include "bscript/blong.h"
#include "clib/clib_endian.h"
#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "clib/refptr.h"
#include "plib/clidata.h"
#include "plib/objtype.h"
#include "plib/systemstate.h"
#include "plib/uconst.h"

#include "pol/accounts/account.h"
#include "pol/containr.h"
#include "pol/gameclck.h"
#include "pol/globals/network.h"
#include "pol/globals/object_storage.h"
#include "pol/globals/uvars.h"
#include "pol/item/item.h"
#include "pol/layers.h"
#include "pol/mkscrobj.h"
#include "pol/mobile/attribute.h"
#include "pol/mobile/charactr.h"
#include "pol/mobile/wornitems.h"
#include "pol/module/uomod.h"
#include "pol/network/client.h"
#include "pol/network/pktdef.h"
#include "pol/network/pktin.h"
#include "pol/realms/WorldChangeReasons.h"
#include "pol/scrsched.h"
#include "pol/scrstore.h"
#include "pol/skillid.h"
#include "pol/startloc.h"
#include "pol/ufunc.h"
#include "pol/uoclient.h"
#include "pol/uoexec.h"
#include "pol/uoskills.h"
#include "pol/uworld.h"


namespace Pol::Core
{
void start_client_char( Network::Client* client );
void run_logon_script( Mobile::Character* chr );

short validhaircolor( u16 /*color*/ )
{
  return 1;
}

/* Ah, I just realized what may be the true way to do this:
   Read the tile file (given an object type, it gives data),
   and make sure the resultant tile is on the right layer.
   Only object types in the 0x2000's should be considered.
   Also, make sure weight is 0 - some wall sconces are on
   the beard layer. (!) (they aren't in the 0x2000's..)
   */

/* hair can be:
    0x203B  Short Hair      // Human
    0x203C  Long Hair
    0x203D  PonyTail
    0x2044  Mohawk
    0x2045  Pageboy Hair
    0x2046  Buns Hair
    0x2047  Afro
    0x2048  Receeding Hair
    0x2049  Two Pig Tails
    0x204A  Krisna Hair

    0x2FBF  Mid Long Hair     // Elf (Mondain's Legacy)
    0x2FC0  Long Feather Hair
    0x2FC1  Short Elf Hair
    0x2FC2  Mullet
    0x2FCC  Flower Hair
    0x2FCD  Long Elf Hair
    0x2FCE  Long Big Knob Hair
    0x2FCF  Long Big Braid Hair
    0x2FD0  Long Big Bun Hair
    0x2FD1  Spiked Hair
    0x2FD2  Long Elf Two Hair

    0x4258  Horn Style 1       // Gargoyle Male (SA)
    0x4259  Horn Style 2
    0x425a  Horn Style 3
    0x425b  Horn Style 4
    0x425c  Horn Style 5
    0x425d  Horn Style 6
    0x425e  Horn Style 7
    0x425f  Horn Style 8

    0x4261  Female Horn Style 1  // Gargoyle Female (SA)
    0x4262  Female Horn Style 2
    0x4273  Female Horn Style 3
    0x4274  Female Horn Style 4
    0x4275  Female Horn Style 5
    0x42aa  Female Horn Style 6
    0x42ab  Female Horn Style 7
    0x42b1  Femaly Horn Style 8
    */
bool validhair( u16 HairStyle )
{
  if ( Plib::systemstate.config.max_tile_id < HairStyle )
  {
    return false;
  }

  if ( ( ( 0x203B <= HairStyle ) && ( HairStyle <= 0x203D ) ) ||
       ( ( 0x2044 <= HairStyle ) && ( HairStyle <= 0x204A ) ) ||
       ( ( 0x2FBF <= HairStyle ) && ( HairStyle <= 0x2FC2 ) ) ||
       ( ( 0x2FCC <= HairStyle ) && ( HairStyle <= 0x2FD2 ) ) ||
       ( ( 0x4258 <= HairStyle ) && ( HairStyle <= 0x425F ) ) ||
       ( ( 0x4261 <= HairStyle ) && ( HairStyle <= 0x4262 ) ) ||
       ( ( 0x4273 <= HairStyle ) && ( HairStyle <= 0x4275 ) ) ||
       ( ( 0x42aa <= HairStyle ) && ( HairStyle <= 0x42ab ) ) || ( HairStyle == 0x42B1 ) )
    return true;
  return false;
}

/* beard can be:
    0x203E  Long Beard     // Human
    0x203F  Short Beard
    0x2040  Goatee
    0x2041  Mustache
    0x204B  Medium Short Beard
    0x204C  Medium Long Beard
    0x204D  Vandyke

    0x42ad  facial horn style 1 // Gargoyle (SA)
    0x42ae  facial horn style 2
    0x42af  facial horn style 3
    0x42b0  facial horn style 4
    */
bool validbeard( u16 BeardStyle )
{
  if ( ( ( 0x203E <= BeardStyle ) && ( BeardStyle <= 0x2041 ) ) ||
       ( ( 0x204B <= BeardStyle ) && ( BeardStyle <= 0x204D ) ) ||
       ( ( 0x42AD <= BeardStyle ) && ( BeardStyle <= 0x42B0 ) &&
         ( Plib::systemstate.config.max_tile_id > BeardStyle ) ) )
    return true;
  return false;
}

/* face can be:
    0x3B44  face 1
    0x3B45  face 2
    0x3B46  face 3
    0x3B47  face 4
    0x3B48  face 5
    0x3B49  face 6
    0x3B4A  face 7
    0x3B4B  face 8
    0x3B4C  face 9
    0x3B4D  face 10
    0x3B4E  anime     //roleplay faces
    0x3B4F  hellian
    0x3B50  juka
    0x3B51  undead
    0x3B52  meer
    0x3B53  elder
    0x3B54  orc
    0x3B55  pirate
    0x3B56  native papuan
    0x3B57  vampire
    */
bool validface( u16 FaceStyle )
{
  switch ( settingsManager.ssopt.features.faceSupport() )
  {
  case Plib::FaceSupport::None:
    return false;
  case Plib::FaceSupport::RolePlay:
    if ( ( 0x3B4E <= FaceStyle ) && ( FaceStyle <= 0x3B57 ) )
      return true;
    [[fallthrough]];
  case Plib::FaceSupport::Basic:
    if ( ( 0x3B44 <= FaceStyle ) && ( FaceStyle <= 0x3B4D ) )
      return true;
  }
  return false;
}

struct CreateCharacterRequest
{
  std::string name;
  u8 strength = 0;
  u8 dexterity = 0;
  u8 intelligence = 0;
  u8 profession = 0;
  u32 skill_total = 0;                    ///< what the skill values must add up to
  std::vector<std::pair<u8, u8>> skills;  ///< skill number and its starting value
};

std::string packet_name( const char* field, size_t width )
{
  return std::string( field, strnlen( field, width ) );
}

/**
 * Check a request before anything is built from it, logging and disconnecting on the first fault.
 * @param noskills answers whether the request asked for a profession's skills rather than its own
 */
bool create_request_is_valid( Network::Client* client, const CreateCharacterRequest& req,
                              bool* noskills )
{
  const auto unprintable = []( char ch ) { return ch < ' ' || ch > '~' || ch == '{' || ch == '}'; };
  if ( const auto bad = std::ranges::find_if( req.name, unprintable ); bad != req.name.end() )
  {
    ERROR_PRINTLN(
        "Create Character: Attempted to use invalid character '{}' pos '{}' in name '{}'. Client "
        "IP: {} Client Name: {}",
        *bad, std::distance( req.name.begin(), bad ), req.name, client->ipaddrAsString(),
        client->acct->name() );
    client->forceDisconnect();
    return false;
  }

  const unsigned stat_total = req.strength + req.intelligence + req.dexterity;
  const auto in_range = [stat_total]( const std::string& spec )
  {
    // "65" is a value, "65-80" a range. Base 0, so a shard may write either in hex.
    char* after = nullptr;
    const unsigned long low = strtoul( spec.c_str(), &after, 0 );
    const unsigned long high = ( *after == '-' ) ? strtoul( after + 1, nullptr, 0 ) : low;
    return stat_total >= low && stat_total <= high;
  };
  if ( !std::ranges::any_of( settingsManager.ssopt.total_stats_at_creation, in_range ) )
  {
    ERROR_PRINTLN( "Create Character: Stats sum to {}.\nValid values/ranges are: {}", stat_total,
                   settingsManager.ssopt.total_stats_at_creation );
    client->forceDisconnect();
    return false;
  }
  if ( req.strength < 10 || req.intelligence < 10 || req.dexterity < 10 )
  {
    ERROR_PRINTLN( "Create Character: A stat was too small. Str={} Int={} Dex={}", req.strength,
                   req.intelligence, req.dexterity );
    client->forceDisconnect();
    return false;
  }

  const auto out_of_range = []( const auto& skill )
  { return skill.first > networkManager.uoclient_general.maxskills; };
  if ( std::ranges::any_of( req.skills, out_of_range ) )
  {
    ERROR_PRINTLN( "Create Character: A skill number was out of range" );
    client->forceDisconnect();
    return false;
  }

  u32 value_total = 0;
  for ( const auto& [number, value] : req.skills )
    value_total += value;

  // A profession picks the skills instead, and then the values arrive empty.
  *noskills = ( value_total == 0 ) && req.profession;
  const auto too_generous = []( const auto& skill ) { return skill.second > 50; };
  if ( !*noskills &&
       ( value_total != req.skill_total || std::ranges::any_of( req.skills, too_generous ) ) )
  {
    ERROR_PRINTLN( "Create Character: Starting skill values incorrect" );
    client->forceDisconnect();
    return false;
  }

  return true;
}

void ClientCreateChar( Network::Client* client, PKTIN_00* msg )
{
  if ( client->acct == nullptr )
  {
    ERROR_PRINTLN( "Client from {} tried to create a character without an account!",
                   client->ipaddrAsString() );
    client->forceDisconnect();
    return;
  }
  if ( Plib::systemstate.config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
  {
    // FIXME: Add send_login_error!
    client->Disconnect();
    return;
  }
  if ( msg->CharNumber >= Plib::systemstate.config.character_slots ||
       client->acct->get_character( msg->CharNumber ) != nullptr ||
       msg->StartIndex >= gamestate.startlocations.size() )
  {
    ERROR_PRINTLN( "Create Character: Invalid parameters." );
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
       client->acct->has_active_characters() )
  {
    send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
    client->Disconnect();
    return;
  }

  unsigned short graphic;
  Plib::URACE race;
  Plib::UGENDER gender = ( ( msg->Sex & Network::FLAG_GENDER ) == Network::FLAG_GENDER )
                             ? Plib::GENDER_FEMALE
                             : Plib::GENDER_MALE;
  if ( client->ClientType & Network::CLIENTTYPE_7000 )
  {
    /*
    0x00 / 0x01 = human male/female
    0x02 / 0x03 = human male/female
    0x04 / 0x05 = elf male/female
    0x06 / 0x07 = gargoyle male/female
    */
    if ( ( msg->Sex & 0x6 ) == 0x6 )
    {
      race = Plib::RACE_GARGOYLE;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_GARGOYLE_FEMALE : UOBJ_GARGOYLE_MALE;
    }
    else if ( ( msg->Sex & 0x4 ) == 0x4 )
    {
      race = Plib::RACE_ELF;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
    }
    else
    {
      race = Plib::RACE_HUMAN;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
    }
  }
  else
  {
    /*
    0x00 / 0x01 = human male/female
    0x02 / 0x03 = elf male/female
    */
    if ( ( msg->Sex & Network::FLAG_RACE ) == Network::FLAG_RACE )
    {
      race = Plib::RACE_ELF;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
    }
    else
    {
      race = Plib::RACE_HUMAN;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
    }
  }

  CreateCharacterRequest req{ .name = packet_name( msg->Name, sizeof msg->Name ),
                              .strength = msg->Strength,
                              .dexterity = msg->Dexterity,
                              .intelligence = msg->Intelligence,
                              .profession = msg->profession,
                              .skill_total = 100,
                              .skills = { { msg->SkillNumber1, msg->SkillValue1 },
                                          { msg->SkillNumber2, msg->SkillValue2 },
                                          { msg->SkillNumber3, msg->SkillValue3 } } };
  bool noskills = false;
  if ( !create_request_is_valid( client, req, &noskills ) )
    return;

  Mobile::Character* chr = new Mobile::Character( graphic );

  chr->acct.set( client->acct );
  chr->client = client;
  chr->set_privs( client->acct->default_privlist() );
  chr->cmdlevel( client->acct->default_cmdlevel(), false );

  client->UOExpansionFlagClient = ctBEu32( msg->clientflag );

  chr->name_ = req.name;

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->wornitems->adopt( *chr );

  chr->graphic = graphic;
  chr->race = race;
  chr->gender = gender;

  chr->trueobjtype = chr->objtype_;
  chr->color = cfBEu16( msg->SkinColor );
  chr->truecolor = chr->color;

  chr->setposition( gamestate.startlocations[msg->StartIndex]->select_coordinate() );
  chr->facing = Core::FACING_W;
  chr->position_changed();

  if ( gamestate.pAttrStrength )
    chr->attribute( gamestate.pAttrStrength->attrid ).base( msg->Strength * 10 );
  if ( gamestate.pAttrIntelligence )
    chr->attribute( gamestate.pAttrIntelligence->attrid ).base( msg->Intelligence * 10 );
  if ( gamestate.pAttrDexterity )
    chr->attribute( gamestate.pAttrDexterity->attrid ).base( msg->Dexterity * 10 );

  ////HASH
  // moved down here, after all error checking passes, else we get a half-created PC in the save.
  objStorageManager.objecthash.Insert( chr );
  ////

  if ( !noskills )
  {
    const Mobile::Attribute* pAttr;
    pAttr = GetUOSkill( msg->SkillNumber1 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue1 * 10 );
    pAttr = GetUOSkill( msg->SkillNumber2 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue2 * 10 );
    pAttr = GetUOSkill( msg->SkillNumber3 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue3 * 10 );
  }

  // not yet entered the world, dont send vital pkts
  chr->calc_vital_stuff( Mobile::Character::VitalCalcFlags::ATTRIBUTES |
                         Mobile::Character::VitalCalcFlags::VITALS );
  chr->set_vitals_to_maximum( false );


  chr->created_at = read_gameclock();

  Items::Item* tmpitem;
  if ( validhair( cfBEu16( msg->HairStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->HairStyle ) );
    tmpitem->color = cfBEu16( msg->HairColor );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip hair {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validbeard( cfBEu16( msg->BeardStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->BeardStyle ) );
    tmpitem->color = cfBEu16( msg->BeardColor );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip beard {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  UContainer* backpack = (UContainer*)Items::Item::create( UOBJ_BACKPACK );
  chr->equip( backpack );

  if ( settingsManager.ssopt.starting_gold != 0 )
  {
    tmpitem = Items::Item::create( 0x0EED );
    tmpitem->setamount( settingsManager.ssopt.starting_gold );
    if ( !Items::move_into( *tmpitem, *backpack, Pos2d( 46, 91 ) ) )
    {
      if ( Items::place_at( *tmpitem, chr->pos() ) )
      {
        // Not a move: it is already where it is going, so this only shows it to everyone standing
        // there.
        send_item_moved( tmpitem, tmpitem->pos() );
      }
    }
  }

  if ( chr->race == Plib::RACE_HUMAN ||
       chr->race == Plib::RACE_ELF )  // Gargoyles dont have shirts, pants, shoes and daggers.
  {
    tmpitem = Items::Item::create( 0x170F );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = 0x021F;
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( 0xF51 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    chr->equip( tmpitem );

    unsigned short pantstype, shirttype;
    if ( chr->gender == Plib::GENDER_FEMALE )
    {
      pantstype = 0x1516;
      shirttype = 0x1517;
    }
    else
    {
      pantstype = 0x152e;
      shirttype = 0x1517;
    }

    tmpitem = Items::Item::create( pantstype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->pantscolor );  // 0x0284;
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( shirttype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    chr->equip( tmpitem );
  }
  else if ( chr->race == Plib::RACE_GARGOYLE )  // Gargoyles have Robes.
  {
    tmpitem = Items::Item::create( 0x1F03 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    chr->equip( tmpitem );
  }

  client->chr = chr;
  client->acct->set_character( msg->CharNumber, client->chr );

  POLLOGLN( "Account {} created character {:#x}", client->acct->name(), chr->serial );
  SetCharacterWorldPosition( chr, Realms::WorldChangeReason::PlayerEnter );
  client->msgtype_filter = networkManager.game_filter.get();
  start_client_char( client );

  // FIXME : Shouldn't this be triggered at the end of creation?
  run_logon_script( chr );

  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/oncreate", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    std::unique_ptr<UOExecutor> ex( create_script_executor() );

    std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
    arr->addElement( new Bscript::BLong( msg->SkillNumber1 ) );
    arr->addElement( new Bscript::BLong( msg->SkillNumber2 ) );
    arr->addElement( new Bscript::BLong( msg->SkillNumber3 ) );

    ex->pushArg( new Bscript::BLong( msg->profession ) );
    ex->pushArg( arr.release() );
    ex->pushArg( make_mobileref( chr ) );

    ex->addModule( new Module::UOExecutorModule( *ex ) );
    ex->critical( true );

    if ( ex->setProgram( prog.get() ) )
    {
      schedule_executor( ex.release() );
    }
    else
    {
      ERROR_PRINTLN( "script misc/oncreate: setProgram failed" );
    }
  }
}

void createchar2( Accounts::Account* acct, unsigned index )
{
  Mobile::Character* chr = new Mobile::Character( UOBJ_HUMAN_MALE );
  chr->acct.set( acct );
  acct->set_character( index, chr );
  chr->setname( "new character" );

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->setposition( Pos4d( 1, 1, 1, find_realm( std::string( "britannia" ) ) ) );
  chr->facing = 1;
  chr->wornitems->adopt( *chr );
  chr->position_changed();
  chr->graphic = UOBJ_HUMAN_MALE;
  chr->gender = Plib::GENDER_MALE;
  chr->trueobjtype = chr->objtype_;
  chr->color = ctBEu16( 0 );
  chr->truecolor = chr->color;
  chr->created_at = read_gameclock();

  objStorageManager.objecthash.Insert( chr );
  chr->logged_in( false );  // constructor sets it
}


void ClientCreateCharKR( Network::Client* client, PKTIN_8D* msg )
{
  int charslot = ctBEu32( msg->char_slot );
  if ( client->acct == nullptr )
  {
    ERROR_PRINTLN( "Client from {} tried to create a character without an account!",
                   client->ipaddrAsString() );
    client->Disconnect();
    return;
  }
  if ( Plib::systemstate.config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
  {
    // FIXME: Add send_login_error ...
    client->Disconnect();
    return;
  }
  if ( charslot >= Plib::systemstate.config.character_slots ||
       client->acct->get_character( charslot ) != nullptr )
  {
    ERROR_PRINTLN( "Create Character: Invalid parameters." );
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
       client->acct->has_active_characters() )
  {
    send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
    client->Disconnect();
    return;
  }

  unsigned short graphic;
  Plib::URACE race = (Plib::URACE)( msg->race - 1 );
  Plib::UGENDER gender =
      ( msg->gender & Plib::GENDER_FEMALE ) ? Plib::GENDER_FEMALE : Plib::GENDER_MALE;
  if ( race == Plib::RACE_HUMAN )
    graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
  else if ( race == Plib::RACE_ELF )
    graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
  else
    graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_GARGOYLE_FEMALE : UOBJ_GARGOYLE_MALE;


  CreateCharacterRequest req{ .name = packet_name( msg->name, sizeof msg->name ),
                              .strength = msg->strength,
                              .dexterity = msg->dexterity,
                              .intelligence = msg->intelligence,
                              .profession = msg->profession,
                              .skill_total = 120,
                              .skills = { { msg->skillnumber1, msg->skillvalue1 },
                                          { msg->skillnumber2, msg->skillvalue2 },
                                          { msg->skillnumber3, msg->skillvalue3 },
                                          { msg->skillnumber4, msg->skillvalue4 } } };
  bool noskills = false;
  if ( !create_request_is_valid( client, req, &noskills ) )
    return;

  Mobile::Character* chr = new Mobile::Character( graphic );

  chr->acct.set( client->acct );
  chr->client = client;
  chr->set_privs( client->acct->default_privlist() );
  chr->cmdlevel( client->acct->default_cmdlevel(), false );

  client->UOExpansionFlagClient = msg->flags;

  chr->name_ = req.name;

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->wornitems->adopt( *chr );

  chr->graphic = graphic;
  chr->race = race;
  chr->gender = gender;

  chr->trueobjtype = chr->objtype_;
  chr->color = cfBEu16( msg->skin_color );
  chr->truecolor = chr->color;

  chr->setposition( gamestate.startlocations[0]->select_coordinate() );
  chr->position_changed();
  chr->facing = Core::FACING_W;

  if ( gamestate.pAttrStrength )
    chr->attribute( gamestate.pAttrStrength->attrid ).base( msg->strength * 10 );
  if ( gamestate.pAttrIntelligence )
    chr->attribute( gamestate.pAttrIntelligence->attrid ).base( msg->intelligence * 10 );
  if ( gamestate.pAttrDexterity )
    chr->attribute( gamestate.pAttrDexterity->attrid ).base( msg->dexterity * 10 );

  ////HASH
  // moved down here, after all error checking passes, else we get a half-created PC in the save.
  objStorageManager.objecthash.Insert( chr );
  ////

  if ( !noskills )
  {
    const Mobile::Attribute* pAttr;
    pAttr = GetUOSkill( msg->skillnumber1 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->skillvalue1 * 10 );
    pAttr = GetUOSkill( msg->skillnumber2 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->skillvalue2 * 10 );
    pAttr = GetUOSkill( msg->skillnumber3 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->skillvalue3 * 10 );
    pAttr = GetUOSkill( msg->skillnumber4 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->skillvalue4 * 10 );
  }

  // not yet entered the world, dont send vital pkts
  chr->calc_vital_stuff( Mobile::Character::VitalCalcFlags::ATTRIBUTES |
                         Mobile::Character::VitalCalcFlags::VITALS );
  chr->set_vitals_to_maximum( false );


  chr->created_at = read_gameclock();

  Items::Item* tmpitem;
  if ( validhair( cfBEu16( msg->hairstyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->hairstyle ) );
    tmpitem->color = cfBEu16( msg->haircolor );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip hair {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validbeard( cfBEu16( msg->beardstyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->beardstyle ) );
    tmpitem->color = cfBEu16( msg->beardcolor );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip beard {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validface( cfBEu16( msg->face_id ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->face_id ) );
    tmpitem->color = cfBEu16( msg->face_color );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip face {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  UContainer* backpack = (UContainer*)Items::Item::create( UOBJ_BACKPACK );
  chr->equip( backpack );

  if ( settingsManager.ssopt.starting_gold != 0 )
  {
    tmpitem = Items::Item::create( 0x0EED );
    tmpitem->setamount( settingsManager.ssopt.starting_gold );
    if ( !Items::move_into( *tmpitem, *backpack, Pos2d( 46, 91 ) ) )
    {
      if ( Items::place_at( *tmpitem, chr->pos() ) )
      {
        // Not a move: it is already where it is going, so this only shows it to everyone standing
        // there.
        send_item_moved( tmpitem, tmpitem->pos() );
      }
    }
  }

  if ( chr->race == Plib::RACE_HUMAN ||
       chr->race == Plib::RACE_ELF )  // Gargoyles dont have shirts, pants, shoes and daggers.
  {
    tmpitem = Items::Item::create( 0x170F );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = 0x021F;
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( 0xF51 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    chr->equip( tmpitem );

    unsigned short pantstype, shirttype;
    if ( chr->gender == Plib::GENDER_FEMALE )
    {
      pantstype = 0x1516;
      shirttype = 0x1517;
    }
    else
    {
      pantstype = 0x152e;
      shirttype = 0x1517;
    }

    tmpitem = Items::Item::create( pantstype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->pantscolor );  // 0x0284;
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( shirttype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    chr->equip( tmpitem );
  }
  else if ( chr->race == Plib::RACE_GARGOYLE )  // Gargoyles have Robes.
  {
    tmpitem = Items::Item::create( 0x1F03 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    chr->equip( tmpitem );
  }

  client->chr = chr;
  client->acct->set_character( charslot, client->chr );

  POLLOGLN( "Account {} created character {:#x}", client->acct->name(), chr->serial );
  SetCharacterWorldPosition( chr, Realms::WorldChangeReason::PlayerEnter );
  client->msgtype_filter = networkManager.game_filter.get();
  start_client_char( client );

  // FIXME : Shouldn't this be triggered at the end of creation?
  run_logon_script( chr );

  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/oncreate", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    std::unique_ptr<UOExecutor> ex( create_script_executor() );

    std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
    arr->addElement( new Bscript::BLong( msg->skillnumber1 ) );
    arr->addElement( new Bscript::BLong( msg->skillnumber2 ) );
    arr->addElement( new Bscript::BLong( msg->skillnumber3 ) );
    arr->addElement( new Bscript::BLong( msg->skillnumber4 ) );

    ex->pushArg( new Bscript::BLong( msg->profession ) );
    ex->pushArg( arr.release() );
    ex->pushArg( make_mobileref( chr ) );

    ex->addModule( new Module::UOExecutorModule( *ex ) );
    ex->critical( true );

    if ( ex->setProgram( prog.get() ) )
    {
      schedule_executor( ex.release() );
    }
    else
    {
      ERROR_PRINTLN( "script misc/oncreate: setProgram failed" );
    }
  }
}

void ClientCreateChar70160( Network::Client* client, PKTIN_F8* msg )
{
  if ( client->acct == nullptr )
  {
    ERROR_PRINTLN( "Client from {} tried to create a character without an account!",
                   client->ipaddrAsString() );
    client->forceDisconnect();
    return;
  }
  if ( Plib::systemstate.config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
  {
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  if ( msg->CharNumber >= Plib::systemstate.config.character_slots ||
       client->acct->get_character( msg->CharNumber ) != nullptr ||
       msg->StartIndex >= gamestate.startlocations.size() )
  {
    ERROR_PRINTLN( "Create Character: Invalid parameters." );
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
       client->acct->has_active_characters() )
  {
    send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
    client->Disconnect();
    return;
  }

  unsigned short graphic;
  Plib::URACE race;
  Plib::UGENDER gender = ( ( msg->Sex & Network::FLAG_GENDER ) == Network::FLAG_GENDER )
                             ? Plib::GENDER_FEMALE
                             : Plib::GENDER_MALE;
  if ( client->ClientType & Network::CLIENTTYPE_7000 )
  {
    /*
    0x00 / 0x01 = human male/female
    0x02 / 0x03 = human male/female
    0x04 / 0x05 = elf male/female
    0x06 / 0x07 = gargoyle male/female
    */
    if ( ( msg->Sex & 0x6 ) == 0x6 )
    {
      race = Plib::RACE_GARGOYLE;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_GARGOYLE_FEMALE : UOBJ_GARGOYLE_MALE;
    }
    else if ( ( msg->Sex & 0x4 ) == 0x4 )
    {
      race = Plib::RACE_ELF;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
    }
    else
    {
      race = Plib::RACE_HUMAN;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
    }
  }
  else
  {
    /*
    0x00 / 0x01 = human male/female
    0x02 / 0x03 = elf male/female
    */
    if ( ( msg->Sex & Network::FLAG_RACE ) == Network::FLAG_RACE )
    {
      race = Plib::RACE_ELF;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
    }
    else
    {
      race = Plib::RACE_HUMAN;
      graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
    }
  }

  // With latest clients EA broke the prof.txt, added Evaluating Intelligence and Spirit Speak which
  // returns SkillNumber 0xFF
  // Check for it here to not crash the client during char creation
  bool broken_prof = ( msg->SkillNumber1 == 0xFF || msg->SkillNumber2 == 0xFF ||
                       msg->SkillNumber3 == 0xFF || msg->SkillNumber4 == 0xFF ) &&
                     msg->profession;

  if ( broken_prof )
  {
    unsigned char temp_skillid = 0;

    if ( msg->profession == 2 )  // Mage profession
      temp_skillid = SKILLID_EVALUATINGINTEL;
    if ( msg->profession == 4 )  // Necromancy profession
      temp_skillid = SKILLID_SPIRITSPEAK;


    if ( msg->SkillNumber1 == 0xFF )
    {
      msg->SkillNumber1 = temp_skillid;
      msg->SkillValue1 = 30;
    }
    else if ( msg->SkillNumber2 == 0xFF )
    {
      msg->SkillNumber2 = temp_skillid;
      msg->SkillValue2 = 30;
    }
    else if ( msg->SkillNumber3 == 0xFF )
    {
      msg->SkillNumber3 = temp_skillid;
      msg->SkillValue3 = 30;
    }
    else if ( msg->SkillNumber4 == 0xFF )
    {
      msg->SkillNumber4 = temp_skillid;
      msg->SkillValue4 = 30;
    }
  }

  CreateCharacterRequest req{ .name = packet_name( msg->Name, sizeof msg->Name ),
                              .strength = msg->Strength,
                              .dexterity = msg->Dexterity,
                              .intelligence = msg->Intelligence,
                              .profession = msg->profession,
                              .skill_total = 120,
                              .skills = { { msg->SkillNumber1, msg->SkillValue1 },
                                          { msg->SkillNumber2, msg->SkillValue2 },
                                          { msg->SkillNumber3, msg->SkillValue3 },
                                          { msg->SkillNumber4, msg->SkillValue4 } } };
  bool noskills = false;
  if ( !create_request_is_valid( client, req, &noskills ) )
    return;

  Mobile::Character* chr = new Mobile::Character( graphic );

  chr->acct.set( client->acct );
  chr->client = client;
  chr->set_privs( client->acct->default_privlist() );
  chr->cmdlevel( client->acct->default_cmdlevel(), false );

  client->UOExpansionFlagClient = ctBEu32( msg->clientflag );

  chr->name_ = req.name;

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->wornitems->adopt( *chr );

  chr->graphic = graphic;
  chr->race = race;
  chr->gender = gender;

  chr->trueobjtype = chr->objtype_;
  chr->color = cfBEu16( msg->SkinColor );
  chr->truecolor = chr->color;

  chr->setposition( gamestate.startlocations[msg->StartIndex]->select_coordinate() );
  chr->position_changed();
  chr->facing = Core::FACING_W;

  if ( gamestate.pAttrStrength )
    chr->attribute( gamestate.pAttrStrength->attrid ).base( msg->Strength * 10 );
  if ( gamestate.pAttrIntelligence )
    chr->attribute( gamestate.pAttrIntelligence->attrid ).base( msg->Intelligence * 10 );
  if ( gamestate.pAttrDexterity )
    chr->attribute( gamestate.pAttrDexterity->attrid ).base( msg->Dexterity * 10 );

  ////HASH
  // moved down here, after all error checking passes, else we get a half-created PC in the save.
  objStorageManager.objecthash.Insert( chr );
  ////

  if ( !noskills )
  {
    const Mobile::Attribute* pAttr;
    pAttr = GetUOSkill( msg->SkillNumber1 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue1 * 10 );
    pAttr = GetUOSkill( msg->SkillNumber2 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue2 * 10 );
    pAttr = GetUOSkill( msg->SkillNumber3 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue3 * 10 );
    pAttr = GetUOSkill( msg->SkillNumber4 ).pAttr;
    if ( pAttr )
      chr->attribute( pAttr->attrid ).base( msg->SkillValue4 * 10 );
  }

  // not yet entered the world, dont send vital pkts
  chr->calc_vital_stuff( Mobile::Character::VitalCalcFlags::ATTRIBUTES |
                         Mobile::Character::VitalCalcFlags::VITALS );
  chr->set_vitals_to_maximum( false );


  chr->created_at = read_gameclock();

  Items::Item* tmpitem;
  if ( validhair( cfBEu16( msg->HairStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->HairStyle ) );
    tmpitem->color = cfBEu16( msg->HairColor );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip hair {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validbeard( cfBEu16( msg->BeardStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->BeardStyle ) );
    tmpitem->color = cfBEu16( msg->BeardColor );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip beard {:#x}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  UContainer* backpack = (UContainer*)Items::Item::create( UOBJ_BACKPACK );
  chr->equip( backpack );

  if ( settingsManager.ssopt.starting_gold != 0 )
  {
    tmpitem = Items::Item::create( 0x0EED );
    tmpitem->setamount( settingsManager.ssopt.starting_gold );
    if ( !Items::move_into( *tmpitem, *backpack, Pos2d( 46, 91 ) ) )
    {
      if ( Items::place_at( *tmpitem, chr->pos() ) )
      {
        // Not a move: it is already where it is going, so this only shows it to everyone standing
        // there.
        send_item_moved( tmpitem, tmpitem->pos() );
      }
    }
  }

  if ( chr->race == Plib::RACE_HUMAN ||
       chr->race == Plib::RACE_ELF )  // Gargoyles dont have shirts, pants, shoes and daggers.
  {
    tmpitem = Items::Item::create( 0x170F );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = 0x021F;
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( 0xF51 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    chr->equip( tmpitem );

    unsigned short pantstype, shirttype;
    if ( chr->gender == Plib::GENDER_FEMALE )
    {
      pantstype = 0x1516;
      shirttype = 0x1517;
    }
    else
    {
      pantstype = 0x152e;
      shirttype = 0x1517;
    }

    tmpitem = Items::Item::create( pantstype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->pantscolor );  // 0x0284;
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( shirttype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    chr->equip( tmpitem );
  }
  else if ( chr->race == Plib::RACE_GARGOYLE )  // Gargoyles have Robes.
  {
    tmpitem = Items::Item::create( 0x1F03 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    chr->equip( tmpitem );
  }

  client->chr = chr;
  client->acct->set_character( msg->CharNumber, client->chr );

  POLLOGLN( "Account {} created character {:#x}", client->acct->name(), chr->serial );
  SetCharacterWorldPosition( chr, Realms::WorldChangeReason::PlayerEnter );
  client->msgtype_filter = networkManager.game_filter.get();
  start_client_char( client );

  // FIXME : Shouldn't this be triggered at the end of creation?
  run_logon_script( chr );

  ref_ptr<Bscript::EScriptProgram> prog =
      find_script( "misc/oncreate", true, Plib::systemstate.config.cache_interactive_scripts );
  if ( prog.get() != nullptr )
  {
    std::unique_ptr<UOExecutor> ex( create_script_executor() );

    std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
    arr->addElement( new Bscript::BLong( msg->SkillNumber1 ) );
    arr->addElement( new Bscript::BLong( msg->SkillNumber2 ) );
    arr->addElement( new Bscript::BLong( msg->SkillNumber3 ) );
    arr->addElement( new Bscript::BLong( msg->SkillNumber4 ) );

    ex->pushArg( new Bscript::BLong( msg->profession ) );
    ex->pushArg( arr.release() );
    ex->pushArg( make_mobileref( chr ) );

    ex->addModule( new Module::UOExecutorModule( *ex ) );
    ex->critical( true );

    if ( ex->setProgram( prog.get() ) )
    {
      schedule_executor( ex.release() );
    }
    else
    {
      ERROR_PRINTLN( "script misc/oncreate: setProgram failed" );
    }
  }
}
}  // namespace Pol::Core
