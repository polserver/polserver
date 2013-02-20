/*
History
=======
2006/03/01 MuadDib:   Altered the decayat_gameclock_ check (to check for != 0).
2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/08/06 MuadDib:   Added gotten_by code for items.
2009/08/25 Shinigami: STLport-5.2.1 fix: new_ar not used
2009/09/03 MuadDib:	  Changes for account related source file relocation
                      Changes for multi related source file relocation
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../../clib/cfgelem.h"
#include "../../clib/endian.h"
#include "../../clib/strutil.h"

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"

#include "../../plib/mapcell.h"

#include "item.h"
#include "armor.h"
#include "../ustruct.h"
#include "../uofile.h"
#include "../containr.h"
#include "../ufunc.h"
#include "../network/client.h"
#include "../mobile/charactr.h"
#include "../item/itemdesc.h"
#include "../objtype.h"
#include "../../plib/pkg.h"
#include "../polcfg.h"
#include "../resource.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../stackcfg.h" //dave 1/26/3
#include "../tooltips.h"
#include "../uoscrobj.h"
#include "../ssopt.h"
/*
#include "clib/endian.h"

#include "armor.h"
#include "multi/boat.h"
#include "door.h"
#include "objtype.h"
#include "spelbook.h"
#include "weapon.h"
*/


/* This is a pretty good clone.  Somewhat inefficient, but does
   work for derived classes that do not have data.
   (since it calls Item::create, virtual tables are copied)
*/
Item* Item::clone() const
{
	Item* item = Item::create( objtype_ );
	item->color = color;
	item->graphic = graphic;
	item->x = x;
	item->y = y;
	item->z = z;
	item->realm = realm;
	item->facing = facing;
	item->setamount( amount_ );
	item->layer = layer;
	item->tile_layer = tile_layer;
	item->container = NULL; // was container
    item->sellprice_ = sellprice_;
    item->buyprice_ = buyprice_;
    item->newbie_ = newbie_;

	item->invisible_ = invisible_;	//dave 12-20
	item->movable_ = movable_;		//dave 12-20

	item->on_use_script_ = on_use_script_;	//dave 12-20
    item->equip_script_ = equip_script_;	//dave 12-20
    item->unequip_script_ = unequip_script_;	//dave 12-20

	item->name_ = name_;
	item->increv();

	item->copyprops( *this );

	item->saveonexit( saveonexit() );

	// Let's build the resistances defaults.
	item->element_resist.cold = element_resist.cold;
	item->element_resist.energy = element_resist.energy;
	item->element_resist.fire = element_resist.fire;
	item->element_resist.poison = element_resist.poison;
	item->element_resist.physical = element_resist.physical;

	item->element_resist_mod.cold = element_resist_mod.cold;
	item->element_resist_mod.energy = element_resist_mod.energy;
	item->element_resist_mod.fire = element_resist_mod.fire;
	item->element_resist_mod.poison = element_resist_mod.poison;
	item->element_resist_mod.physical = element_resist_mod.physical;

	// Let's build the resistances defaults.
	item->element_damage.cold = element_damage.cold;
	item->element_damage.energy = element_damage.energy;
	item->element_damage.fire = element_damage.fire;
	item->element_damage.poison = element_damage.poison;
	item->element_damage.physical = element_damage.physical;

	item->element_damage_mod.cold = element_damage_mod.cold;
	item->element_damage_mod.energy = element_damage_mod.energy;
	item->element_damage_mod.fire = element_damage_mod.fire;
	item->element_damage_mod.poison = element_damage_mod.poison;
	item->element_damage_mod.physical = element_damage_mod.physical;

	return item;
}

string Item::name() const
{
    if (name_ != "")
    {
        return name_;
    }
    else
    {
        return tile_desc( graphic );
    }
}

const ItemDesc& Item::itemdesc() const
{
    return find_itemdesc( objtype_ );
}

