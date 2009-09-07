/*
History
=======
2006/05/16 Shinigami: ClientCreateChar() updated to support Elfs
2006/05/23 Shinigami: added Elf Hair Style to validhair() & comments added
                      validbeard() rewritten & comments added

Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <iostream>
#include <string>


#include "clib/endian.h"
#include "clib/fdump.h"
#include "clib/logfile.h"
#include "clib/random.h"

#include "plib/realm.h"

#include "accounts/account.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "clidata.h"
#include "client.h"
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

#include "bscript/bobject.h"
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
*/
bool validhair(u16 HairStyle)
{
	if (((0x203B<=HairStyle) && (HairStyle<=0x203D)) ||
		((0x2044<=HairStyle) && (HairStyle<=0x204A)) ||
		((0x2FBF<=HairStyle) && (HairStyle<=0x2FC2)) ||
		((0x2FCC<=HairStyle) && (HairStyle<=0x2FD2)))
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
*/
bool validbeard(u16 BeardStyle)
{
	if (((0x203E<=BeardStyle) && (BeardStyle<=0x2041)) ||
		((0x204B<=BeardStyle) && (BeardStyle<=0x204D)))
		return true;
	else
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

	Character* chr = new Character( ((msg->Sex & FLAG_GENDER) == FLAG_GENDER) ?
		(((msg->Sex & FLAG_RACE) == FLAG_RACE) ? UOBJ_ELF_FEMALE : UOBJ_HUMAN_FEMALE) :
		(((msg->Sex & FLAG_RACE) == FLAG_RACE) ? UOBJ_ELF_MALE : UOBJ_HUMAN_MALE) );

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

	if((msg->Sex & FLAG_GENDER) == FLAG_GENDER)
	{
		if((msg->Sex & FLAG_RACE) == FLAG_RACE)
		{
			chr->graphic  = UOBJ_ELF_FEMALE;
			chr->race	 = RACE_ELF;
		}
		else
		{
			chr->graphic  = UOBJ_HUMAN_FEMALE;
			chr->race	 = RACE_HUMAN;
		}

		chr->gender   = GENDER_FEMALE;
	}
	else
	{
		if((msg->Sex & FLAG_RACE) == FLAG_RACE)
		{
			chr->graphic  = UOBJ_ELF_MALE;
			chr->race	 = RACE_ELF;
		}
		else
		{
			chr->graphic  = UOBJ_HUMAN_MALE;
			chr->race	 = RACE_HUMAN;
		}

		chr->gender   = GENDER_MALE;
	}
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
