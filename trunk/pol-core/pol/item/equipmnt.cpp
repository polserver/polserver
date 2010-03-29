/*
History
=======
2007/03/28 Shinigami: added increv() to Equipment::reduce_hp_from_hit

Notes
=======

*/


#include "../../clib/stl_inc.h"

#include "../../clib/cfgelem.h"
#include "../../clib/random.h"

#include "equipmnt.h"
#include "../equipdsc.h"
#include "../tooltips.h"
#include "../mobile/charactr.h"

Equipment::Equipment( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class ) :
    Item( itemdesc, uobj_class ),
    eq_tmpl_(static_cast<const EquipDesc&>(find_itemdesc( objtype_ ))), // be sure to grab the permanent entry
    quality_(1.0),
    hp_( eq_tmpl_.maxhp),
    maxhp_mod_(0)
{
}

Item* Equipment::clone() const
{
    Equipment* eq = static_cast<Equipment*>(base::clone());

    eq->quality_ = quality_;
    eq->hp_ = hp_;
    eq->maxhp_mod_ = maxhp_mod_;

    return eq;
}


void Equipment::printProperties( std::ostream& os ) const
{
    base::printProperties( os );

    os << "\tQuality\t" << quality_ << pf_endl;
    os << "\tHP\t" << hp_ << pf_endl;
    if (maxhp_mod_)
        os << "\tmaxhp_mod\t" << maxhp_mod_ << pf_endl;

}

void Equipment::readProperties( ConfigElem& elem )
{
    base::readProperties( elem );
    quality_ = elem.remove_double( "QUALITY", 1.0 );
    hp_ = elem.remove_ushort( "HP", eq_tmpl_.maxhp );
    maxhp_mod_ = static_cast<short>(elem.remove_int( "MAXHP_MOD", 0 ));
}

unsigned short Equipment::maxhp() const
{
    int mhp = eq_tmpl_.maxhp + maxhp_mod_;
    if (mhp >= 1)
        return static_cast<unsigned short>(mhp);
    else
        return 1;
}

void Equipment::reduce_hp_from_hit()
{
    if (hp_ >= 1 && random_int(100) == 0)
    {
        set_dirty();
        --hp_;
        increv();
		if (isa(CLASS_ARMOR))
		{
			Character* chr= GetCharacterOwner();
			if (chr != NULL)
				chr->refresh_ar();
		}
        send_object_cache_to_inrange(this);
    }
}
