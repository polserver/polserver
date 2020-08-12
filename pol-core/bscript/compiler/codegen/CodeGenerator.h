#ifndef POLSERVER_CODEGENERATOR_H
#define POLSERVER_CODEGENERATOR_H

#include <memory>

namespace Pol::Bscript::Compiler
{
class CompiledScript;
class InstructionEmitter;
struct LegacyFunctionOrder;
class CompilerWorkspace;

class CodeGenerator
{
public:
  static std::unique_ptr<CompiledScript> generate( std::unique_ptr<CompilerWorkspace>,
                                                   const LegacyFunctionOrder* );

private:
  explicit CodeGenerator( InstructionEmitter& );

  void generate_instructions( CompilerWorkspace& );

private:
  // Verb vs noun - see InstructionGenerator for reasoning
  InstructionEmitter& emitter;
  InstructionEmitter& emit;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CODEGENERATOR_H
