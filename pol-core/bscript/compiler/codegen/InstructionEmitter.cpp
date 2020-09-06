#include "InstructionEmitter.h"

#include "StoredToken.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/codegen/CaseJumpDataBlock.h"
#include "compiler/codegen/ModuleDeclarationRegistrar.h"
#include "compiler/model/FlowControlLabel.h"
#include "compiler/model/Variable.h"
#include "compiler/representation/CompiledScript.h"
#include "compiler/representation/ExportedFunction.h"
#include "escriptv.h"
#include "modules.h"
#include "token.h"
#include "tokens.h"

namespace Pol::Bscript::Compiler
{
InstructionEmitter::InstructionEmitter( CodeSection& code, DataSection& data,
                                        ModuleDeclarationRegistrar& module_declaration_registrar )
  : code_emitter( code ),
    data_emitter( data ),
    module_declaration_registrar( module_declaration_registrar )
{
  initialize_data();
}

void InstructionEmitter::initialize_data()
{
  std::byte nul{};
  data_emitter.store( &nul, sizeof nul );
}

void InstructionEmitter::access_variable( const Variable& v )
{
  BTokenId token_id = v.scope == VariableScope::Global ? TOK_GLOBALVAR : TOK_LOCALVAR;
  emit_token( token_id, TYP_OPERAND, v.index );
}

void InstructionEmitter::array_append()
{
  emit_token( TOK_INSERTINTO, TYP_OPERATOR );
}

void InstructionEmitter::array_create()
{
  emit_token( TOK_ARRAY, TYP_OPERAND );
}

void InstructionEmitter::array_declare()
{
  emit_token( INS_DECLARE_ARRAY, TYP_RESERVED );
}

void InstructionEmitter::assign()
{
  emit_token( TOK_ASSIGN, TYP_OPERATOR );
}

void InstructionEmitter::assign_subscript_consume()
{
  unsigned indexes = 1;
  emit_token( INS_SUBSCRIPT_ASSIGN_CONSUME, TYP_UNARY_OPERATOR, indexes );
}

void InstructionEmitter::assign_suscript()
{
  unsigned indexes = 1;
  emit_token( INS_SUBSCRIPT_ASSIGN, TYP_UNARY_OPERATOR, indexes );
}

void InstructionEmitter::assign_multisubscript( unsigned indexes )
{
  emit_token( INS_MULTISUBSCRIPT_ASSIGN, TYP_UNARY_OPERATOR, indexes );
}

void InstructionEmitter::assign_variable( const Variable& v )
{
  auto token_id = v.scope == VariableScope::Global ? INS_ASSIGN_GLOBALVAR : INS_ASSIGN_LOCALVAR;
  emit_token( token_id, TYP_UNARY_OPERATOR, v.index );
}

void InstructionEmitter::binary_operator( BTokenId token_id )
{
  emit_token( token_id, TYP_OPERATOR );
}

void InstructionEmitter::call_method_id( MethodID method_id, unsigned argument_count )
{
  emit_token( INS_CALL_METHOD_ID, (BTokenType)argument_count, method_id );
}

void InstructionEmitter::call_method( const std::string& name, unsigned argument_count )
{
  unsigned offset = emit_data( name );
  emit_token( INS_CALL_METHOD, (BTokenType)argument_count, offset );
}

void InstructionEmitter::call_modulefunc(
    const ModuleFunctionDeclaration& module_function_declaration )
{
  unsigned module_id, function_index;
  module_declaration_registrar.lookup_or_register_module_function( module_function_declaration,
                                                                   module_id, function_index );
  unsigned sympos = include_debug ? emit_data( module_function_declaration.name ) : 0;
  StoredToken token(
      module_id, TOK_FUNC,
      static_cast<BTokenType>(
          function_index ),  // function index, stored in Token.lval, saved in StoredToken.type
      sympos );
  append_token( token );
}

void InstructionEmitter::call_userfunc( FlowControlLabel& label )
{
  unsigned addr = emit_token( CTRL_JSR_USERFUNC, TYP_CONTROL );
  register_with_label( label, addr );
}

unsigned InstructionEmitter::casejmp()
{
  return emit_token( INS_CASEJMP, TYP_RESERVED );
}

unsigned InstructionEmitter::case_dispatch_table( const CaseJumpDataBlock& dispatch_table )
{
  auto& bytes = dispatch_table.get_data();
  return data_emitter.append( bytes.data(), bytes.size() );
}

void InstructionEmitter::consume()
{
  emit_token( TOK_CONSUMER, TYP_UNARY_OPERATOR );
}

void InstructionEmitter::declare_variable( const Variable& v )
{
  BTokenId token_id = v.scope == VariableScope::Global ? RSV_GLOBAL : RSV_LOCAL;
  emit_token( token_id, TYP_RESERVED, v.index );
}

void InstructionEmitter::dictionary_create()
{
  emit_token( TOK_DICTIONARY, TYP_OPERAND );
}

void InstructionEmitter::dictionary_add_member()
{
  emit_token( INS_DICTIONARY_ADDMEMBER, TYP_OPERATOR );
}

void InstructionEmitter::exit()
{
  emit_token( RSV_EXIT, TYP_RESERVED );
}

void InstructionEmitter::foreach_init( FlowControlLabel& label )
{
  register_with_label( label, emit_token( INS_INITFOREACH, TYP_RESERVED ) );
}

void InstructionEmitter::foreach_step( FlowControlLabel& label )
{
  register_with_label( label, emit_token( INS_STEPFOREACH, TYP_RESERVED ) );
}

void InstructionEmitter::get_arg( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_GET_ARG, TYP_OPERATOR, offset );
}

