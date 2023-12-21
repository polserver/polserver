#include "VarStatement.h"


#include <utility>

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
VarStatement::VarStatement( const SourceLocation& identifier_location,
                            const SourceLocation& var_decl_location, std::string name,
                            std::unique_ptr<Expression> initializer )
    : Statement( identifier_location, std::move( initializer ) ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

VarStatement::VarStatement( const SourceLocation& identifier_location,
                            const SourceLocation& var_decl_location, std::string name )
    : Statement( identifier_location ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

VarStatement::VarStatement( const SourceLocation& identifier_location,
                            const SourceLocation& var_decl_location, std::string name,
                            bool initialize_as_empty_array )
    : Statement( identifier_location ),
      name( std::move( name ) ),
      initialize_as_empty_array( initialize_as_empty_array ),
      var_decl_location( var_decl_location )
{
}

void VarStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_var_statement( *this );
}

void VarStatement::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "var-statement({}", name );
  if ( initialize_as_empty_array )
    w += ", initialize-as-empty-array";
  w += ")";
}

}  // namespace Pol::Bscript::Compiler
