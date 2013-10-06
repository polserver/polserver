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
	OFStreamWriter pol;
	OFStreamWriter objects;
	OFStreamWriter pcs;
	OFStreamWriter pcequip;
	OFStreamWriter npcs;
	OFStreamWriter npcequip;
	OFStreamWriter items;
	OFStreamWriter multis;
	OFStreamWriter storage;
	OFStreamWriter resource;
	OFStreamWriter guilds;
	OFStreamWriter datastore;
	OFStreamWriter party;
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
