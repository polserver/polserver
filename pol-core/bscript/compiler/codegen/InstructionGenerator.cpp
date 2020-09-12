#include "InstructionGenerator.h"

#include <boost/range/adaptor/reversed.hpp>

#include "compiler/ast/ArrayInitializer.h"
#include "compiler/ast/AssignVariableConsume.h"
#include "compiler/ast/BasicForLoop.h"
#include "compiler/ast/BinaryOperator.h"
#include "compiler/ast/Block.h"
#include "compiler/ast/BranchSelector.h"
#include "compiler/ast/CaseDispatchGroup.h"
#include "compiler/ast/CaseDispatchGroups.h"
#include "compiler/ast/CaseDispatchSelectors.h"
#include "compiler/ast/CaseStatement.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/CstyleForLoop.h"
#include "compiler/ast/DebugStatementMarker.h"
#include "compiler/ast/DictionaryEntry.h"
#include "compiler/ast/DictionaryInitializer.h"
#include "compiler/ast/DoWhileLoop.h"
#include "compiler/ast/ElementAccess.h"
#include "compiler/ast/ElementAssignment.h"
#include "compiler/ast/ElementIndexes.h"
#include "compiler/ast/ElvisOperator.h"
#include "compiler/ast/ErrorInitializer.h"
#include "compiler/ast/ExitStatement.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/ForeachLoop.h"
#include "compiler/ast/FunctionBody.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/FunctionReference.h"
#include "compiler/ast/GetMember.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/IfThenElseStatement.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/JumpStatement.h"
#include "compiler/ast/MethodCall.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/ProgramParameterDeclaration.h"
#include "compiler/ast/RepeatUntilLoop.h"
#include "compiler/ast/ReturnStatement.h"
#include "compiler/ast/SetMember.h"
#include "compiler/ast/SetMemberByOperator.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/StructInitializer.h"
#include "compiler/ast/StructMemberInitializer.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/UninitializedValue.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/ast/VarStatement.h"
#include "compiler/ast/WhileLoop.h"
#include "compiler/codegen/CaseDispatchGroupVisitor.h"
#include "compiler/codegen/CaseJumpDataBlock.h"
#include "compiler/codegen/DebugBlockGuard.h"
#include "compiler/codegen/InstructionEmitter.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/FlowControlLabel.h"
#include "compiler/model/FunctionLink.h"
#include "compiler/model/Variable.h"
#include "symcont.h"

