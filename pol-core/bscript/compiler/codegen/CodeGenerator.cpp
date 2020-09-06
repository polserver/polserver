#include "CodeGenerator.h"

#include <memory>

#include "StoredToken.h"
#include "compiler/LegacyFunctionOrder.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/ProgramParameterList.h"
#include "compiler/ast/TopLevelStatements.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/codegen/InstructionEmitter.h"
#include "compiler/codegen/InstructionGenerator.h"
#include "compiler/codegen/ModuleDeclarationRegistrar.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/CompilerWorkspace.h"
#include "compiler/model/FlowControlLabel.h"
#include "compiler/representation/CompiledScript.h"
#include "compiler/representation/ExportedFunction.h"
#include "compiler/representation/ModuleDescriptor.h"
#include "compiler/representation/ModuleFunctionDescriptor.h"

namespace Pol::Bscript::Compiler
{
std::unique_ptr<CompiledScript> CodeGenerator::generate(
    std::unique_ptr<CompilerWorkspace> workspace, const LegacyFunctionOrder* legacy_function_order )
{
  auto program_info =
      workspace->program
          ? std::unique_ptr<CompiledScript::ProgramInfo>( new CompiledScript::ProgramInfo{
                static_cast<unsigned>( workspace->program->parameter_list().children.size() ) } )
          : std::unique_ptr<CompiledScript::ProgramInfo>();

  CodeSection code;
  DataSection data;

  ExportedFunctions exported_functions;

  ModuleDeclarationRegistrar module_declaration_registrar;

  InstructionEmitter instruction_emitter( code, data,
                                          module_declaration_registrar );
  CodeGenerator generator( instruction_emitter, module_declaration_registrar );

  generator.register_module_functions( *workspace, legacy_function_order );

  sort_user_functions( *workspace, legacy_function_order );

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
  std::map<std::string, FlowControlLabel> user_function_labels;
  InstructionGenerator outer_instruction_generator( emitter, user_function_labels, false );
  InstructionGenerator function_instruction_generator( emitter, user_function_labels, true );

  workspace.top_level_statements->accept( outer_instruction_generator );

  if ( auto& program = workspace.program )
  {
    program->accept( outer_instruction_generator );
  }

  emit.progend();

  for ( auto& user_function : workspace.user_functions )
  {
    user_function->accept( function_instruction_generator );
  }
}

void CodeGenerator::register_module_functions( CompilerWorkspace& workspace,
                                               const LegacyFunctionOrder* legacy_function_order )
{
  if ( legacy_function_order )
  {
    register_module_functions_as_legacy( workspace );
  }
  else
  {
    register_module_functions_alphabetically( workspace );
  }
}

void CodeGenerator::register_module_functions_as_legacy( CompilerWorkspace& workspace )
{
  for ( auto& module_function : workspace.module_function_declarations )
  {
    // we might assign IDs to more modules that we actually use,
    // but this will help us compare output with the original compiler.
    module_declaration_registrar.register_module( *module_function );
  }
  for ( const auto& decl : workspace.module_functions_in_legacy_order )
  {
    module_declaration_registrar.register_modulefunc( *decl );
  }
}

void CodeGenerator::register_module_functions_alphabetically( CompilerWorkspace& workspace )
{
  sort_module_functions_by_module_name( workspace );
  for ( auto& module_function : workspace.referenced_module_function_declarations )
  {
    module_declaration_registrar.register_module( *module_function );
  }

  sort_module_functions_alphabetically( workspace );
  for ( const auto& decl : workspace.referenced_module_function_declarations )
  {
    module_declaration_registrar.register_modulefunc( *decl );
  }
}

void CodeGenerator::sort_module_functions_by_module_name( CompilerWorkspace& workspace )
{
  auto sortByModuleName = []( ModuleFunctionDeclaration* d1,
                              ModuleFunctionDeclaration* d2 ) -> bool {
    return d1->module_name < d2->module_name;
  };

  std::sort( workspace.referenced_module_function_declarations.begin(),
             workspace.referenced_module_function_declarations.end(), sortByModuleName );
}

void CodeGenerator::sort_module_functions_alphabetically( CompilerWorkspace& workspace )
{
  auto sortByModuleFunctionName = []( ModuleFunctionDeclaration* d1,
                                      ModuleFunctionDeclaration* d2 ) -> bool {
    return d1->name < d2->name;
  };

  std::sort( workspace.referenced_module_function_declarations.begin(),
             workspace.referenced_module_function_declarations.end(), sortByModuleFunctionName );
}

void CodeGenerator::sort_user_functions( CompilerWorkspace& workspace,
                                         const LegacyFunctionOrder* legacy_function_order )
{
  if ( legacy_function_order )
  {
    sort_user_functions_as_legacy( workspace, *legacy_function_order );
  }
  else
  {
    sort_user_functions_alphabetically( workspace );
  }
}

void CodeGenerator::sort_user_functions_as_legacy(
    CompilerWorkspace& workspace, const LegacyFunctionOrder& legacy_function_order )
{
  std::map<std::string, size_t> c1_order;
  for ( auto& n : legacy_function_order.userfunc_emit_order )
  {
    c1_order[n] = c1_order.size();
  }

  auto sortAsLegacy = [c1_order]( const std::unique_ptr<UserFunction>& s1,
                                  const std::unique_ptr<UserFunction>& s2 ) -> bool {
    auto itr1 = c1_order.find( s1->name );
    auto itr2 = c1_order.find( s2->name );
    if ( itr1 != c1_order.end() && itr1 != c1_order.end() )
    {
      return ( *itr1 ).second < ( *itr2 ).second;
    }
    else
    {
      return s1->name < s2->name;
    }
  };
  std::sort( workspace.user_functions.begin(), workspace.user_functions.end(), sortAsLegacy );
}

void CodeGenerator::sort_user_functions_alphabetically( CompilerWorkspace& workspace )
{
  auto sortByName = []( const std::unique_ptr<UserFunction>& s1,
                        const std::unique_ptr<UserFunction>& s2 ) -> bool {
    return s1->name < s2->name;
  };

  std::sort( workspace.user_functions.begin(), workspace.user_functions.end(), sortByName );
}

}  // namespace Pol::Bscript::Compiler
