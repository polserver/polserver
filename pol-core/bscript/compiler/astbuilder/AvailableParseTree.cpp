#include "AvailableParseTree.h"

#include <fmt/format.h>

namespace Pol::Bscript::Compiler
{
AvailableSecondPassTarget::AvailableSecondPassTarget( const SourceLocation& loc, Type type,
                                                      Node* context )
    : source_location( loc ), type( type ), context( context )
{
}

AvailableGeneratedFunction::AvailableGeneratedFunction( const SourceLocation& loc, Node* context,
                                                        const ScopableName& name,
                                                        UserFunctionType type )
    : AvailableSecondPassTarget( loc, Type::GeneratedFunction, context ), name( name ), type( type )
{
}
AvailableParseTree::AvailableParseTree( const SourceLocation& loc,
                                        antlr4::ParserRuleContext* parse_rule_context,
                                        const ScopeName& scope, Node* context )
    : AvailableSecondPassTarget( loc, Type::ParseTree, context ),
      parse_rule_context( parse_rule_context ),
      scope( scope )
{
}
}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator
fmt::formatter<Pol::Bscript::Compiler::AvailableSecondPassTarget>::format(
    const Pol::Bscript::Compiler::AvailableSecondPassTarget& aspt, fmt::format_context& ctx ) const
{
  if ( aspt.type == Pol::Bscript::Compiler::AvailableSecondPassTarget::Type::ParseTree )
  {
    const Pol::Bscript::Compiler::AvailableParseTree& apt =
        static_cast<const Pol::Bscript::Compiler::AvailableParseTree&>( aspt );

    std::string tmp = fmt::format( "apt({}, scope={}, ctx={})", (void*)( &apt ), apt.scope.string(),
                                   (void*)( apt.parse_rule_context ) );

    return fmt::formatter<std::string>::format( std::move( tmp ), ctx );
  }
  else if ( aspt.type ==
            Pol::Bscript::Compiler::AvailableSecondPassTarget::Type::GeneratedFunction )
  {
    const Pol::Bscript::Compiler::AvailableGeneratedFunction& agf =
        static_cast<const Pol::Bscript::Compiler::AvailableGeneratedFunction&>( aspt );
    std::string tmp = fmt::format( "agf({}, name={})", (void*)( &agf ), agf.name );

    return fmt::formatter<std::string>::format( std::move( tmp ), ctx );
  }

  return fmt::formatter<std::string>::format( "", ctx );
}
