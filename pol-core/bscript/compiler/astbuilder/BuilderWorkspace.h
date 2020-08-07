#ifndef POLSERVER_BUILDERWORKSPACE_H
#define POLSERVER_BUILDERWORKSPACE_H

#include <map>
#include <memory>
#include <string>

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Profile;
class Report;
class SourceFile;
class SourceFileCache;
class SourceFileIdentifier;

class BuilderWorkspace
{
public:
  BuilderWorkspace( CompilerWorkspace&, SourceFileCache& em_cache, SourceFileCache& inc_cache,
                             Profile& profile, Report& report );
  ~BuilderWorkspace();

  CompilerWorkspace& compiler_workspace;

  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;

  Profile& profile;
  Report& report;

  std::map<std::string, std::shared_ptr<SourceFile>> source_files;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BUILDERWORKSPACE_H
