/*
History
=======
2005/01/23 Shinigami: decay_items & decay_thread - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)

Notes
=======

*/


#include "clib/stl_inc.h"
#include "clib/esignal.h"
#include "plib/realm.h"

#include "core.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "gameclck.h"
#include "polclock.h"
#include "polsem.h"
#include "realms.h"
#include "scrsched.h"
#include "ufunc.h"
#include "uofile.h"
#include "uoscrobj.h"
#include "uworld.h"

#include "decay.h"

long cycles_per_decay_worldzone;
long cycles_until_decay_worldzone;

///
/// [1] Item Decay Criteria
///     An Item is allowed to decay if ALL of the following are true:
///        - it is not In Use
///        - it is Movable, OR it is a Corpse
///        - its 'decayat' member is nonzero 
///            AND the Game Clock has passed this 'decayat' time
///        - it is not supported by a multi,
///            OR its itemdesc.cfg entry specifies 'DecaysOnMultis 1'
///        - it itemdesc.cfg entry specifies no 'DestroyScript', 
///            OR its 'DestroyScript' returns nonzero.
///
/// [2] Decay Action
///     Container contents are moved to the ground at the Container's location
///     before destroying the container.
///

void decay_worldzone( unsigned wx, unsigned wy, Realm* realm )
{
    Zone& zone = realm->zone[wx][wy];
    gameclock_t now = read_gameclock();
 
    for( ZoneItems::size_type idx = 0; idx < zone.items.size(); ++idx )
    {
        Item* item = zone.items[idx];
        if (item->should_decay( now ))
        {
            const ItemDesc& descriptor = find_itemdesc( item->objtype_ );

	        UMulti* multi = realm->find_supporting_multi( item->x, item->y, item->z );
			// some things don't decay on multis:
            if (multi != NULL && !descriptor.decays_on_multis)
                continue;

            if (!descriptor.destroy_script.empty() && !item->inuse())
            {
                bool decayok = call_script( descriptor.destroy_script, item->make_ref());
                if (!decayok)
                    continue;
            }

            item->spill_contents( multi );
            destroy_item( item );
    		--idx;
        }
    }
}


// this is used in single-thread mode only
void decay_items()
{
	static unsigned wx = ~0u;
	static unsigned wy = 0;
	vector<Realm*>::iterator itr;
	Realm* realm;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		realm = *itr;
		if (!--cycles_until_decay_worldzone)
		{
			cycles_until_decay_worldzone = cycles_per_decay_worldzone;
      
      unsigned int gridwidth = realm->width() / WGRID_SIZE;
      unsigned int gridheight = realm->height() / WGRID_SIZE;
      
      // Tokuno-Fix
      if (gridwidth * WGRID_SIZE < realm->width())
        gridwidth++;
      if (gridheight * WGRID_SIZE < realm->height())
        gridheight++;
      
      if (++wx >= gridwidth )
			{
				wx = 0;
				if (++wy >= gridheight )
				{
					wy = 0;
				}
			}
			decay_worldzone( wx, wy, realm );
		}
	}
}

///
/// [3] Decay Sweep
///     Each 64x64 tile World Zone is checked for decay approximately
///     once every 10 minutes
///

void decay_single_zone(Realm* realm, unsigned gridx, unsigned gridy, unsigned& wx, unsigned& wy)
{
    if (++wx >= gridx)
    {
        wx = 0;
        if (++wy >= gridy)
        {
            wy = 0;
        }
    }
    decay_worldzone( wx, wy, realm );
}

void decay_thread( void* arg ) //Realm*
{
	unsigned wx = ~0u;
	unsigned wy = 0;
	Realm* realm = reinterpret_cast<Realm*>(arg);

	unsigned gridx = (realm->width() / WGRID_SIZE);
	unsigned gridy = (realm->height() / WGRID_SIZE);

	// Tokuno-Fix
	if (gridx * WGRID_SIZE < realm->width())
		gridx++;
	if (gridy * WGRID_SIZE < realm->height())
		gridy++;

	unsigned sleeptime = (60 * 10L * 1000) / (gridx * gridy);
	while (!exit_signalled)
	{
		{
			PolLock lck;
			polclock_checkin();
			decay_single_zone(realm, gridx, gridy, wx, wy);
			restart_all_clients();
		}
		// sweep entire world every 10 minutes
		// (60 * 10 * 1000) / (96 * 64) -> (600000 / 6144) -> 97 ms

		pol_sleep_ms( sleeptime );
	}
}

void decay_thread_shadow( void* arg ) //unsigned
{
	unsigned wx = ~0u;
	unsigned wy = 0;
	unsigned id = reinterpret_cast<unsigned>(arg);

	if (shadowrealms_by_id[id]==NULL)
		return;
	unsigned width = shadowrealms_by_id[id]->width();
	unsigned height = shadowrealms_by_id[id]->height();

	unsigned gridx = (width / WGRID_SIZE);
	unsigned gridy = (height / WGRID_SIZE);

	// Tokuno-Fix
	if (gridx * WGRID_SIZE < width)
		gridx++;
	if (gridy * WGRID_SIZE < height)
		gridy++;

	unsigned sleeptime = (60 * 10L * 1000) / (gridx * gridy);
	while (!exit_signalled)
	{
		{
			PolLock lck;
			polclock_checkin();
			if (shadowrealms_by_id[id]==NULL) // is realm still there?
				break;
			decay_single_zone(shadowrealms_by_id[id], gridx, gridy, wx, wy);
			restart_all_clients();
		}
		// sweep entire world every 10 minutes
		// (60 * 10 * 1000) / (96 * 64) -> (600000 / 6144) -> 97 ms

		pol_sleep_ms( sleeptime );
	}
}