/* there are four forms of 'name' in objinfo:
    name					 (normal)
	name%s					 (percent followed by plural-part, then null-term)
	name%s%					 (percent followed by plural-part, then percent, then more)
	wheat shea%ves/f%		 ( '%', plural part, '/', single part, '%', rest )
   Some examples:
		pil%es/e% of hides
		banana%s%
		feather%s
   Known bugs:
	1 gold coin displays as "gold coin".  There must be a bit somewhere 
	that I just don't understand yet.
*/
string Item::description() const
{
    if (specific_name())
    {
		return ::format_description( 0, name(), amount_ ); //dave monkeyed with this 2/4/3
    }
    else
    {
        const ItemDesc& id = find_itemdesc( objtype_ );
        if (id.desc.empty())
        {
            return ::format_description( tile_flags( graphic ), tile_desc( graphic ), amount_ );
        }
        else
        {
            return ::format_description( tile_flags( graphic ), id.desc, amount_ );
        }
    }
}

string Item::get_use_script_name() const
{
	return on_use_script_;
}

string Item::merchant_description() const
{
    if (specific_name())
    {
        return name();
    }
    else
    {
        const ItemDesc& id = find_itemdesc( objtype_ );
        if (id.desc.empty())
        {
            return ::format_description( 0, tile_desc( graphic ), 1 );
        }
        else
        {
            return ::format_description( 0, id.desc, 1 );
        }
    }
}

unsigned int Item::sellprice() const
{
    if (sellprice_ < UINT_MAX) //dave changed 1/15/3 so 0 means 0, not default to itemdesc value
    {
        return sellprice_;
    }
    else
    {
        return itemdesc().vendor_sells_for;
    }
}

//Dave add buyprice() 11/28. Dont know wtf getbuyprice() is trying to do.
//  Dave, getbuyprice() was trying to return false if the vendor wasn't interested in buying.
//        it would return true if the vendor was interested in buying.
//   -Eric
unsigned int Item::buyprice() const
{
    if (buyprice_ < UINT_MAX) //dave changed 1/15/3 so 0 means 0, not default to itemdesc value
    {
        return buyprice_;
    }
    else
    {
        return itemdesc().vendor_buys_for;
    }
}

bool Item::getbuyprice( unsigned int& bp ) const
{
	bp = buyprice();
    if (bp > 0)
		return true;
	else
		return false;
	//dave ripped out the below, was wrong with the above buyprice() changes.
	/*
        if (buyprice_ > 0)
        {
            buyprice = static_cast<unsigned int>(buyprice_);
            return true;
        }
        else
        {
            buyprice = 0;
            return false;
        }
    }
    else
    {
        buyprice = itemdesc().vendor_buys_for;
        return (buyprice > 0);
    }
	*/
}

u8 Item::los_height() const
{
    return height / 2;
/*
    if (height)
        return height-1;
    else
        return 0;
*/
}

UObject* Item::owner()
{
    if (container != NULL)
        return container->self_as_owner();
    else
        return NULL;
}

const UObject* Item::owner() const
{
    if (container != NULL)
        return container->self_as_owner();
    else
        return NULL;
}

UObject* Item::toplevel_owner()
{
    Item* item = this;
    while (item->container != NULL)
        item = item->container;

    return item;
}

const UObject* Item::toplevel_owner() const
{
    const Item* item = this;
    while (item->container != NULL)
        item = item->container;

    return item;
}

const char* Item::classname() const
{
    return "Item";
}

bool Item::default_movable() const
{
    const ItemDesc& id = find_itemdesc( objtype_ );
    if (id.movable == ItemDesc::DEFAULT)
        return ((tile_flags( graphic ) & FLAG::MOVABLE) != 0);
    else
        return id.movable ? true : false;
}

bool Item::default_invisible() const
{
    return itemdesc().invisible;
}

bool Item::default_newbie() const
{
    return itemdesc().newbie;
}

