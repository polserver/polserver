#ifndef POLSERVER_COMPILEDSCRIPT_H
#define POLSERVER_COMPILEDSCRIPT_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bscript/compiler/representation/DebugStore.h"

namespace Pol::Bscript
{
class StoredToken;
}

namespace Pol::Bscript::Compiler
{
class ExportedFunction;
class FunctionReferenceDescriptor;
class SourceFileIdentifier;
class ModuleDescriptor;
class ClassDescriptor;

using CodeSection = std::vector<StoredToken>;
using DataSection = std::vector<std::byte>;
using ExportedFunctions = std::vector<ExportedFunction>;
using FunctionReferences = std::vector<FunctionReferenceDescriptor>;
using ModuleDescriptors = std::vector<ModuleDescriptor>;
using ClassDescriptors = std::vector<ClassDescriptor>;
using SourceFileIdentifiers = std::vector<std::unique_ptr<SourceFileIdentifier>>;

class CompiledScript
{
public:
  struct ProgramInfo
  {
    const unsigned parameter_count;
  };

  CompiledScript( CodeSection code_section, DataSection data_section, DebugStore debug,
                  ExportedFunctions exported_functions,
                  std::vector<std::string> global_variable_names, ModuleDescriptors modules,
                  FunctionReferences function_references, ClassDescriptors class_descriptors,
                  std::unique_ptr<ProgramInfo>, SourceFileIdentifiers );
  ~CompiledScript();
  CompiledScript( const CompiledScript& ) = delete;
  CompiledScript& operator=( const CompiledScript& ) = delete;

  const CodeSection code;
  const DataSection data;
  const DebugStore debug;
  const ExportedFunctions exported_functions;
  const std::vector<std::string> global_variable_names;
  const ModuleDescriptors module_descriptors;
  const FunctionReferences function_references;
  const ClassDescriptors class_descriptors;
  const std::unique_ptr<ProgramInfo> program_info;
  const SourceFileIdentifiers source_file_identifiers;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILEDSCRIPT_H
