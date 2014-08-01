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
namespace Pol {
  namespace Items {
	Equipment::Equipment( const ItemDesc& itemdesc, UOBJ_CLASS uobj_class ) :
	  Item( itemdesc, uobj_class ),
	  eq_tmpl_( static_cast<const Core::EquipDesc&>( find_itemdesc( objtype_ ) ) ) // be sure to grab the permanent entry
	{}

	Item* Equipment::clone() const
	{
	  Equipment* eq = static_cast<Equipment*>( base::clone() );

	  return eq;
	}

	void Equipment::printProperties( Clib::StreamWriter& sw ) const
	{
	  base::printProperties( sw );
	}

	void Equipment::readProperties( Clib::ConfigElem& elem )
	{
	  base::readProperties( elem );
	}

	void Equipment::reduce_hp_from_hit()
	{
      if ( hp_ >= 1 && Clib::random_int( 99 ) == 0 )
	  {
		set_dirty();
		--hp_;
		increv();
		if ( isa( CLASS_ARMOR ) )
		{
		  Mobile::Character* chr = GetCharacterOwner();
		  if ( chr != NULL )
			chr->refresh_ar();
		}
		send_object_cache_to_inrange( this );
	  }
	}

    size_t Equipment::estimatedSize() const
    {
      return base::estimatedSize();
    }
  }
}