void Item::printProperties( ostream& os ) const
{
    base::printProperties(os);
    if (amount_ != 1)
        os << "\tAmount\t" << amount_ << pf_endl;
    
    if (layer != 0)
        os << "\tLayer\t" << (int) layer << pf_endl;

    if (movable_ != default_movable())
        os << "\tMovable\t" << movable_ << pf_endl;

    if (invisible_ != default_invisible())
        os << "\tInvisible\t" << invisible_ << pf_endl;
    
	if (element_resist_mod.fire != 0)
		os << "\tFireResistMod\t" << static_cast<int>(element_resist_mod.fire) << pf_endl;
	if (element_resist_mod.cold != 0)
		os << "\tColdResistMod\t" << static_cast<int>(element_resist_mod.cold) << pf_endl;
	if (element_resist_mod.energy != 0)
		os << "\tEnergyResistMod\t" << static_cast<int>(element_resist_mod.energy) << pf_endl;
	if (element_resist_mod.poison != 0)
		os << "\tPoisonResistMod\t" << static_cast<int>(element_resist_mod.poison) << pf_endl;
	if (element_resist_mod.physical != 0)
		os << "\tPhysicalResistMod\t" << static_cast<int>(element_resist_mod.physical) << pf_endl;

	if (element_damage_mod.fire != 0)
		os << "\tFireDamageMod\t" << static_cast<int>(element_damage_mod.fire) << pf_endl;
	if (element_damage_mod.cold != 0)
		os << "\tColdDamageMod\t" << static_cast<int>(element_damage_mod.cold) << pf_endl;
	if (element_damage_mod.energy != 0)
		os << "\tEnergyDamageMod\t" << static_cast<int>(element_damage_mod.energy) << pf_endl;
	if (element_damage_mod.poison != 0)
		os << "\tPoisonDamageMod\t" << static_cast<int>(element_damage_mod.poison) << pf_endl;
	if (element_damage_mod.physical != 0)
		os << "\tPhysicalDamageMod\t" << static_cast<int>(element_damage_mod.physical) << pf_endl;

	if (container != NULL)
        os << "\tContainer\t0x" << hex << container->serial << dec << pf_endl;

    if (!on_use_script_.empty())
        os << "\tOnUseScript\t" << on_use_script_ << pf_endl;

    if (equip_script_ != itemdesc().equip_script)
        os << "\tEquipScript\t" << equip_script_ << pf_endl;

    if (unequip_script_ != itemdesc().unequip_script)
        os << "\tUnequipScript\t" << unequip_script_ << pf_endl;

    if (decayat_gameclock_ != 0)
        os << "\tDecayAt\t" << decayat_gameclock_ << pf_endl;

    if (sellprice_ != UINT_MAX) // recall that UINT_MAX means use default
        os << "\tSellPrice\t" << sellprice_ << pf_endl;

    if (buyprice_ != UINT_MAX) // recall that UINT_MAX means use default
        os << "\tBuyPrice\t" << buyprice_ << pf_endl;

    if (newbie_ != default_newbie())
        os << "\tNewbie\t" << newbie_ << pf_endl;
}

void Item::printDebugProperties( ostream& os ) const
{
    base::printDebugProperties( os );
}

void Item::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );

// Changed from Valid Color Mask to cfg mask in ssopt.
	color &= ssopt.item_color_mask;
    
    amount_ = elem.remove_ushort( "AMOUNT", 1 );
    layer = static_cast<unsigned char>(elem.remove_ushort( "LAYER", 0 ));
    movable_ = elem.remove_bool( "MOVABLE", default_movable() );
    invisible_ = elem.remove_bool( "INVISIBLE", default_invisible() );
    
    // NOTE, container is handled specially - it is extracted by the creator.
    
    on_use_script_ = elem.remove_string( "ONUSESCRIPT", "" );
    equip_script_ = elem.remove_string( "EQUIPSCRIPT", equip_script_.c_str() );
    unequip_script_ = elem.remove_string( "UNEQUIPSCRIPT", unequip_script_.c_str() );
    
    decayat_gameclock_ = elem.remove_ulong( "DECAYAT", 0 );
    sellprice_ = elem.remove_ulong( "SELLPRICE", UINT_MAX );
    buyprice_ = elem.remove_ulong( "BUYPRICE", UINT_MAX );

    // buyprice used to be read in with remove_int (which was wrong).
    // the UINT_MAX values used to be written out (which was wrong).
    // when UINT_MAX is read in by atoi, it returned 2147483647 (0x7FFFFFFF)
    // correct for this.
    if (buyprice_ == 2147483647)
        buyprice_ = UINT_MAX;
    newbie_ = elem.remove_bool( "NEWBIE", default_newbie() );

	element_resist_mod.fire = static_cast<s16>(elem.remove_int( "FIRERESISTMOD", 0 ));
	element_resist_mod.cold = static_cast<s16>(elem.remove_int( "COLDRESISTMOD", 0 ));
	element_resist_mod.energy = static_cast<s16>(elem.remove_int( "ENERGYRESISTMOD", 0 ));
	element_resist_mod.poison = static_cast<s16>(elem.remove_int( "POISONRESISTMOD", 0 ));
	element_resist_mod.physical = static_cast<s16>(elem.remove_int( "PHYSICALRESISTMOD", 0 ));

	element_damage_mod.fire = static_cast<s16>(elem.remove_int( "FIREDAMAGEMOD", 0 ));
	element_damage_mod.cold = static_cast<s16>(elem.remove_int( "COLDDAMAGEMOD", 0 ));
	element_damage_mod.energy = static_cast<s16>(elem.remove_int( "ENERGYDAMAGEMOD", 0 ));
	element_damage_mod.poison = static_cast<s16>(elem.remove_int( "POISONDAMAGEMOD", 0 ));
	element_damage_mod.physical = static_cast<s16>(elem.remove_int( "PHYSICALDAMAGEMOD", 0 ));

}

