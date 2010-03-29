/*
History
=======


Notes
=======

*/

#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "../clib/wallclock.h"

int save_incremental(unsigned int& dirty_writes, unsigned int& clean_objects, wallclock_diff_t& elapsed_ms);

void write_system_data( ostream& ofs );
void write_global_properties( ostream& ofs );
void write_shadow_realms( ostream& ofs );

bool commit( const string& basename );
void commit_incremental_saves();
extern bool incremental_saves_disabled;
bool should_write_data();

#endif
