#ifndef POLSERVER_COMPILERWORKSPACE_H
#define POLSERVER_COMPILERWORKSPACE_H

#include <memory>
#include <vector>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class SourceFileIdentifier;

class CompilerWorkspace
{
public:
  CompilerWorkspace();
  ~CompilerWorkspace();

  std::vector<std::unique_ptr<SourceFileIdentifier>> referenced_source_file_identifiers;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_COMPILERWORKSPACE_H
