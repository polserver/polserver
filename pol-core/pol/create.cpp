/*
History
=======
2006/05/16 Shinigami: ClientCreateChar() updated to support Elfs
2006/05/23 Shinigami: added Elf Hair Style to validhair() & comments added
                      validbeard() rewritten & comments added
2009/12/02 Turley:    added gargoyle support, 0x8D char create, face support

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include <iostream>
#include <string>


#include "../clib/endian.h"
#include "../clib/fdump.h"
#include "../clib/logfile.h"
#include "../clib/random.h"

#include "../plib/realm.h"

#include "accounts/account.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "clidata.h"
#include "network/client.h"
#include "gameclck.h"
#include "layers.h"
#include "mkscrobj.h"
#include "msgfiltr.h"
#include "msghandl.h"
#include "objtype.h"
#include "pktin.h"
#include "polcfg.h"
#include "realms.h"
#include "scrdef.h"
#include "skilladv.h"
#include "sockio.h"
#include "ssopt.h"
#include "startloc.h"
#include "uconst.h"
#include "ufunc.h"
#include "uoskills.h"
#include "uvars.h"
#include "uworld.h"

#include "../bscript/bobject.h"
#include "module/osmod.h"
#include "scrsched.h"
#include "scrstore.h"
#include "uoexec.h"
#include "module/uomod.h"

#include "objecthash.h"


void start_client_char( Client *client );
void run_logon_script( Character* chr );

short validhaircolor(u16 color)
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
	0x203B  Short Hair		  // Human
	0x203C  Long Hair
	0x203D  PonyTail
	0x2044  Mohawk
	0x2045  Pageboy Hair
	0x2046  Buns Hair
	0x2047  Afro
	0x2048  Receeding Hair
	0x2049  Two Pig Tails
	0x204A  Krisna Hair
				
	0x2FBF  Mid Long Hair	   // Elf (Mondain's Legacy)
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

    0x4258  Horn Style 1       // Gargoyle (SA)
    0x4259  Horn Style 2
    0x425a  Horn Style 3
    0x425b  Horn Style 4
    0x425c  Horn Style 5
    0x425d  Horn Style 6
    0x425e  Horn Style 7
    0x425f  Horn Style 8
*/
bool validhair(u16 HairStyle)
{
	if (((0x203B<=HairStyle) && (HairStyle<=0x203D)) ||
		((0x2044<=HairStyle) && (HairStyle<=0x204A)) ||
		((0x2FBF<=HairStyle) && (HairStyle<=0x2FC2)) ||
		((0x2FCC<=HairStyle) && (HairStyle<=0x2FD2)) ||
        ((0x4258<=HairStyle) && (HairStyle<=0x425F) && (config.max_tile_id > HairStyle)))
		return true;
	else
		return false;
}

