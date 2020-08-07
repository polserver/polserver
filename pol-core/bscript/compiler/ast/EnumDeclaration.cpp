#include "EnumDeclaration.h"

#include <format/format.h>
#include <utility>

#include "Expression.h"
#include "NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
EnumDeclaration::EnumDeclaration( const SourceLocation& source_location, std::string identifier,
                                  std::vector<std::string> names,
                                  std::vector<std::unique_ptr<Expression>> expressions )
    : Statement( source_location, std::move( expressions ) ),
      identifier( std::move( identifier ) ),
      names( std::move( names ) )
{
}

void EnumDeclaration::accept( NodeVisitor& visitor )
{
  visitor.visit_enum_declaration( *this );
}

void EnumDeclaration::describe_to( fmt::Writer& w ) const
{
  w << "enum-declaration";
}

}  // namespace Pol::Bscript::Compiler
