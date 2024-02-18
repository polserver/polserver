#pragma once

#include "bscript/compiler/file/SourceLocation.h"
#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <memory>
#include <optional>
#include <string>

namespace Pol::Bscript::Compiler
{
struct CommentInfo
{
  std::string text;
  bool linecomment;
  Position pos;
  Position pos_end;
};
// structure to hold one token as string
struct TokenPart
{
  enum Style
  {
    NONE = 0,          // do nothing
    ATTACHED = 1,      // override SPACE of preceding token
    SPACE = 2,         // add a whitespace char after this token
    BREAKPOINT = 4,    // potential linebreak
    FORCED_BREAK = 8,  // force linebreak
  };
  std::string text = {};
  Position pos;
  size_t tokenid = 0;  // TODO use pos
  int lineno = 0;      // todo use pos
  int style = 0;
  size_t group = 0;
  TokenPart() = default;
  TokenPart( std::string&& text, const Position& pos, int style, size_t group )
      : text( std::move( text ) ),
        pos( pos ),
        tokenid( pos.token_index ),
        lineno( pos.line_number ),
        style( style ),
        group( group ){};
};
class PrettifyLineBuilder
{
public:
  PrettifyLineBuilder() = default;
  const std::vector<std::string>& formattedLines() const;

  void setRawLines( std::vector<std::string> rawlines );
  void setComments( std::vector<CommentInfo> comments );
  void setSkipLines( std::vector<Range> skiplines );
  void addPart( TokenPart part );
  void buildLine( size_t current_ident );
  bool finalize();
  const std::vector<TokenPart>& currentTokens() const;

  int closingParenthesisStyle( size_t begin_size );
  int closingBracketStyle( size_t begin_size );
  int openingParenthesisStyle();
  int openingBracketStyle();
  int delimiterStyle();
  int terminatorStyle();
  int assignmentStyle();
  int comparisonStyle();
  int operatorStyle();

private:
  std::vector<std::string> _rawlines = {};
  std::vector<std::string> _lines = {};
  std::vector<TokenPart> _line_parts = {};
  std::vector<CommentInfo> _comments = {};
  std::vector<Range> _skiplines = {};
  int _last_line = 0;
  size_t _currident = 0;
  size_t _currentgroup = 0;
  void mergeRawContent( int nextlineno );
  void mergeComments();
  void mergeCommentsBefore( int nextlineno );
  void addEmptyLines( int line_number );
  void mergeEOFComments();
  std::string identSpacing();
  std::string alignmentSpacing( size_t count );
};
}  // namespace Pol::Bscript::Compiler

template <>
struct fmt::formatter<Pol::Bscript::Compiler::TokenPart> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Bscript::Compiler::TokenPart& t,
                                        fmt::format_context& ctx ) const;
};
