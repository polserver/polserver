#include "InstructionGenerator.h"

#include "compiler/ast/Block.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/FunctionParameterDeclaration.h"
#include "compiler/ast/FunctionParameterList.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/IfThenElseStatement.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/Program.h"
#include "compiler/ast/ProgramParameterDeclaration.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/UnaryOperator.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/ast/VarStatement.h"
#include "compiler/codegen/InstructionEmitter.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/FlowControlLabel.h"
#include "compiler/model/FunctionLink.h"
#include "compiler/model/Variable.h"
#include "symcont.h"

namespace Pol::Bscript::Compiler
{
InstructionGenerator::InstructionGenerator( InstructionEmitter& emitter )
  : emitter( emitter ),
    emit( emitter )
{
}

void InstructionGenerator::generate( Node& node )
{
  // alternative: two identical methods 'evaluate' and 'execute', for readability
  node.accept( *this );
}

void InstructionGenerator::visit_block( Block& node )
{
  visit_children( node );

  if ( node.locals_in_block )
  {
    emit.leaveblock( node.locals_in_block );
  }
}

void InstructionGenerator::visit_float_value( FloatValue& node )
{
  emit.value( node.value );
}

void InstructionGenerator::visit_function_call( FunctionCall& call )
{
  visit_children( call );

  if ( auto mf = call.function_link->module_function_declaration() )
  {
    emit.call_modulefunc( *mf );
  }
  else
  {
    call.internal_error( "neither a module function nor a user function?" );
  }
}

void InstructionGenerator::visit_identifier( Identifier& node )
{
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
  bool invert_jump = false;
  auto predicate = &node.predicate();
  if ( auto unary_operator = dynamic_cast<UnaryOperator*>( predicate ) )
  {
    if ( unary_operator->token_id == TOK_LOG_NOT )
    {
      invert_jump = true;
      predicate = &unary_operator->operand();
    }
  }
  generate( *predicate );

  FlowControlLabel skip_consequent;

  if ( invert_jump )
    emit.jmp_if_true( skip_consequent );
  else
    emit.jmp_if_false( skip_consequent );

  generate( node.consequent() );

  if ( auto alternative = node.alternative() )
  {
    FlowControlLabel skip_alternative;
    emit.jmp_always( skip_alternative );
    emit.label( skip_consequent );
    generate( *alternative );
    emit.label( skip_alternative );
  }
  else
  {
    emit.label( skip_consequent );
  }
}

void InstructionGenerator::visit_integer_value( IntegerValue& node )
{
  emit.value( node.value );
}

void InstructionGenerator::visit_program( Program& program )
{
  visit_children( program );

  if ( program.locals_in_block )
  {
    emit.leaveblock( program.locals_in_block );
  }
}

void InstructionGenerator::visit_program_parameter_declaration( ProgramParameterDeclaration& param )
{
  emit.get_arg( param.name );
}

void InstructionGenerator::visit_string_value( StringValue& lit )
{
  emit.value( lit.value );
}

void InstructionGenerator::visit_unary_operator( UnaryOperator& unary_operator )
{
  visit_children( unary_operator );
  emit.unary_operator( unary_operator.token_id );
}

void InstructionGenerator::visit_value_consumer( ValueConsumer& node )
{
  visit_children( node );

  emit.consume();
}

void InstructionGenerator::visit_var_statement( VarStatement& node )
{
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

}  // namespace Pol::Bscript::Compiler
