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
class CaseJumpDataBlock;
class CompiledScript;
class FlowControlLabel;
class ModuleDeclarationRegistrar;
class ModuleFunctionDeclaration;
class Node;
class Variable;
class SourceLocation;

class InstructionEmitter
{
public:
  InstructionEmitter( CodeSection& code, DataSection& data, ExportedFunctions& exported_functions,
                      ModuleDeclarationRegistrar& );

  void initialize_data();

  void register_exported_function( FlowControlLabel& label, const std::string& name,
                                   unsigned arguments );

  void access_variable( const Variable& );
  void array_append();
  void array_create();
  void array_declare();
  void assign();
  void assign_subscript_consume();
  void assign_subscript();
  void assign_multisubscript( unsigned indexes );
  void assign_variable( const Variable& );
  void basic_for_init( FlowControlLabel& );
  void basic_for_next( FlowControlLabel& );
  void binary_operator( BTokenId token_id );
  void call_method( const std::string& name, unsigned argument_count );
  void call_method_id( MethodID method_id, unsigned argument_count );
  void call_modulefunc( const ModuleFunctionDeclaration& );
  void call_userfunc( FlowControlLabel& );
  unsigned casejmp();
  unsigned case_dispatch_table( const CaseJumpDataBlock& );
  void consume();
  void declare_variable( const Variable& );
  void dictionary_create();
  void dictionary_add_member();
  void error_create();
  void exit();
  void foreach_init( FlowControlLabel& );
  void foreach_step( FlowControlLabel& );
  void function_reference( unsigned parameter_count, FlowControlLabel& );
  void get_arg( const std::string& name );
  void get_member( const std::string& name );
  void get_member_id( MemberID );
  void jmp_always( FlowControlLabel& );
  void jmp_if_false( FlowControlLabel& );
  void jmp_if_true( FlowControlLabel& );
  void label( FlowControlLabel& );
  void leaveblock( unsigned local_vars_to_remove );
  void makelocal();
  void pop_param( const std::string& name );
  void pop_param_byref( const std::string& name );
  void progend();
  void return_from_user_function();
  unsigned skip_if_true_else_consume();
  void set_member( const std::string& name );
  void set_member_id( MemberID member_id );
  void set_member_id_consume( MemberID member_id );
  void set_member_consume( const std::string& name );
  void set_member_by_operator( BTokenId, MemberID );
  void struct_create();
  void struct_add_uninit_member( const std::string& name );
  void struct_add_member( const std::string& name );
  void subscript_single();
  void subscript_multiple( int indexes );
  void unary_operator( BTokenId );
  void uninit();
  void value( double );
  void value( int );
  void value( const std::string& );

  unsigned next_instruction_address();
  void patch_offset( unsigned index, unsigned offset );

private:
  unsigned emit_data( const std::string& );
  unsigned emit_token( BTokenId id, BTokenType type, unsigned offset = 0 );
  unsigned append_token( StoredToken& );
  void register_with_label( FlowControlLabel&, unsigned offset );

  CodeEmitter code_emitter;
  DataEmitter data_emitter;
  ExportedFunctions& exported_functions;
  ModuleDeclarationRegistrar& module_declaration_registrar;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_INSTRUCTIONEMITTER_H
