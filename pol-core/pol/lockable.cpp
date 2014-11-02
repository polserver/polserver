/*
History
=======


Notes
=======

*/

#include "lockable.h"

#include "../clib/cfgelem.h"
#include "../clib/streamsaver.h"

namespace Pol {
  namespace Core {
	ULockable::ULockable( const Items::ItemDesc& itemdesc, UOBJ_CLASS uobj_class ) :
	  Item( itemdesc, uobj_class ),
	  locked_( false )
	{}

	void ULockable::readProperties( Clib::ConfigElem& elem )
	{
	  base::readProperties( elem );
	  locked_ = elem.remove_bool( "Locked", false );
	}

	void ULockable::printProperties( Clib::StreamWriter& sw ) const
	{
	  base::printProperties( sw );

	  if ( locked_ )
		sw() << "\tLocked\t" << locked_ << pf_endl;
	}

	//dave 12-20
	Items::Item* ULockable::clone() const
	{
	  ULockable* item = static_cast<ULockable*>( base::clone() );

	  item->locked_ = locked_;
	  return item;
	}

    size_t ULockable::estimatedSize() const
    {
      return base::estimatedSize( ) + sizeof( bool )/* locked_*/;
    }
  }
}