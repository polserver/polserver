/*
History
=======


Notes
=======

*/

#ifndef LOADDATA_H
#define LOADDATA_H

#include "poltype.h"
namespace Pol {
  namespace Mobile {
	class Character;
  }
  namespace Items {
	class Item;
  }
  namespace Core {
	extern unsigned incremental_save_count;
	extern unsigned current_incremental_save;

	void load_incremental_indexes();
	unsigned get_save_index( pol_serial_t serial );

	void read_incremental_saves();
	void slurp( const char* filename, const char* tags, int sysfind_flags = 0 );
	void register_deleted_serials();
	void clear_save_index();

	void defer_item_insertion( Items::Item* item, pol_serial_t container_serial );
	void insert_deferred_items();
	void equip_loaded_item( Mobile::Character* chr, Items::Item* item );
	void add_loaded_item( Items::Item* cont_item, Items::Item* item );
  }
}
#endif
