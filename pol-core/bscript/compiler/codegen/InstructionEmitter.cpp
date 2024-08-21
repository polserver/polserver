#include "InstructionEmitter.h"

#include <list>
#include <set>

#include "StoredToken.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/codegen/CaseJumpDataBlock.h"
#include "bscript/compiler/codegen/ClassDeclarationRegistrar.h"
#include "bscript/compiler/codegen/FunctionReferenceRegistrar.h"
#include "bscript/compiler/codegen/ModuleDeclarationRegistrar.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/FlowControlLabel.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"
#include "bscript/compiler/model/ScopableName.h"
#include "bscript/compiler/model/Variable.h"
#include "bscript/compiler/representation/ClassDescriptor.h"
#include "bscript/compiler/representation/CompiledScript.h"
#include "bscript/compiler/representation/ExportedFunction.h"
#include "bscript/compiler/representation/MethodDescriptor.h"
#include "escriptv.h"
#include "modules.h"
#include "token.h"
#include "tokens.h"

namespace Pol::Bscript::Compiler
{
InstructionEmitter::InstructionEmitter( CodeSection& code, DataSection& data, DebugStore& debug,
                                        ExportedFunctions& exported_functions,
                                        ModuleDeclarationRegistrar& module_declaration_registrar,
                                        FunctionReferenceRegistrar& function_reference_registrar,
                                        ClassDeclarationRegistrar& class_declaration_registrar,
                                        Report& report )
    : code_emitter( code ),
      data_emitter( data ),
      debug( debug ),
      exported_functions( exported_functions ),
      module_declaration_registrar( module_declaration_registrar ),
      function_reference_registrar( function_reference_registrar ),
      class_declaration_registrar( class_declaration_registrar ),
      report( report )
{
  initialize_data();
}

void InstructionEmitter::initialize_data()
{
  std::byte nul{};
  data_emitter.store( &nul, sizeof nul );
}

void InstructionEmitter::register_exported_function( FlowControlLabel& label,
                                                     const std::string& name, unsigned parameters )
{
  exported_functions.emplace_back( name, parameters, label.address() );
}

void InstructionEmitter::register_class_declaration(
    const ClassDeclaration& node, std::map<std::string, FlowControlLabel>& user_function_labels )
{
  std::set<std::string> visited;
  std::set<std::string, Clib::ci_cmp_pred> visited_methods;
  std::vector<unsigned> constructor_addresses;
  std::set<std::string> method_names;
  std::vector<MethodDescriptor> method_descriptors;
  std::list<const ClassDeclaration*> to_link( { &node } );

  const auto& class_name = node.name;
  auto class_name_offset = this->emit_data( class_name );

  report.debug( node, "Registering class: {}", node.name );
  for ( auto itr = to_link.begin(); itr != to_link.end(); ++itr )
  {
    auto cd = *itr;
    if ( visited.find( cd->name ) != visited.end() )
      continue;

    visited.insert( cd->name );
    report.debug( *cd, "Class {} with {} methods", cd->name, cd->methods.size() );

    if ( cd->constructor_link )
    {
      if ( auto uf = cd->constructor_link->user_function() )
      {
        auto ctor_itr = user_function_labels.find( uf->scoped_name() );
        if ( ctor_itr == user_function_labels.end() )
        {
          report.debug( *cd, " - Constructor: {} PC=???", cd->name );
          cd->internal_error(
              fmt::format( "Constructor {} not found in user_function_labels", cd->name ) );
        }
        constructor_addresses.push_back( ctor_itr->second.address() );
      }
    }

    for ( const auto& [method, uf_link] : cd->methods )
    {
      auto uf = uf_link->user_function();

      if ( !uf )
      {
        cd->internal_error( fmt::format( "method {}::{} no function linked", cd->name, method ) );
      }

      auto method_itr = user_function_labels.find( ScopableName( cd->name, method ).string() );
      if ( method_itr == user_function_labels.end() )
      {
        report.debug( *cd, " - Method: {} label=???", method );
        cd->internal_error( fmt::format( "Method {} not found in user_function_labels", method ) );
      }
      auto address = method_itr->second.address();
      if ( address == 0 )
      {
        report.debug( *cd, " - Method: {} PC=???", method );
        cd->internal_error( fmt::format( "Method {} has no PC for attached label", method ) );
      }

      bool use_method = method_names.find( method ) == method_names.end();

      if ( use_method )
      {
        unsigned funcref_index;
        function_reference_registrar.lookup_or_register_reference( *uf, funcref_index );
        auto name_offset = this->emit_data( method );
        method_descriptors.emplace_back( name_offset, address, funcref_index );
        report.debug( *cd, " - Method: {} PC={} funcref_index={}", method,
                      method_itr->second.address(), funcref_index );
        method_names.insert( method );
      }
      else
      {
        report.debug( *cd, " - Method: {} PC={} [ignored]", method, method_itr->second.address() );
      }
    }

    for ( const auto& base_cd_link : cd->base_class_links )
    {
      if ( auto base_cd = base_cd_link->class_declaration() )
      {
        to_link.push_back( base_cd );
      }
    }
  }
  report.debug( node, fmt::format( "Class: {}", node.name ) );

  for ( const auto& offset : constructor_addresses )
  {
    report.debug( node, fmt::format( " - Constructor @ PC={} ", offset ) );
  }

  for ( const auto& method_info : method_descriptors )
  {
    report.debug( node, fmt::format( " - Method @ PC={} name_offset={} funcref_index={} ",
                                     method_info.address, method_info.name_offset,
                                     method_info.function_reference_index ) );
  }

  class_declaration_registrar.register_class( class_name_offset, constructor_addresses,
                                              method_descriptors );
}

unsigned InstructionEmitter::enter_debug_block(
    const LocalVariableScopeInfo& local_variable_scope_info )
{
  unsigned previous_debug_block_index = debug_instruction_info.block_index;

  if ( !local_variable_scope_info.variables.empty() )
  {
    debug_instruction_info.block_index =
        debug.add_block( debug_instruction_info.block_index, local_variable_scope_info );
  }

  return previous_debug_block_index;
}

void InstructionEmitter::set_debug_block( unsigned block_index )
{
  debug_instruction_info.block_index = block_index;
}

// - When visiting an identifier:
// - If type == Capture: set to Local and ValueStack offset will be current function's param count +
// this variable index
// - If type == Local: if offset >= function param count, add current function's capture count
void InstructionEmitter::access_variable( const Variable& v, VariableIndex function_params_count,
                                          VariableIndex function_capture_count )
{
  BTokenId token_id;
  unsigned offset;

  if ( v.scope == VariableScope::Capture )
  {
    token_id = TOK_LOCALVAR;

    offset = v.index + function_params_count;
  }
  else if ( v.scope == VariableScope::Local )
  {
    token_id = TOK_LOCALVAR;

    if ( v.index >= function_params_count )
    {
      offset = v.index + function_capture_count;
    }
    else
    {
      offset = v.index;
    }
  }
  else
  {
    token_id = TOK_GLOBALVAR;
    offset = v.index;
  }
  emit_token( token_id, TYP_OPERAND, offset );
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

void InstructionEmitter::assign_subscript()
{
  unsigned indexes = 1;
  emit_token( INS_SUBSCRIPT_ASSIGN, TYP_UNARY_OPERATOR, indexes );
}

void InstructionEmitter::assign_multisubscript( unsigned indexes )
{
  emit_token( INS_MULTISUBSCRIPT_ASSIGN, TYP_UNARY_OPERATOR, indexes );
}

void InstructionEmitter::assign_variable( const Variable& v, VariableIndex function_params_count,
                                          VariableIndex function_capture_count )
{
  BTokenId token_id;
  unsigned offset;

  if ( v.scope == VariableScope::Capture )
  {
    token_id = INS_ASSIGN_LOCALVAR;

    offset = v.index + function_params_count;
  }
  else if ( v.scope == VariableScope::Local )
  {
    token_id = INS_ASSIGN_LOCALVAR;

    if ( v.index >= function_params_count )
    {
      offset = v.index + function_capture_count;
    }
    else
    {
      offset = v.index;
    }
  }
  else
  {
    token_id = INS_ASSIGN_GLOBALVAR;
    offset = v.index;
  }

  emit_token( token_id, TYP_UNARY_OPERATOR, offset );
}

void InstructionEmitter::basic_for_init( FlowControlLabel& label )
{
  register_with_label( label, emit_token( INS_INITFOR, TYP_RESERVED ) );
}

void InstructionEmitter::basic_for_next( FlowControlLabel& label )
{
  register_with_label( label, emit_token( INS_NEXTFOR, TYP_RESERVED ) );
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
      static_cast<unsigned char>( module_id ), TOK_FUNC,
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

void InstructionEmitter::classinst_create( unsigned index )
{
  emit_token( TOK_CLASSINST, TYP_OPERAND, index );
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

void InstructionEmitter::ctrl_statementbegin( unsigned file_index, unsigned file_offset,
                                              const std::string& source_text )
{
  unsigned source_offset = emit_data( source_text );
  Pol::Bscript::DebugToken debug_token;
  debug_token.sourceFile = file_index + 1;
  debug_token.offset = file_offset;
  debug_token.strOffset = source_offset;
  unsigned offset =
      data_emitter.store( reinterpret_cast<std::byte*>( &debug_token ), sizeof debug_token );
  emit_token( CTRL_STATEMENTBEGIN, TYP_CONTROL, offset );
}

// - When declaring an identifier:
// - If type == Local: if offset >= function param count, add current function's capture count
void InstructionEmitter::declare_variable( const Variable& v, VariableIndex function_capture_count )
{
  int offset;

  if ( v.scope == VariableScope::Local && v.index >= function_capture_count )
  {
    offset = v.index + function_capture_count;
  }
  else
  {
    offset = v.index;
  }

  BTokenId token_id = v.scope == VariableScope::Global ? RSV_GLOBAL : RSV_LOCAL;
  emit_token( token_id, TYP_RESERVED, offset );
}

void InstructionEmitter::dictionary_create()
{
  emit_token( TOK_DICTIONARY, TYP_OPERAND );
}

void InstructionEmitter::dictionary_add_member()
{
  emit_token( INS_DICTIONARY_ADDMEMBER, TYP_OPERATOR );
}

void InstructionEmitter::error_create()
{
  emit_token( TOK_ERROR, TYP_OPERAND );
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

void InstructionEmitter::function_reference( const UserFunction& uf, FlowControlLabel& label )
{
  unsigned index;

  function_reference_registrar.lookup_or_register_reference( uf, index );

  auto type = static_cast<BTokenType>( index );

  register_with_label( label, emit_token( TOK_FUNCREF, type ) );
}

void InstructionEmitter::functor_create( const UserFunction& uf )
{
  unsigned reference_index;
  function_reference_registrar.lookup_or_register_reference( uf, reference_index );
  StoredToken token( static_cast<unsigned char>( Mod_Basic ), TOK_FUNCTOR,
                     static_cast<BTokenType>(
                         reference_index ),  // index to the EScriptProgram's function_references,
                                             // stored in Token.lval, saved in StoredToken.type
                     0 );
  append_token( token );
}

void InstructionEmitter::get_arg( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_GET_ARG, TYP_OPERATOR, offset );
}

void InstructionEmitter::get_member( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_GET_MEMBER, TYP_UNARY_OPERATOR, offset );
}

void InstructionEmitter::get_member_id( MemberID member_id )
{
  emit_token( INS_GET_MEMBER_ID, TYP_UNARY_OPERATOR, member_id );
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

void InstructionEmitter::return_from_constructor_function( unsigned this_offset )
{
  // Emit `this`
  emit_token( TOK_LOCALVAR, TYP_OPERAND, this_offset );

  // Emit a return
  return_from_user_function();
}

void InstructionEmitter::set_member_id_consume( MemberID member_id )
{
  emit_token( INS_SET_MEMBER_ID_CONSUME, TYP_UNARY_OPERATOR, member_id );
}

void InstructionEmitter::set_member_id( MemberID member_id )
{
  emit_token( INS_SET_MEMBER_ID, TYP_UNARY_OPERATOR, member_id );
}

void InstructionEmitter::set_member_consume( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_SET_MEMBER_CONSUME, TYP_UNARY_OPERATOR, offset );
}

void InstructionEmitter::set_member( const std::string& name )
{
  unsigned offset = emit_data( name );
  emit_token( INS_SET_MEMBER, TYP_UNARY_OPERATOR, offset );
}

void InstructionEmitter::set_member_by_operator( BTokenId token_id, MemberID member_id )
{
  emit_token( token_id, TYP_UNARY_OPERATOR, member_id );
}

void InstructionEmitter::spread()
{
  emit_token( TOK_SPREAD, TYP_OPERAND );
}

unsigned InstructionEmitter::skip_if_true_else_consume()
{
  return emit_token( INS_SKIPIFTRUE_ELSE_CONSUME, TYP_CONTROL );
}

void InstructionEmitter::struct_create()
{
  emit_token( TOK_STRUCT, TYP_OPERAND );
}

void InstructionEmitter::struct_add_member( const std::string& name )
{
  auto offset = emit_data( name );
  emit_token( INS_ADDMEMBER_ASSIGN, TYP_OPERAND, offset );
}

void InstructionEmitter::struct_add_uninit_member( const std::string& name )
{
  auto offset = emit_data( name );
  emit_token( INS_ADDMEMBER2, TYP_OPERAND, offset );
}

void InstructionEmitter::subscript_single()
{
  emit_token( TOK_ARRAY_SUBSCRIPT, TYP_OPERATOR, 1 );
}

void InstructionEmitter::subscript_multiple( unsigned indexes )
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

void InstructionEmitter::value( bool v )
{
  emit_token( TOK_BOOL, TYP_OPERAND, v );
}

void InstructionEmitter::value( const std::string& v )
{
  unsigned data_offset = emit_data( v );
  emit_token( TOK_STRING, TYP_OPERAND, data_offset );
}

void InstructionEmitter::interpolate_string( unsigned count )
{
  emit_token( TOK_INTERPOLATE_STRING, TYP_OPERAND, count );
}

void InstructionEmitter::format_expression()
{
  emit_token( TOK_FORMAT_EXPRESSION, TYP_OPERAND );
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
  debug.add_instruction( debug_instruction_info );
  debug_instruction_info.statement_begin = false;
  return code_emitter.append( token );
}

void InstructionEmitter::debug_file_line( unsigned file, unsigned line )
{
  // debug info always has file #0 = empty (keeping for parity, for now)
  debug_instruction_info.file_index = file + 1;
  debug_instruction_info.line_number = line;
}

void InstructionEmitter::debug_statementbegin()
{
  debug_instruction_info.statement_begin = true;
}

unsigned InstructionEmitter::next_instruction_address()
{
  return code_emitter.next_address();
}

void InstructionEmitter::debug_user_function( const std::string& name, unsigned first_pc,
                                              unsigned last_pc )
{
  DebugStore::UserFunctionInfo ufi{ name, first_pc, last_pc };
  debug.add_user_function( std::move( ufi ) );
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
