/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "charactr.h"
#include "los.h"
namespace Pol {
  namespace Core {
	bool uo_has_los( const UObject& iatt, const UObject& itgt )
	{
	  LosObj att( iatt );
	  LosObj tgt( itgt );

	  if ( iatt.isa( UObject::CLASS_CHARACTER ) )
	  {
		const Character& chr = static_cast<const Character&>( iatt );
		if ( chr.ignores_line_of_sight() )
		  return true;
	  }

	  return uo_has_los( att, tgt );
	}

	bool uo_has_los( const Character& iatt, const UObject& itgt )
	{
	  LosObj att( iatt );
	  LosObj tgt( itgt );

	  if ( iatt.ignores_line_of_sight() )
		return true;

	  return uo_has_los( att, tgt );
	}
  }
}
