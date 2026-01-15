
#include "ucfg.h"

#include <string.h>

#include "../../clib/stlutil.h"
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

ConfigurationBuffer::~ConfigurationBuffer() = default;

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

  usage.misc = Clib::memsize( oldcfgfiles );
  usage.misc +=
      Clib::memsize( file_access_rules, []( const auto& v ) { return v.estimateSize(); } );

  usage.cfg_count = cfgfiles.size();
  usage.cfg_size += Clib::memsize( cfgfiles );
  for ( const auto& pair : cfgfiles )
  {
    if ( pair.second.get() != nullptr )
      usage.cfg_size += pair.second->estimateSize();
  }

  usage.datastore_count = datastore.size();
  usage.datastore_size += Clib::memsize( datastore );
  for ( const auto& data : datastore )
  {
    if ( data.second != nullptr )
      usage.datastore_size += data.second->estimateSize();
  }
  return usage;
}
}  // namespace Core
}  // namespace Pol
