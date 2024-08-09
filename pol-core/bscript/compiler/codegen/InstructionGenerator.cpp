#include "InstructionGenerator.h"

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/sliced.hpp>

#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/BasicForLoop.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/Block.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/BranchSelector.h"
#include "bscript/compiler/ast/CaseDispatchGroup.h"
#include "bscript/compiler/ast/CaseDispatchGroups.h"
#include "bscript/compiler/ast/CaseDispatchSelectors.h"
#include "bscript/compiler/ast/CaseStatement.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/ClassInstance.h"
#include "bscript/compiler/ast/ConditionalOperator.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/CstyleForLoop.h"
#include "bscript/compiler/ast/DebugStatementMarker.h"
#include "bscript/compiler/ast/DefaultConstructorFunction.h"
#include "bscript/compiler/ast/DictionaryEntry.h"
#include "bscript/compiler/ast/DictionaryInitializer.h"
#include "bscript/compiler/ast/DoWhileLoop.h"
#include "bscript/compiler/ast/ElementAccess.h"
#include "bscript/compiler/ast/ElementAssignment.h"
#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/ElvisOperator.h"
#include "bscript/compiler/ast/ErrorInitializer.h"
#include "bscript/compiler/ast/ExitStatement.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/ForeachLoop.h"
#include "bscript/compiler/ast/FormatExpression.h"
#include "bscript/compiler/ast/FunctionBody.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionExpression.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IfThenElseStatement.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/InterpolateString.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/MemberAssignment.h"
#include "bscript/compiler/ast/MemberAssignmentByOperator.h"
#include "bscript/compiler/ast/MethodCall.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/ProgramParameterDeclaration.h"
#include "bscript/compiler/ast/RepeatUntilLoop.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/SpreadElement.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/StructInitializer.h"
#include "bscript/compiler/ast/StructMemberInitializer.h"
#include "bscript/compiler/ast/UnaryOperator.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/ast/ValueConsumer.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/ast/VariableAssignmentStatement.h"
#include "bscript/compiler/ast/WhileLoop.h"
#include "bscript/compiler/codegen/CaseDispatchGroupVisitor.h"
#include "bscript/compiler/codegen/CaseJumpDataBlock.h"
#include "bscript/compiler/codegen/DebugBlockGuard.h"
#include "bscript/compiler/codegen/InstructionEmitter.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/FlowControlLabel.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/Variable.h"
#include "symcont.h"

namespace Pol::Bscript::Compiler
{
InstructionGenerator::InstructionGenerator(
    InstructionEmitter& emitter, std::map<std::string, FlowControlLabel>& user_function_labels )
    : emitter( emitter ),
      emit( emitter ),
      user_function_labels( user_function_labels ),
      user_functions()
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
  emit.debug_file_line( loc.source_file_identifier->index,
                        static_cast<unsigned>( loc.range.start.line_number ) );
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
  for ( size_t i = 0, c = groups.children.size(); i < c; ++i )
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

  data_block.on_default_jump_to( static_cast<unsigned short>( default_label.address() ) );

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
    emit.leaveblock( static_cast<unsigned>( node.local_variable_scope_info.variables.size() ) );
  }
}

