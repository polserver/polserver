#include "StringTreeGenerator.h"

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"

namespace Pol::Bscript::Compiler
{
StringTreeGenerator::StringTreeGenerator() {}

void StringTreeGenerator::visit_children( Node& parent )
{
  if ( parent.source_location.source_file_identifier->index == 0 )
    _tree += parent.to_string_tree();
}
}  // namespace Pol::Bscript::Compiler