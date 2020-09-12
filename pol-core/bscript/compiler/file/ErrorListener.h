#ifndef POLSERVER_ERRORLISTENER_H
#define POLSERVER_ERRORLISTENER_H

#include <antlr4-runtime.h>

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFileIdentifier;

class ErrorListener : public antlr4::BaseErrorListener
{
public:
  ErrorListener( std::string pathname, Profile& profile );
  ErrorListener( const ErrorListener& ) = delete;
  ErrorListener& operator=( const ErrorListener& ) = delete;

  void propagate_errors_to( Report&, const SourceFileIdentifier& );

  void syntaxError( antlr4::Recognizer* recognizer, antlr4::Token* offendingSymbol, size_t line,
                    size_t charPositionInLine, const std::string& msg,
                    std::exception_ptr e ) override;

  void reportAmbiguity( antlr4::Parser* recognizer, const antlr4::dfa::DFA& dfa, size_t startIndex,
                        size_t stopIndex, bool exact, const antlrcpp::BitSet& ambigAlts,
                        antlr4::atn::ATNConfigSet* configs ) override;

private:
  const std::string pathname;
  Profile& profile;

  int ambiguities = 0;

  struct ErrorMessage {
    const std::string message;
    const size_t line_number;
    const size_t char_column;
  };
  std::vector<ErrorMessage> error_messages;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_ERRORLISTENER_H
