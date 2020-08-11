#include "CodeGenerator.h"

#include <memory>

#include "StoredToken.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/representation/CompiledScript.h"
#include "compiler/representation/ExportedFunction.h"
#include "compiler/representation/ModuleDescriptor.h"
#include "compiler/representation/ModuleFunctionDescriptor.h"

namespace Pol::Bscript::Compiler
{
std::unique_ptr<CompiledScript> CodeGenerator::generate(
    std::unique_ptr<CompilerWorkspace> workspace, const LegacyFunctionOrder* )
{
  auto program_info = std::unique_ptr<CompiledScript::ProgramInfo>();

  CodeSection code;
  DataSection data;

  ExportedFunctions exported_functions;
  std::vector<ModuleDescriptor> module_descriptors;

  CodeGenerator generator;

  generator.generate_instructions( *workspace );

  return std::make_unique<CompiledScript>(
      std::move( code ), std::move( data ), std::move( exported_functions ),
      std::move( workspace->global_variable_names ), std::move( module_descriptors ),
      std::move( program_info ), std::move( workspace->referenced_source_file_identifiers ) );
}

CodeGenerator::CodeGenerator()
{
}

void CodeGenerator::generate_instructions( CompilerWorkspace& )
{
}

}  // namespace Pol::Bscript::Compiler