void InstructionGenerator::visit_boolean_value( BooleanValue& node )
{
  update_debug_location( node );
  emit.value( node.value );
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

void InstructionGenerator::visit_class_declaration( ClassDeclaration& node )
{
  // Skip visiting the parameter list (child 0), as their nodes (Identifier) do not
  // generate any instructions.

  for ( const auto& child : node.children | boost::adaptors::sliced( 1, node.children.size() ) )
  {
    child->accept( *this );
  }
}

void InstructionGenerator::visit_class_instance( ClassInstance& node )
{
  update_debug_location( node );
  emit.classinst_create();
}

void InstructionGenerator::visit_debug_statement_marker( DebugStatementMarker& marker )
{
  emit.debug_statementbegin();
  update_debug_location( marker );

  unsigned source_file = marker.source_location.source_file_identifier->index;
  emit.ctrl_statementbegin( source_file, marker.start_index, marker.text );

  visit_children( marker );
}

// Called when emitting the instructions for a compiler-generated, default constructor function.
// This will _most likely_ need to be completely rewritten to handle the super-chaining.
void InstructionGenerator::visit_default_constructor_function( DefaultConstructorFunction& node )
{
  FlowControlLabel& label = user_function_labels[node.scoped_name()];
  emit.label( label );
  visit_children( node );            // emits the `pop this` from the function param decl
  emit.method_this();                // push `this` on the stack
  emit.return_from_user_function();  // return from the function
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
  auto indexes = static_cast<unsigned>( acc.indexes().children.size() );
  if ( indexes == 1 )
    emit.subscript_single();
  else
    emit.subscript_multiple( indexes );
}

void InstructionGenerator::visit_element_assignment( ElementAssignment& node )
{
  visit_children( node );
  update_debug_location( node );
  auto num_indexes = static_cast<unsigned>( node.indexes().children.size() );
  if ( node.consume )
  {
    if ( num_indexes == 1 )
    {
      emit.assign_subscript_consume();
    }
    else
    {
      // there is no assign-multisubscript-consume instruction
      emit.assign_multisubscript( num_indexes );
      emit.consume();
    }
  }
  else
  {
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
  emit.debug_statementbegin();
  update_debug_location( loop );

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
  // A function call will have no link if it is an expression call, eg. `(foo)(1,2)`.
  if ( !call.function_link->function() )
  {
    // Visiting the children emits the instructions for each arguments in the order necessary for a
    // `MTH_CALL`.
    visit_children( call );
    update_debug_location( call );
    // Subtract 1 because the first child is the callee.
    emit.call_method_id( MTH_CALL, static_cast<unsigned int>( call.children.size() - 1 ) );
    // Constructing array from rest arguments is done in executor for dynamic function calls.
  }
  else
  {
    auto emit_args = [&]( Function& function )
    {
      if ( function.is_variadic() )
      {
        auto num_nonrest_args = function.parameter_count() - 1;

        // Push real args, then create an array for the last rest arg.
        // All children for a non-expression-as-callee FunctionCalls are arguments.
        for ( unsigned i = 0; i < call.children.size(); ++i )
        {
          bool is_spread = dynamic_cast<SpreadElement*>( call.children[i].get() );

          if ( is_spread && i < num_nonrest_args )
          {
            // Should be caught by semantic analyzer
            call.internal_error( "spread operator used in location before rest arguments" );
            return;
          }

          // Create the array once we've reached the rest argument.
          if ( i == num_nonrest_args )
          {
            emit.array_create();
          }

          call.children[i]->accept( *this );

          if ( i >= num_nonrest_args )
          {
            // `ins_insert_into` for arrays will spread BSpread's at runtime.
            emit.array_append();
          }
        }

        // If there was no rest argument, create an empty array.
        if ( call.children.size() <= num_nonrest_args )
        {
          emit.array_create();
        }
      }
      else
      {
        // Cannot use spread operator in non-variadic function calls.
        visit_children( call );
      }

      // Update the debug location to the function call after emitting the
      // arguments.
      update_debug_location( call );
    };

    if ( auto mf = call.function_link->module_function_declaration() )
    {
      emit_args( *mf );
      emit.call_modulefunc( *mf );
    }
    else if ( auto uf = call.function_link->user_function() )
    {
      emit_args( *uf );
      FlowControlLabel& label = user_function_labels[uf->scoped_name()];
      emit.makelocal();
      emit.call_userfunc( label );
    }
    else
    {
      call.internal_error( "neither a module function nor a user function?" );
    }
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

// The function expression generation emits the following instructions:
// - captured vars
// - captured vars count
// - parameter count
// - is variadic
// - create-functor <instructions count>
// - function instructions
//
// TODO maybe use a data area to encapsulate counts + variadic flag?
void InstructionGenerator::visit_function_expression( FunctionExpression& node )
{
  update_debug_location( node );
  if ( auto user_function = node.function_link->user_function() )
  {
    // Push the captured variables
    for ( const auto& variable : user_function->capture_variable_scope_info.variables )
    {
      emit_access_variable( *variable->capturing );
    }

    emit.functor_create( *user_function );
    auto index = emitter.next_instruction_address() - 1;

    user_function->accept( *this );

    auto instrs_count = emitter.next_instruction_address() - index - 1;

    // Ensure the capture count does not exceed maximum positive signed `int` size.
    if ( user_function->capture_count() >
         static_cast<unsigned int>( std::numeric_limits<int>::max() ) )
    {
      node.internal_error( "capture count too large" );
    }

    emit.patch_offset( index, instrs_count );
  }
  else
  {
    node.internal_error( "user function for function expression not found" );
  }
}

void InstructionGenerator::visit_function_reference( FunctionReference& function_reference )
{
  if ( auto uf = function_reference.function_link->user_function() )
  {
    update_debug_location( function_reference );
    FlowControlLabel& label = user_function_labels[uf->scoped_name()];
    emit.function_reference( *uf, label );
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
    emit_access_variable( *var );
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
  emit.debug_statementbegin();
  update_debug_location( jump );
  if ( jump.local_variables_to_remove )
    emit.leaveblock( jump.local_variables_to_remove );
  emit.jmp_always( *jump.flow_control_label );
}

void InstructionGenerator::visit_member_access( MemberAccess& member_access )
{
  visit_children( member_access );

  update_debug_location( member_access );
  if ( auto km = member_access.known_member )
    emit.get_member_id( km->id );
  else
    emit.get_member( member_access.name );
}

void InstructionGenerator::visit_member_assignment( MemberAssignment& node )
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

void InstructionGenerator::visit_member_assignment_by_operator( MemberAssignmentByOperator& node )
{
  visit_children( node );

  update_debug_location( node );
  emit.set_member_by_operator( node.token_id, node.known_member.id );
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

  emit.debug_statementbegin();
  update_debug_location( program );

  visit_children( program );

  if ( !program.local_variable_scope_info.variables.empty() )
  {
    emit.leaveblock( static_cast<unsigned>( program.local_variable_scope_info.variables.size() ) );
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
  emit.debug_statementbegin();

  visit_children( ret );

  update_debug_location( ret );
  if ( !user_functions.empty() )
  {
    emit.return_from_user_function();
  }
  else
  {
    emit.progend();
  }
}

void InstructionGenerator::visit_spread_element( SpreadElement& node )
{
  visit_children( node );
  update_debug_location( node );
  emit.spread();
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
  user_functions.push( &user_function );
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

  FlowControlLabel& label = user_function_labels[user_function.scoped_name()];
  emit.label( label );

  // Pop function parameters
  user_function.child<FunctionParameterList>( 0 ).accept( *this );

  // Pop caputured variables
  for ( const auto& variable : user_function.capture_variable_scope_info.variables )
  {
    emit.pop_param_byref( variable->name );
  }

  user_function.body().accept( *this );

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
  user_functions.pop();
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

  int function_capture_count = 0;

  if ( !user_functions.empty() )
  {
    function_capture_count = user_functions.top()->capture_count();
  }

  emit.declare_variable( *node.variable, function_capture_count );

  if ( node.initialize_as_empty_array )
  {
    emit.array_declare();
  }
  else if ( !node.children.empty() )
  {
    visit_children( node );

    emit.assign();
  }

  emit.consume();
}

void InstructionGenerator::visit_variable_assignment_statement( VariableAssignmentStatement& node )
{
  emitter.debug_statementbegin();
  generate( node.rhs() );
  update_debug_location( node );
  auto& identifier = node.identifier();
  auto& variable = identifier.variable;

  int function_params_count = 0;
  int function_capture_count = 0;

  if ( !user_functions.empty() )
  {
    function_params_count = user_functions.top()->parameter_count();
    function_capture_count = user_functions.top()->capture_count();
  }

  emit.assign_variable( *variable, function_params_count, function_capture_count );
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

void InstructionGenerator::visit_interpolate_string( InterpolateString& node )
{
  visit_children( node );

  update_debug_location( node );
  emit.interpolate_string( static_cast<unsigned>( node.children.size() ) );
}

void InstructionGenerator::visit_format_expression( FormatExpression& node )
{
  visit_children( node );

  update_debug_location( node );
  emit.format_expression();
}

void InstructionGenerator::visit_conditional_operator( ConditionalOperator& node )
{
  update_debug_location( node );

  // generate conditional
  generate( node.conditional() );
  // consume+jump to end-of-consequent label if false
  emit.jmp_if_false( *node.consequent_label );
  // generate consequent
  generate( node.consequent() );
  // jump to end-of-alternate label
  emit.jmp_always( *node.alternate_label );
  // end-of-consequent label
  emit.label( *node.consequent_label );
  // generate alternate
  generate( node.alternate() );
  // end-of-alternate label
  emit.label( *node.alternate_label );
}

void InstructionGenerator::emit_access_variable( Variable& variable )
{
  int function_params_count = 0;
  int function_capture_count = 0;

  if ( !user_functions.empty() )
  {
    function_params_count = user_functions.top()->parameter_count();
    function_capture_count = user_functions.top()->capture_count();
  }

  emit.access_variable( variable, function_params_count, function_capture_count );
}
}  // namespace Pol::Bscript::Compiler
