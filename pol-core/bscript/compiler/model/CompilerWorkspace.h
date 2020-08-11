#ifndef POLSERVER_COMPILERWORKSPACE_H
#define POLSERVER_COMPILERWORKSPACE_H

#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class SourceFileIdentifier;

class CompilerWorkspace
{
public:
  CompilerWorkspace();
  ~CompilerWorkspace();

  std::vector<std::unique_ptr<SourceFileIdentifier>> referenced_source_file_identifiers;

  std::vector<std::string> global_variable_names;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACE_H
