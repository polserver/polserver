/** @file
 *
 * @par History
 * - 2006/12/05 Shinigami: removed dummy floor creation in MultiDef::readobjects and MultiDef::readshapes
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "multidef.h"

#include "../item/itemdesc.h"

#include "../tiles.h"
#include "../objtype.h"
#include "../ustruct.h"

#include "../../plib/mapcell.h"
#include "../../plib/mapshape.h"
#include "../../plib/systemstate.h"

namespace Pol {
  namespace Multi {
	// 8/9/03 this seems to be used only by uofile03 -Syz
	bool MultiDef::readobjects( Core::StaticList& vec, short x, short y, short zbase ) const
	{
	  bool result = false;
	  if ( x >= minrx && x <= maxrx && y >= minry && y <= maxry )
	  {
		Components::const_iterator itr, end;
		if ( findcomponents( itr, end, x, y ) )
		{
		  for ( ; itr != end; ++itr )
		  {
			const MULTI_ELEM* elem = ( *itr ).second;
			unsigned short graphic = Items::getgraphic( elem->objtype );
			if ( Core::tile_flags( graphic ) & Plib::FLAG::WALKBLOCK )
			{
			  if ( elem->is_static )
			  {
				vec.push_back( Core::StaticRec( graphic, static_cast<signed char>( elem->z + zbase ) ) );
				result = true;
			  }
			  // Shinigami: removed. doesn't make sense. non-static
			  //            items are normal items an can be removed etc.
			  /* else // put a dummy floor tile there
			   {
			   vec.push_back( StaticRec( 0x495, elem->z+zbase ) );
			   result = true;
			   } */
			}
		  }
		}
	  }
	  return result;
	}

	bool MultiDef::readshapes( Plib::MapShapeList& vec, short x, short y, short zbase, unsigned int anyflags ) const
	{
	  bool result = false;
	  if ( x >= minrx && x <= maxrx && y >= minry && y <= maxry )
	  {
		Components::const_iterator itr, end;
		if ( findcomponents( itr, end, x, y ) )
		{
		  for ( ; itr != end; ++itr )
		  {
			const MULTI_ELEM* elem = ( *itr ).second;
			unsigned short graphic = Items::getgraphic( elem->objtype );
			if ( Core::tile_flags( graphic ) & anyflags )
			{
			  if ( elem->is_static )
			  {
				Plib::MapShape shape;
				shape.z = elem->z + zbase;
				shape.height = Core::tileheight( graphic );
				shape.flags = Plib::systemstate.tile[graphic].flags; // pol_flags_by_tile( graphic );
				if ( !shape.height )
				{
				  ++shape.height;
				  --shape.z;
				}
				vec.push_back( shape );
				result = true;
			  }
			  // Shinigami: removed. doesn't make sense. non-static
			  //            items are normal items an can be removed etc.
			  // Turley: BOAT added so hold count as boat item (who.multi) (and walkable)
			  else if ( type == BOAT ) // put a dummy floor there
			  {
				Plib::MapShape shape;
				shape.z = elem->z + zbase - 1;
				shape.height = 1;
                shape.flags = Plib::FLAG::MOVELAND |
                  Plib::FLAG::ALLOWDROPON |
                  Plib::FLAG::BLOCKSIGHT |
                  Plib::FLAG::OVERFLIGHT;
				vec.push_back( shape );
				result = true;
			  }
			}
		  }
		}
	  }
	  return result;
	}

  }
}