namespace Pol::Bscript::Compiler
{
InstructionGenerator::InstructionGenerator(
    InstructionEmitter& emitter, std::map<std::string, FlowControlLabel>& user_function_labels,
    bool in_function )
  : emitter( emitter ),
    emit( emitter ),
    user_function_labels( user_function_labels ),
    in_function( in_function )
{
}

void InstructionGenerator::generate( Node& node )
{
  // alternative: two identical methods 'evaluate' and 'execute', for readability
  update_debug_location( node );
  node.accept( *this );
}

void InstructionGenerator::update_debug_location( const Node& node )
{
  update_debug_location( node.source_location );
}

void InstructionGenerator::update_debug_location( const SourceLocation& loc )
{
  emit.debug_file_line( loc.source_file_identifier->index, loc.line_number );
}

void InstructionGenerator::visit_array_initializer( ArrayInitializer& node )
{
  update_debug_location( node );
  emit.array_create();
  for ( const auto& child : node.children )
  {
    child->accept( *this );
    emit.array_append();
  }
}

void InstructionGenerator::visit_assign_variable_consume( AssignVariableConsume& node )
{
  generate( node.rhs() );
  update_debug_location( node );
  auto& identifier = node.identifier();
  auto& variable = identifier.variable;

  emit.assign_variable( *variable );
}

void InstructionGenerator::visit_basic_for_loop( BasicForLoop& loop )
{
  emit.debug_statementbegin();
  update_debug_location( loop );
  FlowControlLabel skip, next;

  generate( loop.first() );
  generate( loop.last() );

  DebugBlockGuard debug_block_guard( emitter, loop.local_variable_scope_info );

  emit.basic_for_init( skip );

  emit.label( next );
  generate( loop.block() );

  emit.label( *loop.continue_label );
  emit.basic_for_next( next );

  emit.label( *loop.break_label );
  emit.leaveblock( 2 );

  emit.label( skip );
}

void InstructionGenerator::visit_case_statement( CaseStatement& node )
{
  emit.debug_statementbegin();
  generate( node.expression() );
  update_debug_location( node );
  const unsigned casejmp = emit.casejmp();

  CaseJumpDataBlock data_block;

  FlowControlLabel default_label;

  auto& groups = node.dispatch_groups();
  for ( int i = 0, c = groups.children.size(); i < c; ++i )
  {
    CaseDispatchGroup& group = groups.dispatch_group( i );
    FlowControlLabel group_label;
    emit.label( group_label );
    generate( group.block() );

    emit.label( *group.break_label );
    bool last = i == c - 1;
    if ( !last )
      emit.jmp_always( *node.break_label );

    CaseDispatchGroupVisitor visitor( data_block, group_label, default_label );
    group.selectors().accept( visitor );
  }

  if ( !default_label.has_address() )
    emit.label( default_label );

  data_block.on_default_jump_to( default_label.address() );

  emit.label( *node.break_label );

  unsigned dispatch_table_data_offset = emit.case_dispatch_table( data_block );
  emitter.patch_offset( casejmp, dispatch_table_data_offset );
}

void InstructionGenerator::visit_cstyle_for_loop( CstyleForLoop& loop )
{
  emit.debug_statementbegin();
  update_debug_location( loop );
  generate( loop.initializer() );
  emit.consume();

  FlowControlLabel check_predicate;
  emit.label( check_predicate );
  generate( loop.predicate() );

  emit.jmp_if_false( *loop.break_label );

  generate( loop.block() );

  emit.label( *loop.continue_label );
  generate( loop.advancer() );
  emit.consume();

  emit.jmp_always( check_predicate );

  emit.label( *loop.break_label );
}

void InstructionGenerator::visit_binary_operator( BinaryOperator& node )
{
  visit_children( node );

  update_debug_location( node );
  emit.binary_operator( node.token_id );
}

void InstructionGenerator::visit_block( Block& node )
{
  DebugBlockGuard debug_block_guard( emitter, node.local_variable_scope_info );

  visit_children( node );

  if ( !node.local_variable_scope_info.variables.empty() )
  {
    emit.leaveblock( node.local_variable_scope_info.variables.size() );
  }
}

void InstructionGenerator::visit_branch_selector( BranchSelector& node )
{
  visit_children( node );

  switch ( node.branch_type )
  {
  case BranchSelector::IfTrue:
    emit.jmp_if_true( *node.flow_control_label );
    break;
  case BranchSelector::IfFalse:
    emit.jmp_if_false( *node.flow_control_label );
    break;
  case BranchSelector::Always:
    emit.jmp_always( *node.flow_control_label );
    break;
  case BranchSelector::Never:
    break;
  }
}

void InstructionGenerator::visit_debug_statement_marker( DebugStatementMarker& marker )
{
  emit.debug_statementbegin();
  update_debug_location( marker );

  // OG always puts 0.. marker.source_location.source_file_identifier->index
  unsigned source_file = 0;
  emit.ctrl_statementbegin( source_file, marker.start_index, marker.text );

  visit_children( marker );
}

void InstructionGenerator::visit_dictionary_initializer( DictionaryInitializer& node )
{
  update_debug_location( node );
  emit.dictionary_create();
  visit_children( node );
}

void InstructionGenerator::visit_dictionary_entry( DictionaryEntry& entry )
{
  update_debug_location( entry );
  visit_children( entry );
  emit.dictionary_add_member();
}

void InstructionGenerator::visit_do_while_loop( DoWhileLoop& node )
{
  emit.debug_statementbegin();
  update_debug_location( node );
  FlowControlLabel next;
  emit.label( next );
  generate( node.block() );
  emit.label( *node.continue_label );
  generate( node.predicate() );
  emit.jmp_if_true( next );
  emit.label( *node.break_label );
}

void InstructionGenerator::visit_element_access( ElementAccess& acc )
{
  visit_children( acc );
  update_debug_location( acc );
  int indexes = acc.indexes().children.size();
  if ( indexes == 1 )
    emit.subscript_single();
  else
    emit.subscript_multiple( indexes );
}

void InstructionGenerator::visit_element_assignment( ElementAssignment& node )
{
  visit_children( node );
  update_debug_location( node );
  if ( node.consume )
  {
    emit.assign_subscript_consume();
  }
  else
  {
    auto num_indexes = node.indexes().children.size();
    if ( num_indexes == 1 )
      emit.assign_subscript();
    else
      emit.assign_multisubscript( num_indexes );
  }
}

void InstructionGenerator::visit_elvis_operator( ElvisOperator& elvis )
{
  FlowControlLabel skip_instruction, after_rhs;

  update_debug_location( elvis );
  generate( elvis.lhs() );

  unsigned address = emit.skip_if_true_else_consume();

  unsigned skip_start_address = emitter.next_instruction_address();

  generate( elvis.rhs() );

  unsigned distance = emitter.next_instruction_address() - skip_start_address;

  emitter.patch_offset( address, distance );
}

void InstructionGenerator::visit_error_initializer( ErrorInitializer& node )
{
  update_debug_location( node );
  emit.error_create();
  int i = 0;
  for ( auto& child : node.children )
  {
    child->accept( *this );
    emit.struct_add_member( node.names[i++] );
  }
}

void InstructionGenerator::visit_exit_statement( ExitStatement& node )
{
  emit.debug_statementbegin();
  update_debug_location( node );
  emit.exit();
}

void InstructionGenerator::visit_float_value( FloatValue& node )
{
  update_debug_location( node );
  emit.value( node.value );
}

void InstructionGenerator::visit_foreach_loop( ForeachLoop& loop )
{
  generate( loop.expression() );

  DebugBlockGuard debug_block_guard( emitter, loop.local_variable_scope_info );

  emit.foreach_init( *loop.continue_label );

  FlowControlLabel next;
  emit.label( next );

  generate( loop.block() );

  emit.label( *loop.continue_label );
  emit.foreach_step( next );

  emit.label( *loop.break_label );
  emit.leaveblock( 3 );
}

void InstructionGenerator::visit_function_call( FunctionCall& call )
{
  visit_children( call );

  update_debug_location( call );
  if ( auto mf = call.function_link->module_function_declaration() )
  {
    emit.call_modulefunc( *mf );
  }
  else if ( auto uf = call.function_link->user_function() )
  {
    FlowControlLabel& label = user_function_labels[uf->name];
    emit.makelocal();
    emit.call_userfunc( label );
  }
  else
  {
    call.internal_error( "neither a module function nor a user function?" );
  }
}

void InstructionGenerator::visit_function_parameter_list( FunctionParameterList& node )
{
  for ( auto& child : boost::adaptors::reverse( node.children ) )
  {
    child->accept( *this );
  }
}

void InstructionGenerator::visit_function_parameter_declaration(
    FunctionParameterDeclaration& node )
{
  update_debug_location( node );
  if ( node.byref )
    emit.pop_param_byref( node.name );
  else
    emit.pop_param( node.name );
}

void InstructionGenerator::visit_function_reference( FunctionReference& function_reference )
{
  if ( auto uf = function_reference.function_link->user_function() )
  {
    update_debug_location( function_reference );
    FlowControlLabel& label = user_function_labels[uf->name];
    emit.function_reference( uf->parameter_count(), label );
  }
  else
  {
    function_reference.internal_error( "user function not found" );
  }
}

void InstructionGenerator::visit_identifier( Identifier& node )
{
  update_debug_location( node );
  if ( auto var = node.variable )
  {
    emit.access_variable( *var );
  }
  else
  {
    node.internal_error( "variable is not set" );
  }
}

void InstructionGenerator::visit_if_then_else_statement( IfThenElseStatement& node )
{
  emit.debug_statementbegin();
  update_debug_location( node );

  auto branch_selector = &node.branch_selector();
  generate( *branch_selector );

  std::shared_ptr<FlowControlLabel> skip_consequent = branch_selector->flow_control_label;

  generate( node.consequent() );

  if ( auto alternative = node.alternative() )
  {
    FlowControlLabel skip_alternative;
    emit.jmp_always( skip_alternative );
    emit.label( *skip_consequent );
    generate( *alternative );
    emit.label( skip_alternative );
  }
  else
  {
    emit.label( *skip_consequent );
  }
}

void InstructionGenerator::visit_integer_value( IntegerValue& node )
{
  update_debug_location( node );
  emit.value( node.value );
}

void InstructionGenerator::visit_jump_statement( JumpStatement& jump )
{
  update_debug_location( jump );
  if ( jump.local_variables_to_remove )
    emit.leaveblock( jump.local_variables_to_remove );
  emit.jmp_always( *jump.flow_control_label );
}

void InstructionGenerator::visit_get_member( GetMember& member_access )
{
  visit_children( member_access );

  update_debug_location( member_access );
  if ( auto km = member_access.known_member )
    emit.get_member_id( km->id );
  else
    emit.get_member( member_access.name );
}

void InstructionGenerator::visit_method_call( MethodCall& method_call )
{
  visit_children( method_call );

  update_debug_location( method_call );
  auto argument_count = method_call.argument_count();
  if ( auto km = method_call.known_method )
  {
    emit.call_method_id( km->id, argument_count );
  }
  else
  {
    std::string method_name = method_call.methodname;
    Clib::mklowerASCII( method_name );
    emit.call_method( method_name, argument_count );
  }
}

void InstructionGenerator::visit_program( Program& program )
{
  DebugBlockGuard debug_block_guard( emitter, program.local_variable_scope_info );

  update_debug_location( program );

  visit_children( program );

  if ( !program.local_variable_scope_info.variables.empty() )
  {
    emit.leaveblock( program.local_variable_scope_info.variables.size() );
  }
}

void InstructionGenerator::visit_program_parameter_declaration( ProgramParameterDeclaration& param )
{
  update_debug_location( param );
  emit.get_arg( param.name );
}

void InstructionGenerator::visit_repeat_until_loop( RepeatUntilLoop& loop )
{
  emit.debug_statementbegin();
  update_debug_location( loop );

  FlowControlLabel top;

  emit.label( top );
  generate( loop.block() );
  emit.label( *loop.continue_label );
  generate( loop.expression() );
  emit.jmp_if_false( top );
  emit.label( *loop.break_label );
}

void InstructionGenerator::visit_return_statement( ReturnStatement& ret )
{
  visit_children( ret );

  update_debug_location( ret );
  if ( in_function )
  {
    emit.return_from_user_function();
  }
  else
  {
    emit.progend();
  }
}

void InstructionGenerator::visit_set_member( SetMember& node )
{
  visit_children( node );

  update_debug_location( node );
  if ( auto known_member = node.known_member )
  {
    if ( node.consume )
      emit.set_member_id_consume( known_member->id );
    else
      emit.set_member_id( known_member->id );
  }
  else
  {
    if ( node.consume )
      emit.set_member_consume( node.name );
    else
      emit.set_member( node.name );
  }
}

void InstructionGenerator::visit_set_member_by_operator( SetMemberByOperator& node )
{
  visit_children( node );

  update_debug_location( node );
  emit.set_member_by_operator( node.token_id, node.known_member.id );
}

void InstructionGenerator::visit_string_value( StringValue& lit )
{
  update_debug_location( lit );
  emit.value( lit.value );
}

void InstructionGenerator::visit_struct_initializer( StructInitializer& node )
{
  update_debug_location( node );
  emit.struct_create();
  visit_children( node );
}

void InstructionGenerator::visit_struct_member_initializer( StructMemberInitializer& node )
{
  visit_children( node );

  update_debug_location( node );
  if ( node.children.empty() )
    emit.struct_add_uninit_member( node.name );
  else
    emit.struct_add_member( node.name );
}

void InstructionGenerator::visit_unary_operator( UnaryOperator& unary_operator )
{
  visit_children( unary_operator );
  emit.unary_operator( unary_operator.token_id );
}

void InstructionGenerator::visit_uninitialized_value( UninitializedValue& node )
{
  update_debug_location( node );
  emit.uninit();
}

void InstructionGenerator::visit_user_function( UserFunction& user_function )
{
  unsigned first_instruction_address = emitter.next_instruction_address();
  DebugBlockGuard debug_block_guard( emitter, user_function.local_variable_scope_info );

  emit.debug_statementbegin();
  update_debug_location( user_function );
  if ( user_function.exported )
  {
    // emit the exported entry stub
    FlowControlLabel exported_entrypoint, internal_entrypoint;

    emit.label( exported_entrypoint );
    emit.makelocal();
    emit.call_userfunc( internal_entrypoint );
    emit.progend();

    emit.label( internal_entrypoint );
    emitter.register_exported_function( exported_entrypoint, user_function.name,
                                        user_function.parameter_count() );
  }

  FlowControlLabel& label = user_function_labels[user_function.name];
  emit.label( label );
  visit_children( user_function );

  if ( !dynamic_cast<ReturnStatement*>( user_function.body().last_statement() ) )
  {
    emit.debug_statementbegin();
    update_debug_location( user_function.endfunction_location );
    emit.value( 0 );
    emit.return_from_user_function();
  }
  unsigned last_instruction_address = emitter.next_instruction_address() - 1;
  emitter.debug_user_function( user_function.name, first_instruction_address,
                               last_instruction_address );
}

void InstructionGenerator::visit_value_consumer( ValueConsumer& node )
{
  emitter.debug_statementbegin();
  update_debug_location( node );
  visit_children( node );

  emit.consume();
}

void InstructionGenerator::visit_var_statement( VarStatement& node )
{
  emit.debug_statementbegin();
  update_debug_location( node );
  if ( !node.variable )
    node.internal_error( "variable is not defined" );
  emit.declare_variable( *node.variable );

  if ( node.initialize_as_empty_array )
  {
    emit.array_declare();
  }
  else if ( !node.children.empty() )
  {
    visit_children( node );

    emit.assign();
  }
}

void InstructionGenerator::visit_while_loop( WhileLoop& loop )
{
  emit.debug_statementbegin();
  update_debug_location( loop );
  emit.label( *loop.continue_label );
  generate( loop.predicate() );
  emit.jmp_if_false( *loop.break_label );
  generate( loop.block() );
  emit.jmp_always( *loop.continue_label );
  emit.label( *loop.break_label );
}

}  // namespace Pol::Bscript::Compiler
