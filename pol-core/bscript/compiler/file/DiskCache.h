#ifndef POLSERVER_DISKCACHE_H
#define POLSERVER_DISKCACHE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Pol::Bscript::Compiler
{
class DiskCache
{
public:
  void set( const std::string&, const std::string& );
  std::shared_ptr<std::string> get( const std::string& );

private:
  std::mutex mutex;
  std::map<std::string, std::shared_ptr<std::string>> files;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILECACHE_H
