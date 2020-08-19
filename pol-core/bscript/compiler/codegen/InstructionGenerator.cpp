#include "InstructionGenerator.h"

#include "compiler/ast/FloatValue.h"
#include "compiler/ast/ValueConsumer.h"
#include "compiler/codegen/InstructionEmitter.h"
#include "compiler/file/SourceFileIdentifier.h"

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

void InstructionGenerator::visit_value_consumer( ValueConsumer& node )
{
  visit_children( node );

  emit.consume();
}

}  // namespace Pol::Bscript::Compiler