void Item::builtin_on_use( Client* client )
{
    send_sysmessage( client, "I can't think of a way to use that." );
}

void Item::double_click( Client* client )
{
    const ItemDesc& itemdesc = find_itemdesc( objtype_ );

    if (itemdesc.requires_attention && (client->chr->skill_ex_active() ||
                                        client->chr->casting_spell()))
    {
        send_sysmessage( client, "I am already doing something else." );
        return;
    }

    if (itemdesc.requires_attention && client->chr->hidden())
        client->chr->unhide();

    ref_ptr<EScriptProgram> prog;

    if (!on_use_script_.empty())
    {
        ScriptDef sd( on_use_script_, NULL, "" );
        prog = find_script2( sd, 
                            true, // complain if not found
                            config.cache_interactive_scripts );
    }
    else if (!itemdesc.on_use_script.empty())
    {
        prog = find_script2( itemdesc.on_use_script,
                            true,
                            config.cache_interactive_scripts );
    }

    if (prog.get() != NULL)
    {
        if (client->chr->start_itemuse_script( prog.get(), this, itemdesc.requires_attention ))
            return;
        // else log the fact?
    }
    else
    {
        builtin_on_use( client );
    }
}


unsigned short Item::get_senditem_amount() const
{
    return amount_;
}

bool Item::setlayer( unsigned char in_layer )
{
    if (tilelayer( graphic ) == in_layer)
    {
        layer = in_layer;
        return true;
    }
    else
    {
        return false;
    }
}

bool Item::stackable() const
{
    return (tile_flags( graphic ) & FLAG::STACKABLE) ? true : false;
}

void Item::setamount( u16 amount )
{
    set_dirty();
    int oldweight = weight();

    if (amount > amount_) // adding stuff
        remove_resources( objtype_, amount - amount_ );
    else
        return_resources( objtype_, amount_ - amount );

	amount_ = amount;
    int newweight = weight();
    if (container)
        container->add_bulk( 0, newweight-oldweight );

	increv();
	send_object_cache_to_inrange(this);
}

void Item::subamount( u16 amount_subtract )
{
    setamount( amount_ - amount_subtract );
	increv();
}

void Item::add_to_self( Item*& item )
{
  #ifdef PERGON
  ct_merge_stacks_pergon( item ); // Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
  #endif

	setamount( amount_ + item->amount_ );

    if (!item->newbie())
        newbie_ = false;

	item->destroy();
	item = NULL;
}

