#include "AvailableParseTree.h"

#include <fmt/format.h>

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::AvailableParseTree>::format(
    const Pol::Bscript::Compiler::AvailableParseTree& apt, fmt::format_context& ctx ) const
{
  std::string tmp =
      fmt::format( "apt({}, ctx={})", (void*)( &apt ), (void*)( apt.parse_rule_context ) );

  return fmt::formatter<std::string>::format( std::move( tmp ), ctx );
}