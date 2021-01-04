#include "DiskCache.h"

#include "clib/logfacility.h"

namespace Pol::Bscript::Compiler
{
void DiskCache::set( const std::string& pathname, const std::string& contents )
{
  const std::lock_guard<std::mutex> guard( mutex );
  auto itr = files.find( pathname );
  if ( itr != files.end() )
  {
    itr->second = std::make_shared<std::string>( contents );
  }
  else
  {
    files.insert( { pathname, std::make_shared<std::string>( contents ) } );
  }
}

std::shared_ptr<std::string> DiskCache::get( const std::string& pathname )
{
  const std::lock_guard<std::mutex> guard( mutex );

  auto itr = files.find( pathname );
  if ( itr != files.end() )
  {
    return itr->second;
  }
  return {};
}
}  // namespace Pol::Bscript::Compiler