#ifdef PERGON
// Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
void Item::ct_merge_stacks_pergon( Item*& item_sub )
{
  int time_self, time_sub, time;
  string value_self, value_sub;

  // get "ct" of first stack - if error, init
  if (getprop( "ct", value_self ))
  {
    BObjectImp* imp = BObjectImp::unpack( value_self.c_str() );
    if (imp != NULL && imp->isa( BObjectImp::OTLong ))
      time_self = static_cast<BLong*>(imp)->value();
    else
      time_self = read_gameclock();
  }
  else
    time_self = read_gameclock();

  // get "ct" of second stack - if error, init
  if (item_sub->getprop( "ct", value_sub ))
  {
    BObjectImp* imp = BObjectImp::unpack( value_sub.c_str() );
    if (imp != NULL && imp->isa( BObjectImp::OTLong ))
      time_sub = static_cast<BLong*>(imp)->value();
    else
      time_sub = read_gameclock();
  }
  else
    time_sub = read_gameclock();

  // calculate new time
  if (time_self < time_sub)
  {
    double factor = (item_sub->amount_ * 1.0) / (amount_ + item_sub->amount_);
    time = int((time_sub - time_self) * factor + time_self);
  }
  else if (time_self > time_sub)
  {
    double factor = (amount_ * 1.0) / (amount_ + item_sub->amount_);
    time = int((time_self - time_sub) * factor + time_sub);
  }
  else
    time = time_self;

  setprop("ct", "i" + decint( time ));
  increv();
}

// Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
void Item::ct_merge_stacks_pergon( u16 amount_sub )
{
  int time_self, time_sub, time;
  string value_self;

  // get "ct" of first stack - if error, init
  if (getprop( "ct", value_self ))
  {
    BObjectImp* imp = BObjectImp::unpack( value_self.c_str() );
    if (imp != NULL && imp->isa( BObjectImp::OTLong ))
      time_self = static_cast<BLong*>(imp)->value();
    else
      time_self = read_gameclock();
  }
  else
    time_self = read_gameclock();

  // set "ct" of second new stack
  time_sub = read_gameclock();

  // calculate new time
  if (time_self != time_sub)
  {
    double factor = (amount_sub * 1.0) / (amount_ + amount_sub);
    time = int((time_sub - time_self) * factor + time_self);
  }
  else
    time = time_self;

  setprop("ct", "i" + decint( time ));
  increv();
}
#endif

bool Item::can_add_to_self( unsigned short amount ) const
{
	unsigned int amount1 = (unsigned int) amount_;
	unsigned int amount2 = (unsigned int) amount;

	if ((amount1 + amount2) > this->itemdesc().stack_limit)
		 return false;

    if (container != NULL)
    {
        int more_weight = weight_of(amount_+amount) - weight_of(amount_);
        if (more_weight > USHRT_MAX/*std::numeric_limits<unsigned short>::max()*/)
            return false;
        return container->can_add(static_cast<unsigned short>(more_weight));
    }

    return true;
}

bool Item::can_add_to_self( const Item& item ) const //dave 1/26/03 totally changed this function to handle the cprop comparisons.
{
	bool res = ( (item.objtype_ == objtype_) &&
				 (item.newbie() == newbie()) &&
				 (item.graphic  == graphic)  &&
				 (item.color    == color)    &&
				 (!inuse())                  &&
				 (can_add_to_self( item.amount_ )) 
		       );
	if(res == true)
	{
		//NOTE! this logic is copied in Item::has_only_default_cprops(), so make any necessary changes there too
		PropertyList myprops(getprops()); //make a copy :(
		myprops -= itemdesc().ignore_cprops;
		myprops -= Global_Ignore_CProps;

		PropertyList yourprops(item.getprops()); //make a copy :(
		yourprops -= item.itemdesc().ignore_cprops; 
		yourprops -= Global_Ignore_CProps;

		res = (myprops == yourprops);

		//myprops.printProperties(cout);
	}
	return res;
}

//dave added 5/11/3
bool Item::has_only_default_cprops(const ItemDesc* compare) const
{
    if( compare == NULL )
        compare = &(itemdesc());
	//logic same as Item::can_add_to_self()
	PropertyList myprops(getprops()); //make a copy :(
	myprops -= itemdesc().ignore_cprops;
	myprops -= Global_Ignore_CProps;
	
	PropertyList yourprops(compare->props); //make a copy :(
	yourprops -= compare->ignore_cprops; 
	yourprops -= Global_Ignore_CProps;

	return (myprops == yourprops);
}

