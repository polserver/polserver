#include "bscript/compiler/model/SemanticTokens.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/model/Variable.h"
#include <EscriptGrammar/EscriptLexer.h>

namespace Pol::Bscript::Compiler
{
void SemanticToken::append_from_lexer_token( const antlr4::Token& token, SemanticTokens& tokens )
{
  auto add = [&]( SemanticTokenType type, std::vector<SemanticTokenModifier> modifiers = {} )
  {
    tokens.push_back( SemanticToken{ token.getLine(), token.getCharPositionInLine() + 1,
                                     token.getText().length(), type, std::move( modifiers ) } );
  };

  switch ( token.getType() )
  {
  case EscriptGrammar::EscriptLexer::IF:
  case EscriptGrammar::EscriptLexer::THEN:
  case EscriptGrammar::EscriptLexer::ELSEIF:
  case EscriptGrammar::EscriptLexer::ENDIF:
  case EscriptGrammar::EscriptLexer::ELSE:
  case EscriptGrammar::EscriptLexer::GOTO:
  case EscriptGrammar::EscriptLexer::RETURN:
  case EscriptGrammar::EscriptLexer::TOK_CONST:
  case EscriptGrammar::EscriptLexer::VAR:
  case EscriptGrammar::EscriptLexer::DO:
  case EscriptGrammar::EscriptLexer::DOWHILE:
  case EscriptGrammar::EscriptLexer::WHILE:
  case EscriptGrammar::EscriptLexer::ENDWHILE:
  case EscriptGrammar::EscriptLexer::EXIT:
  case EscriptGrammar::EscriptLexer::FUNCTION:
  case EscriptGrammar::EscriptLexer::ENDFUNCTION:
  case EscriptGrammar::EscriptLexer::EXPORTED:
  case EscriptGrammar::EscriptLexer::USE:
  case EscriptGrammar::EscriptLexer::INCLUDE:
  case EscriptGrammar::EscriptLexer::BREAK:
  case EscriptGrammar::EscriptLexer::CONTINUE:
  case EscriptGrammar::EscriptLexer::FOR:
  case EscriptGrammar::EscriptLexer::ENDFOR:
  case EscriptGrammar::EscriptLexer::TO:
  case EscriptGrammar::EscriptLexer::FOREACH:
  case EscriptGrammar::EscriptLexer::ENDFOREACH:
  case EscriptGrammar::EscriptLexer::REPEAT:
  case EscriptGrammar::EscriptLexer::UNTIL:
  case EscriptGrammar::EscriptLexer::PROGRAM:
  case EscriptGrammar::EscriptLexer::ENDPROGRAM:
  case EscriptGrammar::EscriptLexer::CASE:
  case EscriptGrammar::EscriptLexer::DEFAULT:
  case EscriptGrammar::EscriptLexer::ENDCASE:
  case EscriptGrammar::EscriptLexer::ENUM:
  case EscriptGrammar::EscriptLexer::ENDENUM:
  case EscriptGrammar::EscriptLexer::DOWNTO:
  case EscriptGrammar::EscriptLexer::STEP:
  case EscriptGrammar::EscriptLexer::REFERENCE:
  case EscriptGrammar::EscriptLexer::TOK_OUT:
  case EscriptGrammar::EscriptLexer::INOUT:
  case EscriptGrammar::EscriptLexer::BYVAL:
  case EscriptGrammar::EscriptLexer::STRING:
  case EscriptGrammar::EscriptLexer::TOK_LONG:
  case EscriptGrammar::EscriptLexer::INTEGER:
  case EscriptGrammar::EscriptLexer::UNSIGNED:
  case EscriptGrammar::EscriptLexer::SIGNED:
  case EscriptGrammar::EscriptLexer::REAL:
  case EscriptGrammar::EscriptLexer::FLOAT:
  case EscriptGrammar::EscriptLexer::DOUBLE:
  case EscriptGrammar::EscriptLexer::AS:
  case EscriptGrammar::EscriptLexer::IS:
  case EscriptGrammar::EscriptLexer::CLASS:
  case EscriptGrammar::EscriptLexer::ENDCLASS:
    add( SemanticTokenType::KEYWORD );
    return;
  case EscriptGrammar::EscriptLexer::AND_A:
  case EscriptGrammar::EscriptLexer::AND_B:
  case EscriptGrammar::EscriptLexer::OR_A:
  case EscriptGrammar::EscriptLexer::OR_B:
  case EscriptGrammar::EscriptLexer::BANG_A:
  case EscriptGrammar::EscriptLexer::BANG_B:
  case EscriptGrammar::EscriptLexer::BYREF:
  case EscriptGrammar::EscriptLexer::UNUSED:
  case EscriptGrammar::EscriptLexer::TOK_ERROR:
  case EscriptGrammar::EscriptLexer::HASH:
  case EscriptGrammar::EscriptLexer::DICTIONARY:
  case EscriptGrammar::EscriptLexer::STRUCT:
  case EscriptGrammar::EscriptLexer::ARRAY:
  case EscriptGrammar::EscriptLexer::STACK:
  case EscriptGrammar::EscriptLexer::TOK_IN:
  case EscriptGrammar::EscriptLexer::DOT:
  case EscriptGrammar::EscriptLexer::ARROW:
  case EscriptGrammar::EscriptLexer::MUL:
  case EscriptGrammar::EscriptLexer::DIV:
  case EscriptGrammar::EscriptLexer::MOD:
  case EscriptGrammar::EscriptLexer::ADD:
  case EscriptGrammar::EscriptLexer::SUB:
  case EscriptGrammar::EscriptLexer::ADD_ASSIGN:
  case EscriptGrammar::EscriptLexer::SUB_ASSIGN:
  case EscriptGrammar::EscriptLexer::MUL_ASSIGN:
  case EscriptGrammar::EscriptLexer::DIV_ASSIGN:
  case EscriptGrammar::EscriptLexer::MOD_ASSIGN:
  case EscriptGrammar::EscriptLexer::LE:
  case EscriptGrammar::EscriptLexer::LT:
  case EscriptGrammar::EscriptLexer::GE:
  case EscriptGrammar::EscriptLexer::GT:
  case EscriptGrammar::EscriptLexer::RSHIFT:
  case EscriptGrammar::EscriptLexer::LSHIFT:
  case EscriptGrammar::EscriptLexer::BITAND:
  case EscriptGrammar::EscriptLexer::CARET:
  case EscriptGrammar::EscriptLexer::BITOR:
  case EscriptGrammar::EscriptLexer::NOTEQUAL_A:
  case EscriptGrammar::EscriptLexer::NOTEQUAL_B:
  case EscriptGrammar::EscriptLexer::EQUAL_DEPRECATED:
  case EscriptGrammar::EscriptLexer::EQUAL:
  case EscriptGrammar::EscriptLexer::ASSIGN:
  case EscriptGrammar::EscriptLexer::ADDMEMBER:
  case EscriptGrammar::EscriptLexer::DELMEMBER:
  case EscriptGrammar::EscriptLexer::CHKMEMBER:
  case EscriptGrammar::EscriptLexer::SEMI:
  case EscriptGrammar::EscriptLexer::COMMA:
  case EscriptGrammar::EscriptLexer::TILDE:
  case EscriptGrammar::EscriptLexer::AT:
  case EscriptGrammar::EscriptLexer::COLONCOLON:
  case EscriptGrammar::EscriptLexer::INC:
  case EscriptGrammar::EscriptLexer::DEC:
  case EscriptGrammar::EscriptLexer::ELVIS:
  case EscriptGrammar::EscriptLexer::QUESTION:
    add( SemanticTokenType::OPERATOR );
    return;

  case EscriptGrammar::EscriptLexer::BOOL_FALSE:
  case EscriptGrammar::EscriptLexer::BOOL_TRUE:
  case EscriptGrammar::EscriptLexer::UNINIT:
    add( SemanticTokenType::VARIABLE, { SemanticTokenModifier::READONLY } );
    return;


  case EscriptGrammar::EscriptLexer::DECIMAL_LITERAL:
  case EscriptGrammar::EscriptLexer::HEX_LITERAL:
  case EscriptGrammar::EscriptLexer::OCT_LITERAL:
  case EscriptGrammar::EscriptLexer::BINARY_LITERAL:
  case EscriptGrammar::EscriptLexer::FLOAT_LITERAL:
  case EscriptGrammar::EscriptLexer::HEX_FLOAT_LITERAL:
    add( SemanticTokenType::NUMBER );
    return;
  case EscriptGrammar::EscriptLexer::STRING_LITERAL:
  case EscriptGrammar::EscriptLexer::INTERPOLATED_STRING_START:
  case EscriptGrammar::EscriptLexer::STRING_LITERAL_INSIDE:
  case EscriptGrammar::EscriptLexer::DOUBLE_QUOTE_INSIDE:
  {
    const auto& str = token.getText();
    const auto size = str.size();

    size_t line = token.getLine();
    size_t character = token.getCharPositionInLine() + 1;
    size_t token_length = 0;
    for ( size_t i = 0; i < size; i++ )
    {
      if ( str[i] == '\r' || str[i] == '\n' )
      {
        tokens.push_back(
            SemanticToken{ line, character, token_length, SemanticTokenType::STRING, {} } );
        ++line;
        character = 1;
        token_length = 0;

        if ( str[i] == '\r' && i + 1 < size && str[i + 1] == '\n' )
          ++i;
      }
      else
      {
        ++token_length;
      }
    }
    tokens.push_back(
        SemanticToken{ line, character, token_length, SemanticTokenType::STRING, {} } );
    return;
  }
  case EscriptGrammar::EscriptLexer::REGEXP_LITERAL:
    add( SemanticTokenType::REGEXP );
    return;
  }
}


}  // namespace Pol::Bscript::Compiler