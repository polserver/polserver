/*
History
=======
2005/02/14 Shinigami: find_container_desc - fixed passert condition
2005/12/07 MuadDib:   Changed "has already been specified for objtype" to 
                      "is the same as objtype" for easier non-english understanding.
2006/05/19 MuadDib:   Removed read_itemdesc_file( "config/wepndesc.cfg" ); line, due to
                      no longer being used by ANYTHING in the core. So no longer will
                      require this file to run POL. Same with config/armrdesc.cfg
2009/08/25 Shinigami: STLport-5.2.1 fix: init order changed of max_slots
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../bscript/escrutil.h"
#include "../../bscript/impstr.h"

#include "../../clib/cfgelem.h"
#include "../../clib/cfgfile.h"
#include "../../clib/esignal.h"
#include "../../clib/fileutil.h"
#include "../../clib/rawtypes.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"

#include "../../plib/mapcell.h"

#include "../cfgrepos.h"
#include "../clidata.h"
#include "../objtype.h"
#include "itemdesc.h"
#include "../multi/multidef.h"
#include "../../plib/pkg.h"
#include "../resource.h"
#include "../polcfg.h"
#include "../ssopt.h"
#include "../syshookscript.h"
#include "wepntmpl.h"
#include "armrtmpl.h"
#include "../ucfg.h"
#include "../ustruct.h"

typedef std::map<string,u32, ci_cmp_pred> ObjtypeByNameMap;
ObjtypeByNameMap objtype_byname;

ItemDesc empty_itemdesc( ItemDesc::ITEMDESC );

// The temp_itemdesc is used when something is created by graphic.
// another option is to create such ItemDesc objects on demand as needed, and keep them around.
ItemDesc temp_itemdesc( ItemDesc::ITEMDESC );

map<u32,ItemDesc*> desctable;
map<u32,bool> has_walkon_script_;
map<u32,bool> dont_save_itemtype_;

OldObjtypeConversions old_objtype_conversions;

unsigned int get_objtype_byname( const char* str )
{
	ObjtypeByNameMap::const_iterator itr = objtype_byname.find( str );
	if (itr == objtype_byname.end())
		return 0;
	else
		return (*itr).second;
}

unsigned int get_objtype_from_string( const std::string& str )
{
	unsigned int objtype;
	const char* ot_str = str.c_str();
	if (isdigit( *ot_str ))
	{
		objtype = static_cast<u32>(strtoul( ot_str, NULL, 0 ));
	}
	else
	{
		objtype = get_objtype_byname( ot_str );
		if (!objtype)
		{
			throw std::runtime_error( "Can't convert " + str + " to an objtype" );
		}
	}
	return objtype;
}

ResourceComponent::ResourceComponent( const std::string& rname, unsigned amount ) :
	rd( find_resource_def(rname) ),
	amount(amount)
{
	if (rd == NULL)
	{
		cerr << "itemdesc.cfg: Resource '" << rname <<"' not found\n" << endl;
		throw runtime_error( "Configuration error" );
	}
}


ItemDesc* ItemDesc::create( ConfigElem& elem, const Package* pkg )
{
	u32 objtype = static_cast<u32>(strtoul( elem.rest(), NULL, 0 ));
	if (!objtype)
	{
		elem.throw_error( "Element must have objtype specified" );
	}

	if (old_objtype_conversions.count( objtype ))
	{
		elem.throw_error( "Objtype is defined as an OldObjtype of "
							+ find_itemdesc(old_objtype_conversions[objtype]).objtype_description() );
	}

	ItemDesc* descriptor = NULL;

	if (elem.type_is( "Container" ))
	{
		descriptor = new ContainerDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "Item" ))
	{
		descriptor = new ItemDesc( objtype, elem, ItemDesc::ITEMDESC, pkg );
	}
	else if (elem.type_is( "Door" ))
	{
		descriptor = new DoorDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "Weapon" ))
	{
		// This one is making leaks when the descriptor is not given to a UWeapon object...
		// I really can't figure a good way to avoid this leak everytime
		descriptor = new WeaponDesc( objtype, elem, pkg ); 
	}
	else if (elem.type_is( "Armor" ))
	{
		descriptor = new ArmorDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "Boat" ))
	{
		descriptor = new BoatDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "House" ))
	{
		descriptor = new HouseDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "Spellbook" ))
	{
		descriptor = new SpellbookDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "SpellScroll" ))
	{
		descriptor = new SpellScrollDesc( objtype, elem, pkg );
	}
	else if (elem.type_is( "Map" ))
	{
		descriptor = new MapDesc( objtype, elem, pkg );
	}
	else
	{
		elem.throw_error( string("Unexpected element type: ") + elem.type() );
	}
	return descriptor;
}

ItemDesc::ItemDesc( u32 objtype, ConfigElem& elem, Type type, const Package* pkg ) :
	type( type ),
	pkg( pkg ),
	objtype( objtype ),
	graphic( elem.remove_ushort( "GRAPHIC", 0 ) ),
// Changed from Valid Color Mask to cfg mask in ssopt.
	color( elem.remove_ushort( "COLOR", 0 ) & ssopt.item_color_mask ),
	facing( static_cast<unsigned char>(elem.remove_ushort( "FACING", 127 )) ),
	desc( elem.remove_string( "DESC", "" ) ),
	tooltip( elem.remove_string( "TOOLTIP", "" ) ),
	walk_on_script( elem.remove_string( "WALKONSCRIPT", "" ), pkg, "scripts/items/" ),
	on_use_script( elem.remove_string( "SCRIPT", "" ), pkg, "scripts/items/" ),
	equip_script( elem.remove_string( "EQUIPSCRIPT", "" ) ),
	unequip_script( elem.remove_string( "UNEQUIPSCRIPT", "" ) ),
	control_script( elem.remove_string( "CONTROLSCRIPT", "" ), pkg, "scripts/control/" ),
	create_script( elem.remove_string( "CREATESCRIPT", "" ), pkg, "scripts/control/" ),
	destroy_script( elem.remove_string( "DESTROYSCRIPT", "" ), pkg, "scripts/control/" ),
	requires_attention( elem.remove_bool( "REQUIRESATTENTION", true ) ),
	lockable( elem.remove_bool( "LOCKABLE", false )),
	vendor_sells_for( elem.remove_ulong( "VENDORSELLSFOR", 0 ) ),
	vendor_buys_for( elem.remove_ulong( "VENDORBUYSFOR", 0 ) ),
	decay_time( elem.remove_ulong( "DECAYTIME", ssopt.default_decay_time ) ),
	movable( DEFAULT ),
	doubleclick_range( elem.remove_ushort( "DoubleclickRange", ssopt.default_doubleclick_range ) ),
	use_requires_los( elem.remove_bool( "UseRequiresLOS", true ) ), //Dave 11/24
	ghosts_can_use( elem.remove_bool( "GhostsCanUse", false ) ), //Dave 11/24
	can_use_while_paralyzed( elem.remove_bool( "CanUseWhileParalyzed", false ) ),
	can_use_while_frozen( elem.remove_bool( "CanUseWhileFrozen", false ) ),
	newbie( elem.remove_bool( "NEWBIE", false ) ),
	invisible( elem.remove_bool( "INVISIBLE", false ) ),
	decays_on_multis( elem.remove_bool( "DecaysOnMultis", 0 ) ),
	blocks_casting_if_in_hand( elem.remove_bool( "BlocksCastingIfInHand", true ) ),
	base_str_req( elem.remove_ushort( "StrRequired", 0 ) * 10 ),
	quality(elem.remove_double("QUALITY", 1.0) ),
	method_script(NULL)
{
	if (type == BOATDESC || type == HOUSEDESC)
	{
		multiid = elem.remove_ushort("MultiID", 0xFFFF);

		if (multiid == 0xFFFF)
		{
			cerr << "Itemdesc has no 'multiid' specified for a multi." << endl;
			cerr << "      Note: read corechanges.txt for the new multi format" << endl;
			elem.throw_error( "Configuration error" );
		}
	}
	else
	{
		if (graphic == 0)
		{
			if (objtype <= config.max_tile_id)
			{
				graphic = static_cast<u16>(objtype);
			}
			else
			{
				cerr << "Itemdesc has no 'graphic' specified, and no 'objtype' is valid either." << endl;
				elem.throw_error( "Configuration error" );
			}
		}
	}

	maxhp = elem.remove_ushort( "MAXHP", 0 );

	// Make sure Weapons and Armors ALL have this value defined to not break the core combat system
	if ( maxhp == 0 && ( type == WEAPONDESC || type == ARMORDESC ) )
	{
			cerr << "itemdesc.cfg, objtype " << hexint(objtype) << " has no MaxHP specified." << endl;
			elem.throw_error( "Configuration error" );
	}

	unsigned short stacklimit = elem.remove_ushort( "StackLimit", MAX_STACK_ITEMS );

	if (stacklimit > MAX_STACK_ITEMS)
		stack_limit = MAX_STACK_ITEMS;
	else
		stack_limit = stacklimit;

	if (tooltip.length() > PKTOUT_B7_MAX_CHARACTERS)
	{
		tooltip.erase( PKTOUT_B7_MAX_CHARACTERS, string::npos );
	}

	unsigned short in_movable;
	if (elem.remove_prop( "MOVABLE",  &in_movable ))
	{
		movable = in_movable ? MOVABLE : UNMOVABLE;
	}

	string weight_str;
	if (elem.remove_prop( "WEIGHT", &weight_str ))
	{
		const char* s = weight_str.c_str();
		char *endptr;
		weightmult = strtoul( s, &endptr, 0 );
		s = static_cast<const char*>(endptr);
		while (*s && isspace(*s))
			++s;
		if (*s == '/')
		{
			++s;
			weightdiv = strtoul( s, &endptr, 0 );
			if (!weightdiv)
			{
				elem.throw_error( "Poorly formed weight directive: " + weight_str );
			}
		}
		else if (*s == '\0')
		{
			weightdiv = 1;
		}
		else
		{
			elem.throw_error( "Poorly formed weight directive: " + weight_str );
		}
	}
	else
	{
		weightmult = tileweight(graphic);
		weightdiv = 1;
	}

	std::string temp;
	while (elem.remove_prop( "Resource", &temp ))
	{
		ISTRINGSTREAM is(temp);
		std::string rname;
		unsigned amount;
		if (is >> rname >> amount)
		{
			resources.push_back( ResourceComponent(rname,amount) );
		}
		else
		{
			cerr << "itemdesc.cfg, objtype 0x" << hex << objtype << dec
				 << ": Resource '" << temp << "' is malformed." << endl;
			throw runtime_error( "Configuration file error" );
		}
	}

	while (elem.remove_prop( "Name", &temp ) ||
		   elem.remove_prop( "ObjtypeName", &temp ))
	{

		if (objtype_byname.count( temp.c_str() ))
		{
			cerr << "Warning! objtype " << hexint( objtype )
				 << ": ObjtypeName '" << temp << "' is the same as objtype "
				 << hexint( objtype_byname[ temp.c_str() ] )
				 << endl;
			// throw runtime_error( "Configuration file error" );
		}
		else
		{
			objtype_byname[ temp.c_str() ] = objtype;
		}

		if (objtypename.empty())
			objtypename = temp;

/*
		if (objtype_byname.count( temp.c_str() ))
		{
			cerr << "itemdesc.cfg, objtype 0x" << hex << objtype << dec
				 << ": Name '" << temp << "' has already been specified for objtype 0x"
				 << hex << objtype_byname[ temp.c_str() ] << dec << endl;
			throw runtime_error( "Configuration file error" );
		}
*/
		//if (!objtype_byname.count( temp.c_str() ))
		//	objtype_byname[ temp.c_str() ] = objtype;
	}

	props.readProperties( elem );

	if (elem.remove_prop( "MethodScript", &temp ))
	{
		if (pkg == NULL)
			throw runtime_error( "MethodScripts can only be specified in a package" );
		if( !temp.empty() )
		{
			ExportScript* shs = new ExportScript( pkg, temp );
			if (shs->Initialize())
				method_script = shs;
			else
				delete shs;
		}
	}

	unsigned int old_objtype;
	while (elem.remove_prop( "OldObjtype", &old_objtype ))
	{
		if (old_objtype_conversions.count( old_objtype ))
		{
			elem.throw_error( objtype_description() + " specifies OldObjtype " + hexint(old_objtype)
							  + " which is already mapped to " +
							  find_itemdesc( old_objtype_conversions[old_objtype] ).objtype_description() );
		}
		if (has_itemdesc(old_objtype))
		{
			elem.throw_error( objtype_description() + " specifies OldObjtype " + hexint(old_objtype)
							  + " which is already defined as " + find_itemdesc(old_objtype_conversions[objtype]).objtype_description() );
		}
		old_objtype_conversions[ old_objtype ] = objtype;
	}

	if( elem.remove_prop( "StackingIgnoresCProps", &temp ))
	{
		ISTRINGSTREAM is( temp );
		string cprop_name;
		while (is >> cprop_name)
		{
			ignore_cprops.insert(cprop_name);
		}
	}

	for( unsigned resist = 0; resist <= ELEMENTAL_TYPE_MAX; ++resist )
	{
		string tmp;
		bool passed = false;

		switch(resist)
		{
			case ELEMENTAL_FIRE: passed = elem.remove_prop( "FIRERESIST", &tmp ); break;
			case ELEMENTAL_COLD: passed = elem.remove_prop( "COLDRESIST", &tmp ); break;
			case ELEMENTAL_ENERGY: passed = elem.remove_prop( "ENERGYRESIST", &tmp ); break;
			case ELEMENTAL_POISON: passed = elem.remove_prop( "POISONRESIST", &tmp ); break;
			case ELEMENTAL_PHYSICAL: passed = elem.remove_prop( "PHYSICALRESIST", &tmp ); break;
		}

		if (passed)
		{
            Dice dice;
		    string errmsg;
			if (!dice.load( tmp.c_str(), &errmsg ))
			{
				cerr << "Error loading itemdesc.cfg Elemental Resistances for "<< objtype_description() << " : " << errmsg << endl;
				throw runtime_error( "Error loading Item Elemental Resistances" );
			}
			switch(resist)
			{
				case ELEMENTAL_FIRE: element_resist.fire = dice.roll(); break;
				case ELEMENTAL_COLD: element_resist.cold = dice.roll(); break;
				case ELEMENTAL_ENERGY: element_resist.energy = dice.roll(); break;
				case ELEMENTAL_POISON: element_resist.poison = dice.roll(); break;
				case ELEMENTAL_PHYSICAL: element_resist.physical = dice.roll(); break;
			}
		}
		else
		{
			switch(resist)
			{
				case ELEMENTAL_FIRE: element_resist.fire = 0; break;
				case ELEMENTAL_COLD: element_resist.cold = 0; break;
				case ELEMENTAL_ENERGY: element_resist.energy = 0; break;
				case ELEMENTAL_POISON: element_resist.poison = 0; break;
				case ELEMENTAL_PHYSICAL: element_resist.physical = 0; break;
			}
		}
	}

	for( unsigned edamage = 0; edamage <= ELEMENTAL_TYPE_MAX; ++edamage )
	{
		string tmp;
		bool passed = false;

        switch(edamage)
		{
			case ELEMENTAL_FIRE: passed = elem.remove_prop( "FIREDAMAGE", &tmp ); break;
			case ELEMENTAL_COLD: passed = elem.remove_prop( "COLDDAMAGE", &tmp ); break;
			case ELEMENTAL_ENERGY: passed = elem.remove_prop( "ENERGYDAMAGE", &tmp ); break;
			case ELEMENTAL_POISON: passed = elem.remove_prop( "POISONDAMAGE", &tmp ); break;
			case ELEMENTAL_PHYSICAL: passed = elem.remove_prop( "PHYSICALDAMAGE", &tmp ); break;
		}

		if (passed)
		{
            Dice dice;
		    string errmsg;
			if (!dice.load( tmp.c_str(), &errmsg ))
			{
				cerr << "Error loading itemdesc.cfg elemental damages for "<< objtype_description() << " : " << errmsg << endl;
				throw runtime_error( "Error loading Item Elemental Damages" );
			}
			switch(edamage)
			{
				case ELEMENTAL_FIRE: element_damage.fire = dice.roll(); break;
				case ELEMENTAL_COLD: element_damage.cold = dice.roll(); break;
				case ELEMENTAL_ENERGY: element_damage.energy = dice.roll(); break;
				case ELEMENTAL_POISON: element_damage.poison = dice.roll(); break;
				case ELEMENTAL_PHYSICAL: element_damage.physical = dice.roll(); break;
			}
		}
		else
		{
			switch(edamage)
			{
				case ELEMENTAL_FIRE: element_damage.fire = 0; break;
				case ELEMENTAL_COLD: element_damage.cold = 0; break;
				case ELEMENTAL_ENERGY: element_damage.energy = 0; break;
				case ELEMENTAL_POISON: element_damage.poison = 0; break;
				case ELEMENTAL_PHYSICAL: element_damage.physical = 0; break;
			}
		}
	}
}