void InstructionEmitter::jmp_always( FlowControlLabel& label )
{
  register_with_label( label, emit_token( RSV_GOTO, TYP_RESERVED ) );
}

void InstructionEmitter::jmp_if_false( FlowControlLabel& label )
{
  register_with_label( label, emit_token( RSV_JMPIFFALSE, TYP_RESERVED ) );
}

void InstructionEmitter::jmp_if_true( FlowControlLabel& label )
{
  register_with_label( label, emit_token( RSV_JMPIFTRUE, TYP_RESERVED ) );
}

void InstructionEmitter::label( FlowControlLabel& label )
{
  label.assign_address( code_emitter.next_address() );

  for ( auto referencing_address : label.get_referencing_instruction_addresses() )
  {
    patch_offset( referencing_address, label.address() );
  }
}

void InstructionEmitter::leaveblock( unsigned local_vars_to_remove )
{
  emit_token( CTRL_LEAVE_BLOCK, TYP_CONTROL, local_vars_to_remove );
}

void InstructionEmitter::makelocal()
{
  emit_token( CTRL_MAKELOCAL, TYP_CONTROL );
}

void InstructionEmitter::pop_param( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_POP_PARAM, TYP_OPERATOR, offset );
}

void InstructionEmitter::pop_param_byref( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_POP_PARAM_BYREF, TYP_OPERATOR, offset );
}

void InstructionEmitter::progend()
{
  emit_token( CTRL_PROGEND, TYP_CONTROL );
}

void InstructionEmitter::return_from_user_function()
{
  emit_token( RSV_RETURN, TYP_RESERVED );
}

void InstructionEmitter::subscript_single()
{
  emit_token( TOK_ARRAY_SUBSCRIPT, TYP_OPERATOR, 1 );
}

void InstructionEmitter::subscript_multiple( int indexes )
{
  emit_token( INS_MULTISUBSCRIPT, TYP_OPERATOR, indexes );
}

void InstructionEmitter::unary_operator( BTokenId token_id )
{
  emit_token( token_id, TYP_UNARY_OPERATOR );
}

void InstructionEmitter::uninit()
{
  emit_token( INS_UNINIT, TYP_OPERAND );
}

void InstructionEmitter::value( double v )
{
  unsigned offset = data_emitter.append( v );
  emit_token( TOK_DOUBLE, TYP_OPERAND, offset );
}

void InstructionEmitter::value( int v )
{
  unsigned offset = data_emitter.append( v );
  emit_token( TOK_LONG, TYP_OPERAND, offset );
}

void InstructionEmitter::value( const std::string& v )
{
  unsigned data_offset = emit_data( v );
  emit_token( TOK_STRING, TYP_OPERAND, data_offset );
}

unsigned InstructionEmitter::emit_data( const std::string& s )
{
  return data_emitter.store( s );
}

unsigned InstructionEmitter::emit_token( BTokenId id, BTokenType type, unsigned offset )
{
  StoredToken token( Mod_Basic, id, type, offset );
  return append_token( token );
}

unsigned InstructionEmitter::append_token( StoredToken& token )
{
  return code_emitter.append( token );
}

void InstructionEmitter::patch_offset( unsigned index, unsigned offset )
{
  code_emitter.update_offset( index, offset );
}

void InstructionEmitter::register_with_label( FlowControlLabel& label, unsigned offset )
{
  if ( label.has_address() )
  {
    patch_offset( offset, label.address() );
  }
  else
  {
    label.add_referencing_instruction_address( offset );
  }
}

}  // namespace Pol::Bscript::Compiler
