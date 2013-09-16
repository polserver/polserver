/*
History
=======
2009/08/07 MuadDib:   Added layer_list_ and functions like WornItems to corpse class. Used to handle showing equippable items
                      on a corpse.

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif

#include "../../clib/cfgelem.h"
#include "../../clib/endian.h"

#include "../containr.h"
#include "../layers.h"
#include "../ufunc.h"

UCorpse::UCorpse( const ContainerDesc& descriptor ) :
    UContainer( descriptor ),
    corpsetype(0),
    take_contents_to_grave(false)
{
    movable_ = false;
	layer_list_.resize( HIGHEST_LAYER + 1, EMPTY_ELEM );
}

u16 UCorpse::get_senditem_amount() const
{
    return corpsetype;
}

void UCorpse::spill_contents( UMulti* multi )
{
    bool any;
    do {
        any = false;
        for( iterator itr = begin(); itr != end(); ++itr )
        {
            Item *item = GET_ITEM_PTR( itr );
            if (item->tile_layer == LAYER_HAIR || item->tile_layer == LAYER_BEARD || item->tile_layer == LAYER_FACE || item->movable() == false)
            {
                ::destroy_item( item );
                any = true;
                break; // our iterators are now useless, so start over
            }
        }
    } while (any);

    if (!take_contents_to_grave)
        base::spill_contents( multi );
}

void UCorpse::PutItemOnLayer( Item* item )
{
	item->set_dirty();
	set_dirty();
	item->layer = item->tile_layer;
	layer_list_[ item->tile_layer ] = Contents::value_type( item );
	add_bulk( item );
}

void UCorpse::RemoveItemFromLayer( Item* item )
{
	item->set_dirty();
	set_dirty();
	layer_list_[ item->tile_layer ] = EMPTY_ELEM;
	item->layer = 0;
}

void UCorpse::printProperties( std::ostream& os ) const
{
    base::printProperties( os );
    os << "\tCorpseType\t" << corpsetype << pf_endl;
	os << "\tOwnerSerial\t" << ownerserial << pf_endl;
    os << "\tTakeContentsToGrave\t" << take_contents_to_grave << pf_endl;
}

void UCorpse::readProperties( ConfigElem& elem )
{
    // corpses can be the same color as mobiles
    u16 savecolor = elem.remove_ushort( "COLOR", 0 );
    
    base::readProperties( elem );

    color = savecolor;

    elem.remove_prop( "CorpseType", &corpsetype );
	elem.remove_prop("OwnerSerial", &ownerserial);
    take_contents_to_grave = elem.remove_bool( "TakeContentsToGrave", false );
    movable_ = false;
}


