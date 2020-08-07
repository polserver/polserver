#include "CodeGenerator.h"

#include <memory>

#include "InstructionEmitter.h"
#include "InstructionGenerator.h"
#include "ModuleDeclarationRegistrar.h"
#include "StoredToken.h"
#include "compiler/ast/TopLevelStatements.h"
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

  ModuleDeclarationRegistrar module_declaration_registrar;

  InstructionEmitter instruction_emitter( code, data,// debug, exported_functions,
                                          module_declaration_registrar );
  CodeGenerator generator( instruction_emitter, module_declaration_registrar );

  generator.generate_instructions( *workspace );

  std::vector<ModuleDescriptor> module_descriptors =
      module_declaration_registrar.take_module_descriptors();

  return std::make_unique<CompiledScript>(
      std::move( code ), std::move( data ), std::move( exported_functions ),
      std::move( workspace->global_variable_names ), std::move( module_descriptors ),
      std::move( program_info ), std::move( workspace->referenced_source_file_identifiers ) );
}

CodeGenerator::CodeGenerator( InstructionEmitter& emitter,
    ModuleDeclarationRegistrar& module_declaration_registrar )
  : module_declaration_registrar( module_declaration_registrar ),
    emitter( emitter ),
    emit( emitter )
{
}

void CodeGenerator::generate_instructions( CompilerWorkspace& workspace )
{
  InstructionGenerator top_level_instruction_generator( emitter );
  workspace.top_level_statements->accept( top_level_instruction_generator );

  emit.progend();
}

}  // namespace Pol::Bscript::Compiler