ItemDesc::ItemDesc(Type type) :
	type(type),
	pkg(NULL),
	objtype(0),
	graphic(0),
	color(0),
	facing(127),
	weightmult(1),
	weightdiv(1),
	desc(""),
	tooltip(""),
	//walk_on_script2(""),
	//on_use_script2(""),
	//control_script2(""),
	//create_script2(""),
	//destroy_script2(""),
	requires_attention(false),
	lockable(false),
	vendor_sells_for(0),
	vendor_buys_for(0),
	decay_time(ssopt.default_decay_time),
	movable( DEFAULT ),
	doubleclick_range(ssopt.default_doubleclick_range),
	newbie(false),
	invisible(false),
	decays_on_multis(false),
	blocks_casting_if_in_hand( true ),
	base_str_req(0),
	stack_limit(MAX_STACK_ITEMS),
	method_script(NULL)
{
}

ItemDesc::~ItemDesc()
{
	unload_scripts();
}

void ItemDesc::PopulateStruct( BStruct* descriptor ) const
{
	string typestr;
	switch( type )
	{
		case ITEMDESC:			typestr = "Item"; break;
		case CONTAINERDESC:		typestr = "Container"; break;
		case DOORDESC:			typestr = "Door"; break;
		case WEAPONDESC:		typestr = "Weapon"; break;
		case ARMORDESC:		typestr = "Armor"; break;
		case BOATDESC:			typestr = "Boat"; break;
		case HOUSEDESC:		typestr = "House"; break;
		case SPELLBOOKDESC:		typestr = "Spellbook"; break;
		case SPELLSCROLLDESC:	typestr = "Spellscroll"; break;
		case MAPDESC:			typestr = "Map"; break;
		default:				typestr = "Unknown"; break;
	}
	descriptor->addMember( "ObjClass",	new String(typestr) );
	descriptor->addMember( "ObjType",	new BLong(objtype) );
	descriptor->addMember( "Graphic",	new BLong(graphic) );
	descriptor->addMember( "Name",	new String(objtypename) );
	descriptor->addMember( "Color",	new BLong(color) );
	descriptor->addMember( "Facing",	new BLong(facing) );
	descriptor->addMember( "Desc",	new String(desc) );
	descriptor->addMember( "Tooltip",	new String(tooltip) );
	descriptor->addMember( "WalkOnScript",	new String(walk_on_script.relativename(pkg)) );
	descriptor->addMember( "Script",		new String(on_use_script.relativename(pkg)) );
	descriptor->addMember( "EquipScript",	new String(equip_script) );
	descriptor->addMember( "UnequipScript", new String(unequip_script) );
	descriptor->addMember( "ControlScript", new String(control_script.relativename(pkg)) );
	descriptor->addMember( "CreateScript",	new String(create_script.relativename(pkg)) );
	descriptor->addMember( "DestroyScript", new String(destroy_script.relativename(pkg)) );
	descriptor->addMember( "MethodScript",	new String( method_script?method_script->scriptname():"") );
	descriptor->addMember( "RequiresAttention",	new BLong(requires_attention) );
	descriptor->addMember( "Lockable",			new BLong(lockable) );
	descriptor->addMember( "VendorSellsFor",	new BLong(vendor_sells_for) );
	descriptor->addMember( "VendorBuysFor",		new BLong(vendor_buys_for) );
	descriptor->addMember( "DecayTime",		new BLong(decay_time) );
	descriptor->addMember( "Movable",			new BLong(default_movable()) );
	descriptor->addMember( "DoubleClickRange",	new BLong(doubleclick_range) );
	descriptor->addMember( "UseRequiresLOS",	new BLong(use_requires_los) );
	descriptor->addMember( "GhostsCanUse",		new BLong(ghosts_can_use) );
	descriptor->addMember( "CanUseWhileFrozen",	new BLong(can_use_while_frozen) );
	descriptor->addMember( "CanUseWhileParalyzed", new BLong(can_use_while_paralyzed) );
	descriptor->addMember( "Newbie",			new BLong(newbie) );
	descriptor->addMember( "Invisible",		new BLong(invisible) );
	descriptor->addMember( "DecaysOnMultis",	new BLong(decays_on_multis) );
	descriptor->addMember( "BlocksCastingIfInHand", new BLong(blocks_casting_if_in_hand) );
	descriptor->addMember( "StrRequired",		new BLong(base_str_req) );
	descriptor->addMember( "StackLimit",		new BLong(stack_limit) );
	descriptor->addMember( "Weight",			new Double(static_cast<double>(weightmult)/weightdiv) );
	descriptor->addMember( "FireResist",		new BLong(element_resist.fire) );
	descriptor->addMember( "ColdResist",		new BLong(element_resist.cold) );
	descriptor->addMember( "EnergyResist",		new BLong(element_resist.energy) );
	descriptor->addMember( "PoisonResist",		new BLong(element_resist.poison) );
	descriptor->addMember( "PhysicalResist",	new BLong(element_resist.physical) );
	descriptor->addMember( "FireDamage",		new BLong(element_damage.fire) );
	descriptor->addMember( "ColdDamage",		new BLong(element_damage.cold) );
	descriptor->addMember( "EnergyDamage",		new BLong(element_damage.energy) );
	descriptor->addMember( "PoisonDamage",		new BLong(element_damage.poison) );
	descriptor->addMember( "PhysicalDamage",	new BLong(element_damage.physical) );
	descriptor->addMember( "Quality",			new Double(quality) );
	descriptor->addMember( "MultiID",			new BLong(multiid) );
	descriptor->addMember( "MaxHp",				new BLong(maxhp) );

	std::set<std::string>::const_iterator set_itr;
	std::unique_ptr<ObjArray> ignorecp (new ObjArray);
	for(set_itr = ignore_cprops.begin(); set_itr != ignore_cprops.end(); ++set_itr)
		ignorecp->addElement( new String(*set_itr) );

	descriptor->addMember( "StackingIgnoresCProps", ignorecp.release() );

	auto cpropdict = new BDictionary();
	vector<string> propnames;
	std::vector<std::string>::const_iterator vec_itr;
	string tempval;
	props.getpropnames(propnames);
	for(vec_itr = propnames.begin(); vec_itr != propnames.end(); ++vec_itr)
	{
		props.getprop(*vec_itr,tempval);
		cpropdict->addMember( vec_itr->c_str(), BObjectImp::unpack(tempval.c_str()) );
	}

	descriptor->addMember( "CProps", cpropdict );

	// FIXME: this should be a single member similar to CProps, called Resources
	std::vector<ResourceComponent>::const_iterator resource_itr;
	for( resource_itr = resources.begin(); resource_itr != resources.end(); ++resource_itr )
	{
		descriptor->addMember( "Resource",	new String(resource_itr->rd->name() + " " + decint(resource_itr->amount)));
	}

}

