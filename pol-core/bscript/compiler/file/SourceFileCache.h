#ifndef POLSERVER_SOURCEFILECACHE_H
#define POLSERVER_SOURCEFILECACHE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Pol::Bscript::Compiler
{
class SourceFile;
class SourceFileIdentifier;
class SourceFileLoader;
class Profile;
class Report;

class SourceFileCache
{
public:
  explicit SourceFileCache( SourceFileLoader& source_loader, Profile& profile )
      : source_loader( source_loader ), keep( 0 ), profile( profile )
  {
  }

  void configure( unsigned keep );

  std::shared_ptr<SourceFile> load( const SourceFileIdentifier&, Report& report );

  void keep_some();
  void clear();

private:
  SourceFileLoader& source_loader;
  unsigned keep;

  std::mutex mutex;
  std::map<std::string, std::shared_ptr<SourceFile>> files;
  std::map<std::string, unsigned> frequency;
  Profile& profile;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILECACHE_H
