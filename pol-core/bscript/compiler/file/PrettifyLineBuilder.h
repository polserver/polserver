#pragma once

#include "bscript/compiler/file/SourceLocation.h"
#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <memory>
#include <optional>
#include <string>

namespace Pol::Bscript::Compiler
{
// TODO: something like that to set in FileProcessor all needed meta info to define the actual style
enum class FmtContext
{
  NONE = 0,
  LINE_COMMENT,
  COMMENT,
  VAR_STATEMENT,
  VAR_COMMA,

  // unused:
  KEYWORD,  // space
  KEYWORD_BREAKING,
  LITERAL,
  TERMINATOR,                    // terminatorStyle
  OPENING_PARANTHESIS,           //
  OPENING_PARANTHESIS_ATTACHED,  //
  CLOSING_PARANTHESIS,           // closing..
  OPENING_BRACKET,               //
  OPENING_BRACKET_ATTACHED,      //
  CLOSING_BRACKET,               //
  DELIMITER,                     // delimiterStyle
  DELIMITER_NONBREAKING,         // really?
  DELIMITER_SPACE_ATTACHED,      // enum
  ASSIGNMENT,
  OPER_PREFIX,
  OPER_PREFIX_SPACE,
  OPER_POSTFIX,
  BINARY_SPACE_ATTACHED,
  BINARY_ATTACHED,
  BINARY_SPACE_BREAK,  // assignment
  BINARY_ASSIGNMENT,
  BINARY_SPACE,
  BINARY_COMPARISON,
  BINARY_OPERATOR,
  BINARY_ELVIS,
  BINARY_QUESTION,
  SUFFIX_ATTACHED,
  COLON,
  FUNC_REF,
  INTER_STRING,
  INTER_STRING_PART,
  INTER_STRING_END,

};

// structure to hold one token as string
struct FmtToken
{
  enum Style
  {
    NONE = 0,                      // do nothing
    ATTACHED = 1,                  // override SPACE of preceding token
    SPACE = 2,                     // add a whitespace char after this token
    BREAKPOINT = 4,                // potential linebreak
    FORCED_BREAK = 8,              // force linebreak
    PREFERRED_BREAK = 16,          // preferred linebreak , in params
    PREFERRED_BREAK_LOGICAL = 32,  // preferred linebreak eg &&
    PREFERRED_BREAK_VAR = 64,
  };
  enum class Scope : uint8_t
  {
    NONE = 0,
    VAR = 1,
    FUNCTION = 2,
    ARRAY = 4,
    STRUCT = 8,
    DICT = 16,
  };
  std::string text = {};
  Position pos = {};
  Position pos_end = {};
  int style = 0;
  size_t group = 0;
  size_t firstgroup = 0;  // internally set
  size_t token_type = 0;
  FmtContext context = FmtContext::NONE;
  Scope scope = Scope::NONE;
  FmtToken() = default;
  FmtToken( std::string&& text, const Position& pos, int style, size_t group, size_t token_type,
            FmtContext context, Scope scope )
      : text( std::move( text ) ),
        pos( pos ),
        pos_end( pos ),
        style( style ),
        group( group ),
        token_type( token_type ),
        context( context ),
        scope( scope ){};
};

class PrettifyLineBuilder
{
public:
  PrettifyLineBuilder() = default;
  const std::vector<std::string>& formattedLines() const;

  void setRawLines( std::vector<std::string> rawlines );
  void setComments( std::vector<FmtToken> comments );
  void setSkipLines( std::vector<Range> skiplines );
  void addPart( FmtToken part );
  void buildLine( size_t current_indent );
  bool finalize();
  const std::vector<FmtToken>& currentTokens() const;

  int closingParenthesisStyle( size_t begin_size );
  int closingBracketStyle( size_t begin_size );
  int openingParenthesisStyle() const;
  int openingBracketStyle() const;
  int delimiterStyle() const;
  int terminatorStyle() const;
  int assignmentStyle() const;
  int comparisonStyle() const;
  int operatorStyle() const;

private:
  std::vector<std::string> _rawlines = {};
  std::vector<std::string> _lines = {};
  std::vector<FmtToken> _line_parts = {};
  std::vector<FmtToken> _comments = {};
  std::vector<Range> _skiplines = {};
  size_t _last_line = 0;
  size_t _currindent = 0;
  size_t _currentgroup = 0;
  void mergeRawContent( size_t nextlineno );
  void mergeComments();
  void mergeCommentsBefore( size_t nextlineno );
  void addEmptyLines( size_t line_number );
  void mergeEOFNonTokens();
  std::string indentSpacing() const;
  std::string alignmentSpacing( size_t count ) const;
  void stripline( std::string& line ) const;

  std::vector<FmtToken> buildLineSplits();
  std::vector<std::string> createBasedOnGroups( const std::vector<FmtToken>& lines ) const;
  std::vector<std::string> createBasedOnPreferredBreaks( const std::vector<FmtToken>& lines,
                                                         bool logical ) const;
  std::vector<std::string> createSimple( const std::vector<FmtToken>& lines ) const;
  void parenthesisAlign( const std::vector<std::string>& finallines, size_t alignmentspace,
                         std::string& line ) const;
  bool binPack( const FmtToken& part, std::string line, size_t index,
                const std::vector<FmtToken>& lines, bool only_single_line,
                std::vector<std::string>* finallines, std::map<size_t, size_t>* alignmentspace,
                size_t* skipindex ) const;
  void alignComments( std::vector<std::string>& finallines );
};

// operator for enum bitflag handling
constexpr enum FmtToken::Scope operator~( const enum FmtToken::Scope a )
{
  return static_cast<enum FmtToken::Scope>( ~static_cast<uint8_t>( a ) );
}
constexpr enum FmtToken::Scope operator|( const enum FmtToken::Scope a,
                                          const enum FmtToken::Scope b )
{
  return static_cast<enum FmtToken::Scope>( static_cast<uint8_t>( a ) | static_cast<uint8_t>( b ) );
}
constexpr enum FmtToken::Scope operator&( const enum FmtToken::Scope a,
                                          const enum FmtToken::Scope b )
{
  return static_cast<enum FmtToken::Scope>( static_cast<uint8_t>( a ) & static_cast<uint8_t>( b ) );
}
constexpr enum FmtToken::Scope& operator|=( enum FmtToken::Scope& a, const enum FmtToken::Scope b )
{
  a = a | b;
  return a;
}
constexpr enum FmtToken::Scope& operator&=( enum FmtToken::Scope& a, const enum FmtToken::Scope b )
{
  a = a & b;
  return a;
}
}  // namespace Pol::Bscript::Compiler

template <>
struct fmt::formatter<Pol::Bscript::Compiler::FmtToken> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Bscript::Compiler::FmtToken& t,
                                        fmt::format_context& ctx ) const;
};