void ItemDesc::unload_scripts()
{
	if (method_script != NULL)
	{
		delete method_script;
		method_script = NULL;
	}
}

string ItemDesc::objtype_description() const
{
	string tmp;
	if (pkg)
		tmp = ":" + pkg->name() + ":";
	tmp += objtypename;
	tmp += " (" + hexint( objtype ) + ")";
	return tmp;
}

bool ItemDesc::default_movable() const
{
	if (movable == DEFAULT)
		return ((tile_flags( graphic ) & FLAG::MOVABLE) != 0);
	else
		return movable ? true : false;
}

ContainerDesc::ContainerDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	ItemDesc( objtype, elem, CONTAINERDESC, pkg ),
	gump(elem.remove_ushort( "GUMP" )),
	minx(elem.remove_ushort( "MINX" )),
	maxx(elem.remove_ushort( "MAXX" )),
	miny(elem.remove_ushort( "MINY" )),
	maxy(elem.remove_ushort( "MAXY" )),
	max_weight(elem.remove_ushort( "MAXWEIGHT", ssopt.default_container_max_weight )),
	max_items(elem.remove_ushort( "MAXITEMS", ssopt.default_container_max_items )),
	max_slots(static_cast<u8>(elem.remove_ushort( "MAXSLOTS", ssopt.default_max_slots ))),
	can_insert_script(elem.remove_string( "CANINSERTSCRIPT", "" ), pkg, "scripts/control/" ),
	on_insert_script(elem.remove_string( "ONINSERTSCRIPT", "" ), pkg, "scripts/control/" ),
	can_remove_script(elem.remove_string( "CANREMOVESCRIPT", "" ), pkg, "scripts/control/" ),
	on_remove_script(elem.remove_string( "ONREMOVESCRIPT", "" ), pkg, "scripts/control/" )
{
	// FIXME: in theory, should never happen due to conversion to u8. Maybe here as note during rewrite. Add a remove_uchar/remove_char for allowing
	// use of max 0-255 integers control due to packet limits, in configuration files. Yay.
//	if ( max_slots > 255 )
//	{
//			cerr << "Warning! Container " << hexint( objtype ) << ": Invalid MaxSlots defined. MaxSlots max value is 255. Setting to 255." << endl;
//			max_slots = 255;
//	}
}

void ContainerDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
	descriptor->addMember( "Gump", new BLong(gump) );
	descriptor->addMember( "MinX", new BLong(minx) );
	descriptor->addMember( "MinY", new BLong(miny) );
	descriptor->addMember( "MaxX", new BLong(maxx) );
	descriptor->addMember( "MaxY", new BLong(maxy) );
	descriptor->addMember( "MaxWeight", new BLong(max_weight) );
	descriptor->addMember( "MaxItems", new BLong(max_items) );
	descriptor->addMember( "MaxSlots", new BLong(max_slots) );
	descriptor->addMember( "CanInsertScript", new String(can_insert_script.relativename(pkg)) );
	descriptor->addMember( "CanRemoveScript", new String(can_remove_script.relativename(pkg)) );
	descriptor->addMember( "OnInsertScript", new String(on_insert_script.relativename(pkg)) );
	descriptor->addMember( "OnRemoveScript", new String(on_remove_script.relativename(pkg)) );
}

DoorDesc::DoorDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	ItemDesc( objtype, elem, DOORDESC, pkg ),
	xmod( static_cast<s16>(elem.remove_int( "XMOD" )) ),
	ymod( static_cast<s16>(elem.remove_int( "YMOD" )) ),
	open_graphic( elem.remove_ushort( "OPENGRAPHIC", 0 ) )
{
}

void DoorDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
	descriptor->addMember( "XMod", new BLong(xmod) );
	descriptor->addMember( "YMod", new BLong(ymod) );
	descriptor->addMember( "OpenGraphic", new BLong(open_graphic) );
}

SpellbookDesc::SpellbookDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	ContainerDesc( objtype, elem, pkg ),
	spelltype( elem.remove_string( "SPELLTYPE" ))
{
	type = SPELLBOOKDESC;
}

void SpellbookDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
	descriptor->addMember( "Spelltype", new String(spelltype) );
}

SpellScrollDesc::SpellScrollDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	ItemDesc( objtype, elem, SPELLSCROLLDESC, pkg ),
	spelltype( elem.remove_string( "SPELLTYPE" ))
{
}

void SpellScrollDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
	descriptor->addMember( "Spelltype", new String(spelltype) );
}

MultiDesc::MultiDesc( u32 objtype, ConfigElem& elem, Type type, const Package* pkg ) :
	ItemDesc( objtype, elem, type, pkg )
{
	if (!MultiDefByMultiIDExists(multiid))
	{
		elem.throw_error( "Multi definition not found.  Objtype="
							+ hexint(objtype)
							+ ", multiid="
							+ hexint(multiid) );
	}
}
void MultiDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
}

BoatDesc::BoatDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	MultiDesc( objtype, elem, BOATDESC, pkg )
{
}

void BoatDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
}

HouseDesc::HouseDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	MultiDesc( objtype, elem, HOUSEDESC, pkg )
{
}

void HouseDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
}

MapDesc::MapDesc( u32 objtype, ConfigElem& elem, const Package* pkg ) :
	ItemDesc( objtype, elem, MAPDESC, pkg ),
	editable( elem.remove_bool( "EDITABLE",true ))
{
}

void MapDesc::PopulateStruct( BStruct* descriptor ) const
{
	base::PopulateStruct( descriptor );
	descriptor->addMember( "Editable", new BLong(editable) );
}

bool objtype_is_lockable( u32 objtype )
{
	const ItemDesc& id = find_itemdesc(objtype);
	return id.lockable;
}

unsigned short getgraphic( u32 objtype )
{
	const ItemDesc& id = find_itemdesc(objtype);

	// Check here to make sure multis are created without a graphic entry in itemdesc.
	if (id.type == id.BOATDESC || id.type == id.HOUSEDESC)
		return 0;

	if (id.graphic)
	{
		return id.graphic;
	}
	else if (objtype <= config.max_tile_id)
	{
		return static_cast<u16>(objtype);
	}
	else
	{
		//Log( "Objtype "  + hexint(objtype) + " must define a graphic\n" );
		throw std::runtime_error( "Objtype "  + hexint(objtype) + " must define a graphic" );
		passert(0);
		return 0;
	}
}
unsigned short getcolor( unsigned int objtype )
{
	const ItemDesc& id = find_itemdesc(objtype);
	return id.color;
}

const ItemDesc& find_itemdesc( unsigned int objtype )
{
    if( desctable.count(objtype) )
		return *desctable[objtype];
	else
		return empty_itemdesc;
}

const ContainerDesc& find_container_desc( u32 objtype )
{
	ContainerDesc* cd;
	if( has_itemdesc(objtype) )
		cd = static_cast<ContainerDesc*>(desctable[objtype]);
	else
		cd = static_cast<ContainerDesc*>(&empty_itemdesc);
	passert_r( (cd->type == ItemDesc::CONTAINERDESC) || (cd->type == ItemDesc::SPELLBOOKDESC),
	  "ObjType " + hexint(objtype) + " should be defined as container or spellbook, but is not");
   // if ( (cd->type != ItemDesc::CONTAINERDESC) && (cd->type != ItemDesc::SPELLBOOKDESC) )
   //	 throw runtime_error( "Objtype " + hexint(objtype) + " should be defined as a container or spellbook, but is not." );

	return *cd;
}

