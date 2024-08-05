#include "CodeGenerator.h"

#include <memory>

#include "StoredToken.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/ProgramParameterList.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/codegen/FunctionReferenceRegistrar.h"
#include "bscript/compiler/codegen/InstructionEmitter.h"
#include "bscript/compiler/codegen/InstructionGenerator.h"
#include "bscript/compiler/codegen/ModuleDeclarationRegistrar.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FlowControlLabel.h"
#include "bscript/compiler/representation/CompiledScript.h"
#include "bscript/compiler/representation/ExportedFunction.h"
#include "bscript/compiler/representation/FunctionReferenceDescriptor.h"
#include "bscript/compiler/representation/ModuleDescriptor.h"
#include "bscript/compiler/representation/ModuleFunctionDescriptor.h"

namespace Pol::Bscript::Compiler
{
std::unique_ptr<CompiledScript> CodeGenerator::generate(
    std::unique_ptr<CompilerWorkspace> workspace )
{
  auto program_info =
      workspace->program
          ? std::unique_ptr<CompiledScript::ProgramInfo>( new CompiledScript::ProgramInfo{
                static_cast<unsigned>( workspace->program->parameter_list().children.size() ) } )
          : std::unique_ptr<CompiledScript::ProgramInfo>();

  CodeSection code;
  DataSection data;

  std::vector<std::string> debug_filenames;
  // For parity with OG compiler, debug file #0 is always empty.
  debug_filenames.emplace_back( "" );
  for ( auto& ident : workspace->referenced_source_file_identifiers )
  {
    debug_filenames.push_back( ident->pathname );
  }
  DebugStore debug( std::move( debug_filenames ) );

  ExportedFunctions exported_functions;

  ModuleDeclarationRegistrar module_declaration_registrar;
  FunctionReferenceRegistrar function_reference_registrar;

  InstructionEmitter instruction_emitter( code, data, debug, exported_functions,
                                          module_declaration_registrar,
                                          function_reference_registrar );
  CodeGenerator generator( instruction_emitter, module_declaration_registrar );

  generator.register_module_functions_alphabetically( *workspace );

  sort_user_functions_alphabetically( *workspace );

  generator.generate_instructions( *workspace );

  std::vector<ModuleDescriptor> module_descriptors =
      module_declaration_registrar.take_module_descriptors();

  std::vector<FunctionReferenceDescriptor> function_references =
      function_reference_registrar.take_descriptors();

  return std::make_unique<CompiledScript>(
      std::move( code ), std::move( data ), std::move( debug ), std::move( exported_functions ),
      std::move( workspace->global_variable_names ), std::move( module_descriptors ),
      std::move( function_references ), std::move( program_info ),
      std::move( workspace->referenced_source_file_identifiers ) );
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
  emitter.debug_statementbegin();
  emitter.debug_file_line( 0, 1 );

  std::map<std::string, FlowControlLabel> user_function_labels;
  InstructionGenerator generator( emitter, user_function_labels );

  workspace.top_level_statements->accept( generator );

  if ( auto& program = workspace.program )
  {
    program->accept( generator );
  }

  emit.progend();

  for ( auto& user_function : workspace.user_functions )
  {
    // Function expressions are handled inside InstructionGenerator visit_function_expression
    if ( !user_function->expression )
    {
      user_function->accept( generator );
    }
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

void CodeGenerator::sort_user_functions_alphabetically( CompilerWorkspace& workspace )
{
  auto sortByName = []( const std::unique_ptr<UserFunction>& s1,
                        const std::unique_ptr<UserFunction>& s2 ) -> bool {
    return s1->name < s2->name;
  };

  std::sort( workspace.user_functions.begin(), workspace.user_functions.end(), sortByName );
}

}  // namespace Pol::Bscript::Compiler
