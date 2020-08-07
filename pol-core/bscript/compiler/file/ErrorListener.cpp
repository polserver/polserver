#include "ErrorListener.h"

#include "compiler/Profile.h"
#include "compiler/Report.h"

namespace Pol::Bscript::Compiler
{
ErrorListener::ErrorListener( std::string pathname, Profile& profile )
    : pathname( std::move( pathname ) ), profile( profile )
{
}

void ErrorListener::propagate_errors_to( Report& report, const SourceFileIdentifier& ident )
{
  for ( auto& msg : error_messages )
  {
    SourceLocation location( &ident, msg.line_number, msg.char_column );
    report.error( location, msg.message, "\n" );
  }
}

void ErrorListener::syntaxError( antlr4::Recognizer*, antlr4::Token* /*offendingSymbol*/,
                                 size_t line, size_t charPositionInLine, const std::string& msg,
                                 std::exception_ptr )
{
  error_messages.push_back( { msg, line, charPositionInLine+1 } );
}

void ErrorListener::reportAmbiguity( antlr4::Parser* /*recognizer*/, const antlr4::dfa::DFA&,
                                     size_t /*startIndex*/, size_t /*stopIndex*/, bool /*exact*/,
                                     const antlrcpp::BitSet& /*ambigAlts*/,
                                     antlr4::atn::ATNConfigSet* /*configs*/ )
{
  profile.ambiguities.fetch_add( 1 );
}

}  // namespace Pol::Bscript::Compiler
