/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"
#include "realm.h"
#include "../pol/los.h"
#include "../pol/mobile/charactr.h"

namespace Pol {
  namespace Plib {
    bool Realm::has_los( const Core::UObject& iatt, const Core::UObject& itgt ) const
	{
      Core::LosObj att( iatt );
      Core::LosObj tgt( itgt );

	  if ( iatt.realm != itgt.realm )
		return false;
      if ( iatt.isa( Core::UObject::CLASS_CHARACTER ) )
	  {
		const Mobile::Character& chr = static_cast<const Mobile::Character&>( iatt );
		if ( chr.ignores_line_of_sight() )
		  return true;
	  }

	  return has_los( att, tgt );
	}

    bool Realm::has_los( const Mobile::Character& iatt, const Core::UObject& itgt ) const
	{
      Core::LosObj att( iatt );
      Core::LosObj tgt( itgt );

	  bool remote;
	  Items::Item* remote_container = iatt.search_remote_containers( itgt.serial, &remote );
	  if ( ( remote_container != NULL ) && remote )
		return true;
	  if ( iatt.realm != itgt.realm )
		return false;
	  if ( iatt.ignores_line_of_sight() )
		return true;

	  return has_los( att, tgt );
	}

  }
}