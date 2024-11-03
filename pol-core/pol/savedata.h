/** @file
 *
 * @par History
 */


#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <fstream>
#include <future>
#include <string>

#include "../clib/streamsaver.h"

namespace Pol
{
namespace Core
{
class SaveContext
{
  typedef Clib::StreamWriter SaveStrategy;

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
  ~SaveContext();
  SaveContext( const SaveContext& ) = delete;
  SaveContext& operator=( const SaveContext& ) = delete;
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
  static std::shared_future<bool> finished;
  static void ready();
};

void write_system_data( Clib::StreamWriter& sw );
void write_global_properties( Clib::StreamWriter& sw );
void write_shadow_realms( Clib::StreamWriter& sw );

bool commit( const std::string& basename );
bool should_write_data();
int write_data( unsigned int& dirty_writes, unsigned int& clean_writes, long long& elapsed_ms );
}  // namespace Core
}  // namespace Pol
#endif
