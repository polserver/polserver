#include "FunctionBody.h"

#include <format/format.h>

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/Statement.h"

namespace Pol::Bscript::Compiler
{
FunctionBody::FunctionBody( const SourceLocation& source_location,
                            std::vector<std::unique_ptr<Statement>> statements )
    : Node( source_location, std::move( statements ) )
{
}

void FunctionBody::accept( NodeVisitor& visitor )
{
  visitor.visit_function_body( *this );
}

void FunctionBody::describe_to( fmt::Writer& w ) const
{
  w << "user-function-body";
}

Statement* FunctionBody::last_statement()
{
  return children.empty() ? nullptr : static_cast<Statement*>( children.back().get() );
}

}  // namespace Pol::Bscript::Compiler