
#include "ucfg.h"

#include <string.h>

#include "../cfgrepos.h"
#include "../module/datastoreimp.h"

namespace Pol
{
namespace Core
{
ConfigurationBuffer configurationbuffer;

ConfigurationBuffer::ConfigurationBuffer()
    : cfgfiles(), oldcfgfiles(), datastore(), file_access_rules()
{
}

ConfigurationBuffer::~ConfigurationBuffer() {}

void ConfigurationBuffer::deinitialize()
{
  // ToDo: we have to think over... it's a problem with script-inside references
  oldcfgfiles.clear();
  cfgfiles.clear();

  for ( Core::DataStore::iterator itr = datastore.begin(); itr != datastore.end(); ++itr )
  {
    Module::DataStoreFile* dsf = ( *itr ).second;
    delete dsf;
  }
  datastore.clear();
  file_access_rules.clear();
}

ConfigurationBuffer::Memory ConfigurationBuffer::estimateSize() const
{
  Memory usage;
  memset( &usage, 0, sizeof( usage ) );

  usage.misc = 3 * sizeof( std::string* );
  for ( const auto& oldcfg : oldcfgfiles )
    usage.misc += oldcfg.capacity();
  usage.misc += 3 * sizeof( Module::FileAccess* );
  for ( const auto& rule : file_access_rules )
    usage.misc += rule.estimateSize();


  usage.cfg_count = cfgfiles.size();
  for ( const auto& pair : cfgfiles )
  {
    size_t cfgsize = 0;
    if ( pair.second.get() != nullptr )
      cfgsize += pair.second->estimateSize();
    usage.cfg_size += ( pair.first.capacity() + cfgsize ) + ( sizeof( void* ) * 3 + 1 ) / 2;
  }

  usage.datastore_count = datastore.size();
  for ( const auto& data : datastore )
  {
    usage.datastore_size += ( data.first.capacity() + sizeof( Module::DataStoreFile* ) +
                              ( sizeof( void* ) * 3 + 1 ) / 2 );
    if ( data.second != nullptr )
      usage.datastore_size += data.second->estimateSize();
  }
  return usage;
}
}
}