bool Item::amount_to_remove_is_partial( u16 this_item_new_amount ) const
{
	return ((this_item_new_amount < amount_) &&  // less than what we have
			(amount_ > 1) &&					 // we are a stack
			(this_item_new_amount > 0));		// if new size 0, assume whole stack
}

/* slice_stacked_item: 
		Used when a portion of a stack is dragged.
		Given ITEM1, with, for example, amount 75.
		If 25 units are dragged off, this function will get called
		with 25.  What happens then:
		ITEM2 is created, with Amount 50.  It it placed where ITEM1 used to be.
		ITEM1's Amount is set to 25.
	In short, we go from:
		ITEM1 (Amount:75)
				TO
		ITEM1 (Amount:25) -under cursor, ITEM2(Amount:50) where ITEM1 was.
*/
Item* Item::slice_stacked_item( u16 this_item_new_amount )
{
	Item* new_item = clone();
	if (new_item != NULL) 
	{
		new_item->setamount( new_item->amount_ - this_item_new_amount );
		setamount( this_item_new_amount );
	}
	return new_item;
}

Item* Item::remove_part_of_stack( u16 amount_to_remove )
{
	Item* new_item = clone();
	if (new_item != NULL) 
	{
		new_item->setamount( amount_to_remove );
		subamount( amount_to_remove );
	}
	return new_item;
}



void Item::set_use_script( const string& scriptname )
{
    set_dirty();
    on_use_script_ = scriptname;
}

bool Item::saveonexit() const
{
    return saveonexit_;
}

void Item::saveonexit( bool newvalue )
{
    saveonexit_ = newvalue;
}

bool Item::setgraphic( u16 newgraphic )
{
	if (layer && layer != LAYER_MOUNT )
		return false;

	if (graphic <= config.max_tile_id && newgraphic <= config.max_tile_id)
    {
        set_dirty();
        graphic = newgraphic;
        height = tileheight( graphic );
		tile_layer = tilelayer( graphic );

		// Update facing on graphic change
		const ItemDesc& id = find_itemdesc( graphic );

		if ( id.facing == 127 )
			facing = tile_layer;
		else
			facing = id.facing;

		facing = tile_layer;
		increv();
		update_item_to_inrange( this );
        return true;
    }
    else
    {
        return false;
    }
}

bool Item::setcolor( u16 newcolor )
{
    // return false if the color is invalid (high nibble set)
    bool res = true;
	u16  theMask = (u16) ssopt.item_color_mask;
    if ((newcolor & (~theMask)) != 0)
        res = false;
    
    set_dirty();
    newcolor &= (theMask);
    color = newcolor;

    return res;
}

void Item::on_color_changed()
{
    update_item_to_inrange( this );
}

void Item::on_invisible_changed()
{
    update_item_to_inrange( this );
}

void Item::setfacing( u8 newfacing )
{
    // allow 0-127 (all but MSB)
    facing = newfacing; 
    if (facing > 127)
        facing = 0;
}

void Item::on_facing_changed()
{
    update_item_to_inrange( this );
}

void Item::extricate()
{
    if (container != NULL)
    {
        // hmm, a good place for a virtual?
        if (IsCharacter( container->serial ))
        {
            Character* chr = chr_from_wornitems( container );
            passert_always( chr != NULL ); // PRODFIXME linux-crash
            passert_always( chr->is_equipped(this) );

            chr->unequip(this); //FIXME: should run unequip script
        }
        else
        {
            container->remove( this );//FIXME: should probably call can/onRemove for the container
        }
    }
}

void Item::spill_contents( UMulti* multi )
{
}

unsigned int Item::weight_of( unsigned short amount ) const
{
    const ItemDesc& id = find_itemdesc( objtype_ );
    unsigned int amt = amount;
    amt *= id.weightmult;
    if (id.weightdiv != 1 && id.weightdiv != 0)
    {
        amt = (amt + id.weightdiv - 1 ) / id.weightdiv;
    }
    return amt;
}

