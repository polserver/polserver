#ifndef POLSERVER_INSTRUCTIONEMITTER_H
#define POLSERVER_INSTRUCTIONEMITTER_H

#include <memory>
#include <string>
#include <vector>

#ifndef OBJMEMBERS_H
#include "objmembers.h"
#endif
#ifndef OBJMETHODS_H
#include "objmethods.h"
#endif
#ifndef __TOKENS_H
#include "tokens.h"
#endif
#include "compiler/codegen/CodeEmitter.h"
#include "compiler/codegen/DataEmitter.h"
#include "compiler/representation/CompiledScript.h"

namespace Pol::Bscript
{
class StoredToken;
}

namespace Pol::Bscript::Compiler
{
class ModuleDeclarationRegistrar;
class ModuleFunctionDeclaration;
class Node;
class Variable;
class SourceLocation;

class InstructionEmitter
{
public:
  InstructionEmitter( CodeSection& code, DataSection& data,
                      ModuleDeclarationRegistrar& );

  void initialize_data();

  void access_variable( const Variable& );
  void array_declare();
  void assign();
  void call_method( const std::string& name, unsigned argument_count );
  void call_method_id( MethodID method_id, unsigned argument_count );
  void call_modulefunc( const ModuleFunctionDeclaration& );
  void consume();
  void declare_variable( const Variable& );
  void progend();
  void unary_operator( BTokenId );
  void value( double );
  void value( int );
  void value( const std::string& );

private:
  unsigned emit_data( const std::string& );
  unsigned emit_token( BTokenId id, BTokenType type, unsigned offset = 0 );
  unsigned append_token( StoredToken& );

  CodeEmitter code_emitter;
  DataEmitter data_emitter;
  ModuleDeclarationRegistrar& module_declaration_registrar;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INSTRUCTIONEMITTER_H