/* beard can be:
	0x203E  Long Beard		 // Human
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
bool validbeard(u16 BeardStyle)
{
	if (((0x203E<=BeardStyle) && (BeardStyle<=0x2041)) ||
		((0x204B<=BeardStyle) && (BeardStyle<=0x204D)) ||
        ((0x42AD<=BeardStyle) && (BeardStyle<=0x42B0) && (config.max_tile_id > BeardStyle)))
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
bool validface(u16 FaceStyle)
{
    if (ssopt.support_faces > 0)
    {
        if ((0x3B44<=FaceStyle) && (FaceStyle<=0x3B4D))
            return true;
        if (ssopt.support_faces == 2)
        {
            if ((0x3B4E<=FaceStyle) && (FaceStyle<=0x3B57))
                return true;
        }
    }
    return false;
}

void ClientCreateChar( Client* client, PKTIN_00* msg)
{
	if ( client->acct == NULL )
	{
		cerr << "Client from " << AddressToString( &client->ipaddr ) << " tried to create a character without an account!" << endl;
		client->disconnect = 1;
		return;
	}
	else if ( config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
	{
		client->disconnect = 1;
		return;
	}
	else if ( msg->CharNumber >= config.character_slots ||
		client->acct->get_character( msg->CharNumber ) != NULL ||
		msg->StartIndex >= startlocations.size() )
	{
		cerr << "Create Character: Invalid parameters." << endl;
		send_login_error( client, LOGIN_ERROR_MISC );
		client->disconnect = 1;
		return;
	}
	else if ( client->acct->active_character != NULL )
	{
		send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
		client->disconnect = 1;
		return;
	}

    unsigned short graphic;
    URACE race;
    UGENDER gender = ((msg->Sex & FLAG_GENDER) == FLAG_GENDER) ? GENDER_FEMALE : GENDER_MALE;
    if (client->ClientType & CLIENTTYPE_7000)
    {
        /*
        0x00 / 0x01 = human male/female
        0x02 / 0x03 = human male/female
        0x04 / 0x05 = elf male/female
        0x06 / 0x07 = gargoyle male/female
        */
        if ((msg->Sex & 0x6)==0x6)
        {
            race = RACE_GARGOYLE;
            graphic = (gender==GENDER_FEMALE) ? UOBJ_GARGOYLE_FEMALE : UOBJ_GARGOYLE_MALE;
        }
        else if ((msg->Sex & 0x4)==0x4)
        {
            race = RACE_ELF;
            graphic = (gender==GENDER_FEMALE) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
        }
        else
        {
            race = RACE_HUMAN;
            graphic = (gender==GENDER_FEMALE) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
        }
    }
    else
    {
        /*
        0x00 / 0x01 = human male/female
        0x02 / 0x03 = elf male/female
        */
        if ((msg->Sex & FLAG_RACE) == FLAG_RACE)
        {
            race = RACE_ELF;
            graphic = (gender==GENDER_FEMALE) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
        }
        else
        {
            race = RACE_HUMAN;
            graphic = (gender==GENDER_FEMALE) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
        }
    }

	Character* chr = new Character( graphic );

	chr->acct.set( client->acct );
	chr->client = client;
	chr->set_privs( client->acct->default_privlist() );
	chr->cmdlevel = client->acct->default_cmdlevel();

	client->UOExpansionFlagClient = ctBEu32(msg->clientflag);

	string tmpstr(msg->Name, sizeof msg->Name);
	const char *tstr = tmpstr.c_str();
	for ( unsigned int i = 0; i < strlen(tstr); i++ )
	{
		char tmpchr = tstr[i];
		if ( tmpchr >= ' ' && tmpchr <= '~' )
		{
			if ( tmpchr != '{' && tmpchr != '}' )
				continue;
		}
		
		cerr << "Create Character: Attempted to use invalid character '"<<tmpchr<<"' pos '"<<i<<"' in name '"<<tstr<<"'. Client IP: "
			<< client->ipaddrAsString() << " Client Name: " << client->acct->name()<< endl;
	        client->disconnect = 1;
		    return;
	}
	chr->name_ = tstr; 

	chr->serial = GetNextSerialNumber();
	chr->serial_ext = ctBEu32(chr->serial);
	chr->realm = find_realm(string("britannia"));
	chr->wornitems.serial = chr->serial;
	chr->wornitems.serial_ext = chr->serial_ext;
	chr->wornitems.realm = chr->realm;

    chr->graphic = graphic;
    chr->race = race;
    chr->gender = gender;
	
	chr->graphic_ext = ctBEu16( chr->graphic );
	chr->trueobjtype = chr->objtype_;
	chr->color = cfBEu16(msg->SkinColor);
	chr->color_ext = ctBEu16( chr->color );
	chr->truecolor = chr->color;

	Coordinate coord = startlocations[ msg->StartIndex ]->select_coordinate();

	chr->x = coord.x;
	chr->y = coord.y;
	chr->z = coord.z;
	chr->facing = FACING_W;

	bool valid_stats = false;
	unsigned long stat_total = msg->Strength + msg->Intelligence + msg->Dexterity;
	unsigned long stat_min, stat_max;
	char *maxpos;
	std::vector<string>::size_type sidx;
	for( sidx = 0; !valid_stats && sidx < ssopt.total_stats_at_creation.size(); ++sidx )
	{
		const char *statstr = ssopt.total_stats_at_creation[sidx].c_str();
		stat_max = (stat_min = strtoul(statstr, &maxpos, 0));
		if ( *(maxpos++) == '-' )
			stat_max = strtoul(maxpos, 0, 0);
		if ( stat_total >= stat_min && stat_total <= stat_max )
			valid_stats = true;
	}
	if ( !valid_stats )
	{
		cerr << "Create Character: Stats sum to "
			 << stat_total << "." << endl
			 << "Valid values/ranges are: ";
		for ( sidx = 0; sidx < ssopt.total_stats_at_creation.size(); ++sidx )
		{
			if ( sidx > 0 )
				cerr << ",";
			cerr << ssopt.total_stats_at_creation[sidx];
		}
		cerr << endl;
		client->disconnect = 1;
		return;
	}
	if (msg->Strength < 10 || msg->Intelligence < 10 || msg->Dexterity < 10)
	{
		cerr << "Create Character: A stat was too small."
			 << " Str=" << msg->Strength 
			 << " Int=" << msg->Intelligence 
			 << " Dex=" << msg->Dexterity
			 << endl;

		client->disconnect = 1;
		return;
	}
	if (pAttrStrength)
		chr->attribute(pAttrStrength->attrid).base( msg->Strength * 10 );
	if (pAttrIntelligence)
		chr->attribute(pAttrIntelligence->attrid).base( msg->Intelligence * 10 );
	if (pAttrDexterity)
		chr->attribute(pAttrDexterity->attrid).base( msg->Dexterity * 10 );


	if (msg->SkillNumber1 > uoclient_general.maxskills ||
		msg->SkillNumber2 > uoclient_general.maxskills ||
		msg->SkillNumber3 > uoclient_general.maxskills)
	{
		cerr << "Create Character: A skill number was out of range" << endl;
		client->disconnect = 1;
		return;
	}
	if ((msg->SkillValue1 + msg->SkillValue2 + msg->SkillValue3 != 100) ||
		msg->SkillValue1 > 50 ||
		msg->SkillValue2 > 50 ||
		msg->SkillValue3 > 50)
	{
		cerr << "Create Character: Starting skill values incorrect" << endl;
		client->disconnect = 1;
		return;
	}

	////HASH
	//moved down here, after all error checking passes, else we get a half-created PC in the save.
	objecthash.Insert(chr);
	////
	
	const Attribute* pAttr;
	pAttr = GetUOSkill(msg->SkillNumber1).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->SkillValue1 * 10 );
	pAttr = GetUOSkill(msg->SkillNumber2).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->SkillValue2 * 10 );
	pAttr = GetUOSkill(msg->SkillNumber3).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->SkillValue3 * 10 );

	chr->calc_vital_stuff();
	chr->set_vitals_to_maximum();
  

	chr->created_at = read_gameclock();

	Item* tmpitem;
	if(validhair(cfBEu16(msg->HairStyle)))
	{
		tmpitem=Item::create( cfBEu16(msg->HairStyle) );
		tmpitem->layer = LAYER_HAIR;
		tmpitem->color = cfBEu16(msg->HairColor);
		tmpitem->color_ext = ctBEu16(tmpitem->color);
		tmpitem->realm = chr->realm;
		chr->equip(tmpitem);
	}

	if( validbeard(cfBEu16(msg->BeardStyle)) )
	{
		tmpitem=Item::create( cfBEu16(msg->BeardStyle) );
		tmpitem->layer = LAYER_BEARD;
		tmpitem->color = cfBEu16(msg->BeardColor);
		tmpitem->color_ext = ctBEu16(tmpitem->color);
		tmpitem->realm = chr->realm;
		chr->equip(tmpitem);
	}

	UContainer *backpack = (UContainer *) Item::create( UOBJ_BACKPACK );
	backpack->layer = LAYER_BACKPACK;
	backpack->realm = chr->realm;
	chr->equip( backpack );

	if (ssopt.starting_gold != 0 )
	{
		tmpitem = Item::create( 0x0EED );
		tmpitem->setamount( ssopt.starting_gold );
		tmpitem->x = 46;
		tmpitem->y = 91;
		tmpitem->z = 0;
		tmpitem->realm = chr->realm;
		u8 newSlot = 1;
		if ( !backpack->can_add_to_slot(newSlot) || !tmpitem->slot_index(newSlot) )
		{
			tmpitem->x = chr->x;
			tmpitem->y = chr->y;
			tmpitem->z = chr->z;
			add_item_to_world( tmpitem );
			register_with_supporting_multi( tmpitem );
			move_item( tmpitem, tmpitem->x, tmpitem->y, tmpitem->z, NULL );
		}
		else
			backpack->add(tmpitem); 
	}

	tmpitem=Item::create( 0x170F );
	tmpitem->newbie(true);
	tmpitem->layer = LAYER_SHOES;
	tmpitem->color = 0x021F;
	tmpitem->color_ext = ctBEu16(tmpitem->color);
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem); 

	tmpitem=Item::create( 0xF51 );
	tmpitem->newbie(true);
	tmpitem->layer = LAYER_HAND1;
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem); 

	unsigned short pantstype, shirttype;
	if (chr->gender == GENDER_FEMALE)
	{
		pantstype = 0x1516;
		shirttype = 0x1517;
	}
	else
	{
		pantstype = 0x152e;
		shirttype = 0x1517;
	}

	tmpitem=Item::create( pantstype );
	tmpitem->newbie(true);
	tmpitem->layer = tilelayer( pantstype );
	tmpitem->color = cfBEu16( msg->pantscolor ); // 0x0284;
	tmpitem->color_ext = ctBEu16(tmpitem->color);
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem);

	tmpitem=Item::create( shirttype );
	tmpitem->newbie(true);
	tmpitem->layer = tilelayer( shirttype );
	tmpitem->color = cfBEu16( msg->shirtcolor ); 
	tmpitem->color_ext = ctBEu16(tmpitem->color);
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem);

	client->chr = chr;
	client->acct->set_character( msg->CharNumber, client->chr );
	client->acct->active_character = chr;

	Log( "Account %s created character 0x%lu\n", client->acct->name(), chr->serial );
	SetCharacterWorldPosition( chr );
	client->msgtype_filter = &game_filter;
	start_client_char( client );

	// FIXME : Shouldn't this be triggered at the end of creation?
	run_logon_script( chr );

	ref_ptr<EScriptProgram> prog = find_script( "misc/oncreate", true, config.cache_interactive_scripts );
	if (prog.get() != NULL)
	{
		auto_ptr<UOExecutor> ex(create_script_executor());
		
		ObjArray* arr = new ObjArray;
		arr->addElement( new BLong( msg->SkillNumber1 ) );
		arr->addElement( new BLong( msg->SkillNumber2 ) );
		arr->addElement( new BLong( msg->SkillNumber3 ) );
		
		ex->pushArg( arr );
		ex->pushArg( make_mobileref( chr ) );

		ex->addModule( new UOExecutorModule( *ex ) );
		ex->os_module->critical = true;

		if (ex->setProgram( prog.get() ))
		{
			schedule_executor( ex.release() );
		}
		else
		{
			cerr << "script misc/oncreate: setProgram failed" << endl;
		}
	}
}
MESSAGE_HANDLER(PKTIN_00, ClientCreateChar);