unsigned int Item::weight() const
{
    return weight_of(amount_);
}
unsigned int Item::item_count() const
{
    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Decay-related functions
/////////////////////////////////////////////////////////////////////////////

#include "../gameclck.h"
void Item::set_decay_after( unsigned int seconds )
{
    set_dirty();
	// Why alter it, if it should not have it to begin with??
	if ( decayat_gameclock_ != 0 )
	{
		decayat_gameclock_ = read_gameclock() + seconds;
	}
}

bool Item::should_decay( unsigned int gameclock ) const
{
    return !inuse() &&
           (movable_ || (objtype_ == UOBJ_CORPSE)) && 
           decayat_gameclock_ && 
           (gameclock > decayat_gameclock_);
}

void Item::restart_decay_timer()
{
	// Why restart it, if it shouldn't have it to begin with??
	if ( decayat_gameclock_ != 0 )
	{
		unsigned int dtime = itemdesc().decay_time * 60;
		if ( should_decay( read_gameclock() + dtime ) )
		{
			set_decay_after( dtime );
		}
    }
}

void Item::disable_decay()
{
    set_dirty();
    decayat_gameclock_ = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Equip-Script related functions
/////////////////////////////////////////////////////////////////////////////
bool Item::has_equip_script() const
{
    return !equip_script_.empty();
}

BObjectImp* Item::run_equip_script( Character* who, bool startup )
{
    ScriptDef sd;

    try 
    {
        sd.config( equip_script_, itemdesc().pkg, "scripts/control/" );
    }
    catch( std::exception& ex )
    {
        return new BError( string("Script descriptor error") + ex.what() );
    }

    try 
    {
        return run_script_to_completion( sd,
                                     new ECharacterRefObjImp( who ),
                                     new EItemRefObjImp( this ),
                                     new BLong( startup ? 1 : 0 ) );
    }
    catch( std::exception& ex )
    {
        return new BError( string("Script execution error") + ex.what() );
    }
}

BObjectImp* Item::run_unequip_script( Character* who )
{
    ScriptDef sd;

    try 
    {
        sd.config( unequip_script_, itemdesc().pkg, "scripts/control/" );
    }
    catch( std::exception& ex )
    {
        return new BError( string("Script descriptor error: ") + ex.what() );
    }

    try 
    {
        return run_script_to_completion( sd,
                                         new ECharacterRefObjImp( who ),
                                         new EItemRefObjImp( this ) );
    }
    catch( std::exception& ex )
    {
        return new BError( string("Script execution error:") + ex.what() );
    }
}


bool Item::check_equip_script( Character* chr, bool startup )
{
    if (has_equip_script())
    {
        BObject obj( run_equip_script( chr, startup ) );
        return obj.isTrue();
    }
    else
    {
        return true;
    }
}

bool Item::check_unequip_script()
{
    if (!unequip_script_.empty() &&
        container != NULL &&
        IsCharacter( container->serial ))
    {
        Character* chr = chr_from_wornitems( container );
        passert_always( chr != NULL );
        passert_always( chr->is_equipped(this) );

        BObject obj( run_unequip_script( chr ) );
        return obj.isTrue();
    }
    else
    {
        return true;
    }
}

void preload_test_scripts(const std::string& script_ecl)
{
    ScriptDef sd;
    sd.quickconfig( "scripts/misc/" + script_ecl );
    if (sd.exists())
    {
        find_script2( sd, true, true );
    }
    Packages::iterator itr = packages.begin(), end = packages.end();
    for( ; itr != end; ++itr )
    {
        Package* pkg = *itr;
        sd.quickconfig( pkg, script_ecl );
        if (sd.exists())
        {
            find_script2( sd, true, true );
        }
    }
}
void preload_test_scripts()
{
    preload_test_scripts( "equiptest.ecl" );
    preload_test_scripts( "unequiptest.ecl" );
}

bool Item::check_test_scripts( Character* chr, const std::string& script_ecl, bool startup )
{
    ScriptDef sd;
    sd.quickconfig( "scripts/misc/" + script_ecl );
	this->inuse( true );
	if (script_loaded( sd ))
    {

		bool res = call_script( sd, 
                                new ECharacterRefObjImp(chr),
                                new EItemRefObjImp(this),
                                new BLong(startup));
		this->inuse( false );
		if (!res)
            return false;
    }
    Packages::iterator itr = packages.begin(), end = packages.end();
    for( ; itr != end; ++itr )
    {
        Package* pkg = *itr;
        sd.quickconfig( pkg, script_ecl );
        if ( script_loaded( sd ) )
        {
            bool res = call_script( sd,
                                    new ECharacterRefObjImp(chr),
                                    new EItemRefObjImp(this),
                                    new BLong(startup));
			this->inuse( false );
			if (!res)
                return false;
        }
    }

	this->inuse( false );
    return true;
}

bool Item::check_equiptest_scripts( Character* chr, bool startup )
{
    return check_test_scripts( chr, "equiptest.ecl", startup );
}

bool Item::check_unequiptest_scripts( Character* chr )
{
    return check_test_scripts( chr, "unequiptest.ecl", false );
}

bool Item::check_unequiptest_scripts()
{
    if (container != NULL &&
        IsCharacter( container->serial ))
    {
        Character* chr = chr_from_wornitems( container );
        passert_always( chr != NULL );
        passert_always( chr->is_equipped(this) );

        return check_unequiptest_scripts( chr );
    }
    else
    {
        return true;
    }
}

s16 Item::calc_element_resist( unsigned element ) const
{
	switch(element)
	{
		case ELEMENTAL_FIRE: return (element_resist.fire + element_resist_mod.fire); break;
		case ELEMENTAL_COLD: return (element_resist.cold + element_resist_mod.cold); break;
		case ELEMENTAL_ENERGY: return (element_resist.energy + element_resist_mod.energy); break;
		case ELEMENTAL_POISON: return (element_resist.poison + element_resist_mod.poison); break;
		case ELEMENTAL_PHYSICAL: return (element_resist.physical + element_resist_mod.physical); break;
	}
	return 0;
}

s16 Item::calc_element_damage( unsigned element ) const
{
	switch(element)
	{
		case ELEMENTAL_FIRE: return (element_damage.fire + element_damage_mod.fire); break;
		case ELEMENTAL_COLD: return (element_damage.cold + element_damage_mod.cold); break;
		case ELEMENTAL_ENERGY: return (element_damage.energy + element_damage_mod.energy); break;
		case ELEMENTAL_POISON: return (element_damage.poison + element_damage_mod.poison); break;
		case ELEMENTAL_PHYSICAL: return (element_damage.physical + element_damage_mod.physical); break;
	}
	return 0;
}

bool Item::has_resistance(Character* chr)
{
	if ( ((element_resist.fire + element_resist_mod.fire) != 0) || ((element_resist.cold + element_resist_mod.cold) != 0) ||
		((element_resist.energy + element_resist_mod.energy) != 0) || ((element_resist.poison + element_resist_mod.poison) != 0) || 
		((element_resist.physical + element_resist_mod.physical) != 0))
		return true;
	else
	{
		//double new_ar = 0.0;
		UArmor* armor = static_cast<UArmor*>(this);  // Not sure if I like this method.
		if (armor != NULL)
		{
			if (armor->ar() > 0)
				return true;
		}
	}
	return false;
}

bool Item::has_element_damage()
{
	if ( ((element_damage.fire + element_damage_mod.fire) != 0) || ((element_damage.cold + element_damage_mod.cold) != 0) ||
		((element_damage.energy + element_damage_mod.energy) != 0) || ((element_damage.poison + element_damage_mod.poison) != 0) || 
		((element_damage.physical + element_damage_mod.physical) != 0))
	{
		return true;
	}
	return false;
}

//DAVE added this 11/17, Shortcut function to get a pointer to the owner character
Character* Item::GetCharacterOwner()
{
	UObject* top_level_item = toplevel_owner();
	if(top_level_item->isa(CLASS_CONTAINER))
	{
		Character* chr_owner = chr_from_wornitems( static_cast<UContainer*>(top_level_item) );
		if(chr_owner != NULL)
		{	
			return chr_owner;
		}
		else
			return NULL;
	}
	else
		return NULL;
}

const char* Item::target_tag() const
{
    return "item";
}

