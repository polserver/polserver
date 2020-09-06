#ifndef POLSERVER_COMPILEDSCRIPT_H
#define POLSERVER_COMPILEDSCRIPT_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript
{
class StoredToken;
}

namespace Pol::Bscript::Compiler
{
class ExportedFunction;
class SourceFileIdentifier;
class ModuleDescriptor;

using CodeSection = std::vector<StoredToken>;
using DataSection = std::vector<std::byte>;
using ExportedFunctions = std::vector<ExportedFunction>;
using ModuleDescriptors = std::vector<ModuleDescriptor>;
using SourceFileIdentifiers = std::vector<std::unique_ptr<SourceFileIdentifier>>;

class CompiledScript
{
public:
  struct ProgramInfo
  {
    const unsigned parameter_count;
  };

  CompiledScript( CodeSection code_section,
                  DataSection data_section,
                  ExportedFunctions exported_functions,
                  std::vector<std::string> global_variable_names,
                  ModuleDescriptors modules,
                  std::unique_ptr<ProgramInfo>,
                  SourceFileIdentifiers );
  ~CompiledScript();
  CompiledScript( const CompiledScript& ) = delete;
  CompiledScript& operator=( const CompiledScript& ) = delete;

  const CodeSection code;
  const DataSection data;
  const ExportedFunctions exported_functions;
  const std::vector<std::string> global_variable_names;
  const ModuleDescriptors module_descriptors;
  const std::unique_ptr<ProgramInfo> program_info;
  const SourceFileIdentifiers source_file_identifiers;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILEDSCRIPT_H