const DoorDesc& fast_find_doordesc( u32 objtype )
{
	DoorDesc* dd;
	if( has_itemdesc(objtype) )
		dd = static_cast<DoorDesc*>(desctable[objtype]);
	else
		dd = static_cast<DoorDesc*>(&empty_itemdesc);
	passert( dd->type == ItemDesc::DOORDESC );
	return *dd;
}

const MultiDesc& find_multidesc( u32 objtype )
{
   const ItemDesc* id;
   if( has_itemdesc(objtype) )
	   id = desctable[objtype];
   else
	   id = &empty_itemdesc;
   passert( id->type == ItemDesc::BOATDESC || id->type == ItemDesc::HOUSEDESC );
   return *static_cast<const MultiDesc*>(id);
}

vector< ItemDesc* > dynamic_item_descriptors;
const ItemDesc* CreateItemDescriptor( BStruct* itemdesc_struct )
{
	ConfigElem elem;
	StubConfigSource stub_source;
	elem.set_source( &stub_source );

	const BStruct::Contents& struct_cont = itemdesc_struct->contents();
	BStruct::Contents::const_iterator itr;
	for( itr = struct_cont.begin(); itr != struct_cont.end(); ++itr )
	{
		const string& key = (*itr).first;
		BObjectImp* val_imp = (*itr).second->impptr();

		if( key == "CProps" )
		{
			if (val_imp->isa( BObjectImp::OTDictionary ))
			{
				BDictionary* cpropdict = static_cast<BDictionary*>(val_imp);
				const BDictionary::Contents& cprop_cont = cpropdict->contents();
				BDictionary::Contents::const_iterator ditr;
				for( ditr = cprop_cont.begin(); ditr != cprop_cont.end(); ++ditr )
				{
					elem.add_prop("cprop", ((*ditr).first->getStringRep() + "\t" + (*ditr).second->impptr()->pack()).c_str() );
				}
			}
			else
			{
				throw runtime_error( "CreateItemDescriptor: CProps must be a dictionary, but is: " + string( val_imp->typeOf() ) );
			}
		}
		else if( key == "StackingIgnoresCProps" )
		{
			if (val_imp->isa( BObjectImp::OTArray))
			{
				OSTRINGSTREAM os;
				// FIXME verify that it's an ObjArray...
				ObjArray* ignorecp = static_cast<ObjArray*>(itr->second->impptr());
				const ObjArray::Cont& conts = ignorecp->ref_arr;
				ObjArray::Cont::const_iterator aitr;
				for( aitr = conts.begin(); aitr != conts.end(); ++aitr )
				{
					os << (*aitr).get()->impptr()->getStringRep() << " ";
				}
				elem.add_prop(key.c_str(),OSTRINGSTREAM_STR(os).c_str());
			}
			else
			{
				throw runtime_error( "CreateItemDescriptor: StackingIgnoresCProps must be an array, but is: " + string( val_imp->typeOf() ) );
			}
		}
		else if( key == "Coverage" ) //Dave 7/13 needs to be parsed out into individual lines
		{
			if (val_imp->isa( BObjectImp::OTArray))
			{
				// FIXME verify that it's an ObjArray...
				ObjArray* coverage = static_cast<ObjArray*>(itr->second->impptr());
				const ObjArray::Cont& conts = coverage->ref_arr;
				ObjArray::Cont::const_iterator aitr;
				for( aitr = conts.begin(); aitr != conts.end(); ++aitr )
				{
					OSTRINGSTREAM os;
					os << (*aitr).get()->impptr()->getStringRep();
					elem.add_prop(key.c_str(),OSTRINGSTREAM_STR(os).c_str());
				}
			}
			else
			{
				throw runtime_error( "CreateItemDescriptor: Coverage must be an array, but is: " + string( val_imp->typeOf() ) );
			}
		}
		else if( key == "ObjClass" )
		{
			string value = val_imp->getStringRep();
			elem.set_type( value.c_str() );
		}
		else if( key == "ObjType" )
		{
			string value = val_imp->getStringRep();
			elem.set_rest( value.c_str() );
		}
		else if( strlower(key) == "name" ||
				 strlower(key) == "objtypename" ||
				 strlower(key) == "oldobjtype" ||
				 strlower(key) == "methodscript" ||
				 strlower(key) == "weight")
		{
			// all of these only affect the main descriptor, so they're left out.
			//   name, objtypename, and oldobjtype would try to insert aliases
			//   methodscript would create scripts and such.
			//   weight, for values like 0.1 (came in as 1/10), won't parse correctly.
			continue;
		}
		else
		{
			string value = val_imp->getStringRep();
			elem.add_prop(key.c_str(), value.c_str());
		}
	}

	unsigned int objtype = static_cast<unsigned int>(strtoul(elem.rest(),NULL,0));
	ItemDesc* id = ItemDesc::create( elem, find_itemdesc(objtype).pkg );

	dynamic_item_descriptors.push_back(id);

	return id;
}


void adddesc_container( ConfigElem& elem );
#include "itemdesc.h"

