#include "ConstDeclaration.h"


#include <utility>

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
ConstDeclaration::ConstDeclaration( const SourceLocation& source_location, ScopableName name,
                                    std::unique_ptr<Expression> expression,
                                    bool ignore_overwrite_attempt )
    : Node( source_location, std::move( expression ) ),
      name( std::move( name ) ),
      ignore_overwrite_attempt( ignore_overwrite_attempt )
{
}

Expression& ConstDeclaration::expression()
{
  return child<Expression>( 0 );
}

void ConstDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_const_declaration( *this );
}

void ConstDeclaration::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "const-declaration({})", name.string() );
}

}  // namespace Pol::Bscript::Compiler
