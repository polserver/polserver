/*
History
=======


Notes
=======

*/

#ifndef SAVEDATA_H
#define SAVEDATA_H

#include "../clib/streamsaver.h"

class SaveContext : boost::noncopyable
{
	typedef ThreadedOFStreamWriter SaveStrategy;
	//typedef OFStreamWriter SaveStrategy;
private:
	std::ofstream _pol;
	std::ofstream _objects;
	std::ofstream _pcs;
	std::ofstream _pcequip;
	std::ofstream _npcs;
	std::ofstream _npcequip;
	std::ofstream _items;
	std::ofstream _multis;
	std::ofstream _storage;
	std::ofstream _resource;
	std::ofstream _guilds;
	std::ofstream _datastore;
	std::ofstream _party;
public:
	SaveContext();
	SaveStrategy pol;
	SaveStrategy objects;
	SaveStrategy pcs;
	SaveStrategy pcequip;
	SaveStrategy npcs;
	SaveStrategy npcequip;
	SaveStrategy items;
	SaveStrategy multis;
	SaveStrategy storage;
	SaveStrategy resource;
	SaveStrategy guilds;
	SaveStrategy datastore;
	SaveStrategy party;
};

int save_incremental(unsigned int& dirty_writes, unsigned int& clean_objects, long long& elapsed_ms);

void write_system_data( StreamWriter& sw );
void write_global_properties( StreamWriter& sw );
void write_shadow_realms( StreamWriter& sw );

bool commit( const std::string& basename );
void commit_incremental_saves();
extern bool incremental_saves_disabled;
bool should_write_data();

#endif
