#include "MethodCall.h"


#include <utility>

#include "bscript/compiler/ast/MethodCallArgumentList.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "compilercfg.h"

namespace Pol::Bscript::Compiler
{
MethodCall::MethodCall( const SourceLocation& source_location, std::unique_ptr<Expression> lhs,
                        std::string methodname,
                        std::unique_ptr<MethodCallArgumentList> argument_list )
    : Expression( source_location ),
      methodname( std::move( methodname ) ),
      known_method( compilercfg.OptimizeObjectMembers
                        ? getKnownObjMethod( this->methodname.c_str() )
                        : nullptr )
{
  children.reserve( 2 );
  children.push_back( std::move( lhs ) );
  children.push_back( std::move( argument_list ) );
}

void MethodCall::accept( NodeVisitor& visitor )
{
  visitor.visit_method_call( *this );
}

void MethodCall::describe_to( std::string& w ) const
{
  fmt::format_to( std::back_inserter( w ), "method-call({})", methodname );
}

unsigned MethodCall::argument_count() const
{
  return static_cast<unsigned>( children.at( 1 )->children.size() );
}

}  // namespace Pol::Bscript::Compiler