void read_itemdesc_file( const char* filename, Package* pkg = NULL )
{
/*
	if (1)
	{
		ref_ptr<StoredConfigFile> scfg = FindConfigFile( "config/itemdesc.cfg" );
		ConfigFile cf( filename );
		scfg->load( cf );
	}
*/
	ConfigFile cf( filename, "CONTAINER ITEM DOOR WEAPON ARMOR BOAT HOUSE SPELLBOOK SPELLSCROLL MAP" );

	ConfigElem elem;
	while (cf.read( elem ))
	{
		ItemDesc* descriptor = ItemDesc::create( elem, pkg );


		//string unused_name, unused_value;
		//while (elem.remove_first_prop( &unused_name, &unused_value ))
		//{
		//	elem.warn_with_line( "Property '" + unused_name + "' (value '" + unused_value + "') is unused." );
		//}

		if ( has_itemdesc(descriptor->objtype) )
		{
			cerr << "Error: Objtype " << hexint(descriptor->objtype) << " is already defined in ";
			if ( find_itemdesc(descriptor->objtype).pkg == NULL)
				cerr << "config/itemdesc.cfg";
			else
				cerr << find_itemdesc(descriptor->objtype).pkg->dir() << "itemdesc.cfg";
			cerr << endl;

			elem.throw_error( "ObjType " + hexint( descriptor->objtype ) + " defined more than once." );
		}
		has_walkon_script_[ descriptor->objtype ] = (!descriptor->walk_on_script.empty());
		desctable[ descriptor->objtype ] = descriptor;

		dont_save_itemtype_[descriptor->objtype] = !elem.remove_bool( "SaveOnExit", true );
		// just make sure this will work later.
		getgraphic( descriptor->objtype );
	}
}

void load_package_itemdesc( Package* pkg )
{
	//string filename = pkg->dir() + "itemdesc.cfg";
	string filename = GetPackageCfgPath(pkg, "itemdesc.cfg");
	if (FileExists( filename.c_str() ))
	{
		read_itemdesc_file( filename.c_str(), pkg );
	}
}

void write_objtypes_txt()
{
	ofstream ofs( "objtypes.txt" );
	unsigned int last_objtype = 0;
	for(const auto &elem : desctable)
	{
		const ItemDesc* itemdesc = elem.second;
		unsigned int i = elem.first;

		if (i != last_objtype+1)
		{
			unsigned int first = last_objtype+1;
			unsigned int last = i-1;
			if (first == last)
			{
				ofs << "# " << hexint(first) << " unused\n";
			}
			else
			{
				ofs << "# " << hexint(first) << " - " << hexint(last) << " unused\n";
			}
		}

		if (!old_objtype_conversions.count(i))
		{
			ofs << hexint(i) << " ";
			if (itemdesc->objtypename.empty() == false)
				ofs << itemdesc->objtypename;
			else
				ofs << "[n/a]";
			if (itemdesc->pkg)
				ofs << " " << itemdesc->pkg->name();
			ofs << '\n';
		}
		else // it's converted
		{
			ofs << "# " << hexint(i)
				<< " converts to "
				<< hexint((int)old_objtype_conversions[i])
				<< '\n';
		}

		last_objtype = i;
	}

	if (last_objtype != config.max_objtype)
	{
		unsigned int first = last_objtype+1;
		unsigned int last = config.max_objtype;
		if (first == last)
		{
			ofs << "# " << hexint(first) << " unused\n";
		}
		else
		{
			ofs << "# " << hexint(first) << " - " << hexint(last) << " unused\n";
		}
	}
}

void load_itemdesc()
{
//	CreateEmptyStoredConfigFile( "config/itemdesc.cfg" );
	if ( FileExists("config/itemdesc.cfg") )
		read_itemdesc_file( "config/itemdesc.cfg" );
//	read_itemdesc_file( "config/wepndesc.cfg" );
//	read_itemdesc_file( "config/armrdesc.cfg" );

	ForEach( packages, load_package_itemdesc );

	write_objtypes_txt();
}

void unload_itemdesc()
{
	for(auto &elem : desctable)
	{
		//cout << "Objtype: " << hexint(itr->first) << " " << itr->second << " " << &empty_itemdesc << endl;
		if (elem.second != &empty_itemdesc) {
			delete elem.second;
			elem.second = &empty_itemdesc;
		}
	}

	objtype_byname.clear();
	old_objtype_conversions.clear();
	
}

void unload_itemdesc_scripts()
{
	for(auto &elem : desctable)
	{
		elem.second->unload_scripts();
	}
}

void remove_resources( u32 objtype, u16 amount )
{
	const ItemDesc& id = find_itemdesc( objtype );
	for(const auto& rc : id.resources)
	{
		rc.rd->consume( rc.amount );
	}
}

void return_resources( u32 objtype, u16 amount )
{
	// MuadDib Added 03/22/09. This can cause a crash in shutdown with orphaned/leaked items
	// after saving of data files, and clearing all objects. At this stage, there is no need
	// to free and replenish ore/wood/etc resources in the world. o_O
	if ( !exit_signalled )
	{
		const ItemDesc& id = find_itemdesc( objtype );
		for(const auto& rc : id.resources)
		{
			rc.rd->produce( rc.amount );
		}
	}
}
