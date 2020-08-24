#include "InstructionGenerator.h"

#include "compiler/ast/FloatValue.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/codegen/InstructionEmitter.h"
#include "compiler/file/SourceFileIdentifier.h"
#include "compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
InstructionGenerator::InstructionGenerator( InstructionEmitter& emitter )
  : emitter( emitter ),
    emit( emitter )
{
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

void InstructionGenerator::visit_string_value( StringValue& lit )
{
  emit.value( lit.value );
}

void InstructionGenerator::visit_value_consumer( ValueConsumer& node )
{
  visit_children( node );

  emit.consume();
}

}  // namespace Pol::Bscript::Compiler
