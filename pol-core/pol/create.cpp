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
#include <string>

#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../plib/clidata.h"
#include "../plib/systemstate.h"
#include "../plib/uconst.h"
#include "accounts/account.h"
#include "containr.h"
#include "gameclck.h"
#include "globals/network.h"
#include "globals/object_storage.h"
#include "globals/uvars.h"
#include "item/item.h"
#include "layers.h"
#include "mkscrobj.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "mobile/wornitems.h"
#include "module/uomod.h"
#include "network/client.h"
#include "network/pktdef.h"
#include "network/pktin.h"
#include "objtype.h"
#include "polcfg.h"
#include "realms/WorldChangeReasons.h"
#include "scrsched.h"
#include "scrstore.h"
#include "skillid.h"
#include "startloc.h"
#include "ufunc.h"
#include "uoclient.h"
#include "uoexec.h"
#include "uoskills.h"
#include "uworld.h"
#include <format/format.h>


namespace Pol
{
namespace Core
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
  else
  {
    if ( ( ( 0x203B <= HairStyle ) && ( HairStyle <= 0x203D ) ) ||
         ( ( 0x2044 <= HairStyle ) && ( HairStyle <= 0x204A ) ) ||
         ( ( 0x2FBF <= HairStyle ) && ( HairStyle <= 0x2FC2 ) ) ||
         ( ( 0x2FCC <= HairStyle ) && ( HairStyle <= 0x2FD2 ) ) ||
         ( ( 0x4258 <= HairStyle ) && ( HairStyle <= 0x425F ) ) ||
         ( ( 0x4261 <= HairStyle ) && ( HairStyle <= 0x4262 ) ) ||
         ( ( 0x4273 <= HairStyle ) && ( HairStyle <= 0x4275 ) ) ||
         ( ( 0x42aa <= HairStyle ) && ( HairStyle <= 0x42ab ) ) || ( HairStyle == 0x42B1 ) )
      return true;
    else
      return false;
  }
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
  else
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
  if ( settingsManager.ssopt.support_faces > 0 )
  {
    if ( ( 0x3B44 <= FaceStyle ) && ( FaceStyle <= 0x3B4D ) )
      return true;
    if ( settingsManager.ssopt.support_faces == 2 )
    {
      if ( ( 0x3B4E <= FaceStyle ) && ( FaceStyle <= 0x3B57 ) )
        return true;
    }
  }
  return false;
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
  else if ( Plib::systemstate.config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
  {
    // FIXME: Add send_login_error!
    client->Disconnect();
    return;
  }
  else if ( msg->CharNumber >= Plib::systemstate.config.character_slots ||
            client->acct->get_character( msg->CharNumber ) != nullptr ||
            msg->StartIndex >= gamestate.startlocations.size() )
  {
    ERROR_PRINTLN( "Create Character: Invalid parameters." );
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  else if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
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

  Mobile::Character* chr = new Mobile::Character( graphic );

  chr->acct.set( client->acct );
  chr->client = client;
  chr->set_privs( client->acct->default_privlist() );
  chr->cmdlevel( client->acct->default_cmdlevel(), false );

  client->UOExpansionFlagClient = ctBEu32( msg->clientflag );

  std::string tmpstr( msg->Name, sizeof msg->Name );
  const char* tstr = tmpstr.c_str();
  for ( unsigned int i = 0; i < strlen( tstr ); i++ )
  {
    char tmpchr = tstr[i];
    if ( tmpchr >= ' ' && tmpchr <= '~' )
    {
      if ( tmpchr != '{' && tmpchr != '}' )
        continue;
    }

    ERROR_PRINTLN(
        "Create Character: Attempted to use invalid character '{}' pos '{}' in name '{}'. Client "
        "IP: {} Client Name: {}",
        tmpchr, i, tstr, client->ipaddrAsString(), client->acct->name() );
    client->forceDisconnect();
    return;
  }
  chr->name_ = tstr;

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->wornitems->serial = chr->serial;
  chr->wornitems->serial_ext = chr->serial_ext;

  chr->graphic = graphic;
  chr->race = race;
  chr->gender = gender;

  chr->trueobjtype = chr->objtype_;
  chr->color = cfBEu16( msg->SkinColor );
  chr->truecolor = chr->color;

  Pos3d coord = gamestate.startlocations[msg->StartIndex]->select_coordinate();
  Realms::Realm* realm = gamestate.startlocations[msg->StartIndex]->realm;
  chr->setposition( Pos4d( coord, realm ) );
  chr->facing = Core::FACING_W;
  chr->position_changed();

  bool valid_stats = false;
  unsigned int stat_total = msg->Strength + msg->Intelligence + msg->Dexterity;
  unsigned int stat_min, stat_max;
  char* maxpos;
  std::vector<std::string>::size_type sidx;
  for ( sidx = 0; !valid_stats && sidx < settingsManager.ssopt.total_stats_at_creation.size();
        ++sidx )
  {
    const char* statstr = settingsManager.ssopt.total_stats_at_creation[sidx].c_str();
    stat_max = ( stat_min = strtoul( statstr, &maxpos, 0 ) );
    if ( *( maxpos++ ) == '-' )
      stat_max = strtoul( maxpos, nullptr, 0 );
    if ( stat_total >= stat_min && stat_total <= stat_max )
      valid_stats = true;
  }
  if ( !valid_stats )
  {
    ERROR_PRINTLN( "Create Character: Stats sum to {}.\nValid values/ranges are: {}", stat_total,
                   settingsManager.ssopt.total_stats_at_creation );
    client->forceDisconnect();
    return;
  }
  if ( msg->Strength < 10 || msg->Intelligence < 10 || msg->Dexterity < 10 )
  {
    ERROR_PRINTLN( "Create Character: A stat was too small. Str={} Int={} Dex={}", msg->Strength,
                   msg->Intelligence, msg->Dexterity );

    client->forceDisconnect();
    return;
  }
  if ( gamestate.pAttrStrength )
    chr->attribute( gamestate.pAttrStrength->attrid ).base( msg->Strength * 10 );
  if ( gamestate.pAttrIntelligence )
    chr->attribute( gamestate.pAttrIntelligence->attrid ).base( msg->Intelligence * 10 );
  if ( gamestate.pAttrDexterity )
    chr->attribute( gamestate.pAttrDexterity->attrid ).base( msg->Dexterity * 10 );

  if ( msg->SkillNumber1 > networkManager.uoclient_general.maxskills ||
       msg->SkillNumber2 > networkManager.uoclient_general.maxskills ||
       msg->SkillNumber3 > networkManager.uoclient_general.maxskills )
  {
    ERROR_PRINTLN( "Create Character: A skill number was out of range" );
    client->forceDisconnect();
    return;
  }
  bool noskills =
      ( msg->SkillValue1 + msg->SkillValue2 + msg->SkillValue3 == 0 ) && msg->profession;
  if ( ( !noskills ) &&
       ( ( msg->SkillValue1 + msg->SkillValue2 + msg->SkillValue3 != 100 ) ||
         msg->SkillValue1 > 50 || msg->SkillValue2 > 50 || msg->SkillValue3 > 50 ) )
  {
    ERROR_PRINTLN( "Create Character: Starting skill values incorrect" );
    client->forceDisconnect();
    return;
  }

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

  chr->calc_vital_stuff();
  chr->set_vitals_to_maximum();


  chr->created_at = read_gameclock();

  Items::Item* tmpitem;
  if ( validhair( cfBEu16( msg->HairStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->HairStyle ) );
    tmpitem->layer = LAYER_HAIR;
    tmpitem->color = cfBEu16( msg->HairColor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    if ( chr->equippable( tmpitem ) )                        // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip hair {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validbeard( cfBEu16( msg->BeardStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->BeardStyle ) );
    tmpitem->layer = LAYER_BEARD;
    tmpitem->color = cfBEu16( msg->BeardColor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );
    if ( chr->equippable( tmpitem ) )  // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip beard {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  UContainer* backpack = (UContainer*)Items::Item::create( UOBJ_BACKPACK );
  backpack->layer = LAYER_BACKPACK;
  backpack->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
  chr->equip( backpack );

  if ( settingsManager.ssopt.starting_gold != 0 )
  {
    tmpitem = Items::Item::create( 0x0EED );
    tmpitem->setamount( settingsManager.ssopt.starting_gold );
    tmpitem->setposition( Pos4d( 46, 91, 0, chr->realm() ) );
    u8 newSlot = 1;
    if ( !backpack->can_add_to_slot( newSlot ) || !tmpitem->slot_index( newSlot ) )
    {
      tmpitem->setposition( chr->pos() );
      add_item_to_world( tmpitem );
      register_with_supporting_multi( tmpitem );
      move_item( tmpitem, tmpitem->pos() );
    }
    else
      backpack->add( tmpitem );
  }

  if ( chr->race == Plib::RACE_HUMAN ||
       chr->race == Plib::RACE_ELF )  // Gargoyles dont have shirts, pants, shoes and daggers.
  {
    tmpitem = Items::Item::create( 0x170F );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_SHOES;
    tmpitem->color = 0x021F;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( 0xF51 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_HAND1;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
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
    tmpitem->layer = Plib::tilelayer( pantstype );
    tmpitem->color = cfBEu16( msg->pantscolor );             // 0x0284;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( shirttype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = Plib::tilelayer( shirttype );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );
  }
  else if ( chr->race == Plib::RACE_GARGOYLE )  // Gargoyles have Robes.
  {
    tmpitem = Items::Item::create( 0x1F03 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_ROBE_DRESS;
    tmpitem->color = cfBEu16( msg->shirtcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );
  }

  client->chr = chr;
  client->acct->set_character( msg->CharNumber, client->chr );

  POLLOG.Format( "Account {} created character 0x{:X}\n" ) << client->acct->name() << chr->serial;
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
  chr->wornitems->serial = chr->serial;
  chr->wornitems->serial_ext = chr->serial_ext;
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
  else if ( Plib::systemstate.config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
  {
    // FIXME: Add send_login_error ...
    client->Disconnect();
    return;
  }
  else if ( charslot >= Plib::systemstate.config.character_slots ||
            client->acct->get_character( charslot ) != nullptr )
  {
    ERROR_PRINTLN( "Create Character: Invalid parameters." );
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  else if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
            client->acct->has_active_characters() )
  {
    send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
    client->Disconnect();
    return;
  }

  unsigned short graphic;
  Plib::URACE race = ( Plib::URACE )( msg->race - 1 );
  Plib::UGENDER gender =
      ( msg->gender & Plib::GENDER_FEMALE ) ? Plib::GENDER_FEMALE : Plib::GENDER_MALE;
  if ( race == Plib::RACE_HUMAN )
    graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
  else if ( race == Plib::RACE_ELF )
    graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
  else
    graphic = ( gender == Plib::GENDER_FEMALE ) ? UOBJ_GARGOYLE_FEMALE : UOBJ_GARGOYLE_MALE;


  Mobile::Character* chr = new Mobile::Character( graphic );

  chr->acct.set( client->acct );
  chr->client = client;
  chr->set_privs( client->acct->default_privlist() );
  chr->cmdlevel( client->acct->default_cmdlevel(), false );

  client->UOExpansionFlagClient = msg->flags;

  std::string tmpstr( msg->name, sizeof msg->name );
  const char* tstr = tmpstr.c_str();
  for ( unsigned int i = 0; i < strlen( tstr ); i++ )
  {
    char tmpchr = tstr[i];
    if ( tmpchr >= ' ' && tmpchr <= '~' )
    {
      if ( tmpchr != '{' && tmpchr != '}' )
        continue;
    }

    ERROR_PRINTLN(
        "Create Character: Attempted to use invalid character '{}' pos '{}' in name '{}'. Client "
        "IP: {} Client Name: {}",
        tmpchr, i, tstr, client->ipaddrAsString(), client->acct->name() );
    client->forceDisconnect();
    return;
  }
  chr->name_ = tstr;

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->wornitems->serial = chr->serial;
  chr->wornitems->serial_ext = chr->serial_ext;

  chr->graphic = graphic;
  chr->race = race;
  chr->gender = gender;

  chr->trueobjtype = chr->objtype_;
  chr->color = cfBEu16( msg->skin_color );
  chr->truecolor = chr->color;

  Pos3d coord = gamestate.startlocations[0]->select_coordinate();
  Realms::Realm* realm = gamestate.startlocations[0]->realm;

  chr->setposition( Pos4d( coord, realm ) );
  chr->position_changed();
  chr->facing = Core::FACING_W;

  bool valid_stats = false;
  unsigned int stat_total = msg->strength + msg->intelligence + msg->dexterity;
  unsigned int stat_min, stat_max;
  char* maxpos;
  std::vector<std::string>::size_type sidx;
  for ( sidx = 0; !valid_stats && sidx < settingsManager.ssopt.total_stats_at_creation.size();
        ++sidx )
  {
    const char* statstr = settingsManager.ssopt.total_stats_at_creation[sidx].c_str();
    stat_max = ( stat_min = strtoul( statstr, &maxpos, 0 ) );
    if ( *( maxpos++ ) == '-' )
      stat_max = strtoul( maxpos, nullptr, 0 );
    if ( stat_total >= stat_min && stat_total <= stat_max )
      valid_stats = true;
  }
  if ( !valid_stats )
  {
    ERROR_PRINTLN(
        "Create Character: Stats sum to {}. "
        "Valid values/ranges are: {}",
        stat_total, settingsManager.ssopt.total_stats_at_creation );
    client->forceDisconnect();
    return;
  }
  if ( msg->strength < 10 || msg->intelligence < 10 || msg->dexterity < 10 )
  {
    ERROR_PRINTLN( "Create Character: A stat was too small. Str={} Int={} Dex={}", msg->strength,
                   msg->intelligence, msg->dexterity );

    client->forceDisconnect();
    return;
  }
  if ( gamestate.pAttrStrength )
    chr->attribute( gamestate.pAttrStrength->attrid ).base( msg->strength * 10 );
  if ( gamestate.pAttrIntelligence )
    chr->attribute( gamestate.pAttrIntelligence->attrid ).base( msg->intelligence * 10 );
  if ( gamestate.pAttrDexterity )
    chr->attribute( gamestate.pAttrDexterity->attrid ).base( msg->dexterity * 10 );


  if ( msg->skillnumber1 > networkManager.uoclient_general.maxskills ||
       msg->skillnumber2 > networkManager.uoclient_general.maxskills ||
       msg->skillnumber3 > networkManager.uoclient_general.maxskills ||
       msg->skillnumber4 > networkManager.uoclient_general.maxskills )
  {
    ERROR_PRINTLN( "Create Character: A skill number was out of range" );
    client->forceDisconnect();
    return;
  }

  bool noskills =
      ( msg->skillvalue1 + msg->skillvalue2 + msg->skillvalue3 + msg->skillvalue4 == 0 ) &&
      msg->profession;
  if ( ( !noskills ) &&
       ( ( msg->skillvalue1 + msg->skillvalue2 + msg->skillvalue3 + msg->skillvalue4 != 120 ) ||
         msg->skillvalue1 > 50 || msg->skillvalue2 > 50 || msg->skillvalue3 > 50 ||
         msg->skillvalue4 > 50 ) )
  {
    ERROR_PRINTLN( "Create Character: Starting skill values incorrect" );
    client->forceDisconnect();
    return;
  }

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

  chr->calc_vital_stuff();
  chr->set_vitals_to_maximum();


  chr->created_at = read_gameclock();

  Items::Item* tmpitem;
  if ( validhair( cfBEu16( msg->hairstyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->hairstyle ) );
    tmpitem->layer = LAYER_HAIR;
    tmpitem->color = cfBEu16( msg->haircolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    if ( chr->equippable( tmpitem ) )                        // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip hair {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validbeard( cfBEu16( msg->beardstyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->beardstyle ) );
    tmpitem->layer = LAYER_BEARD;
    tmpitem->color = cfBEu16( msg->beardcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    if ( chr->equippable( tmpitem ) )                        // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip beard {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validface( cfBEu16( msg->face_id ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->face_id ) );
    tmpitem->layer = LAYER_FACE;
    tmpitem->color = cfBEu16( msg->face_color );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    if ( chr->equippable( tmpitem ) )                        // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip face {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  UContainer* backpack = (UContainer*)Items::Item::create( UOBJ_BACKPACK );
  backpack->layer = LAYER_BACKPACK;
  backpack->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
  chr->equip( backpack );

  if ( settingsManager.ssopt.starting_gold != 0 )
  {
    tmpitem = Items::Item::create( 0x0EED );
    tmpitem->setamount( settingsManager.ssopt.starting_gold );
    tmpitem->setposition( Pos4d( 46, 91, 0, chr->realm() ) );
    u8 newSlot = 1;
    if ( !backpack->can_add_to_slot( newSlot ) || !tmpitem->slot_index( newSlot ) )
    {
      tmpitem->setposition( chr->pos() );
      add_item_to_world( tmpitem );
      register_with_supporting_multi( tmpitem );
      move_item( tmpitem, tmpitem->pos() );
    }
    else
      backpack->add( tmpitem );
  }

  if ( chr->race == Plib::RACE_HUMAN ||
       chr->race == Plib::RACE_ELF )  // Gargoyles dont have shirts, pants, shoes and daggers.
  {
    tmpitem = Items::Item::create( 0x170F );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_SHOES;
    tmpitem->color = 0x021F;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( 0xF51 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_HAND1;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
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
    tmpitem->layer = Plib::tilelayer( pantstype );
    tmpitem->color = cfBEu16( msg->pantscolor );             // 0x0284;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( shirttype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = Plib::tilelayer( shirttype );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );
  }
  else if ( chr->race == Plib::RACE_GARGOYLE )  // Gargoyles have Robes.
  {
    tmpitem = Items::Item::create( 0x1F03 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_ROBE_DRESS;
    tmpitem->color = cfBEu16( msg->shirtcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );
  }

  client->chr = chr;
  client->acct->set_character( charslot, client->chr );

  POLLOG.Format( "Account {} created character 0x{:X}\n" ) << client->acct->name() << chr->serial;
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
  else if ( Plib::systemstate.config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
  {
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  else if ( msg->CharNumber >= Plib::systemstate.config.character_slots ||
            client->acct->get_character( msg->CharNumber ) != nullptr ||
            msg->StartIndex >= gamestate.startlocations.size() )
  {
    ERROR_PRINTLN( "Create Character: Invalid parameters." );
    send_login_error( client, LOGIN_ERROR_MISC );
    client->Disconnect();
    return;
  }
  else if ( !Plib::systemstate.config.allow_multi_clients_per_account &&
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

  Mobile::Character* chr = new Mobile::Character( graphic );

  chr->acct.set( client->acct );
  chr->client = client;
  chr->set_privs( client->acct->default_privlist() );
  chr->cmdlevel( client->acct->default_cmdlevel(), false );

  client->UOExpansionFlagClient = ctBEu32( msg->clientflag );

  std::string tmpstr( msg->Name, sizeof msg->Name );
  const char* tstr = tmpstr.c_str();
  for ( unsigned int i = 0; i < strlen( tstr ); i++ )
  {
    char tmpchr = tstr[i];
    if ( tmpchr >= ' ' && tmpchr <= '~' )
    {
      if ( tmpchr != '{' && tmpchr != '}' )
        continue;
    }

    ERROR_PRINTLN(
        "Create Character: Attempted to use invalid character '{}' pos '{}' in name '{}'. Client "
        "IP: {} Client Name: {}",
        tmpchr, i, tstr, client->ipaddrAsString(), client->acct->name() );
    client->Disconnect();
    return;
  }
  chr->name_ = tstr;

  chr->serial = GetNextSerialNumber();
  chr->serial_ext = ctBEu32( chr->serial );
  chr->wornitems->serial = chr->serial;
  chr->wornitems->serial_ext = chr->serial_ext;

  chr->graphic = graphic;
  chr->race = race;
  chr->gender = gender;

  chr->trueobjtype = chr->objtype_;
  chr->color = cfBEu16( msg->SkinColor );
  chr->truecolor = chr->color;

  Pos3d coord = gamestate.startlocations[msg->StartIndex]->select_coordinate();
  Realms::Realm* realm = gamestate.startlocations[msg->StartIndex]->realm;

  chr->setposition( Pos4d( coord, realm ) );
  chr->position_changed();
  chr->facing = Core::FACING_W;

  bool valid_stats = false;
  unsigned int stat_total = msg->Strength + msg->Intelligence + msg->Dexterity;
  unsigned int stat_min, stat_max;
  char* maxpos;
  std::vector<std::string>::size_type sidx;
  for ( sidx = 0; !valid_stats && sidx < settingsManager.ssopt.total_stats_at_creation.size();
        ++sidx )
  {
    const char* statstr = settingsManager.ssopt.total_stats_at_creation[sidx].c_str();
    stat_max = ( stat_min = strtoul( statstr, &maxpos, 0 ) );
    if ( *( maxpos++ ) == '-' )
      stat_max = strtoul( maxpos, nullptr, 0 );
    if ( stat_total >= stat_min && stat_total <= stat_max )
      valid_stats = true;
  }
  if ( !valid_stats )
  {
    ERROR_PRINTLN( "Create Character: Stats sum to {}.\nValid values/ranges are: {}", stat_total,
                   settingsManager.ssopt.total_stats_at_creation );
    client->forceDisconnect();
    return;
  }
  if ( msg->Strength < 10 || msg->Intelligence < 10 || msg->Dexterity < 10 )
  {
    ERROR_PRINTLN( "Create Character: A stat was too small. Str={} Int={} Dex={}", msg->Strength,
                   msg->Intelligence, msg->Dexterity );

    client->forceDisconnect();
    return;
  }
  if ( gamestate.pAttrStrength )
    chr->attribute( gamestate.pAttrStrength->attrid ).base( msg->Strength * 10 );
  if ( gamestate.pAttrIntelligence )
    chr->attribute( gamestate.pAttrIntelligence->attrid ).base( msg->Intelligence * 10 );
  if ( gamestate.pAttrDexterity )
    chr->attribute( gamestate.pAttrDexterity->attrid ).base( msg->Dexterity * 10 );

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


  if ( msg->SkillNumber1 > networkManager.uoclient_general.maxskills ||
       msg->SkillNumber2 > networkManager.uoclient_general.maxskills ||
       msg->SkillNumber3 > networkManager.uoclient_general.maxskills ||
       msg->SkillNumber4 > networkManager.uoclient_general.maxskills )
  {
    ERROR_PRINTLN( "Create Character: A skill number was out of range" );
    client->forceDisconnect();
    return;
  }

  bool noskills =
      ( msg->SkillValue1 + msg->SkillValue2 + msg->SkillValue3 + msg->SkillValue4 == 0 ) &&
      msg->profession;

  if ( ( !noskills ) &&
       ( ( msg->SkillValue1 + msg->SkillValue2 + msg->SkillValue3 + msg->SkillValue4 != 120 ) ||
         msg->SkillValue1 > 50 || msg->SkillValue2 > 50 || msg->SkillValue3 > 50 ||
         msg->SkillValue4 > 50 ) )
  {
    ERROR_PRINTLN( "Create Character: Starting skill values incorrect" );
    client->forceDisconnect();
    return;
  }

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

  chr->calc_vital_stuff();
  chr->set_vitals_to_maximum();


  chr->created_at = read_gameclock();

  Items::Item* tmpitem;
  if ( validhair( cfBEu16( msg->HairStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->HairStyle ) );
    tmpitem->layer = LAYER_HAIR;
    tmpitem->color = cfBEu16( msg->HairColor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    if ( chr->equippable( tmpitem ) )                        // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip hair {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  if ( validbeard( cfBEu16( msg->BeardStyle ) ) )
  {
    tmpitem = Items::Item::create( cfBEu16( msg->BeardStyle ) );
    tmpitem->layer = LAYER_BEARD;
    tmpitem->color = cfBEu16( msg->BeardColor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    if ( chr->equippable( tmpitem ) )                        // check it or passert will trigger
      chr->equip( tmpitem );
    else
    {
      ERROR_PRINTLN( "Create Character: Failed to equip beard {:#X}", tmpitem->graphic );
      tmpitem->destroy();
    }
  }

  UContainer* backpack = (UContainer*)Items::Item::create( UOBJ_BACKPACK );
  backpack->layer = LAYER_BACKPACK;
  backpack->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
  chr->equip( backpack );

  if ( settingsManager.ssopt.starting_gold != 0 )
  {
    tmpitem = Items::Item::create( 0x0EED );
    tmpitem->setamount( settingsManager.ssopt.starting_gold );
    tmpitem->setposition( Pos4d( 46, 91, 0, chr->realm() ) );
    u8 newSlot = 1;
    if ( !backpack->can_add_to_slot( newSlot ) || !tmpitem->slot_index( newSlot ) )
    {
      tmpitem->setposition( chr->pos() );
      add_item_to_world( tmpitem );
      register_with_supporting_multi( tmpitem );
      move_item( tmpitem, tmpitem->pos() );
    }
    else
      backpack->add( tmpitem );
  }

  if ( chr->race == Plib::RACE_HUMAN ||
       chr->race == Plib::RACE_ELF )  // Gargoyles dont have shirts, pants, shoes and daggers.
  {
    tmpitem = Items::Item::create( 0x170F );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_SHOES;
    tmpitem->color = 0x021F;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( 0xF51 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_HAND1;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
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
    tmpitem->layer = Plib::tilelayer( pantstype );
    tmpitem->color = cfBEu16( msg->pantscolor );             // 0x0284;
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );

    tmpitem = Items::Item::create( shirttype );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = Plib::tilelayer( shirttype );
    tmpitem->color = cfBEu16( msg->shirtcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );
  }
  else if ( chr->race == Plib::RACE_GARGOYLE )  // Gargoyles have Robes.
  {
    tmpitem = Items::Item::create( 0x1F03 );
    tmpitem->newbie( settingsManager.ssopt.newbie_starting_equipment );
    tmpitem->layer = LAYER_ROBE_DRESS;
    tmpitem->color = cfBEu16( msg->shirtcolor );
    tmpitem->setposition( Pos4d( 0, 0, 0, chr->realm() ) );  // TODO POS equip should set the pos
    chr->equip( tmpitem );
  }

  client->chr = chr;
  client->acct->set_character( msg->CharNumber, client->chr );

  POLLOG.Format( "Account {} created character 0x{:X}\n" ) << client->acct->name() << chr->serial;
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
}  // namespace Core
}  // namespace Pol