void createchar2(Account* acct, unsigned index)
{
	Character* chr = new Character(UOBJ_HUMAN_MALE);
	chr->acct.set(acct);
	acct->set_character(index, chr);
	chr->setname("new character");

	chr->serial = GetNextSerialNumber();
	chr->serial_ext = ctBEu32(chr->serial);
	chr->realm = find_realm(string("britannia"));
	chr->x = chr->y = chr->z = chr->facing = 1;
	chr->wornitems.serial = chr->serial;
	chr->wornitems.serial_ext = chr->serial_ext;
	chr->wornitems.realm = chr->realm;
	chr->graphic = UOBJ_HUMAN_MALE;
	chr->gender = GENDER_MALE;
	chr->graphic_ext = ctBEu16( chr->graphic );
	chr->trueobjtype = chr->objtype_;
	chr->color = ctBEu16(0);
	chr->color_ext = ctBEu16(chr->color);
	chr->truecolor = chr->color;
	chr->created_at = read_gameclock();

	objecthash.Insert(chr);
}

void handle_unknown_packet( Client* client );

void ClientCreateChar6017( Client* client, PKTIN_8D* msg)
{
    handle_unknown_packet( client );
    int charslot=ctBEu32(msg->char_slot);
	if ( client->acct == NULL )
	{
		cerr << "Client from " << AddressToString( &client->ipaddr ) << " tried to create a character without an account!" << endl;
		client->disconnect = 1;
		return;
	}
	else if ( config.min_cmdlevel_to_login > client->acct->default_cmdlevel() )
	{
		client->disconnect = 1;
		return;
	}
	else if ( charslot >= config.character_slots ||
		client->acct->get_character( charslot ) != NULL )
	{
		cerr << "Create Character: Invalid parameters." << endl;
		send_login_error( client, LOGIN_ERROR_MISC );
		client->disconnect = 1;
		return;
	}
	else if ( client->acct->active_character != NULL )
	{
		send_login_error( client, LOGIN_ERROR_OTHER_CHAR_INUSE );
		client->disconnect = 1;
		return;
	}

    unsigned short graphic;
    URACE race=(URACE)(msg->race-1);
    UGENDER gender = (msg->gender & GENDER_FEMALE) ? GENDER_FEMALE : GENDER_MALE;
    if (race == RACE_HUMAN)
        graphic = (gender==GENDER_FEMALE) ? UOBJ_HUMAN_FEMALE : UOBJ_HUMAN_MALE;
    else if (race == RACE_ELF)
        graphic = (gender==GENDER_FEMALE) ? UOBJ_ELF_FEMALE : UOBJ_ELF_MALE;
    else
        graphic = (gender==GENDER_FEMALE) ? UOBJ_GARGOYLE_FEMALE : UOBJ_GARGOYLE_MALE;


	Character* chr = new Character( graphic );

	chr->acct.set( client->acct );
	chr->client = client;
	chr->set_privs( client->acct->default_privlist() );
	chr->cmdlevel = client->acct->default_cmdlevel();

	client->UOExpansionFlagClient = msg->flags;

	string tmpstr(msg->name, sizeof msg->name);
	const char *tstr = tmpstr.c_str();
	for ( unsigned int i = 0; i < strlen(tstr); i++ )
	{
		char tmpchr = tstr[i];
		if ( tmpchr >= ' ' && tmpchr <= '~' )
		{
			if ( tmpchr != '{' && tmpchr != '}' )
				continue;
		}
		
		cerr << "Create Character: Attempted to use invalid character '"<<tmpchr<<"' pos '"<<i<<"' in name '"<<tstr<<"'. Client IP: "
			<< client->ipaddrAsString() << " Client Name: " << client->acct->name()<< endl;
	        client->disconnect = 1;
		    return;
	}
	chr->name_ = tstr; 

	chr->serial = GetNextSerialNumber();
	chr->serial_ext = ctBEu32(chr->serial);
	chr->realm = find_realm(string("britannia"));
	chr->wornitems.serial = chr->serial;
	chr->wornitems.serial_ext = chr->serial_ext;
	chr->wornitems.realm = chr->realm;

    chr->graphic = graphic;
    chr->race = race;
    chr->gender = gender;
	
	chr->graphic_ext = ctBEu16( chr->graphic );
	chr->trueobjtype = chr->objtype_;
	chr->color = cfBEu16(msg->skin_color);
	chr->color_ext = ctBEu16( chr->color );
	chr->truecolor = chr->color;

	Coordinate coord = startlocations[ 0 ]->select_coordinate();

	chr->x = coord.x;
	chr->y = coord.y;
	chr->z = coord.z;
	chr->facing = FACING_W;

	bool valid_stats = false;
	unsigned long stat_total = msg->strength + msg->intelligence + msg->dexterity;
	unsigned long stat_min, stat_max;
	char *maxpos;
	std::vector<string>::size_type sidx;
	for( sidx = 0; !valid_stats && sidx < ssopt.total_stats_at_creation.size(); ++sidx )
	{
		const char *statstr = ssopt.total_stats_at_creation[sidx].c_str();
		stat_max = (stat_min = strtoul(statstr, &maxpos, 0));
		if ( *(maxpos++) == '-' )
			stat_max = strtoul(maxpos, 0, 0);
		if ( stat_total >= stat_min && stat_total <= stat_max )
			valid_stats = true;
	}
	if ( !valid_stats )
	{
		cerr << "Create Character: Stats sum to "
			 << stat_total << "." << endl
			 << "Valid values/ranges are: ";
		for ( sidx = 0; sidx < ssopt.total_stats_at_creation.size(); ++sidx )
		{
			if ( sidx > 0 )
				cerr << ",";
			cerr << ssopt.total_stats_at_creation[sidx];
		}
		cerr << endl;
		client->disconnect = 1;
		return;
	}
	if (msg->strength < 10 || msg->intelligence < 10 || msg->dexterity < 10)
	{
		cerr << "Create Character: A stat was too small."
			 << " Str=" << msg->strength 
			 << " Int=" << msg->intelligence 
			 << " Dex=" << msg->dexterity
			 << endl;

		client->disconnect = 1;
		return;
	}
	if (pAttrStrength)
		chr->attribute(pAttrStrength->attrid).base( msg->strength * 10 );
	if (pAttrIntelligence)
		chr->attribute(pAttrIntelligence->attrid).base( msg->intelligence * 10 );
	if (pAttrDexterity)
		chr->attribute(pAttrDexterity->attrid).base( msg->dexterity * 10 );


	if (msg->skillnumber1 > uoclient_general.maxskills ||
		msg->skillnumber2 > uoclient_general.maxskills ||
        msg->skillnumber3 > uoclient_general.maxskills ||
		msg->skillnumber4 > uoclient_general.maxskills)
	{
		cerr << "Create Character: A skill number was out of range" << endl;
		client->disconnect = 1;
		return;
	}
	if ((msg->skillvalue1 + msg->skillvalue2 + msg->skillvalue3 + msg->skillvalue4 != 120) ||
		msg->skillvalue1 > 50 ||
		msg->skillvalue2 > 50 ||
        msg->skillvalue3 > 50 ||
		msg->skillvalue4 > 50)
	{
		cerr << "Create Character: Starting skill values incorrect" << endl;
		client->disconnect = 1;
		return;
	}

	////HASH
	//moved down here, after all error checking passes, else we get a half-created PC in the save.
	objecthash.Insert(chr);
	////
	
	const Attribute* pAttr;
	pAttr = GetUOSkill(msg->skillnumber1).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->skillvalue1 * 10 );
	pAttr = GetUOSkill(msg->skillnumber2).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->skillvalue2 * 10 );
	pAttr = GetUOSkill(msg->skillnumber3).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->skillvalue3 * 10 );
    pAttr = GetUOSkill(msg->skillnumber4).pAttr;
	if (pAttr) chr->attribute( pAttr->attrid ).base( msg->skillvalue4 * 10 );

	chr->calc_vital_stuff();
	chr->set_vitals_to_maximum();
  

	chr->created_at = read_gameclock();

	Item* tmpitem;
	if(validhair(cfBEu16(msg->hairstyle)))
	{
		tmpitem=Item::create( cfBEu16(msg->hairstyle) );
		tmpitem->layer = LAYER_HAIR;
		tmpitem->color = cfBEu16(msg->haircolor);
		tmpitem->color_ext = ctBEu16(tmpitem->color);
		tmpitem->realm = chr->realm;
		chr->equip(tmpitem);
	}

	if( validbeard(cfBEu16(msg->beardstyle)) )
	{
		tmpitem=Item::create( cfBEu16(msg->beardstyle) );
		tmpitem->layer = LAYER_BEARD;
		tmpitem->color = cfBEu16(msg->beardcolor);
		tmpitem->color_ext = ctBEu16(tmpitem->color);
		tmpitem->realm = chr->realm;
		chr->equip(tmpitem);
	}

    if( validface(cfBEu16(msg->face_id)) )
    {
        tmpitem=Item::create( cfBEu16(msg->face_id) );
        tmpitem->layer = LAYER_FACE;
        tmpitem->color = cfBEu16(msg->face_color);
        tmpitem->color_ext = ctBEu16(tmpitem->color);
        tmpitem->realm = chr->realm;
        chr->equip(tmpitem);
    }

	UContainer *backpack = (UContainer *) Item::create( UOBJ_BACKPACK );
	backpack->layer = LAYER_BACKPACK;
	backpack->realm = chr->realm;
	chr->equip( backpack );

	if (ssopt.starting_gold != 0 )
	{
		tmpitem = Item::create( 0x0EED );
		tmpitem->setamount( ssopt.starting_gold );
		tmpitem->x = 46;
		tmpitem->y = 91;
		tmpitem->z = 0;
		tmpitem->realm = chr->realm;
		u8 newSlot = 1;
		if ( !backpack->can_add_to_slot(newSlot) || !tmpitem->slot_index(newSlot) )
		{
			tmpitem->x = chr->x;
			tmpitem->y = chr->y;
			tmpitem->z = chr->z;
			add_item_to_world( tmpitem );
			register_with_supporting_multi( tmpitem );
			move_item( tmpitem, tmpitem->x, tmpitem->y, tmpitem->z, NULL );
		}
		else
			backpack->add(tmpitem); 
	}

	tmpitem=Item::create( 0x170F );
	tmpitem->newbie(true);
	tmpitem->layer = LAYER_SHOES;
	tmpitem->color = 0x021F;
	tmpitem->color_ext = ctBEu16(tmpitem->color);
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem); 

	tmpitem=Item::create( 0xF51 );
	tmpitem->newbie(true);
	tmpitem->layer = LAYER_HAND1;
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem); 

	unsigned short pantstype, shirttype;
	if (chr->gender == GENDER_FEMALE)
	{
		pantstype = 0x1516;
		shirttype = 0x1517;
	}
	else
	{
		pantstype = 0x152e;
		shirttype = 0x1517;
	}

	tmpitem=Item::create( pantstype );
	tmpitem->newbie(true);
	tmpitem->layer = tilelayer( pantstype );
	tmpitem->color = cfBEu16( msg->pantscolor ); // 0x0284;
	tmpitem->color_ext = ctBEu16(tmpitem->color);
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem);

	tmpitem=Item::create( shirttype );
	tmpitem->newbie(true);
	tmpitem->layer = tilelayer( shirttype );
	tmpitem->color = cfBEu16( msg->shirtcolor ); 
	tmpitem->color_ext = ctBEu16(tmpitem->color);
	tmpitem->realm = chr->realm;
	chr->equip(tmpitem);

	client->chr = chr;
	client->acct->set_character( charslot, client->chr );
	client->acct->active_character = chr;

	Log( "Account %s created character 0x%lu\n", client->acct->name(), chr->serial );
	SetCharacterWorldPosition( chr );
	client->msgtype_filter = &game_filter;
	start_client_char( client );

	// FIXME : Shouldn't this be triggered at the end of creation?
	run_logon_script( chr );

	ref_ptr<EScriptProgram> prog = find_script( "misc/oncreate", true, config.cache_interactive_scripts );
	if (prog.get() != NULL)
	{
		auto_ptr<UOExecutor> ex(create_script_executor());
		
		ObjArray* arr = new ObjArray;
		arr->addElement( new BLong( msg->skillnumber1 ) );
		arr->addElement( new BLong( msg->skillnumber2 ) );
		arr->addElement( new BLong( msg->skillnumber3 ) );
        arr->addElement( new BLong( msg->skillnumber4 ) );
		
		ex->pushArg( arr );
		ex->pushArg( make_mobileref( chr ) );

		ex->addModule( new UOExecutorModule( *ex ) );
		ex->os_module->critical = true;

		if (ex->setProgram( prog.get() ))
		{
			schedule_executor( ex.release() );
		}
		else
		{
			cerr << "script misc/oncreate: setProgram failed" << endl;
		}
	}
}
MESSAGE_HANDLER(PKTIN_8D, ClientCreateChar6017);
