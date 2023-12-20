#include "VarStatement.h"

#include <format/format.h>
#include <utility>

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
VarStatement::VarStatement( const SourceLocation& source_location,
                            const SourceLocation& var_decl_location, std::string name,
                            std::unique_ptr<Expression> initializer )
    : Statement( source_location, std::move( initializer ) ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

VarStatement::VarStatement( const SourceLocation& source_location,
                            const SourceLocation& var_decl_location, std::string name )
    : Statement( source_location ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

VarStatement::VarStatement( const SourceLocation& source_location,
                            const SourceLocation& var_decl_location, std::string name,
                            bool initialize_as_empty_array )
    : Statement( source_location ),
      name( std::move( name ) ),
      initialize_as_empty_array( initialize_as_empty_array ),
      var_decl_location( var_decl_location )
{
}

void VarStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_var_statement( *this );
}

void VarStatement::describe_to( fmt::Writer& w ) const
{
  w << "var-statement(" << name;
  if ( initialize_as_empty_array )
    w << ", initialize-as-empty-array";
  w << ")";
}

}  // namespace Pol::Bscript::Compiler
