#ifndef POLSERVER_CODEGENERATOR_H
#define POLSERVER_CODEGENERATOR_H

#include <memory>

namespace Pol::Bscript::Compiler
{
class CompiledScript;
class CompilerWorkspace;
class InstructionEmitter;
class ModuleDeclarationRegistrar;

class CodeGenerator
{
public:
  static std::unique_ptr<CompiledScript> generate( std::unique_ptr<CompilerWorkspace> );

private:
  CodeGenerator( InstructionEmitter& , ModuleDeclarationRegistrar& );

  void generate_instructions( CompilerWorkspace& );

  void register_module_functions_alphabetically( CompilerWorkspace& );

  static void sort_module_functions_by_module_name( CompilerWorkspace& );
  static void sort_module_functions_alphabetically( CompilerWorkspace& );

  static void sort_user_functions_alphabetically( CompilerWorkspace& );

private:
  ModuleDeclarationRegistrar& module_declaration_registrar;

  // Verb vs noun - see InstructionGenerator for reasoning
  InstructionEmitter& emitter;
  InstructionEmitter& emit;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CODEGENERATOR_H
