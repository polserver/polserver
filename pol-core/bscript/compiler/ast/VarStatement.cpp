#include "VarStatement.h"


#include <utility>

#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
VarStatement::VarStatement( const SourceLocation& identifier_location,
                            const SourceLocation& var_decl_location, std::string scope,
                            std::string name, std::unique_ptr<Expression> initializer )
    : Statement( source_location, std::move( initializer ) ),
      scope( std::move( scope ) ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

VarStatement::VarStatement( const SourceLocation& identifier_location,
                            const SourceLocation& var_decl_location, std::string scope,
                            std::string name )
    : Statement( source_location ),
      scope( std::move( scope ) ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

VarStatement::VarStatement( const SourceLocation& identifier_location,
                            const SourceLocation& var_decl_location, std::string scope,
                            std::string name, bool initialize_as_empty_array )
    : Statement( source_location ),
      scope( std::move( scope ) ),
      name( std::move( name ) ),
      var_decl_location( var_decl_location )
{
}

void VarStatement::accept( NodeVisitor& visitor )
{
  visitor.visit_var_statement( *this );
}

void VarStatement::describe_to( std::string& w ) const
{
  auto scope_prefix = scope.empty() ? "" : fmt::format( "{}::", scope );
  fmt::format_to( std::back_inserter( w ), "var-statement({}{}", scope_prefix, name );
  if ( initialize_as_empty_array )
    w += ", initialize-as-empty-array";
  w += ")";
}

}  // namespace Pol::Bscript::Compiler
