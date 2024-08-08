


// Generated from EscriptParser.g4 by ANTLR 4.13.1


#include "EscriptParserListener.h"
#include "EscriptParserVisitor.h"

#include "EscriptParser.h"


using namespace antlrcpp;
using namespace EscriptGrammar;

using namespace antlr4;

namespace {

struct EscriptParserStaticData final {
  EscriptParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  EscriptParserStaticData(const EscriptParserStaticData&) = delete;
  EscriptParserStaticData(EscriptParserStaticData&&) = delete;
  EscriptParserStaticData& operator=(const EscriptParserStaticData&) = delete;
  EscriptParserStaticData& operator=(EscriptParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag escriptparserParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
EscriptParserStaticData *escriptparserParserStaticData = nullptr;

void escriptparserParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (escriptparserParserStaticData != nullptr) {
    return;
  }
#else
  assert(escriptparserParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<EscriptParserStaticData>(
    std::vector<std::string>{
      "compilationUnit", "moduleUnit", "evaluateUnit", "moduleDeclarationStatement", 
      "moduleFunctionDeclaration", "moduleFunctionParameterList", "moduleFunctionParameter", 
      "topLevelDeclaration", "classDeclaration", "classParameters", "classParameterList", 
      "classBody", "classStatement", "functionDeclaration", "stringIdentifier", 
      "useDeclaration", "includeDeclaration", "programDeclaration", "statement", 
      "statementLabel", "ifStatement", "gotoStatement", "returnStatement", 
      "constStatement", "varStatement", "doStatement", "whileStatement", 
      "exitStatement", "breakStatement", "continueStatement", "forStatement", 
      "foreachIterableExpression", "foreachStatement", "repeatStatement", 
      "caseStatement", "enumStatement", "block", "variableDeclarationInitializer", 
      "enumList", "enumListEntry", "switchBlockStatementGroup", "switchLabel", 
      "forGroup", "basicForStatement", "cstyleForStatement", "identifierList", 
      "variableDeclarationList", "constantDeclaration", "variableDeclaration", 
      "programParameters", "programParameterList", "programParameter", "functionParameters", 
      "functionParameterList", "functionParameter", "scopedFunctionCall", 
      "functionReference", "expression", "primary", "scopedIdentifier", 
      "functionExpression", "explicitArrayInitializer", "explicitStructInitializer", 
      "explicitDictInitializer", "explicitErrorInitializer", "bareArrayInitializer", 
      "parExpression", "expressionList", "expressionListEntry", "expressionSuffix", 
      "indexingSuffix", "indexList", "navigationSuffix", "methodCallSuffix", 
      "functionCallSuffix", "functionCall", "structInitializerExpression", 
      "structInitializerExpressionList", "structInitializer", "dictInitializerExpression", 
      "dictInitializerExpressionList", "dictInitializer", "arrayInitializer", 
      "literal", "interpolatedString", "interpolatedStringPart", "integerLiteral", 
      "floatLiteral", "boolLiteral"
    },
    std::vector<std::string>{
      "", "'if'", "'then'", "'elseif'", "'endif'", "'else'", "'goto'", "'return'", 
      "'const'", "'var'", "'do'", "'dowhile'", "'while'", "'endwhile'", 
      "'exit'", "'function'", "'endfunction'", "'exported'", "'use'", "'include'", 
      "'break'", "'continue'", "'for'", "'endfor'", "'to'", "'foreach'", 
      "'endforeach'", "'repeat'", "'until'", "'program'", "'endprogram'", 
      "'case'", "'default'", "'endcase'", "'enum'", "'endenum'", "'class'", 
      "'endclass'", "'downto'", "'step'", "'reference'", "'out'", "'inout'", 
      "'ByVal'", "'string'", "'long'", "'integer'", "'unsigned'", "'signed'", 
      "'real'", "'float'", "'double'", "'as'", "'is'", "'...'", "'&&'", 
      "'and'", "'||'", "'or'", "'!'", "'not'", "'byref'", "'unused'", "'error'", 
      "'hash'", "'dictionary'", "'struct'", "'array'", "'stack'", "'in'", 
      "'uninit'", "'true'", "'false'", "", "", "", "", "", "", "", "", "'('", 
      "')'", "'['", "']'", "'{'", "'}'", "'.'", "'->'", "'*'", "'/'", "'%'", 
      "'+'", "'-'", "'+='", "'-='", "'*='", "'/='", "'%='", "'<='", "'<'", 
      "'>='", "'>'", "'>>'", "'<<'", "'&'", "'^'", "'|'", "'<>'", "'!='", 
      "'='", "'=='", "':='", "'.+'", "'.-'", "'.\\u003F'", "';'", "','", 
      "'~'", "'@'", "'::'", "':'", "'++'", "'--'", "'\\u003F:'", "'\\u003F'", 
      "", "", "", "", "'{{'"
    },
    std::vector<std::string>{
      "", "IF", "THEN", "ELSEIF", "ENDIF", "ELSE", "GOTO", "RETURN", "TOK_CONST", 
      "VAR", "DO", "DOWHILE", "WHILE", "ENDWHILE", "EXIT", "FUNCTION", "ENDFUNCTION", 
      "EXPORTED", "USE", "INCLUDE", "BREAK", "CONTINUE", "FOR", "ENDFOR", 
      "TO", "FOREACH", "ENDFOREACH", "REPEAT", "UNTIL", "PROGRAM", "ENDPROGRAM", 
      "CASE", "DEFAULT", "ENDCASE", "ENUM", "ENDENUM", "CLASS", "ENDCLASS", 
      "DOWNTO", "STEP", "REFERENCE", "TOK_OUT", "INOUT", "BYVAL", "STRING", 
      "TOK_LONG", "INTEGER", "UNSIGNED", "SIGNED", "REAL", "FLOAT", "DOUBLE", 
      "AS", "IS", "ELLIPSIS", "AND_A", "AND_B", "OR_A", "OR_B", "BANG_A", 
      "BANG_B", "BYREF", "UNUSED", "TOK_ERROR", "HASH", "DICTIONARY", "STRUCT", 
      "ARRAY", "STACK", "TOK_IN", "UNINIT", "BOOL_TRUE", "BOOL_FALSE", "DECIMAL_LITERAL", 
      "HEX_LITERAL", "OCT_LITERAL", "BINARY_LITERAL", "FLOAT_LITERAL", "HEX_FLOAT_LITERAL", 
      "STRING_LITERAL", "INTERPOLATED_STRING_START", "LPAREN", "RPAREN", 
      "LBRACK", "RBRACK", "LBRACE", "RBRACE", "DOT", "ARROW", "MUL", "DIV", 
      "MOD", "ADD", "SUB", "ADD_ASSIGN", "SUB_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN", 
      "MOD_ASSIGN", "LE", "LT", "GE", "GT", "RSHIFT", "LSHIFT", "BITAND", 
      "CARET", "BITOR", "NOTEQUAL_A", "NOTEQUAL_B", "EQUAL_DEPRECATED", 
      "EQUAL", "ASSIGN", "ADDMEMBER", "DELMEMBER", "CHKMEMBER", "SEMI", 
      "COMMA", "TILDE", "AT", "COLONCOLON", "COLON", "INC", "DEC", "ELVIS", 
      "QUESTION", "WS", "COMMENT", "LINE_COMMENT", "IDENTIFIER", "DOUBLE_LBRACE_INSIDE", 
      "LBRACE_INSIDE", "REGULAR_CHAR_INSIDE", "DOUBLE_QUOTE_INSIDE", "DOUBLE_RBRACE", 
      "STRING_LITERAL_INSIDE", "CLOSE_RBRACE_INSIDE", "FORMAT_STRING"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,137,892,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,2,42,7,
  	42,2,43,7,43,2,44,7,44,2,45,7,45,2,46,7,46,2,47,7,47,2,48,7,48,2,49,7,
  	49,2,50,7,50,2,51,7,51,2,52,7,52,2,53,7,53,2,54,7,54,2,55,7,55,2,56,7,
  	56,2,57,7,57,2,58,7,58,2,59,7,59,2,60,7,60,2,61,7,61,2,62,7,62,2,63,7,
  	63,2,64,7,64,2,65,7,65,2,66,7,66,2,67,7,67,2,68,7,68,2,69,7,69,2,70,7,
  	70,2,71,7,71,2,72,7,72,2,73,7,73,2,74,7,74,2,75,7,75,2,76,7,76,2,77,7,
  	77,2,78,7,78,2,79,7,79,2,80,7,80,2,81,7,81,2,82,7,82,2,83,7,83,2,84,7,
  	84,2,85,7,85,2,86,7,86,2,87,7,87,2,88,7,88,1,0,5,0,180,8,0,10,0,12,0,
  	183,9,0,1,0,1,0,1,1,5,1,188,8,1,10,1,12,1,191,9,1,1,1,1,1,1,2,1,2,1,2,
  	1,3,1,3,3,3,200,8,3,1,4,1,4,1,4,3,4,205,8,4,1,4,1,4,1,4,1,5,1,5,1,5,5,
  	5,213,8,5,10,5,12,5,216,9,5,1,6,1,6,1,6,3,6,221,8,6,1,7,1,7,1,7,1,7,1,
  	7,1,7,3,7,229,8,7,1,8,1,8,1,8,3,8,234,8,8,1,8,1,8,1,8,1,9,1,9,3,9,241,
  	8,9,1,9,1,9,1,10,1,10,1,10,5,10,248,8,10,10,10,12,10,251,9,10,1,11,5,
  	11,254,8,11,10,11,12,11,257,9,11,1,12,1,12,3,12,261,8,12,1,13,3,13,264,
  	8,13,1,13,1,13,1,13,1,13,1,13,1,13,1,14,1,14,1,15,1,15,1,15,1,15,1,16,
  	1,16,1,16,1,16,1,17,1,17,1,17,1,17,1,17,1,17,1,18,1,18,1,18,1,18,1,18,
  	1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,1,18,
  	3,18,307,8,18,1,19,1,19,1,19,1,20,1,20,1,20,3,20,315,8,20,1,20,1,20,1,
  	20,1,20,1,20,5,20,322,8,20,10,20,12,20,325,9,20,1,20,1,20,3,20,329,8,
  	20,1,20,1,20,1,21,1,21,1,21,1,21,1,22,1,22,3,22,339,8,22,1,22,1,22,1,
  	23,1,23,1,23,1,23,1,24,1,24,1,24,1,24,1,25,3,25,352,8,25,1,25,1,25,1,
  	25,1,25,1,25,1,25,1,26,3,26,361,8,26,1,26,1,26,1,26,1,26,1,26,1,27,1,
  	27,1,27,1,28,1,28,3,28,373,8,28,1,28,1,28,1,29,1,29,3,29,379,8,29,1,29,
  	1,29,1,30,3,30,384,8,30,1,30,1,30,1,30,1,30,1,31,1,31,1,31,1,31,1,31,
  	1,31,1,31,3,31,397,8,31,1,32,3,32,400,8,32,1,32,1,32,1,32,1,32,1,32,1,
  	32,1,32,1,33,3,33,410,8,33,1,33,1,33,1,33,1,33,1,33,1,33,1,34,3,34,419,
  	8,34,1,34,1,34,1,34,1,34,1,34,4,34,426,8,34,11,34,12,34,427,1,34,1,34,
  	1,35,1,35,1,35,1,35,1,35,1,36,5,36,438,8,36,10,36,12,36,441,9,36,1,37,
  	1,37,1,37,1,37,1,37,1,37,1,37,3,37,450,8,37,1,38,1,38,1,38,5,38,455,8,
  	38,10,38,12,38,458,9,38,1,38,3,38,461,8,38,1,39,1,39,1,39,3,39,466,8,
  	39,1,40,4,40,469,8,40,11,40,12,40,470,1,40,1,40,1,41,1,41,1,41,1,41,1,
  	41,1,41,3,41,481,8,41,1,41,1,41,1,41,3,41,486,8,41,1,42,1,42,3,42,490,
  	8,42,1,43,1,43,1,43,1,43,1,43,1,43,1,43,1,44,1,44,1,44,1,44,1,44,1,44,
  	1,44,1,44,1,44,1,45,1,45,1,45,3,45,511,8,45,1,46,1,46,1,46,5,46,516,8,
  	46,10,46,12,46,519,9,46,1,47,1,47,1,47,1,48,1,48,3,48,526,8,48,1,49,1,
  	49,3,49,530,8,49,1,49,1,49,1,50,1,50,3,50,536,8,50,1,50,5,50,539,8,50,
  	10,50,12,50,542,9,50,1,51,1,51,1,51,1,51,1,51,3,51,549,8,51,3,51,551,
  	8,51,1,52,1,52,3,52,555,8,52,1,52,1,52,1,53,1,53,1,53,5,53,562,8,53,10,
  	53,12,53,565,9,53,1,54,3,54,568,8,54,1,54,3,54,571,8,54,1,54,1,54,3,54,
  	575,8,54,1,54,1,54,3,54,579,8,54,1,55,1,55,1,55,1,55,1,56,1,56,1,56,3,
  	56,588,8,56,1,56,1,56,1,57,1,57,1,57,1,57,1,57,1,57,3,57,598,8,57,1,57,
  	1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,
  	1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,
  	1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,1,57,
  	1,57,5,57,644,8,57,10,57,12,57,647,9,57,1,58,1,58,1,58,1,58,1,58,1,58,
  	1,58,1,58,1,58,1,58,1,58,1,58,1,58,1,58,3,58,663,8,58,1,59,1,59,1,59,
  	1,59,1,60,1,60,3,60,671,8,60,1,60,1,60,1,60,1,60,1,61,1,61,3,61,679,8,
  	61,1,62,1,62,3,62,683,8,62,1,63,1,63,3,63,687,8,63,1,64,1,64,3,64,691,
  	8,64,1,65,1,65,3,65,695,8,65,1,65,1,65,1,65,3,65,700,8,65,1,65,1,65,1,
  	65,3,65,705,8,65,1,66,1,66,1,66,1,66,1,67,1,67,1,67,5,67,714,8,67,10,
  	67,12,67,717,9,67,1,68,1,68,3,68,721,8,68,1,69,1,69,1,69,1,69,3,69,727,
  	8,69,1,70,1,70,1,70,1,70,1,71,1,71,1,71,5,71,736,8,71,10,71,12,71,739,
  	9,71,1,72,1,72,1,72,1,73,1,73,1,73,1,73,3,73,748,8,73,1,73,1,73,1,74,
  	1,74,3,74,754,8,74,1,74,1,74,1,75,1,75,1,75,3,75,761,8,75,1,75,1,75,1,
  	76,1,76,1,76,3,76,768,8,76,1,76,1,76,1,76,3,76,773,8,76,3,76,775,8,76,
  	1,77,1,77,1,77,5,77,780,8,77,10,77,12,77,783,9,77,1,78,1,78,3,78,787,
  	8,78,1,78,1,78,1,78,3,78,792,8,78,1,78,1,78,1,78,3,78,797,8,78,1,79,1,
  	79,1,79,3,79,802,8,79,1,80,1,80,1,80,5,80,807,8,80,10,80,12,80,810,9,
  	80,1,81,1,81,3,81,814,8,81,1,81,1,81,1,81,3,81,819,8,81,1,81,1,81,1,81,
  	3,81,824,8,81,1,82,1,82,3,82,828,8,82,1,82,1,82,1,82,3,82,833,8,82,1,
  	82,1,82,1,82,1,82,1,82,3,82,840,8,82,1,82,1,82,1,82,3,82,845,8,82,1,82,
  	1,82,1,82,3,82,850,8,82,1,83,1,83,1,83,1,83,1,83,3,83,857,8,83,1,84,1,
  	84,5,84,861,8,84,10,84,12,84,864,9,84,1,84,1,84,1,85,1,85,1,85,1,85,3,
  	85,872,8,85,1,85,1,85,1,85,3,85,877,8,85,1,85,1,85,1,85,1,85,1,85,3,85,
  	884,8,85,1,86,1,86,1,87,1,87,1,88,1,88,1,88,0,1,114,89,0,2,4,6,8,10,12,
  	14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,
  	60,62,64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,
  	106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,
  	142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,
  	0,15,2,0,79,79,129,129,2,0,92,93,122,123,2,0,59,60,118,118,2,0,89,91,
  	103,105,2,0,92,93,106,107,1,0,99,102,2,0,108,109,111,111,1,0,55,56,1,
  	0,57,58,1,0,113,115,2,0,94,98,112,112,1,0,122,123,1,0,73,76,1,0,77,78,
  	1,0,71,72,960,0,181,1,0,0,0,2,189,1,0,0,0,4,194,1,0,0,0,6,199,1,0,0,0,
  	8,201,1,0,0,0,10,209,1,0,0,0,12,217,1,0,0,0,14,228,1,0,0,0,16,230,1,0,
  	0,0,18,238,1,0,0,0,20,244,1,0,0,0,22,255,1,0,0,0,24,260,1,0,0,0,26,263,
  	1,0,0,0,28,271,1,0,0,0,30,273,1,0,0,0,32,277,1,0,0,0,34,281,1,0,0,0,36,
  	306,1,0,0,0,38,308,1,0,0,0,40,311,1,0,0,0,42,332,1,0,0,0,44,336,1,0,0,
  	0,46,342,1,0,0,0,48,346,1,0,0,0,50,351,1,0,0,0,52,360,1,0,0,0,54,367,
  	1,0,0,0,56,370,1,0,0,0,58,376,1,0,0,0,60,383,1,0,0,0,62,396,1,0,0,0,64,
  	399,1,0,0,0,66,409,1,0,0,0,68,418,1,0,0,0,70,431,1,0,0,0,72,439,1,0,0,
  	0,74,449,1,0,0,0,76,451,1,0,0,0,78,462,1,0,0,0,80,468,1,0,0,0,82,485,
  	1,0,0,0,84,489,1,0,0,0,86,491,1,0,0,0,88,498,1,0,0,0,90,507,1,0,0,0,92,
  	512,1,0,0,0,94,520,1,0,0,0,96,523,1,0,0,0,98,527,1,0,0,0,100,533,1,0,
  	0,0,102,550,1,0,0,0,104,552,1,0,0,0,106,558,1,0,0,0,108,567,1,0,0,0,110,
  	580,1,0,0,0,112,584,1,0,0,0,114,597,1,0,0,0,116,662,1,0,0,0,118,664,1,
  	0,0,0,120,668,1,0,0,0,122,676,1,0,0,0,124,680,1,0,0,0,126,684,1,0,0,0,
  	128,688,1,0,0,0,130,704,1,0,0,0,132,706,1,0,0,0,134,710,1,0,0,0,136,718,
  	1,0,0,0,138,726,1,0,0,0,140,728,1,0,0,0,142,732,1,0,0,0,144,740,1,0,0,
  	0,146,743,1,0,0,0,148,751,1,0,0,0,150,757,1,0,0,0,152,774,1,0,0,0,154,
  	776,1,0,0,0,156,796,1,0,0,0,158,798,1,0,0,0,160,803,1,0,0,0,162,823,1,
  	0,0,0,164,849,1,0,0,0,166,856,1,0,0,0,168,858,1,0,0,0,170,883,1,0,0,0,
  	172,885,1,0,0,0,174,887,1,0,0,0,176,889,1,0,0,0,178,180,3,14,7,0,179,
  	178,1,0,0,0,180,183,1,0,0,0,181,179,1,0,0,0,181,182,1,0,0,0,182,184,1,
  	0,0,0,183,181,1,0,0,0,184,185,5,0,0,1,185,1,1,0,0,0,186,188,3,6,3,0,187,
  	186,1,0,0,0,188,191,1,0,0,0,189,187,1,0,0,0,189,190,1,0,0,0,190,192,1,
  	0,0,0,191,189,1,0,0,0,192,193,5,0,0,1,193,3,1,0,0,0,194,195,3,114,57,
  	0,195,196,5,0,0,1,196,5,1,0,0,0,197,200,3,8,4,0,198,200,3,46,23,0,199,
  	197,1,0,0,0,199,198,1,0,0,0,200,7,1,0,0,0,201,202,5,129,0,0,202,204,5,
  	81,0,0,203,205,3,10,5,0,204,203,1,0,0,0,204,205,1,0,0,0,205,206,1,0,0,
  	0,206,207,5,82,0,0,207,208,5,116,0,0,208,9,1,0,0,0,209,214,3,12,6,0,210,
  	211,5,117,0,0,211,213,3,12,6,0,212,210,1,0,0,0,213,216,1,0,0,0,214,212,
  	1,0,0,0,214,215,1,0,0,0,215,11,1,0,0,0,216,214,1,0,0,0,217,220,5,129,
  	0,0,218,219,5,112,0,0,219,221,3,114,57,0,220,218,1,0,0,0,220,221,1,0,
  	0,0,221,13,1,0,0,0,222,229,3,30,15,0,223,229,3,32,16,0,224,229,3,34,17,
  	0,225,229,3,26,13,0,226,229,3,16,8,0,227,229,3,36,18,0,228,222,1,0,0,
  	0,228,223,1,0,0,0,228,224,1,0,0,0,228,225,1,0,0,0,228,226,1,0,0,0,228,
  	227,1,0,0,0,229,15,1,0,0,0,230,231,5,36,0,0,231,233,5,129,0,0,232,234,
  	3,18,9,0,233,232,1,0,0,0,233,234,1,0,0,0,234,235,1,0,0,0,235,236,3,22,
  	11,0,236,237,5,37,0,0,237,17,1,0,0,0,238,240,5,81,0,0,239,241,3,20,10,
  	0,240,239,1,0,0,0,240,241,1,0,0,0,241,242,1,0,0,0,242,243,5,82,0,0,243,
  	19,1,0,0,0,244,249,5,129,0,0,245,246,5,117,0,0,246,248,5,129,0,0,247,
  	245,1,0,0,0,248,251,1,0,0,0,249,247,1,0,0,0,249,250,1,0,0,0,250,21,1,
  	0,0,0,251,249,1,0,0,0,252,254,3,24,12,0,253,252,1,0,0,0,254,257,1,0,0,
  	0,255,253,1,0,0,0,255,256,1,0,0,0,256,23,1,0,0,0,257,255,1,0,0,0,258,
  	261,3,26,13,0,259,261,3,48,24,0,260,258,1,0,0,0,260,259,1,0,0,0,261,25,
  	1,0,0,0,262,264,5,17,0,0,263,262,1,0,0,0,263,264,1,0,0,0,264,265,1,0,
  	0,0,265,266,5,15,0,0,266,267,5,129,0,0,267,268,3,104,52,0,268,269,3,72,
  	36,0,269,270,5,16,0,0,270,27,1,0,0,0,271,272,7,0,0,0,272,29,1,0,0,0,273,
  	274,5,18,0,0,274,275,3,28,14,0,275,276,5,116,0,0,276,31,1,0,0,0,277,278,
  	5,19,0,0,278,279,3,28,14,0,279,280,5,116,0,0,280,33,1,0,0,0,281,282,5,
  	29,0,0,282,283,5,129,0,0,283,284,3,98,49,0,284,285,3,72,36,0,285,286,
  	5,30,0,0,286,35,1,0,0,0,287,307,3,40,20,0,288,307,3,42,21,0,289,307,3,
  	44,22,0,290,307,3,46,23,0,291,307,3,48,24,0,292,307,3,50,25,0,293,307,
  	3,52,26,0,294,307,3,54,27,0,295,307,3,56,28,0,296,307,3,58,29,0,297,307,
  	3,60,30,0,298,307,3,64,32,0,299,307,3,66,33,0,300,307,3,68,34,0,301,307,
  	3,70,35,0,302,307,5,116,0,0,303,304,3,114,57,0,304,305,5,116,0,0,305,
  	307,1,0,0,0,306,287,1,0,0,0,306,288,1,0,0,0,306,289,1,0,0,0,306,290,1,
  	0,0,0,306,291,1,0,0,0,306,292,1,0,0,0,306,293,1,0,0,0,306,294,1,0,0,0,
  	306,295,1,0,0,0,306,296,1,0,0,0,306,297,1,0,0,0,306,298,1,0,0,0,306,299,
  	1,0,0,0,306,300,1,0,0,0,306,301,1,0,0,0,306,302,1,0,0,0,306,303,1,0,0,
  	0,307,37,1,0,0,0,308,309,5,129,0,0,309,310,5,121,0,0,310,39,1,0,0,0,311,
  	312,5,1,0,0,312,314,3,132,66,0,313,315,5,2,0,0,314,313,1,0,0,0,314,315,
  	1,0,0,0,315,316,1,0,0,0,316,323,3,72,36,0,317,318,5,3,0,0,318,319,3,132,
  	66,0,319,320,3,72,36,0,320,322,1,0,0,0,321,317,1,0,0,0,322,325,1,0,0,
  	0,323,321,1,0,0,0,323,324,1,0,0,0,324,328,1,0,0,0,325,323,1,0,0,0,326,
  	327,5,5,0,0,327,329,3,72,36,0,328,326,1,0,0,0,328,329,1,0,0,0,329,330,
  	1,0,0,0,330,331,5,4,0,0,331,41,1,0,0,0,332,333,5,6,0,0,333,334,5,129,
  	0,0,334,335,5,116,0,0,335,43,1,0,0,0,336,338,5,7,0,0,337,339,3,114,57,
  	0,338,337,1,0,0,0,338,339,1,0,0,0,339,340,1,0,0,0,340,341,5,116,0,0,341,
  	45,1,0,0,0,342,343,5,8,0,0,343,344,3,94,47,0,344,345,5,116,0,0,345,47,
  	1,0,0,0,346,347,5,9,0,0,347,348,3,92,46,0,348,349,5,116,0,0,349,49,1,
  	0,0,0,350,352,3,38,19,0,351,350,1,0,0,0,351,352,1,0,0,0,352,353,1,0,0,
  	0,353,354,5,10,0,0,354,355,3,72,36,0,355,356,5,11,0,0,356,357,3,132,66,
  	0,357,358,5,116,0,0,358,51,1,0,0,0,359,361,3,38,19,0,360,359,1,0,0,0,
  	360,361,1,0,0,0,361,362,1,0,0,0,362,363,5,12,0,0,363,364,3,132,66,0,364,
  	365,3,72,36,0,365,366,5,13,0,0,366,53,1,0,0,0,367,368,5,14,0,0,368,369,
  	5,116,0,0,369,55,1,0,0,0,370,372,5,20,0,0,371,373,5,129,0,0,372,371,1,
  	0,0,0,372,373,1,0,0,0,373,374,1,0,0,0,374,375,5,116,0,0,375,57,1,0,0,
  	0,376,378,5,21,0,0,377,379,5,129,0,0,378,377,1,0,0,0,378,379,1,0,0,0,
  	379,380,1,0,0,0,380,381,5,116,0,0,381,59,1,0,0,0,382,384,3,38,19,0,383,
  	382,1,0,0,0,383,384,1,0,0,0,384,385,1,0,0,0,385,386,5,22,0,0,386,387,
  	3,84,42,0,387,388,5,23,0,0,388,61,1,0,0,0,389,397,3,150,75,0,390,397,
  	3,110,55,0,391,397,3,118,59,0,392,397,5,129,0,0,393,397,3,132,66,0,394,
  	397,3,130,65,0,395,397,3,122,61,0,396,389,1,0,0,0,396,390,1,0,0,0,396,
  	391,1,0,0,0,396,392,1,0,0,0,396,393,1,0,0,0,396,394,1,0,0,0,396,395,1,
  	0,0,0,397,63,1,0,0,0,398,400,3,38,19,0,399,398,1,0,0,0,399,400,1,0,0,
  	0,400,401,1,0,0,0,401,402,5,25,0,0,402,403,5,129,0,0,403,404,5,69,0,0,
  	404,405,3,62,31,0,405,406,3,72,36,0,406,407,5,26,0,0,407,65,1,0,0,0,408,
  	410,3,38,19,0,409,408,1,0,0,0,409,410,1,0,0,0,410,411,1,0,0,0,411,412,
  	5,27,0,0,412,413,3,72,36,0,413,414,5,28,0,0,414,415,3,114,57,0,415,416,
  	5,116,0,0,416,67,1,0,0,0,417,419,3,38,19,0,418,417,1,0,0,0,418,419,1,
  	0,0,0,419,420,1,0,0,0,420,421,5,31,0,0,421,422,5,81,0,0,422,423,3,114,
  	57,0,423,425,5,82,0,0,424,426,3,80,40,0,425,424,1,0,0,0,426,427,1,0,0,
  	0,427,425,1,0,0,0,427,428,1,0,0,0,428,429,1,0,0,0,429,430,5,33,0,0,430,
  	69,1,0,0,0,431,432,5,34,0,0,432,433,5,129,0,0,433,434,3,76,38,0,434,435,
  	5,35,0,0,435,71,1,0,0,0,436,438,3,36,18,0,437,436,1,0,0,0,438,441,1,0,
  	0,0,439,437,1,0,0,0,439,440,1,0,0,0,440,73,1,0,0,0,441,439,1,0,0,0,442,
  	443,5,112,0,0,443,450,3,114,57,0,444,445,5,110,0,0,445,446,3,114,57,0,
  	446,447,6,37,-1,0,447,450,1,0,0,0,448,450,5,67,0,0,449,442,1,0,0,0,449,
  	444,1,0,0,0,449,448,1,0,0,0,450,75,1,0,0,0,451,456,3,78,39,0,452,453,
  	5,117,0,0,453,455,3,78,39,0,454,452,1,0,0,0,455,458,1,0,0,0,456,454,1,
  	0,0,0,456,457,1,0,0,0,457,460,1,0,0,0,458,456,1,0,0,0,459,461,5,117,0,
  	0,460,459,1,0,0,0,460,461,1,0,0,0,461,77,1,0,0,0,462,465,5,129,0,0,463,
  	464,5,112,0,0,464,466,3,114,57,0,465,463,1,0,0,0,465,466,1,0,0,0,466,
  	79,1,0,0,0,467,469,3,82,41,0,468,467,1,0,0,0,469,470,1,0,0,0,470,468,
  	1,0,0,0,470,471,1,0,0,0,471,472,1,0,0,0,472,473,3,72,36,0,473,81,1,0,
  	0,0,474,481,3,172,86,0,475,481,3,176,88,0,476,481,3,118,59,0,477,481,
  	5,70,0,0,478,481,5,129,0,0,479,481,5,79,0,0,480,474,1,0,0,0,480,475,1,
  	0,0,0,480,476,1,0,0,0,480,477,1,0,0,0,480,478,1,0,0,0,480,479,1,0,0,0,
  	481,482,1,0,0,0,482,486,5,121,0,0,483,484,5,32,0,0,484,486,5,121,0,0,
  	485,480,1,0,0,0,485,483,1,0,0,0,486,83,1,0,0,0,487,490,3,88,44,0,488,
  	490,3,86,43,0,489,487,1,0,0,0,489,488,1,0,0,0,490,85,1,0,0,0,491,492,
  	5,129,0,0,492,493,5,112,0,0,493,494,3,114,57,0,494,495,5,24,0,0,495,496,
  	3,114,57,0,496,497,3,72,36,0,497,87,1,0,0,0,498,499,5,81,0,0,499,500,
  	3,114,57,0,500,501,5,116,0,0,501,502,3,114,57,0,502,503,5,116,0,0,503,
  	504,3,114,57,0,504,505,5,82,0,0,505,506,3,72,36,0,506,89,1,0,0,0,507,
  	510,5,129,0,0,508,509,5,117,0,0,509,511,3,90,45,0,510,508,1,0,0,0,510,
  	511,1,0,0,0,511,91,1,0,0,0,512,517,3,96,48,0,513,514,5,117,0,0,514,516,
  	3,96,48,0,515,513,1,0,0,0,516,519,1,0,0,0,517,515,1,0,0,0,517,518,1,0,
  	0,0,518,93,1,0,0,0,519,517,1,0,0,0,520,521,5,129,0,0,521,522,3,74,37,
  	0,522,95,1,0,0,0,523,525,5,129,0,0,524,526,3,74,37,0,525,524,1,0,0,0,
  	525,526,1,0,0,0,526,97,1,0,0,0,527,529,5,81,0,0,528,530,3,100,50,0,529,
  	528,1,0,0,0,529,530,1,0,0,0,530,531,1,0,0,0,531,532,5,82,0,0,532,99,1,
  	0,0,0,533,540,3,102,51,0,534,536,5,117,0,0,535,534,1,0,0,0,535,536,1,
  	0,0,0,536,537,1,0,0,0,537,539,3,102,51,0,538,535,1,0,0,0,539,542,1,0,
  	0,0,540,538,1,0,0,0,540,541,1,0,0,0,541,101,1,0,0,0,542,540,1,0,0,0,543,
  	544,5,62,0,0,544,551,5,129,0,0,545,548,5,129,0,0,546,547,5,112,0,0,547,
  	549,3,114,57,0,548,546,1,0,0,0,548,549,1,0,0,0,549,551,1,0,0,0,550,543,
  	1,0,0,0,550,545,1,0,0,0,551,103,1,0,0,0,552,554,5,81,0,0,553,555,3,106,
  	53,0,554,553,1,0,0,0,554,555,1,0,0,0,555,556,1,0,0,0,556,557,5,82,0,0,
  	557,105,1,0,0,0,558,563,3,108,54,0,559,560,5,117,0,0,560,562,3,108,54,
  	0,561,559,1,0,0,0,562,565,1,0,0,0,563,561,1,0,0,0,563,564,1,0,0,0,564,
  	107,1,0,0,0,565,563,1,0,0,0,566,568,5,61,0,0,567,566,1,0,0,0,567,568,
  	1,0,0,0,568,570,1,0,0,0,569,571,5,62,0,0,570,569,1,0,0,0,570,571,1,0,
  	0,0,571,572,1,0,0,0,572,574,5,129,0,0,573,575,5,54,0,0,574,573,1,0,0,
  	0,574,575,1,0,0,0,575,578,1,0,0,0,576,577,5,112,0,0,577,579,3,114,57,
  	0,578,576,1,0,0,0,578,579,1,0,0,0,579,109,1,0,0,0,580,581,5,129,0,0,581,
  	582,5,120,0,0,582,583,3,150,75,0,583,111,1,0,0,0,584,587,5,119,0,0,585,
  	586,5,129,0,0,586,588,5,120,0,0,587,585,1,0,0,0,587,588,1,0,0,0,588,589,
  	1,0,0,0,589,590,5,129,0,0,590,113,1,0,0,0,591,592,6,57,-1,0,592,598,3,
  	116,58,0,593,594,7,1,0,0,594,598,3,114,57,14,595,596,7,2,0,0,596,598,
  	3,114,57,13,597,591,1,0,0,0,597,593,1,0,0,0,597,595,1,0,0,0,598,645,1,
  	0,0,0,599,600,10,12,0,0,600,601,7,3,0,0,601,644,3,114,57,13,602,603,10,
  	11,0,0,603,604,7,4,0,0,604,644,3,114,57,12,605,606,10,10,0,0,606,607,
  	5,124,0,0,607,644,3,114,57,11,608,609,10,9,0,0,609,610,5,69,0,0,610,644,
  	3,114,57,10,611,612,10,8,0,0,612,613,7,5,0,0,613,644,3,114,57,9,614,615,
  	10,7,0,0,615,616,5,110,0,0,616,617,6,57,-1,0,617,644,3,114,57,8,618,619,
  	10,6,0,0,619,620,7,6,0,0,620,644,3,114,57,7,621,622,10,5,0,0,622,623,
  	7,7,0,0,623,644,3,114,57,6,624,625,10,4,0,0,625,626,7,8,0,0,626,644,3,
  	114,57,5,627,628,10,3,0,0,628,629,5,125,0,0,629,630,3,114,57,0,630,631,
  	5,121,0,0,631,632,3,114,57,4,632,644,1,0,0,0,633,634,10,2,0,0,634,635,
  	7,9,0,0,635,644,3,114,57,2,636,637,10,1,0,0,637,638,7,10,0,0,638,644,
  	3,114,57,1,639,640,10,16,0,0,640,644,3,138,69,0,641,642,10,15,0,0,642,
  	644,7,11,0,0,643,599,1,0,0,0,643,602,1,0,0,0,643,605,1,0,0,0,643,608,
  	1,0,0,0,643,611,1,0,0,0,643,614,1,0,0,0,643,618,1,0,0,0,643,621,1,0,0,
  	0,643,624,1,0,0,0,643,627,1,0,0,0,643,633,1,0,0,0,643,636,1,0,0,0,643,
  	639,1,0,0,0,643,641,1,0,0,0,644,647,1,0,0,0,645,643,1,0,0,0,645,646,1,
  	0,0,0,646,115,1,0,0,0,647,645,1,0,0,0,648,663,3,166,83,0,649,663,3,132,
  	66,0,650,663,3,150,75,0,651,663,3,110,55,0,652,663,3,118,59,0,653,663,
  	5,129,0,0,654,663,3,112,56,0,655,663,3,120,60,0,656,663,3,122,61,0,657,
  	663,3,124,62,0,658,663,3,126,63,0,659,663,3,128,64,0,660,663,3,130,65,
  	0,661,663,3,168,84,0,662,648,1,0,0,0,662,649,1,0,0,0,662,650,1,0,0,0,
  	662,651,1,0,0,0,662,652,1,0,0,0,662,653,1,0,0,0,662,654,1,0,0,0,662,655,
  	1,0,0,0,662,656,1,0,0,0,662,657,1,0,0,0,662,658,1,0,0,0,662,659,1,0,0,
  	0,662,660,1,0,0,0,662,661,1,0,0,0,663,117,1,0,0,0,664,665,5,129,0,0,665,
  	666,5,120,0,0,666,667,5,129,0,0,667,119,1,0,0,0,668,670,5,119,0,0,669,
  	671,3,104,52,0,670,669,1,0,0,0,670,671,1,0,0,0,671,672,1,0,0,0,672,673,
  	5,85,0,0,673,674,3,72,36,0,674,675,5,86,0,0,675,121,1,0,0,0,676,678,5,
  	67,0,0,677,679,3,164,82,0,678,677,1,0,0,0,678,679,1,0,0,0,679,123,1,0,
  	0,0,680,682,5,66,0,0,681,683,3,156,78,0,682,681,1,0,0,0,682,683,1,0,0,
  	0,683,125,1,0,0,0,684,686,5,65,0,0,685,687,3,162,81,0,686,685,1,0,0,0,
  	686,687,1,0,0,0,687,127,1,0,0,0,688,690,5,63,0,0,689,691,3,156,78,0,690,
  	689,1,0,0,0,690,691,1,0,0,0,691,129,1,0,0,0,692,694,5,85,0,0,693,695,
  	3,134,67,0,694,693,1,0,0,0,694,695,1,0,0,0,695,696,1,0,0,0,696,705,5,
  	86,0,0,697,699,5,85,0,0,698,700,3,134,67,0,699,698,1,0,0,0,699,700,1,
  	0,0,0,700,701,1,0,0,0,701,702,5,117,0,0,702,703,5,86,0,0,703,705,6,65,
  	-1,0,704,692,1,0,0,0,704,697,1,0,0,0,705,131,1,0,0,0,706,707,5,81,0,0,
  	707,708,3,114,57,0,708,709,5,82,0,0,709,133,1,0,0,0,710,715,3,136,68,
  	0,711,712,5,117,0,0,712,714,3,136,68,0,713,711,1,0,0,0,714,717,1,0,0,
  	0,715,713,1,0,0,0,715,716,1,0,0,0,716,135,1,0,0,0,717,715,1,0,0,0,718,
  	720,3,114,57,0,719,721,5,54,0,0,720,719,1,0,0,0,720,721,1,0,0,0,721,137,
  	1,0,0,0,722,727,3,140,70,0,723,727,3,146,73,0,724,727,3,144,72,0,725,
  	727,3,148,74,0,726,722,1,0,0,0,726,723,1,0,0,0,726,724,1,0,0,0,726,725,
  	1,0,0,0,727,139,1,0,0,0,728,729,5,83,0,0,729,730,3,142,71,0,730,731,5,
  	84,0,0,731,141,1,0,0,0,732,737,3,114,57,0,733,734,5,117,0,0,734,736,3,
  	114,57,0,735,733,1,0,0,0,736,739,1,0,0,0,737,735,1,0,0,0,737,738,1,0,
  	0,0,738,143,1,0,0,0,739,737,1,0,0,0,740,741,5,87,0,0,741,742,7,0,0,0,
  	742,145,1,0,0,0,743,744,5,87,0,0,744,745,5,129,0,0,745,747,5,81,0,0,746,
  	748,3,134,67,0,747,746,1,0,0,0,747,748,1,0,0,0,748,749,1,0,0,0,749,750,
  	5,82,0,0,750,147,1,0,0,0,751,753,5,81,0,0,752,754,3,134,67,0,753,752,
  	1,0,0,0,753,754,1,0,0,0,754,755,1,0,0,0,755,756,5,82,0,0,756,149,1,0,
  	0,0,757,758,5,129,0,0,758,760,5,81,0,0,759,761,3,134,67,0,760,759,1,0,
  	0,0,760,761,1,0,0,0,761,762,1,0,0,0,762,763,5,82,0,0,763,151,1,0,0,0,
  	764,767,5,129,0,0,765,766,5,112,0,0,766,768,3,114,57,0,767,765,1,0,0,
  	0,767,768,1,0,0,0,768,775,1,0,0,0,769,772,5,79,0,0,770,771,5,112,0,0,
  	771,773,3,114,57,0,772,770,1,0,0,0,772,773,1,0,0,0,773,775,1,0,0,0,774,
  	764,1,0,0,0,774,769,1,0,0,0,775,153,1,0,0,0,776,781,3,152,76,0,777,778,
  	5,117,0,0,778,780,3,152,76,0,779,777,1,0,0,0,780,783,1,0,0,0,781,779,
  	1,0,0,0,781,782,1,0,0,0,782,155,1,0,0,0,783,781,1,0,0,0,784,786,5,85,
  	0,0,785,787,3,154,77,0,786,785,1,0,0,0,786,787,1,0,0,0,787,788,1,0,0,
  	0,788,797,5,86,0,0,789,791,5,85,0,0,790,792,3,154,77,0,791,790,1,0,0,
  	0,791,792,1,0,0,0,792,793,1,0,0,0,793,794,5,117,0,0,794,795,5,86,0,0,
  	795,797,6,78,-1,0,796,784,1,0,0,0,796,789,1,0,0,0,797,157,1,0,0,0,798,
  	801,3,114,57,0,799,800,5,88,0,0,800,802,3,114,57,0,801,799,1,0,0,0,801,
  	802,1,0,0,0,802,159,1,0,0,0,803,808,3,158,79,0,804,805,5,117,0,0,805,
  	807,3,158,79,0,806,804,1,0,0,0,807,810,1,0,0,0,808,806,1,0,0,0,808,809,
  	1,0,0,0,809,161,1,0,0,0,810,808,1,0,0,0,811,813,5,85,0,0,812,814,3,160,
  	80,0,813,812,1,0,0,0,813,814,1,0,0,0,814,815,1,0,0,0,815,824,5,86,0,0,
  	816,818,5,85,0,0,817,819,3,160,80,0,818,817,1,0,0,0,818,819,1,0,0,0,819,
  	820,1,0,0,0,820,821,5,117,0,0,821,822,5,86,0,0,822,824,6,81,-1,0,823,
  	811,1,0,0,0,823,816,1,0,0,0,824,163,1,0,0,0,825,827,5,85,0,0,826,828,
  	3,134,67,0,827,826,1,0,0,0,827,828,1,0,0,0,828,829,1,0,0,0,829,850,5,
  	86,0,0,830,832,5,85,0,0,831,833,3,134,67,0,832,831,1,0,0,0,832,833,1,
  	0,0,0,833,834,1,0,0,0,834,835,5,117,0,0,835,836,5,86,0,0,836,850,6,82,
  	-1,0,837,839,5,81,0,0,838,840,3,134,67,0,839,838,1,0,0,0,839,840,1,0,
  	0,0,840,841,1,0,0,0,841,850,5,82,0,0,842,844,5,81,0,0,843,845,3,134,67,
  	0,844,843,1,0,0,0,844,845,1,0,0,0,845,846,1,0,0,0,846,847,5,117,0,0,847,
  	848,5,82,0,0,848,850,6,82,-1,0,849,825,1,0,0,0,849,830,1,0,0,0,849,837,
  	1,0,0,0,849,842,1,0,0,0,850,165,1,0,0,0,851,857,3,172,86,0,852,857,3,
  	174,87,0,853,857,3,176,88,0,854,857,5,79,0,0,855,857,5,70,0,0,856,851,
  	1,0,0,0,856,852,1,0,0,0,856,853,1,0,0,0,856,854,1,0,0,0,856,855,1,0,0,
  	0,857,167,1,0,0,0,858,862,5,80,0,0,859,861,3,170,85,0,860,859,1,0,0,0,
  	861,864,1,0,0,0,862,860,1,0,0,0,862,863,1,0,0,0,863,865,1,0,0,0,864,862,
  	1,0,0,0,865,866,5,133,0,0,866,169,1,0,0,0,867,868,5,131,0,0,868,871,3,
  	114,57,0,869,870,5,121,0,0,870,872,5,137,0,0,871,869,1,0,0,0,871,872,
  	1,0,0,0,872,884,1,0,0,0,873,876,5,131,0,0,874,875,5,121,0,0,875,877,5,
  	137,0,0,876,874,1,0,0,0,876,877,1,0,0,0,877,878,1,0,0,0,878,884,6,85,
  	-1,0,879,884,5,130,0,0,880,884,5,132,0,0,881,884,5,134,0,0,882,884,5,
  	135,0,0,883,867,1,0,0,0,883,873,1,0,0,0,883,879,1,0,0,0,883,880,1,0,0,
  	0,883,881,1,0,0,0,883,882,1,0,0,0,884,171,1,0,0,0,885,886,7,12,0,0,886,
  	173,1,0,0,0,887,888,7,13,0,0,888,175,1,0,0,0,889,890,7,14,0,0,890,177,
  	1,0,0,0,93,181,189,199,204,214,220,228,233,240,249,255,260,263,306,314,
  	323,328,338,351,360,372,378,383,396,399,409,418,427,439,449,456,460,465,
  	470,480,485,489,510,517,525,529,535,540,548,550,554,563,567,570,574,578,
  	587,597,643,645,662,670,678,682,686,690,694,699,704,715,720,726,737,747,
  	753,760,767,772,774,781,786,791,796,801,808,813,818,823,827,832,839,844,
  	849,856,862,871,876,883
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  escriptparserParserStaticData = staticData.release();
}

}

EscriptParser::EscriptParser(TokenStream *input) : EscriptParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

EscriptParser::EscriptParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  EscriptParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *escriptparserParserStaticData->atn, escriptparserParserStaticData->decisionToDFA, escriptparserParserStaticData->sharedContextCache, options);
}

EscriptParser::~EscriptParser() {
  delete _interpreter;
}

const atn::ATN& EscriptParser::getATN() const {
  return *escriptparserParserStaticData->atn;
}

std::string EscriptParser::getGrammarFileName() const {
  return "EscriptParser.g4";
}

const std::vector<std::string>& EscriptParser::getRuleNames() const {
  return escriptparserParserStaticData->ruleNames;
}

const dfa::Vocabulary& EscriptParser::getVocabulary() const {
  return escriptparserParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView EscriptParser::getSerializedATN() const {
  return escriptparserParserStaticData->serializedATN;
}


//----------------- CompilationUnitContext ------------------------------------------------------------------

EscriptParser::CompilationUnitContext::CompilationUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::CompilationUnitContext::EOF() {
  return getToken(EscriptParser::EOF, 0);
}

std::vector<EscriptParser::TopLevelDeclarationContext *> EscriptParser::CompilationUnitContext::topLevelDeclaration() {
  return getRuleContexts<EscriptParser::TopLevelDeclarationContext>();
}

EscriptParser::TopLevelDeclarationContext* EscriptParser::CompilationUnitContext::topLevelDeclaration(size_t i) {
  return getRuleContext<EscriptParser::TopLevelDeclarationContext>(i);
}


size_t EscriptParser::CompilationUnitContext::getRuleIndex() const {
  return EscriptParser::RuleCompilationUnit;
}

void EscriptParser::CompilationUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCompilationUnit(this);
}

void EscriptParser::CompilationUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCompilationUnit(this);
}


std::any EscriptParser::CompilationUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCompilationUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CompilationUnitContext* EscriptParser::compilationUnit() {
  CompilationUnitContext *_localctx = _tracker.createInstance<CompilationUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, EscriptParser::RuleCompilationUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(181);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & -7493989691184719934) != 0) || ((((_la - 65) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 65)) & 461618962209308647) != 0) || _la == EscriptParser::IDENTIFIER) {
      setState(178);
      topLevelDeclaration();
      setState(183);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(184);
    match(EscriptParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModuleUnitContext ------------------------------------------------------------------

EscriptParser::ModuleUnitContext::ModuleUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ModuleUnitContext::EOF() {
  return getToken(EscriptParser::EOF, 0);
}

std::vector<EscriptParser::ModuleDeclarationStatementContext *> EscriptParser::ModuleUnitContext::moduleDeclarationStatement() {
  return getRuleContexts<EscriptParser::ModuleDeclarationStatementContext>();
}

EscriptParser::ModuleDeclarationStatementContext* EscriptParser::ModuleUnitContext::moduleDeclarationStatement(size_t i) {
  return getRuleContext<EscriptParser::ModuleDeclarationStatementContext>(i);
}


size_t EscriptParser::ModuleUnitContext::getRuleIndex() const {
  return EscriptParser::RuleModuleUnit;
}

void EscriptParser::ModuleUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterModuleUnit(this);
}

void EscriptParser::ModuleUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitModuleUnit(this);
}


std::any EscriptParser::ModuleUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleUnitContext* EscriptParser::moduleUnit() {
  ModuleUnitContext *_localctx = _tracker.createInstance<ModuleUnitContext>(_ctx, getState());
  enterRule(_localctx, 2, EscriptParser::RuleModuleUnit);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(189);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::TOK_CONST || _la == EscriptParser::IDENTIFIER) {
      setState(186);
      moduleDeclarationStatement();
      setState(191);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(192);
    match(EscriptParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EvaluateUnitContext ------------------------------------------------------------------

EscriptParser::EvaluateUnitContext::EvaluateUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::ExpressionContext* EscriptParser::EvaluateUnitContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::EvaluateUnitContext::EOF() {
  return getToken(EscriptParser::EOF, 0);
}


size_t EscriptParser::EvaluateUnitContext::getRuleIndex() const {
  return EscriptParser::RuleEvaluateUnit;
}

void EscriptParser::EvaluateUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEvaluateUnit(this);
}

void EscriptParser::EvaluateUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEvaluateUnit(this);
}


std::any EscriptParser::EvaluateUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEvaluateUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EvaluateUnitContext* EscriptParser::evaluateUnit() {
  EvaluateUnitContext *_localctx = _tracker.createInstance<EvaluateUnitContext>(_ctx, getState());
  enterRule(_localctx, 4, EscriptParser::RuleEvaluateUnit);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(194);
    expression(0);
    setState(195);
    match(EscriptParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModuleDeclarationStatementContext ------------------------------------------------------------------

EscriptParser::ModuleDeclarationStatementContext::ModuleDeclarationStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::ModuleFunctionDeclarationContext* EscriptParser::ModuleDeclarationStatementContext::moduleFunctionDeclaration() {
  return getRuleContext<EscriptParser::ModuleFunctionDeclarationContext>(0);
}

EscriptParser::ConstStatementContext* EscriptParser::ModuleDeclarationStatementContext::constStatement() {
  return getRuleContext<EscriptParser::ConstStatementContext>(0);
}


size_t EscriptParser::ModuleDeclarationStatementContext::getRuleIndex() const {
  return EscriptParser::RuleModuleDeclarationStatement;
}

void EscriptParser::ModuleDeclarationStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterModuleDeclarationStatement(this);
}

void EscriptParser::ModuleDeclarationStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitModuleDeclarationStatement(this);
}


std::any EscriptParser::ModuleDeclarationStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleDeclarationStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleDeclarationStatementContext* EscriptParser::moduleDeclarationStatement() {
  ModuleDeclarationStatementContext *_localctx = _tracker.createInstance<ModuleDeclarationStatementContext>(_ctx, getState());
  enterRule(_localctx, 6, EscriptParser::RuleModuleDeclarationStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(199);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(197);
        moduleFunctionDeclaration();
        break;
      }

      case EscriptParser::TOK_CONST: {
        enterOuterAlt(_localctx, 2);
        setState(198);
        constStatement();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModuleFunctionDeclarationContext ------------------------------------------------------------------

EscriptParser::ModuleFunctionDeclarationContext::ModuleFunctionDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ModuleFunctionDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ModuleFunctionDeclarationContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::ModuleFunctionDeclarationContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

tree::TerminalNode* EscriptParser::ModuleFunctionDeclarationContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::ModuleFunctionParameterListContext* EscriptParser::ModuleFunctionDeclarationContext::moduleFunctionParameterList() {
  return getRuleContext<EscriptParser::ModuleFunctionParameterListContext>(0);
}


size_t EscriptParser::ModuleFunctionDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleModuleFunctionDeclaration;
}

void EscriptParser::ModuleFunctionDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterModuleFunctionDeclaration(this);
}

void EscriptParser::ModuleFunctionDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitModuleFunctionDeclaration(this);
}


std::any EscriptParser::ModuleFunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleFunctionDeclarationContext* EscriptParser::moduleFunctionDeclaration() {
  ModuleFunctionDeclarationContext *_localctx = _tracker.createInstance<ModuleFunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 8, EscriptParser::RuleModuleFunctionDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(201);
    match(EscriptParser::IDENTIFIER);
    setState(202);
    match(EscriptParser::LPAREN);
    setState(204);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(203);
      moduleFunctionParameterList();
    }
    setState(206);
    match(EscriptParser::RPAREN);
    setState(207);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModuleFunctionParameterListContext ------------------------------------------------------------------

EscriptParser::ModuleFunctionParameterListContext::ModuleFunctionParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ModuleFunctionParameterContext *> EscriptParser::ModuleFunctionParameterListContext::moduleFunctionParameter() {
  return getRuleContexts<EscriptParser::ModuleFunctionParameterContext>();
}

EscriptParser::ModuleFunctionParameterContext* EscriptParser::ModuleFunctionParameterListContext::moduleFunctionParameter(size_t i) {
  return getRuleContext<EscriptParser::ModuleFunctionParameterContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::ModuleFunctionParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::ModuleFunctionParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::ModuleFunctionParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleModuleFunctionParameterList;
}

void EscriptParser::ModuleFunctionParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterModuleFunctionParameterList(this);
}

void EscriptParser::ModuleFunctionParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitModuleFunctionParameterList(this);
}


std::any EscriptParser::ModuleFunctionParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleFunctionParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleFunctionParameterListContext* EscriptParser::moduleFunctionParameterList() {
  ModuleFunctionParameterListContext *_localctx = _tracker.createInstance<ModuleFunctionParameterListContext>(_ctx, getState());
  enterRule(_localctx, 10, EscriptParser::RuleModuleFunctionParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(209);
    moduleFunctionParameter();
    setState(214);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(210);
      match(EscriptParser::COMMA);
      setState(211);
      moduleFunctionParameter();
      setState(216);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ModuleFunctionParameterContext ------------------------------------------------------------------

EscriptParser::ModuleFunctionParameterContext::ModuleFunctionParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ModuleFunctionParameterContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ModuleFunctionParameterContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::ModuleFunctionParameterContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}


size_t EscriptParser::ModuleFunctionParameterContext::getRuleIndex() const {
  return EscriptParser::RuleModuleFunctionParameter;
}

void EscriptParser::ModuleFunctionParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterModuleFunctionParameter(this);
}

void EscriptParser::ModuleFunctionParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitModuleFunctionParameter(this);
}


std::any EscriptParser::ModuleFunctionParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleFunctionParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleFunctionParameterContext* EscriptParser::moduleFunctionParameter() {
  ModuleFunctionParameterContext *_localctx = _tracker.createInstance<ModuleFunctionParameterContext>(_ctx, getState());
  enterRule(_localctx, 12, EscriptParser::RuleModuleFunctionParameter);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(217);
    match(EscriptParser::IDENTIFIER);
    setState(220);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(218);
      match(EscriptParser::ASSIGN);
      setState(219);
      expression(0);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TopLevelDeclarationContext ------------------------------------------------------------------

EscriptParser::TopLevelDeclarationContext::TopLevelDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::UseDeclarationContext* EscriptParser::TopLevelDeclarationContext::useDeclaration() {
  return getRuleContext<EscriptParser::UseDeclarationContext>(0);
}

EscriptParser::IncludeDeclarationContext* EscriptParser::TopLevelDeclarationContext::includeDeclaration() {
  return getRuleContext<EscriptParser::IncludeDeclarationContext>(0);
}

EscriptParser::ProgramDeclarationContext* EscriptParser::TopLevelDeclarationContext::programDeclaration() {
  return getRuleContext<EscriptParser::ProgramDeclarationContext>(0);
}

EscriptParser::FunctionDeclarationContext* EscriptParser::TopLevelDeclarationContext::functionDeclaration() {
  return getRuleContext<EscriptParser::FunctionDeclarationContext>(0);
}

EscriptParser::ClassDeclarationContext* EscriptParser::TopLevelDeclarationContext::classDeclaration() {
  return getRuleContext<EscriptParser::ClassDeclarationContext>(0);
}

EscriptParser::StatementContext* EscriptParser::TopLevelDeclarationContext::statement() {
  return getRuleContext<EscriptParser::StatementContext>(0);
}


size_t EscriptParser::TopLevelDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleTopLevelDeclaration;
}

void EscriptParser::TopLevelDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTopLevelDeclaration(this);
}

void EscriptParser::TopLevelDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTopLevelDeclaration(this);
}


std::any EscriptParser::TopLevelDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitTopLevelDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::TopLevelDeclarationContext* EscriptParser::topLevelDeclaration() {
  TopLevelDeclarationContext *_localctx = _tracker.createInstance<TopLevelDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 14, EscriptParser::RuleTopLevelDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(228);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::USE: {
        enterOuterAlt(_localctx, 1);
        setState(222);
        useDeclaration();
        break;
      }

      case EscriptParser::INCLUDE: {
        enterOuterAlt(_localctx, 2);
        setState(223);
        includeDeclaration();
        break;
      }

      case EscriptParser::PROGRAM: {
        enterOuterAlt(_localctx, 3);
        setState(224);
        programDeclaration();
        break;
      }

      case EscriptParser::FUNCTION:
      case EscriptParser::EXPORTED: {
        enterOuterAlt(_localctx, 4);
        setState(225);
        functionDeclaration();
        break;
      }

      case EscriptParser::CLASS: {
        enterOuterAlt(_localctx, 5);
        setState(226);
        classDeclaration();
        break;
      }

      case EscriptParser::IF:
      case EscriptParser::GOTO:
      case EscriptParser::RETURN:
      case EscriptParser::TOK_CONST:
      case EscriptParser::VAR:
      case EscriptParser::DO:
      case EscriptParser::WHILE:
      case EscriptParser::EXIT:
      case EscriptParser::BREAK:
      case EscriptParser::CONTINUE:
      case EscriptParser::FOR:
      case EscriptParser::FOREACH:
      case EscriptParser::REPEAT:
      case EscriptParser::CASE:
      case EscriptParser::ENUM:
      case EscriptParser::BANG_A:
      case EscriptParser::BANG_B:
      case EscriptParser::TOK_ERROR:
      case EscriptParser::DICTIONARY:
      case EscriptParser::STRUCT:
      case EscriptParser::ARRAY:
      case EscriptParser::UNINIT:
      case EscriptParser::BOOL_TRUE:
      case EscriptParser::BOOL_FALSE:
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::INTERPOLATED_STRING_START:
      case EscriptParser::LPAREN:
      case EscriptParser::LBRACE:
      case EscriptParser::ADD:
      case EscriptParser::SUB:
      case EscriptParser::SEMI:
      case EscriptParser::TILDE:
      case EscriptParser::AT:
      case EscriptParser::INC:
      case EscriptParser::DEC:
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 6);
        setState(227);
        statement();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassDeclarationContext ------------------------------------------------------------------

EscriptParser::ClassDeclarationContext::ClassDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ClassDeclarationContext::CLASS() {
  return getToken(EscriptParser::CLASS, 0);
}

tree::TerminalNode* EscriptParser::ClassDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::ClassBodyContext* EscriptParser::ClassDeclarationContext::classBody() {
  return getRuleContext<EscriptParser::ClassBodyContext>(0);
}

tree::TerminalNode* EscriptParser::ClassDeclarationContext::ENDCLASS() {
  return getToken(EscriptParser::ENDCLASS, 0);
}

EscriptParser::ClassParametersContext* EscriptParser::ClassDeclarationContext::classParameters() {
  return getRuleContext<EscriptParser::ClassParametersContext>(0);
}


size_t EscriptParser::ClassDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleClassDeclaration;
}

void EscriptParser::ClassDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassDeclaration(this);
}

void EscriptParser::ClassDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassDeclaration(this);
}


std::any EscriptParser::ClassDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitClassDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ClassDeclarationContext* EscriptParser::classDeclaration() {
  ClassDeclarationContext *_localctx = _tracker.createInstance<ClassDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 16, EscriptParser::RuleClassDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(230);
    match(EscriptParser::CLASS);
    setState(231);
    match(EscriptParser::IDENTIFIER);
    setState(233);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::LPAREN) {
      setState(232);
      classParameters();
    }
    setState(235);
    classBody();
    setState(236);
    match(EscriptParser::ENDCLASS);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassParametersContext ------------------------------------------------------------------

EscriptParser::ClassParametersContext::ClassParametersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ClassParametersContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::ClassParametersContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::ClassParameterListContext* EscriptParser::ClassParametersContext::classParameterList() {
  return getRuleContext<EscriptParser::ClassParameterListContext>(0);
}


size_t EscriptParser::ClassParametersContext::getRuleIndex() const {
  return EscriptParser::RuleClassParameters;
}

void EscriptParser::ClassParametersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassParameters(this);
}

void EscriptParser::ClassParametersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassParameters(this);
}


std::any EscriptParser::ClassParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitClassParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ClassParametersContext* EscriptParser::classParameters() {
  ClassParametersContext *_localctx = _tracker.createInstance<ClassParametersContext>(_ctx, getState());
  enterRule(_localctx, 18, EscriptParser::RuleClassParameters);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(238);
    match(EscriptParser::LPAREN);
    setState(240);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(239);
      classParameterList();
    }
    setState(242);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassParameterListContext ------------------------------------------------------------------

EscriptParser::ClassParameterListContext::ClassParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> EscriptParser::ClassParameterListContext::IDENTIFIER() {
  return getTokens(EscriptParser::IDENTIFIER);
}

tree::TerminalNode* EscriptParser::ClassParameterListContext::IDENTIFIER(size_t i) {
  return getToken(EscriptParser::IDENTIFIER, i);
}

std::vector<tree::TerminalNode *> EscriptParser::ClassParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::ClassParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::ClassParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleClassParameterList;
}

void EscriptParser::ClassParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassParameterList(this);
}

void EscriptParser::ClassParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassParameterList(this);
}


std::any EscriptParser::ClassParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitClassParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ClassParameterListContext* EscriptParser::classParameterList() {
  ClassParameterListContext *_localctx = _tracker.createInstance<ClassParameterListContext>(_ctx, getState());
  enterRule(_localctx, 20, EscriptParser::RuleClassParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(244);
    match(EscriptParser::IDENTIFIER);
    setState(249);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(245);
      match(EscriptParser::COMMA);
      setState(246);
      match(EscriptParser::IDENTIFIER);
      setState(251);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassBodyContext ------------------------------------------------------------------

EscriptParser::ClassBodyContext::ClassBodyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ClassStatementContext *> EscriptParser::ClassBodyContext::classStatement() {
  return getRuleContexts<EscriptParser::ClassStatementContext>();
}

EscriptParser::ClassStatementContext* EscriptParser::ClassBodyContext::classStatement(size_t i) {
  return getRuleContext<EscriptParser::ClassStatementContext>(i);
}


size_t EscriptParser::ClassBodyContext::getRuleIndex() const {
  return EscriptParser::RuleClassBody;
}

void EscriptParser::ClassBodyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassBody(this);
}

void EscriptParser::ClassBodyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassBody(this);
}


std::any EscriptParser::ClassBodyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitClassBody(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ClassBodyContext* EscriptParser::classBody() {
  ClassBodyContext *_localctx = _tracker.createInstance<ClassBodyContext>(_ctx, getState());
  enterRule(_localctx, 22, EscriptParser::RuleClassBody);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(255);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 164352) != 0)) {
      setState(252);
      classStatement();
      setState(257);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassStatementContext ------------------------------------------------------------------

EscriptParser::ClassStatementContext::ClassStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::FunctionDeclarationContext* EscriptParser::ClassStatementContext::functionDeclaration() {
  return getRuleContext<EscriptParser::FunctionDeclarationContext>(0);
}

EscriptParser::VarStatementContext* EscriptParser::ClassStatementContext::varStatement() {
  return getRuleContext<EscriptParser::VarStatementContext>(0);
}


size_t EscriptParser::ClassStatementContext::getRuleIndex() const {
  return EscriptParser::RuleClassStatement;
}

void EscriptParser::ClassStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassStatement(this);
}

void EscriptParser::ClassStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassStatement(this);
}


std::any EscriptParser::ClassStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitClassStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ClassStatementContext* EscriptParser::classStatement() {
  ClassStatementContext *_localctx = _tracker.createInstance<ClassStatementContext>(_ctx, getState());
  enterRule(_localctx, 24, EscriptParser::RuleClassStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(260);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::FUNCTION:
      case EscriptParser::EXPORTED: {
        enterOuterAlt(_localctx, 1);
        setState(258);
        functionDeclaration();
        break;
      }

      case EscriptParser::VAR: {
        enterOuterAlt(_localctx, 2);
        setState(259);
        varStatement();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionDeclarationContext ------------------------------------------------------------------

EscriptParser::FunctionDeclarationContext::FunctionDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionDeclarationContext::FUNCTION() {
  return getToken(EscriptParser::FUNCTION, 0);
}

tree::TerminalNode* EscriptParser::FunctionDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::FunctionParametersContext* EscriptParser::FunctionDeclarationContext::functionParameters() {
  return getRuleContext<EscriptParser::FunctionParametersContext>(0);
}

EscriptParser::BlockContext* EscriptParser::FunctionDeclarationContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::FunctionDeclarationContext::ENDFUNCTION() {
  return getToken(EscriptParser::ENDFUNCTION, 0);
}

tree::TerminalNode* EscriptParser::FunctionDeclarationContext::EXPORTED() {
  return getToken(EscriptParser::EXPORTED, 0);
}


size_t EscriptParser::FunctionDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionDeclaration;
}

void EscriptParser::FunctionDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionDeclaration(this);
}

void EscriptParser::FunctionDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionDeclaration(this);
}


std::any EscriptParser::FunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionDeclarationContext* EscriptParser::functionDeclaration() {
  FunctionDeclarationContext *_localctx = _tracker.createInstance<FunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 26, EscriptParser::RuleFunctionDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(263);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::EXPORTED) {
      setState(262);
      match(EscriptParser::EXPORTED);
    }
    setState(265);
    match(EscriptParser::FUNCTION);
    setState(266);
    match(EscriptParser::IDENTIFIER);
    setState(267);
    functionParameters();
    setState(268);
    block();
    setState(269);
    match(EscriptParser::ENDFUNCTION);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StringIdentifierContext ------------------------------------------------------------------

EscriptParser::StringIdentifierContext::StringIdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::StringIdentifierContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::StringIdentifierContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}


size_t EscriptParser::StringIdentifierContext::getRuleIndex() const {
  return EscriptParser::RuleStringIdentifier;
}

void EscriptParser::StringIdentifierContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStringIdentifier(this);
}

void EscriptParser::StringIdentifierContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStringIdentifier(this);
}


std::any EscriptParser::StringIdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStringIdentifier(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StringIdentifierContext* EscriptParser::stringIdentifier() {
  StringIdentifierContext *_localctx = _tracker.createInstance<StringIdentifierContext>(_ctx, getState());
  enterRule(_localctx, 28, EscriptParser::RuleStringIdentifier);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(271);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::STRING_LITERAL

    || _la == EscriptParser::IDENTIFIER)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UseDeclarationContext ------------------------------------------------------------------

EscriptParser::UseDeclarationContext::UseDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UseDeclarationContext::USE() {
  return getToken(EscriptParser::USE, 0);
}

EscriptParser::StringIdentifierContext* EscriptParser::UseDeclarationContext::stringIdentifier() {
  return getRuleContext<EscriptParser::StringIdentifierContext>(0);
}

tree::TerminalNode* EscriptParser::UseDeclarationContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::UseDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleUseDeclaration;
}

void EscriptParser::UseDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUseDeclaration(this);
}

void EscriptParser::UseDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUseDeclaration(this);
}


std::any EscriptParser::UseDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUseDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UseDeclarationContext* EscriptParser::useDeclaration() {
  UseDeclarationContext *_localctx = _tracker.createInstance<UseDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 30, EscriptParser::RuleUseDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(273);
    match(EscriptParser::USE);
    setState(274);
    stringIdentifier();
    setState(275);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IncludeDeclarationContext ------------------------------------------------------------------

EscriptParser::IncludeDeclarationContext::IncludeDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::IncludeDeclarationContext::INCLUDE() {
  return getToken(EscriptParser::INCLUDE, 0);
}

EscriptParser::StringIdentifierContext* EscriptParser::IncludeDeclarationContext::stringIdentifier() {
  return getRuleContext<EscriptParser::StringIdentifierContext>(0);
}

tree::TerminalNode* EscriptParser::IncludeDeclarationContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::IncludeDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleIncludeDeclaration;
}

void EscriptParser::IncludeDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIncludeDeclaration(this);
}

void EscriptParser::IncludeDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIncludeDeclaration(this);
}


std::any EscriptParser::IncludeDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIncludeDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IncludeDeclarationContext* EscriptParser::includeDeclaration() {
  IncludeDeclarationContext *_localctx = _tracker.createInstance<IncludeDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 32, EscriptParser::RuleIncludeDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(277);
    match(EscriptParser::INCLUDE);
    setState(278);
    stringIdentifier();
    setState(279);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramDeclarationContext ------------------------------------------------------------------

EscriptParser::ProgramDeclarationContext::ProgramDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ProgramDeclarationContext::PROGRAM() {
  return getToken(EscriptParser::PROGRAM, 0);
}

tree::TerminalNode* EscriptParser::ProgramDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::ProgramParametersContext* EscriptParser::ProgramDeclarationContext::programParameters() {
  return getRuleContext<EscriptParser::ProgramParametersContext>(0);
}

EscriptParser::BlockContext* EscriptParser::ProgramDeclarationContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::ProgramDeclarationContext::ENDPROGRAM() {
  return getToken(EscriptParser::ENDPROGRAM, 0);
}


size_t EscriptParser::ProgramDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleProgramDeclaration;
}

void EscriptParser::ProgramDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgramDeclaration(this);
}

void EscriptParser::ProgramDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgramDeclaration(this);
}


std::any EscriptParser::ProgramDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramDeclarationContext* EscriptParser::programDeclaration() {
  ProgramDeclarationContext *_localctx = _tracker.createInstance<ProgramDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 34, EscriptParser::RuleProgramDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(281);
    match(EscriptParser::PROGRAM);
    setState(282);
    match(EscriptParser::IDENTIFIER);
    setState(283);
    programParameters();
    setState(284);
    block();
    setState(285);
    match(EscriptParser::ENDPROGRAM);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementContext ------------------------------------------------------------------

EscriptParser::StatementContext::StatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::IfStatementContext* EscriptParser::StatementContext::ifStatement() {
  return getRuleContext<EscriptParser::IfStatementContext>(0);
}

EscriptParser::GotoStatementContext* EscriptParser::StatementContext::gotoStatement() {
  return getRuleContext<EscriptParser::GotoStatementContext>(0);
}

EscriptParser::ReturnStatementContext* EscriptParser::StatementContext::returnStatement() {
  return getRuleContext<EscriptParser::ReturnStatementContext>(0);
}

EscriptParser::ConstStatementContext* EscriptParser::StatementContext::constStatement() {
  return getRuleContext<EscriptParser::ConstStatementContext>(0);
}

EscriptParser::VarStatementContext* EscriptParser::StatementContext::varStatement() {
  return getRuleContext<EscriptParser::VarStatementContext>(0);
}

EscriptParser::DoStatementContext* EscriptParser::StatementContext::doStatement() {
  return getRuleContext<EscriptParser::DoStatementContext>(0);
}

EscriptParser::WhileStatementContext* EscriptParser::StatementContext::whileStatement() {
  return getRuleContext<EscriptParser::WhileStatementContext>(0);
}

EscriptParser::ExitStatementContext* EscriptParser::StatementContext::exitStatement() {
  return getRuleContext<EscriptParser::ExitStatementContext>(0);
}

EscriptParser::BreakStatementContext* EscriptParser::StatementContext::breakStatement() {
  return getRuleContext<EscriptParser::BreakStatementContext>(0);
}

EscriptParser::ContinueStatementContext* EscriptParser::StatementContext::continueStatement() {
  return getRuleContext<EscriptParser::ContinueStatementContext>(0);
}

EscriptParser::ForStatementContext* EscriptParser::StatementContext::forStatement() {
  return getRuleContext<EscriptParser::ForStatementContext>(0);
}

EscriptParser::ForeachStatementContext* EscriptParser::StatementContext::foreachStatement() {
  return getRuleContext<EscriptParser::ForeachStatementContext>(0);
}

EscriptParser::RepeatStatementContext* EscriptParser::StatementContext::repeatStatement() {
  return getRuleContext<EscriptParser::RepeatStatementContext>(0);
}

EscriptParser::CaseStatementContext* EscriptParser::StatementContext::caseStatement() {
  return getRuleContext<EscriptParser::CaseStatementContext>(0);
}

EscriptParser::EnumStatementContext* EscriptParser::StatementContext::enumStatement() {
  return getRuleContext<EscriptParser::EnumStatementContext>(0);
}

tree::TerminalNode* EscriptParser::StatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::ExpressionContext* EscriptParser::StatementContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}


size_t EscriptParser::StatementContext::getRuleIndex() const {
  return EscriptParser::RuleStatement;
}

void EscriptParser::StatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStatement(this);
}

void EscriptParser::StatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStatement(this);
}


std::any EscriptParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StatementContext* EscriptParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 36, EscriptParser::RuleStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(306);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 13, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(287);
      ifStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(288);
      gotoStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(289);
      returnStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(290);
      constStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(291);
      varStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(292);
      doStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(293);
      whileStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(294);
      exitStatement();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(295);
      breakStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(296);
      continueStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(297);
      forStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(298);
      foreachStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(299);
      repeatStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(300);
      caseStatement();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(301);
      enumStatement();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(302);
      match(EscriptParser::SEMI);
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(303);
      antlrcpp::downCast<StatementContext *>(_localctx)->statementExpression = expression(0);
      setState(304);
      match(EscriptParser::SEMI);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StatementLabelContext ------------------------------------------------------------------

EscriptParser::StatementLabelContext::StatementLabelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::StatementLabelContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::StatementLabelContext::COLON() {
  return getToken(EscriptParser::COLON, 0);
}


size_t EscriptParser::StatementLabelContext::getRuleIndex() const {
  return EscriptParser::RuleStatementLabel;
}

void EscriptParser::StatementLabelContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStatementLabel(this);
}

void EscriptParser::StatementLabelContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStatementLabel(this);
}


std::any EscriptParser::StatementLabelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStatementLabel(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StatementLabelContext* EscriptParser::statementLabel() {
  StatementLabelContext *_localctx = _tracker.createInstance<StatementLabelContext>(_ctx, getState());
  enterRule(_localctx, 38, EscriptParser::RuleStatementLabel);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(308);
    match(EscriptParser::IDENTIFIER);
    setState(309);
    match(EscriptParser::COLON);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IfStatementContext ------------------------------------------------------------------

EscriptParser::IfStatementContext::IfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::IfStatementContext::IF() {
  return getToken(EscriptParser::IF, 0);
}

std::vector<EscriptParser::ParExpressionContext *> EscriptParser::IfStatementContext::parExpression() {
  return getRuleContexts<EscriptParser::ParExpressionContext>();
}

EscriptParser::ParExpressionContext* EscriptParser::IfStatementContext::parExpression(size_t i) {
  return getRuleContext<EscriptParser::ParExpressionContext>(i);
}

std::vector<EscriptParser::BlockContext *> EscriptParser::IfStatementContext::block() {
  return getRuleContexts<EscriptParser::BlockContext>();
}

EscriptParser::BlockContext* EscriptParser::IfStatementContext::block(size_t i) {
  return getRuleContext<EscriptParser::BlockContext>(i);
}

tree::TerminalNode* EscriptParser::IfStatementContext::ENDIF() {
  return getToken(EscriptParser::ENDIF, 0);
}

tree::TerminalNode* EscriptParser::IfStatementContext::THEN() {
  return getToken(EscriptParser::THEN, 0);
}

std::vector<tree::TerminalNode *> EscriptParser::IfStatementContext::ELSEIF() {
  return getTokens(EscriptParser::ELSEIF);
}

tree::TerminalNode* EscriptParser::IfStatementContext::ELSEIF(size_t i) {
  return getToken(EscriptParser::ELSEIF, i);
}

tree::TerminalNode* EscriptParser::IfStatementContext::ELSE() {
  return getToken(EscriptParser::ELSE, 0);
}


size_t EscriptParser::IfStatementContext::getRuleIndex() const {
  return EscriptParser::RuleIfStatement;
}

void EscriptParser::IfStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIfStatement(this);
}

void EscriptParser::IfStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIfStatement(this);
}


std::any EscriptParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IfStatementContext* EscriptParser::ifStatement() {
  IfStatementContext *_localctx = _tracker.createInstance<IfStatementContext>(_ctx, getState());
  enterRule(_localctx, 40, EscriptParser::RuleIfStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(311);
    match(EscriptParser::IF);
    setState(312);
    parExpression();
    setState(314);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::THEN) {
      setState(313);
      match(EscriptParser::THEN);
    }
    setState(316);
    block();
    setState(323);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::ELSEIF) {
      setState(317);
      match(EscriptParser::ELSEIF);
      setState(318);
      parExpression();
      setState(319);
      block();
      setState(325);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(328);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ELSE) {
      setState(326);
      match(EscriptParser::ELSE);
      setState(327);
      block();
    }
    setState(330);
    match(EscriptParser::ENDIF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GotoStatementContext ------------------------------------------------------------------

EscriptParser::GotoStatementContext::GotoStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::GotoStatementContext::GOTO() {
  return getToken(EscriptParser::GOTO, 0);
}

tree::TerminalNode* EscriptParser::GotoStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::GotoStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::GotoStatementContext::getRuleIndex() const {
  return EscriptParser::RuleGotoStatement;
}

void EscriptParser::GotoStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterGotoStatement(this);
}

void EscriptParser::GotoStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitGotoStatement(this);
}


std::any EscriptParser::GotoStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitGotoStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::GotoStatementContext* EscriptParser::gotoStatement() {
  GotoStatementContext *_localctx = _tracker.createInstance<GotoStatementContext>(_ctx, getState());
  enterRule(_localctx, 42, EscriptParser::RuleGotoStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(332);
    match(EscriptParser::GOTO);
    setState(333);
    match(EscriptParser::IDENTIFIER);
    setState(334);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnStatementContext ------------------------------------------------------------------

EscriptParser::ReturnStatementContext::ReturnStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ReturnStatementContext::RETURN() {
  return getToken(EscriptParser::RETURN, 0);
}

tree::TerminalNode* EscriptParser::ReturnStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::ExpressionContext* EscriptParser::ReturnStatementContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}


size_t EscriptParser::ReturnStatementContext::getRuleIndex() const {
  return EscriptParser::RuleReturnStatement;
}

void EscriptParser::ReturnStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterReturnStatement(this);
}

void EscriptParser::ReturnStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitReturnStatement(this);
}


std::any EscriptParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ReturnStatementContext* EscriptParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 44, EscriptParser::RuleReturnStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(336);
    match(EscriptParser::RETURN);
    setState(338);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 59) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

    || _la == EscriptParser::IDENTIFIER) {
      setState(337);
      expression(0);
    }
    setState(340);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstStatementContext ------------------------------------------------------------------

EscriptParser::ConstStatementContext::ConstStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ConstStatementContext::TOK_CONST() {
  return getToken(EscriptParser::TOK_CONST, 0);
}

EscriptParser::ConstantDeclarationContext* EscriptParser::ConstStatementContext::constantDeclaration() {
  return getRuleContext<EscriptParser::ConstantDeclarationContext>(0);
}

tree::TerminalNode* EscriptParser::ConstStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::ConstStatementContext::getRuleIndex() const {
  return EscriptParser::RuleConstStatement;
}

void EscriptParser::ConstStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstStatement(this);
}

void EscriptParser::ConstStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstStatement(this);
}


std::any EscriptParser::ConstStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitConstStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ConstStatementContext* EscriptParser::constStatement() {
  ConstStatementContext *_localctx = _tracker.createInstance<ConstStatementContext>(_ctx, getState());
  enterRule(_localctx, 46, EscriptParser::RuleConstStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(342);
    match(EscriptParser::TOK_CONST);
    setState(343);
    constantDeclaration();
    setState(344);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VarStatementContext ------------------------------------------------------------------

EscriptParser::VarStatementContext::VarStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::VarStatementContext::VAR() {
  return getToken(EscriptParser::VAR, 0);
}

EscriptParser::VariableDeclarationListContext* EscriptParser::VarStatementContext::variableDeclarationList() {
  return getRuleContext<EscriptParser::VariableDeclarationListContext>(0);
}

tree::TerminalNode* EscriptParser::VarStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::VarStatementContext::getRuleIndex() const {
  return EscriptParser::RuleVarStatement;
}

void EscriptParser::VarStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVarStatement(this);
}

void EscriptParser::VarStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVarStatement(this);
}


std::any EscriptParser::VarStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVarStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VarStatementContext* EscriptParser::varStatement() {
  VarStatementContext *_localctx = _tracker.createInstance<VarStatementContext>(_ctx, getState());
  enterRule(_localctx, 48, EscriptParser::RuleVarStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(346);
    match(EscriptParser::VAR);
    setState(347);
    variableDeclarationList();
    setState(348);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DoStatementContext ------------------------------------------------------------------

EscriptParser::DoStatementContext::DoStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::DoStatementContext::DO() {
  return getToken(EscriptParser::DO, 0);
}

EscriptParser::BlockContext* EscriptParser::DoStatementContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::DoStatementContext::DOWHILE() {
  return getToken(EscriptParser::DOWHILE, 0);
}

EscriptParser::ParExpressionContext* EscriptParser::DoStatementContext::parExpression() {
  return getRuleContext<EscriptParser::ParExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::DoStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::DoStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::DoStatementContext::getRuleIndex() const {
  return EscriptParser::RuleDoStatement;
}

void EscriptParser::DoStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDoStatement(this);
}

void EscriptParser::DoStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDoStatement(this);
}


std::any EscriptParser::DoStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDoStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DoStatementContext* EscriptParser::doStatement() {
  DoStatementContext *_localctx = _tracker.createInstance<DoStatementContext>(_ctx, getState());
  enterRule(_localctx, 50, EscriptParser::RuleDoStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(351);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(350);
      statementLabel();
    }
    setState(353);
    match(EscriptParser::DO);
    setState(354);
    block();
    setState(355);
    match(EscriptParser::DOWHILE);
    setState(356);
    parExpression();
    setState(357);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- WhileStatementContext ------------------------------------------------------------------

EscriptParser::WhileStatementContext::WhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::WhileStatementContext::WHILE() {
  return getToken(EscriptParser::WHILE, 0);
}

EscriptParser::ParExpressionContext* EscriptParser::WhileStatementContext::parExpression() {
  return getRuleContext<EscriptParser::ParExpressionContext>(0);
}

EscriptParser::BlockContext* EscriptParser::WhileStatementContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::WhileStatementContext::ENDWHILE() {
  return getToken(EscriptParser::ENDWHILE, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::WhileStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::WhileStatementContext::getRuleIndex() const {
  return EscriptParser::RuleWhileStatement;
}

void EscriptParser::WhileStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterWhileStatement(this);
}

void EscriptParser::WhileStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitWhileStatement(this);
}


std::any EscriptParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::WhileStatementContext* EscriptParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 52, EscriptParser::RuleWhileStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(360);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(359);
      statementLabel();
    }
    setState(362);
    match(EscriptParser::WHILE);
    setState(363);
    parExpression();
    setState(364);
    block();
    setState(365);
    match(EscriptParser::ENDWHILE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExitStatementContext ------------------------------------------------------------------

EscriptParser::ExitStatementContext::ExitStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ExitStatementContext::EXIT() {
  return getToken(EscriptParser::EXIT, 0);
}

tree::TerminalNode* EscriptParser::ExitStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::ExitStatementContext::getRuleIndex() const {
  return EscriptParser::RuleExitStatement;
}

void EscriptParser::ExitStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExitStatement(this);
}

void EscriptParser::ExitStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExitStatement(this);
}


std::any EscriptParser::ExitStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExitStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExitStatementContext* EscriptParser::exitStatement() {
  ExitStatementContext *_localctx = _tracker.createInstance<ExitStatementContext>(_ctx, getState());
  enterRule(_localctx, 54, EscriptParser::RuleExitStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(367);
    match(EscriptParser::EXIT);
    setState(368);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BreakStatementContext ------------------------------------------------------------------

EscriptParser::BreakStatementContext::BreakStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::BreakStatementContext::BREAK() {
  return getToken(EscriptParser::BREAK, 0);
}

tree::TerminalNode* EscriptParser::BreakStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

tree::TerminalNode* EscriptParser::BreakStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}


size_t EscriptParser::BreakStatementContext::getRuleIndex() const {
  return EscriptParser::RuleBreakStatement;
}

void EscriptParser::BreakStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBreakStatement(this);
}

void EscriptParser::BreakStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBreakStatement(this);
}


std::any EscriptParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BreakStatementContext* EscriptParser::breakStatement() {
  BreakStatementContext *_localctx = _tracker.createInstance<BreakStatementContext>(_ctx, getState());
  enterRule(_localctx, 56, EscriptParser::RuleBreakStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(370);
    match(EscriptParser::BREAK);
    setState(372);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(371);
      match(EscriptParser::IDENTIFIER);
    }
    setState(374);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ContinueStatementContext ------------------------------------------------------------------

EscriptParser::ContinueStatementContext::ContinueStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ContinueStatementContext::CONTINUE() {
  return getToken(EscriptParser::CONTINUE, 0);
}

tree::TerminalNode* EscriptParser::ContinueStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

tree::TerminalNode* EscriptParser::ContinueStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}


size_t EscriptParser::ContinueStatementContext::getRuleIndex() const {
  return EscriptParser::RuleContinueStatement;
}

void EscriptParser::ContinueStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterContinueStatement(this);
}

void EscriptParser::ContinueStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitContinueStatement(this);
}


std::any EscriptParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ContinueStatementContext* EscriptParser::continueStatement() {
  ContinueStatementContext *_localctx = _tracker.createInstance<ContinueStatementContext>(_ctx, getState());
  enterRule(_localctx, 58, EscriptParser::RuleContinueStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(376);
    match(EscriptParser::CONTINUE);
    setState(378);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(377);
      match(EscriptParser::IDENTIFIER);
    }
    setState(380);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForStatementContext ------------------------------------------------------------------

EscriptParser::ForStatementContext::ForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ForStatementContext::FOR() {
  return getToken(EscriptParser::FOR, 0);
}

EscriptParser::ForGroupContext* EscriptParser::ForStatementContext::forGroup() {
  return getRuleContext<EscriptParser::ForGroupContext>(0);
}

tree::TerminalNode* EscriptParser::ForStatementContext::ENDFOR() {
  return getToken(EscriptParser::ENDFOR, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::ForStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::ForStatementContext::getRuleIndex() const {
  return EscriptParser::RuleForStatement;
}

void EscriptParser::ForStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForStatement(this);
}

void EscriptParser::ForStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForStatement(this);
}


std::any EscriptParser::ForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForStatementContext* EscriptParser::forStatement() {
  ForStatementContext *_localctx = _tracker.createInstance<ForStatementContext>(_ctx, getState());
  enterRule(_localctx, 60, EscriptParser::RuleForStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(383);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(382);
      statementLabel();
    }
    setState(385);
    match(EscriptParser::FOR);
    setState(386);
    forGroup();
    setState(387);
    match(EscriptParser::ENDFOR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForeachIterableExpressionContext ------------------------------------------------------------------

EscriptParser::ForeachIterableExpressionContext::ForeachIterableExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::FunctionCallContext* EscriptParser::ForeachIterableExpressionContext::functionCall() {
  return getRuleContext<EscriptParser::FunctionCallContext>(0);
}

EscriptParser::ScopedFunctionCallContext* EscriptParser::ForeachIterableExpressionContext::scopedFunctionCall() {
  return getRuleContext<EscriptParser::ScopedFunctionCallContext>(0);
}

EscriptParser::ScopedIdentifierContext* EscriptParser::ForeachIterableExpressionContext::scopedIdentifier() {
  return getRuleContext<EscriptParser::ScopedIdentifierContext>(0);
}

tree::TerminalNode* EscriptParser::ForeachIterableExpressionContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::ParExpressionContext* EscriptParser::ForeachIterableExpressionContext::parExpression() {
  return getRuleContext<EscriptParser::ParExpressionContext>(0);
}

EscriptParser::BareArrayInitializerContext* EscriptParser::ForeachIterableExpressionContext::bareArrayInitializer() {
  return getRuleContext<EscriptParser::BareArrayInitializerContext>(0);
}

EscriptParser::ExplicitArrayInitializerContext* EscriptParser::ForeachIterableExpressionContext::explicitArrayInitializer() {
  return getRuleContext<EscriptParser::ExplicitArrayInitializerContext>(0);
}


size_t EscriptParser::ForeachIterableExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleForeachIterableExpression;
}

void EscriptParser::ForeachIterableExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForeachIterableExpression(this);
}

void EscriptParser::ForeachIterableExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForeachIterableExpression(this);
}


std::any EscriptParser::ForeachIterableExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForeachIterableExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForeachIterableExpressionContext* EscriptParser::foreachIterableExpression() {
  ForeachIterableExpressionContext *_localctx = _tracker.createInstance<ForeachIterableExpressionContext>(_ctx, getState());
  enterRule(_localctx, 62, EscriptParser::RuleForeachIterableExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(396);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 23, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(389);
      functionCall();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(390);
      scopedFunctionCall();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(391);
      scopedIdentifier();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(392);
      match(EscriptParser::IDENTIFIER);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(393);
      parExpression();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(394);
      bareArrayInitializer();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(395);
      explicitArrayInitializer();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForeachStatementContext ------------------------------------------------------------------

EscriptParser::ForeachStatementContext::ForeachStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ForeachStatementContext::FOREACH() {
  return getToken(EscriptParser::FOREACH, 0);
}

tree::TerminalNode* EscriptParser::ForeachStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ForeachStatementContext::TOK_IN() {
  return getToken(EscriptParser::TOK_IN, 0);
}

EscriptParser::ForeachIterableExpressionContext* EscriptParser::ForeachStatementContext::foreachIterableExpression() {
  return getRuleContext<EscriptParser::ForeachIterableExpressionContext>(0);
}

EscriptParser::BlockContext* EscriptParser::ForeachStatementContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::ForeachStatementContext::ENDFOREACH() {
  return getToken(EscriptParser::ENDFOREACH, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::ForeachStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::ForeachStatementContext::getRuleIndex() const {
  return EscriptParser::RuleForeachStatement;
}

void EscriptParser::ForeachStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForeachStatement(this);
}

void EscriptParser::ForeachStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForeachStatement(this);
}


std::any EscriptParser::ForeachStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForeachStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForeachStatementContext* EscriptParser::foreachStatement() {
  ForeachStatementContext *_localctx = _tracker.createInstance<ForeachStatementContext>(_ctx, getState());
  enterRule(_localctx, 64, EscriptParser::RuleForeachStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(399);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(398);
      statementLabel();
    }
    setState(401);
    match(EscriptParser::FOREACH);
    setState(402);
    match(EscriptParser::IDENTIFIER);
    setState(403);
    match(EscriptParser::TOK_IN);
    setState(404);
    foreachIterableExpression();
    setState(405);
    block();
    setState(406);
    match(EscriptParser::ENDFOREACH);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- RepeatStatementContext ------------------------------------------------------------------

EscriptParser::RepeatStatementContext::RepeatStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::RepeatStatementContext::REPEAT() {
  return getToken(EscriptParser::REPEAT, 0);
}

EscriptParser::BlockContext* EscriptParser::RepeatStatementContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::RepeatStatementContext::UNTIL() {
  return getToken(EscriptParser::UNTIL, 0);
}

EscriptParser::ExpressionContext* EscriptParser::RepeatStatementContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::RepeatStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::RepeatStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::RepeatStatementContext::getRuleIndex() const {
  return EscriptParser::RuleRepeatStatement;
}

void EscriptParser::RepeatStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterRepeatStatement(this);
}

void EscriptParser::RepeatStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitRepeatStatement(this);
}


std::any EscriptParser::RepeatStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitRepeatStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::RepeatStatementContext* EscriptParser::repeatStatement() {
  RepeatStatementContext *_localctx = _tracker.createInstance<RepeatStatementContext>(_ctx, getState());
  enterRule(_localctx, 66, EscriptParser::RuleRepeatStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(409);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(408);
      statementLabel();
    }
    setState(411);
    match(EscriptParser::REPEAT);
    setState(412);
    block();
    setState(413);
    match(EscriptParser::UNTIL);
    setState(414);
    expression(0);
    setState(415);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CaseStatementContext ------------------------------------------------------------------

EscriptParser::CaseStatementContext::CaseStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::CaseStatementContext::CASE() {
  return getToken(EscriptParser::CASE, 0);
}

tree::TerminalNode* EscriptParser::CaseStatementContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::CaseStatementContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::CaseStatementContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

tree::TerminalNode* EscriptParser::CaseStatementContext::ENDCASE() {
  return getToken(EscriptParser::ENDCASE, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::CaseStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}

std::vector<EscriptParser::SwitchBlockStatementGroupContext *> EscriptParser::CaseStatementContext::switchBlockStatementGroup() {
  return getRuleContexts<EscriptParser::SwitchBlockStatementGroupContext>();
}

EscriptParser::SwitchBlockStatementGroupContext* EscriptParser::CaseStatementContext::switchBlockStatementGroup(size_t i) {
  return getRuleContext<EscriptParser::SwitchBlockStatementGroupContext>(i);
}


size_t EscriptParser::CaseStatementContext::getRuleIndex() const {
  return EscriptParser::RuleCaseStatement;
}

void EscriptParser::CaseStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCaseStatement(this);
}

void EscriptParser::CaseStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCaseStatement(this);
}


std::any EscriptParser::CaseStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCaseStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CaseStatementContext* EscriptParser::caseStatement() {
  CaseStatementContext *_localctx = _tracker.createInstance<CaseStatementContext>(_ctx, getState());
  enterRule(_localctx, 68, EscriptParser::RuleCaseStatement);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(418);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(417);
      statementLabel();
    }
    setState(420);
    match(EscriptParser::CASE);
    setState(421);
    match(EscriptParser::LPAREN);
    setState(422);
    expression(0);
    setState(423);
    match(EscriptParser::RPAREN);
    setState(425); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(424);
      switchBlockStatementGroup();
      setState(427); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == EscriptParser::DEFAULT || ((((_la - 70) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 70)) & 576460752303424127) != 0));
    setState(429);
    match(EscriptParser::ENDCASE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumStatementContext ------------------------------------------------------------------

EscriptParser::EnumStatementContext::EnumStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::EnumStatementContext::ENUM() {
  return getToken(EscriptParser::ENUM, 0);
}

tree::TerminalNode* EscriptParser::EnumStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::EnumListContext* EscriptParser::EnumStatementContext::enumList() {
  return getRuleContext<EscriptParser::EnumListContext>(0);
}

tree::TerminalNode* EscriptParser::EnumStatementContext::ENDENUM() {
  return getToken(EscriptParser::ENDENUM, 0);
}


size_t EscriptParser::EnumStatementContext::getRuleIndex() const {
  return EscriptParser::RuleEnumStatement;
}

void EscriptParser::EnumStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnumStatement(this);
}

void EscriptParser::EnumStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnumStatement(this);
}


std::any EscriptParser::EnumStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEnumStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EnumStatementContext* EscriptParser::enumStatement() {
  EnumStatementContext *_localctx = _tracker.createInstance<EnumStatementContext>(_ctx, getState());
  enterRule(_localctx, 70, EscriptParser::RuleEnumStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(431);
    match(EscriptParser::ENUM);
    setState(432);
    match(EscriptParser::IDENTIFIER);
    setState(433);
    enumList();
    setState(434);
    match(EscriptParser::ENDENUM);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BlockContext ------------------------------------------------------------------

EscriptParser::BlockContext::BlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::StatementContext *> EscriptParser::BlockContext::statement() {
  return getRuleContexts<EscriptParser::StatementContext>();
}

EscriptParser::StatementContext* EscriptParser::BlockContext::statement(size_t i) {
  return getRuleContext<EscriptParser::StatementContext>(i);
}


size_t EscriptParser::BlockContext::getRuleIndex() const {
  return EscriptParser::RuleBlock;
}

void EscriptParser::BlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBlock(this);
}

void EscriptParser::BlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBlock(this);
}


std::any EscriptParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BlockContext* EscriptParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 72, EscriptParser::RuleBlock);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(439);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(436);
        statement(); 
      }
      setState(441);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 28, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationInitializerContext ------------------------------------------------------------------

EscriptParser::VariableDeclarationInitializerContext::VariableDeclarationInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::VariableDeclarationInitializerContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::VariableDeclarationInitializerContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::VariableDeclarationInitializerContext::EQUAL_DEPRECATED() {
  return getToken(EscriptParser::EQUAL_DEPRECATED, 0);
}

tree::TerminalNode* EscriptParser::VariableDeclarationInitializerContext::ARRAY() {
  return getToken(EscriptParser::ARRAY, 0);
}


size_t EscriptParser::VariableDeclarationInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleVariableDeclarationInitializer;
}

void EscriptParser::VariableDeclarationInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVariableDeclarationInitializer(this);
}

void EscriptParser::VariableDeclarationInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVariableDeclarationInitializer(this);
}


std::any EscriptParser::VariableDeclarationInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VariableDeclarationInitializerContext* EscriptParser::variableDeclarationInitializer() {
  VariableDeclarationInitializerContext *_localctx = _tracker.createInstance<VariableDeclarationInitializerContext>(_ctx, getState());
  enterRule(_localctx, 74, EscriptParser::RuleVariableDeclarationInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(449);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::ASSIGN: {
        enterOuterAlt(_localctx, 1);
        setState(442);
        match(EscriptParser::ASSIGN);
        setState(443);
        expression(0);
        break;
      }

      case EscriptParser::EQUAL_DEPRECATED: {
        enterOuterAlt(_localctx, 2);
        setState(444);
        match(EscriptParser::EQUAL_DEPRECATED);
        setState(445);
        expression(0);
         notifyErrorListeners("Unexpected token: '='. Did you mean := for assign?\n"); 
        break;
      }

      case EscriptParser::ARRAY: {
        enterOuterAlt(_localctx, 3);
        setState(448);
        match(EscriptParser::ARRAY);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumListContext ------------------------------------------------------------------

EscriptParser::EnumListContext::EnumListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::EnumListEntryContext *> EscriptParser::EnumListContext::enumListEntry() {
  return getRuleContexts<EscriptParser::EnumListEntryContext>();
}

EscriptParser::EnumListEntryContext* EscriptParser::EnumListContext::enumListEntry(size_t i) {
  return getRuleContext<EscriptParser::EnumListEntryContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::EnumListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::EnumListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::EnumListContext::getRuleIndex() const {
  return EscriptParser::RuleEnumList;
}

void EscriptParser::EnumListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnumList(this);
}

void EscriptParser::EnumListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnumList(this);
}


std::any EscriptParser::EnumListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEnumList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EnumListContext* EscriptParser::enumList() {
  EnumListContext *_localctx = _tracker.createInstance<EnumListContext>(_ctx, getState());
  enterRule(_localctx, 76, EscriptParser::RuleEnumList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(451);
    enumListEntry();
    setState(456);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(452);
        match(EscriptParser::COMMA);
        setState(453);
        enumListEntry(); 
      }
      setState(458);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx);
    }
    setState(460);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::COMMA) {
      setState(459);
      match(EscriptParser::COMMA);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- EnumListEntryContext ------------------------------------------------------------------

EscriptParser::EnumListEntryContext::EnumListEntryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::EnumListEntryContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::EnumListEntryContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::EnumListEntryContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}


size_t EscriptParser::EnumListEntryContext::getRuleIndex() const {
  return EscriptParser::RuleEnumListEntry;
}

void EscriptParser::EnumListEntryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEnumListEntry(this);
}

void EscriptParser::EnumListEntryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEnumListEntry(this);
}


std::any EscriptParser::EnumListEntryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEnumListEntry(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EnumListEntryContext* EscriptParser::enumListEntry() {
  EnumListEntryContext *_localctx = _tracker.createInstance<EnumListEntryContext>(_ctx, getState());
  enterRule(_localctx, 78, EscriptParser::RuleEnumListEntry);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(462);
    match(EscriptParser::IDENTIFIER);
    setState(465);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(463);
      match(EscriptParser::ASSIGN);
      setState(464);
      expression(0);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SwitchBlockStatementGroupContext ------------------------------------------------------------------

EscriptParser::SwitchBlockStatementGroupContext::SwitchBlockStatementGroupContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::BlockContext* EscriptParser::SwitchBlockStatementGroupContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

std::vector<EscriptParser::SwitchLabelContext *> EscriptParser::SwitchBlockStatementGroupContext::switchLabel() {
  return getRuleContexts<EscriptParser::SwitchLabelContext>();
}

EscriptParser::SwitchLabelContext* EscriptParser::SwitchBlockStatementGroupContext::switchLabel(size_t i) {
  return getRuleContext<EscriptParser::SwitchLabelContext>(i);
}


size_t EscriptParser::SwitchBlockStatementGroupContext::getRuleIndex() const {
  return EscriptParser::RuleSwitchBlockStatementGroup;
}

void EscriptParser::SwitchBlockStatementGroupContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSwitchBlockStatementGroup(this);
}

void EscriptParser::SwitchBlockStatementGroupContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSwitchBlockStatementGroup(this);
}


std::any EscriptParser::SwitchBlockStatementGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitSwitchBlockStatementGroup(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::SwitchBlockStatementGroupContext* EscriptParser::switchBlockStatementGroup() {
  SwitchBlockStatementGroupContext *_localctx = _tracker.createInstance<SwitchBlockStatementGroupContext>(_ctx, getState());
  enterRule(_localctx, 80, EscriptParser::RuleSwitchBlockStatementGroup);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(468); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(467);
              switchLabel();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(470); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
    setState(472);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SwitchLabelContext ------------------------------------------------------------------

EscriptParser::SwitchLabelContext::SwitchLabelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::SwitchLabelContext::COLON() {
  return getToken(EscriptParser::COLON, 0);
}

EscriptParser::IntegerLiteralContext* EscriptParser::SwitchLabelContext::integerLiteral() {
  return getRuleContext<EscriptParser::IntegerLiteralContext>(0);
}

EscriptParser::BoolLiteralContext* EscriptParser::SwitchLabelContext::boolLiteral() {
  return getRuleContext<EscriptParser::BoolLiteralContext>(0);
}

EscriptParser::ScopedIdentifierContext* EscriptParser::SwitchLabelContext::scopedIdentifier() {
  return getRuleContext<EscriptParser::ScopedIdentifierContext>(0);
}

tree::TerminalNode* EscriptParser::SwitchLabelContext::UNINIT() {
  return getToken(EscriptParser::UNINIT, 0);
}

tree::TerminalNode* EscriptParser::SwitchLabelContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::SwitchLabelContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::SwitchLabelContext::DEFAULT() {
  return getToken(EscriptParser::DEFAULT, 0);
}


size_t EscriptParser::SwitchLabelContext::getRuleIndex() const {
  return EscriptParser::RuleSwitchLabel;
}

void EscriptParser::SwitchLabelContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSwitchLabel(this);
}

void EscriptParser::SwitchLabelContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSwitchLabel(this);
}


std::any EscriptParser::SwitchLabelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitSwitchLabel(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::SwitchLabelContext* EscriptParser::switchLabel() {
  SwitchLabelContext *_localctx = _tracker.createInstance<SwitchLabelContext>(_ctx, getState());
  enterRule(_localctx, 82, EscriptParser::RuleSwitchLabel);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(485);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::UNINIT:
      case EscriptParser::BOOL_TRUE:
      case EscriptParser::BOOL_FALSE:
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(480);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 34, _ctx)) {
        case 1: {
          setState(474);
          integerLiteral();
          break;
        }

        case 2: {
          setState(475);
          boolLiteral();
          break;
        }

        case 3: {
          setState(476);
          scopedIdentifier();
          break;
        }

        case 4: {
          setState(477);
          match(EscriptParser::UNINIT);
          break;
        }

        case 5: {
          setState(478);
          match(EscriptParser::IDENTIFIER);
          break;
        }

        case 6: {
          setState(479);
          match(EscriptParser::STRING_LITERAL);
          break;
        }

        default:
          break;
        }
        setState(482);
        match(EscriptParser::COLON);
        break;
      }

      case EscriptParser::DEFAULT: {
        enterOuterAlt(_localctx, 2);
        setState(483);
        match(EscriptParser::DEFAULT);
        setState(484);
        match(EscriptParser::COLON);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ForGroupContext ------------------------------------------------------------------

EscriptParser::ForGroupContext::ForGroupContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::CstyleForStatementContext* EscriptParser::ForGroupContext::cstyleForStatement() {
  return getRuleContext<EscriptParser::CstyleForStatementContext>(0);
}

EscriptParser::BasicForStatementContext* EscriptParser::ForGroupContext::basicForStatement() {
  return getRuleContext<EscriptParser::BasicForStatementContext>(0);
}


size_t EscriptParser::ForGroupContext::getRuleIndex() const {
  return EscriptParser::RuleForGroup;
}

void EscriptParser::ForGroupContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterForGroup(this);
}

void EscriptParser::ForGroupContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitForGroup(this);
}


std::any EscriptParser::ForGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForGroup(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForGroupContext* EscriptParser::forGroup() {
  ForGroupContext *_localctx = _tracker.createInstance<ForGroupContext>(_ctx, getState());
  enterRule(_localctx, 84, EscriptParser::RuleForGroup);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(489);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::LPAREN: {
        enterOuterAlt(_localctx, 1);
        setState(487);
        cstyleForStatement();
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(488);
        basicForStatement();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BasicForStatementContext ------------------------------------------------------------------

EscriptParser::BasicForStatementContext::BasicForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::BasicForStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::BasicForStatementContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

std::vector<EscriptParser::ExpressionContext *> EscriptParser::BasicForStatementContext::expression() {
  return getRuleContexts<EscriptParser::ExpressionContext>();
}

EscriptParser::ExpressionContext* EscriptParser::BasicForStatementContext::expression(size_t i) {
  return getRuleContext<EscriptParser::ExpressionContext>(i);
}

tree::TerminalNode* EscriptParser::BasicForStatementContext::TO() {
  return getToken(EscriptParser::TO, 0);
}

EscriptParser::BlockContext* EscriptParser::BasicForStatementContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}


size_t EscriptParser::BasicForStatementContext::getRuleIndex() const {
  return EscriptParser::RuleBasicForStatement;
}

void EscriptParser::BasicForStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBasicForStatement(this);
}

void EscriptParser::BasicForStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBasicForStatement(this);
}


std::any EscriptParser::BasicForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBasicForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BasicForStatementContext* EscriptParser::basicForStatement() {
  BasicForStatementContext *_localctx = _tracker.createInstance<BasicForStatementContext>(_ctx, getState());
  enterRule(_localctx, 86, EscriptParser::RuleBasicForStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(491);
    match(EscriptParser::IDENTIFIER);
    setState(492);
    match(EscriptParser::ASSIGN);
    setState(493);
    expression(0);
    setState(494);
    match(EscriptParser::TO);
    setState(495);
    expression(0);
    setState(496);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CstyleForStatementContext ------------------------------------------------------------------

EscriptParser::CstyleForStatementContext::CstyleForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::CstyleForStatementContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

std::vector<EscriptParser::ExpressionContext *> EscriptParser::CstyleForStatementContext::expression() {
  return getRuleContexts<EscriptParser::ExpressionContext>();
}

EscriptParser::ExpressionContext* EscriptParser::CstyleForStatementContext::expression(size_t i) {
  return getRuleContext<EscriptParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::CstyleForStatementContext::SEMI() {
  return getTokens(EscriptParser::SEMI);
}

tree::TerminalNode* EscriptParser::CstyleForStatementContext::SEMI(size_t i) {
  return getToken(EscriptParser::SEMI, i);
}

tree::TerminalNode* EscriptParser::CstyleForStatementContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::BlockContext* EscriptParser::CstyleForStatementContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}


size_t EscriptParser::CstyleForStatementContext::getRuleIndex() const {
  return EscriptParser::RuleCstyleForStatement;
}

void EscriptParser::CstyleForStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCstyleForStatement(this);
}

void EscriptParser::CstyleForStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCstyleForStatement(this);
}


std::any EscriptParser::CstyleForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCstyleForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CstyleForStatementContext* EscriptParser::cstyleForStatement() {
  CstyleForStatementContext *_localctx = _tracker.createInstance<CstyleForStatementContext>(_ctx, getState());
  enterRule(_localctx, 88, EscriptParser::RuleCstyleForStatement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(498);
    match(EscriptParser::LPAREN);
    setState(499);
    expression(0);
    setState(500);
    match(EscriptParser::SEMI);
    setState(501);
    expression(0);
    setState(502);
    match(EscriptParser::SEMI);
    setState(503);
    expression(0);
    setState(504);
    match(EscriptParser::RPAREN);
    setState(505);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IdentifierListContext ------------------------------------------------------------------

EscriptParser::IdentifierListContext::IdentifierListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::IdentifierListContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::IdentifierListContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}

EscriptParser::IdentifierListContext* EscriptParser::IdentifierListContext::identifierList() {
  return getRuleContext<EscriptParser::IdentifierListContext>(0);
}


size_t EscriptParser::IdentifierListContext::getRuleIndex() const {
  return EscriptParser::RuleIdentifierList;
}

void EscriptParser::IdentifierListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIdentifierList(this);
}

void EscriptParser::IdentifierListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIdentifierList(this);
}


std::any EscriptParser::IdentifierListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIdentifierList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IdentifierListContext* EscriptParser::identifierList() {
  IdentifierListContext *_localctx = _tracker.createInstance<IdentifierListContext>(_ctx, getState());
  enterRule(_localctx, 90, EscriptParser::RuleIdentifierList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(507);
    match(EscriptParser::IDENTIFIER);
    setState(510);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx)) {
    case 1: {
      setState(508);
      match(EscriptParser::COMMA);
      setState(509);
      identifierList();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationListContext ------------------------------------------------------------------

EscriptParser::VariableDeclarationListContext::VariableDeclarationListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::VariableDeclarationContext *> EscriptParser::VariableDeclarationListContext::variableDeclaration() {
  return getRuleContexts<EscriptParser::VariableDeclarationContext>();
}

EscriptParser::VariableDeclarationContext* EscriptParser::VariableDeclarationListContext::variableDeclaration(size_t i) {
  return getRuleContext<EscriptParser::VariableDeclarationContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::VariableDeclarationListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::VariableDeclarationListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::VariableDeclarationListContext::getRuleIndex() const {
  return EscriptParser::RuleVariableDeclarationList;
}

void EscriptParser::VariableDeclarationListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVariableDeclarationList(this);
}

void EscriptParser::VariableDeclarationListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVariableDeclarationList(this);
}


std::any EscriptParser::VariableDeclarationListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VariableDeclarationListContext* EscriptParser::variableDeclarationList() {
  VariableDeclarationListContext *_localctx = _tracker.createInstance<VariableDeclarationListContext>(_ctx, getState());
  enterRule(_localctx, 92, EscriptParser::RuleVariableDeclarationList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(512);
    variableDeclaration();
    setState(517);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(513);
      match(EscriptParser::COMMA);
      setState(514);
      variableDeclaration();
      setState(519);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstantDeclarationContext ------------------------------------------------------------------

EscriptParser::ConstantDeclarationContext::ConstantDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ConstantDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::VariableDeclarationInitializerContext* EscriptParser::ConstantDeclarationContext::variableDeclarationInitializer() {
  return getRuleContext<EscriptParser::VariableDeclarationInitializerContext>(0);
}


size_t EscriptParser::ConstantDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleConstantDeclaration;
}

void EscriptParser::ConstantDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConstantDeclaration(this);
}

void EscriptParser::ConstantDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConstantDeclaration(this);
}


std::any EscriptParser::ConstantDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitConstantDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ConstantDeclarationContext* EscriptParser::constantDeclaration() {
  ConstantDeclarationContext *_localctx = _tracker.createInstance<ConstantDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 94, EscriptParser::RuleConstantDeclaration);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(520);
    match(EscriptParser::IDENTIFIER);
    setState(521);
    variableDeclarationInitializer();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariableDeclarationContext ------------------------------------------------------------------

EscriptParser::VariableDeclarationContext::VariableDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::VariableDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::VariableDeclarationInitializerContext* EscriptParser::VariableDeclarationContext::variableDeclarationInitializer() {
  return getRuleContext<EscriptParser::VariableDeclarationInitializerContext>(0);
}


size_t EscriptParser::VariableDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleVariableDeclaration;
}

void EscriptParser::VariableDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVariableDeclaration(this);
}

void EscriptParser::VariableDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVariableDeclaration(this);
}


std::any EscriptParser::VariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VariableDeclarationContext* EscriptParser::variableDeclaration() {
  VariableDeclarationContext *_localctx = _tracker.createInstance<VariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 96, EscriptParser::RuleVariableDeclaration);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(523);
    match(EscriptParser::IDENTIFIER);
    setState(525);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 67) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 67)) & 43980465111041) != 0)) {
      setState(524);
      variableDeclarationInitializer();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramParametersContext ------------------------------------------------------------------

EscriptParser::ProgramParametersContext::ProgramParametersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ProgramParametersContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::ProgramParametersContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::ProgramParameterListContext* EscriptParser::ProgramParametersContext::programParameterList() {
  return getRuleContext<EscriptParser::ProgramParameterListContext>(0);
}


size_t EscriptParser::ProgramParametersContext::getRuleIndex() const {
  return EscriptParser::RuleProgramParameters;
}

void EscriptParser::ProgramParametersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgramParameters(this);
}

void EscriptParser::ProgramParametersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgramParameters(this);
}


std::any EscriptParser::ProgramParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramParametersContext* EscriptParser::programParameters() {
  ProgramParametersContext *_localctx = _tracker.createInstance<ProgramParametersContext>(_ctx, getState());
  enterRule(_localctx, 98, EscriptParser::RuleProgramParameters);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(527);
    match(EscriptParser::LPAREN);
    setState(529);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::UNUSED || _la == EscriptParser::IDENTIFIER) {
      setState(528);
      programParameterList();
    }
    setState(531);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramParameterListContext ------------------------------------------------------------------

EscriptParser::ProgramParameterListContext::ProgramParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ProgramParameterContext *> EscriptParser::ProgramParameterListContext::programParameter() {
  return getRuleContexts<EscriptParser::ProgramParameterContext>();
}

EscriptParser::ProgramParameterContext* EscriptParser::ProgramParameterListContext::programParameter(size_t i) {
  return getRuleContext<EscriptParser::ProgramParameterContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::ProgramParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::ProgramParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::ProgramParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleProgramParameterList;
}

void EscriptParser::ProgramParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgramParameterList(this);
}

void EscriptParser::ProgramParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgramParameterList(this);
}


std::any EscriptParser::ProgramParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramParameterListContext* EscriptParser::programParameterList() {
  ProgramParameterListContext *_localctx = _tracker.createInstance<ProgramParameterListContext>(_ctx, getState());
  enterRule(_localctx, 100, EscriptParser::RuleProgramParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(533);
    programParameter();
    setState(540);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::UNUSED || _la == EscriptParser::COMMA

    || _la == EscriptParser::IDENTIFIER) {
      setState(535);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::COMMA) {
        setState(534);
        match(EscriptParser::COMMA);
      }
      setState(537);
      programParameter();
      setState(542);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ProgramParameterContext ------------------------------------------------------------------

EscriptParser::ProgramParameterContext::ProgramParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ProgramParameterContext::UNUSED() {
  return getToken(EscriptParser::UNUSED, 0);
}

tree::TerminalNode* EscriptParser::ProgramParameterContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ProgramParameterContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::ProgramParameterContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}


size_t EscriptParser::ProgramParameterContext::getRuleIndex() const {
  return EscriptParser::RuleProgramParameter;
}

void EscriptParser::ProgramParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterProgramParameter(this);
}

void EscriptParser::ProgramParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitProgramParameter(this);
}


std::any EscriptParser::ProgramParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramParameterContext* EscriptParser::programParameter() {
  ProgramParameterContext *_localctx = _tracker.createInstance<ProgramParameterContext>(_ctx, getState());
  enterRule(_localctx, 102, EscriptParser::RuleProgramParameter);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(550);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::UNUSED: {
        enterOuterAlt(_localctx, 1);
        setState(543);
        match(EscriptParser::UNUSED);
        setState(544);
        match(EscriptParser::IDENTIFIER);
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(545);
        match(EscriptParser::IDENTIFIER);
        setState(548);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(546);
          match(EscriptParser::ASSIGN);
          setState(547);
          expression(0);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionParametersContext ------------------------------------------------------------------

EscriptParser::FunctionParametersContext::FunctionParametersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionParametersContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::FunctionParametersContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::FunctionParameterListContext* EscriptParser::FunctionParametersContext::functionParameterList() {
  return getRuleContext<EscriptParser::FunctionParameterListContext>(0);
}


size_t EscriptParser::FunctionParametersContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionParameters;
}

void EscriptParser::FunctionParametersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionParameters(this);
}

void EscriptParser::FunctionParametersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionParameters(this);
}


std::any EscriptParser::FunctionParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionParametersContext* EscriptParser::functionParameters() {
  FunctionParametersContext *_localctx = _tracker.createInstance<FunctionParametersContext>(_ctx, getState());
  enterRule(_localctx, 104, EscriptParser::RuleFunctionParameters);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(552);
    match(EscriptParser::LPAREN);
    setState(554);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::BYREF

    || _la == EscriptParser::UNUSED || _la == EscriptParser::IDENTIFIER) {
      setState(553);
      functionParameterList();
    }
    setState(556);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionParameterListContext ------------------------------------------------------------------

EscriptParser::FunctionParameterListContext::FunctionParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::FunctionParameterContext *> EscriptParser::FunctionParameterListContext::functionParameter() {
  return getRuleContexts<EscriptParser::FunctionParameterContext>();
}

EscriptParser::FunctionParameterContext* EscriptParser::FunctionParameterListContext::functionParameter(size_t i) {
  return getRuleContext<EscriptParser::FunctionParameterContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::FunctionParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::FunctionParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::FunctionParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionParameterList;
}

void EscriptParser::FunctionParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionParameterList(this);
}

void EscriptParser::FunctionParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionParameterList(this);
}


std::any EscriptParser::FunctionParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionParameterListContext* EscriptParser::functionParameterList() {
  FunctionParameterListContext *_localctx = _tracker.createInstance<FunctionParameterListContext>(_ctx, getState());
  enterRule(_localctx, 106, EscriptParser::RuleFunctionParameterList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(558);
    functionParameter();
    setState(563);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(559);
      match(EscriptParser::COMMA);
      setState(560);
      functionParameter();
      setState(565);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionParameterContext ------------------------------------------------------------------

EscriptParser::FunctionParameterContext::FunctionParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionParameterContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::FunctionParameterContext::BYREF() {
  return getToken(EscriptParser::BYREF, 0);
}

tree::TerminalNode* EscriptParser::FunctionParameterContext::UNUSED() {
  return getToken(EscriptParser::UNUSED, 0);
}

tree::TerminalNode* EscriptParser::FunctionParameterContext::ELLIPSIS() {
  return getToken(EscriptParser::ELLIPSIS, 0);
}

tree::TerminalNode* EscriptParser::FunctionParameterContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::FunctionParameterContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}


size_t EscriptParser::FunctionParameterContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionParameter;
}

void EscriptParser::FunctionParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionParameter(this);
}

void EscriptParser::FunctionParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionParameter(this);
}


std::any EscriptParser::FunctionParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionParameterContext* EscriptParser::functionParameter() {
  FunctionParameterContext *_localctx = _tracker.createInstance<FunctionParameterContext>(_ctx, getState());
  enterRule(_localctx, 108, EscriptParser::RuleFunctionParameter);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(567);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::BYREF) {
      setState(566);
      match(EscriptParser::BYREF);
    }
    setState(570);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::UNUSED) {
      setState(569);
      match(EscriptParser::UNUSED);
    }
    setState(572);
    match(EscriptParser::IDENTIFIER);
    setState(574);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ELLIPSIS) {
      setState(573);
      match(EscriptParser::ELLIPSIS);
    }
    setState(578);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(576);
      match(EscriptParser::ASSIGN);
      setState(577);
      expression(0);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ScopedFunctionCallContext ------------------------------------------------------------------

EscriptParser::ScopedFunctionCallContext::ScopedFunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ScopedFunctionCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ScopedFunctionCallContext::COLONCOLON() {
  return getToken(EscriptParser::COLONCOLON, 0);
}

EscriptParser::FunctionCallContext* EscriptParser::ScopedFunctionCallContext::functionCall() {
  return getRuleContext<EscriptParser::FunctionCallContext>(0);
}


size_t EscriptParser::ScopedFunctionCallContext::getRuleIndex() const {
  return EscriptParser::RuleScopedFunctionCall;
}

void EscriptParser::ScopedFunctionCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterScopedFunctionCall(this);
}

void EscriptParser::ScopedFunctionCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitScopedFunctionCall(this);
}


std::any EscriptParser::ScopedFunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitScopedFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ScopedFunctionCallContext* EscriptParser::scopedFunctionCall() {
  ScopedFunctionCallContext *_localctx = _tracker.createInstance<ScopedFunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 110, EscriptParser::RuleScopedFunctionCall);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(580);
    match(EscriptParser::IDENTIFIER);
    setState(581);
    match(EscriptParser::COLONCOLON);
    setState(582);
    functionCall();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionReferenceContext ------------------------------------------------------------------

EscriptParser::FunctionReferenceContext::FunctionReferenceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionReferenceContext::AT() {
  return getToken(EscriptParser::AT, 0);
}

std::vector<tree::TerminalNode *> EscriptParser::FunctionReferenceContext::IDENTIFIER() {
  return getTokens(EscriptParser::IDENTIFIER);
}

tree::TerminalNode* EscriptParser::FunctionReferenceContext::IDENTIFIER(size_t i) {
  return getToken(EscriptParser::IDENTIFIER, i);
}

tree::TerminalNode* EscriptParser::FunctionReferenceContext::COLONCOLON() {
  return getToken(EscriptParser::COLONCOLON, 0);
}


size_t EscriptParser::FunctionReferenceContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionReference;
}

void EscriptParser::FunctionReferenceContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionReference(this);
}

void EscriptParser::FunctionReferenceContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionReference(this);
}


std::any EscriptParser::FunctionReferenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionReference(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionReferenceContext* EscriptParser::functionReference() {
  FunctionReferenceContext *_localctx = _tracker.createInstance<FunctionReferenceContext>(_ctx, getState());
  enterRule(_localctx, 112, EscriptParser::RuleFunctionReference);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(584);
    match(EscriptParser::AT);
    setState(587);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 51, _ctx)) {
    case 1: {
      setState(585);
      antlrcpp::downCast<FunctionReferenceContext *>(_localctx)->scope = match(EscriptParser::IDENTIFIER);
      setState(586);
      match(EscriptParser::COLONCOLON);
      break;
    }

    default:
      break;
    }
    setState(589);
    antlrcpp::downCast<FunctionReferenceContext *>(_localctx)->function = match(EscriptParser::IDENTIFIER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

EscriptParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::PrimaryContext* EscriptParser::ExpressionContext::primary() {
  return getRuleContext<EscriptParser::PrimaryContext>(0);
}

std::vector<EscriptParser::ExpressionContext *> EscriptParser::ExpressionContext::expression() {
  return getRuleContexts<EscriptParser::ExpressionContext>();
}

EscriptParser::ExpressionContext* EscriptParser::ExpressionContext::expression(size_t i) {
  return getRuleContext<EscriptParser::ExpressionContext>(i);
}

tree::TerminalNode* EscriptParser::ExpressionContext::ADD() {
  return getToken(EscriptParser::ADD, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::SUB() {
  return getToken(EscriptParser::SUB, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::INC() {
  return getToken(EscriptParser::INC, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::DEC() {
  return getToken(EscriptParser::DEC, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::TILDE() {
  return getToken(EscriptParser::TILDE, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::BANG_A() {
  return getToken(EscriptParser::BANG_A, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::BANG_B() {
  return getToken(EscriptParser::BANG_B, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::MUL() {
  return getToken(EscriptParser::MUL, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::DIV() {
  return getToken(EscriptParser::DIV, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::MOD() {
  return getToken(EscriptParser::MOD, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::LSHIFT() {
  return getToken(EscriptParser::LSHIFT, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::RSHIFT() {
  return getToken(EscriptParser::RSHIFT, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::BITAND() {
  return getToken(EscriptParser::BITAND, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::BITOR() {
  return getToken(EscriptParser::BITOR, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::CARET() {
  return getToken(EscriptParser::CARET, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::ELVIS() {
  return getToken(EscriptParser::ELVIS, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::TOK_IN() {
  return getToken(EscriptParser::TOK_IN, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::LE() {
  return getToken(EscriptParser::LE, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::GE() {
  return getToken(EscriptParser::GE, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::GT() {
  return getToken(EscriptParser::GT, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::LT() {
  return getToken(EscriptParser::LT, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::EQUAL_DEPRECATED() {
  return getToken(EscriptParser::EQUAL_DEPRECATED, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::EQUAL() {
  return getToken(EscriptParser::EQUAL, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::NOTEQUAL_B() {
  return getToken(EscriptParser::NOTEQUAL_B, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::NOTEQUAL_A() {
  return getToken(EscriptParser::NOTEQUAL_A, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::AND_A() {
  return getToken(EscriptParser::AND_A, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::AND_B() {
  return getToken(EscriptParser::AND_B, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::OR_A() {
  return getToken(EscriptParser::OR_A, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::OR_B() {
  return getToken(EscriptParser::OR_B, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::QUESTION() {
  return getToken(EscriptParser::QUESTION, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::COLON() {
  return getToken(EscriptParser::COLON, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::ADDMEMBER() {
  return getToken(EscriptParser::ADDMEMBER, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::DELMEMBER() {
  return getToken(EscriptParser::DELMEMBER, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::CHKMEMBER() {
  return getToken(EscriptParser::CHKMEMBER, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::ADD_ASSIGN() {
  return getToken(EscriptParser::ADD_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::SUB_ASSIGN() {
  return getToken(EscriptParser::SUB_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::MUL_ASSIGN() {
  return getToken(EscriptParser::MUL_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::DIV_ASSIGN() {
  return getToken(EscriptParser::DIV_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::MOD_ASSIGN() {
  return getToken(EscriptParser::MOD_ASSIGN, 0);
}

EscriptParser::ExpressionSuffixContext* EscriptParser::ExpressionContext::expressionSuffix() {
  return getRuleContext<EscriptParser::ExpressionSuffixContext>(0);
}


size_t EscriptParser::ExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleExpression;
}

void EscriptParser::ExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExpression(this);
}

void EscriptParser::ExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExpression(this);
}


std::any EscriptParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExpression(this);
  else
    return visitor->visitChildren(this);
}


EscriptParser::ExpressionContext* EscriptParser::expression() {
   return expression(0);
}

EscriptParser::ExpressionContext* EscriptParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  EscriptParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  EscriptParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 114;
  enterRecursionRule(_localctx, 114, EscriptParser::RuleExpression, precedence);

    size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(597);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::TOK_ERROR:
      case EscriptParser::DICTIONARY:
      case EscriptParser::STRUCT:
      case EscriptParser::ARRAY:
      case EscriptParser::UNINIT:
      case EscriptParser::BOOL_TRUE:
      case EscriptParser::BOOL_FALSE:
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::INTERPOLATED_STRING_START:
      case EscriptParser::LPAREN:
      case EscriptParser::LBRACE:
      case EscriptParser::AT:
      case EscriptParser::IDENTIFIER: {
        setState(592);
        primary();
        break;
      }

      case EscriptParser::ADD:
      case EscriptParser::SUB:
      case EscriptParser::INC:
      case EscriptParser::DEC: {
        setState(593);
        antlrcpp::downCast<ExpressionContext *>(_localctx)->prefix = _input->LT(1);
        _la = _input->LA(1);
        if (!(((((_la - 92) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 92)) & 3221225475) != 0))) {
          antlrcpp::downCast<ExpressionContext *>(_localctx)->prefix = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(594);
        expression(14);
        break;
      }

      case EscriptParser::BANG_A:
      case EscriptParser::BANG_B:
      case EscriptParser::TILDE: {
        setState(595);
        antlrcpp::downCast<ExpressionContext *>(_localctx)->prefix = _input->LT(1);
        _la = _input->LA(1);
        if (!(((((_la - 59) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 59)) & 576460752303423491) != 0))) {
          antlrcpp::downCast<ExpressionContext *>(_localctx)->prefix = _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        }
        setState(596);
        expression(13);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(645);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(643);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(599);

          if (!(precpred(_ctx, 12))) throw FailedPredicateException(this, "precpred(_ctx, 12)");
          setState(600);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 89) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 89)) & 114695) != 0))) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(601);
          expression(13);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(602);

          if (!(precpred(_ctx, 11))) throw FailedPredicateException(this, "precpred(_ctx, 11)");
          setState(603);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 92) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 92)) & 49155) != 0))) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(604);
          expression(12);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(605);

          if (!(precpred(_ctx, 10))) throw FailedPredicateException(this, "precpred(_ctx, 10)");
          setState(606);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::ELVIS);
          setState(607);
          expression(11);
          break;
        }

        case 4: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(608);

          if (!(precpred(_ctx, 9))) throw FailedPredicateException(this, "precpred(_ctx, 9)");
          setState(609);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::TOK_IN);
          setState(610);
          expression(10);
          break;
        }

        case 5: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(611);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(612);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 99) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 99)) & 15) != 0))) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(613);
          expression(9);
          break;
        }

        case 6: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(614);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(615);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::EQUAL_DEPRECATED);
           notifyErrorListeners("Deprecated '=' found: did you mean '==' or ':='?\n"); 
          setState(617);
          expression(8);
          break;
        }

        case 7: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(618);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(619);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 108) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 108)) & 11) != 0))) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(620);
          expression(7);
          break;
        }

        case 8: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(621);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(622);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::AND_A

          || _la == EscriptParser::AND_B)) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(623);
          expression(6);
          break;
        }

        case 9: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(624);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(625);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::OR_A

          || _la == EscriptParser::OR_B)) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(626);
          expression(5);
          break;
        }

        case 10: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(627);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(628);
          match(EscriptParser::QUESTION);
          setState(629);
          expression(0);
          setState(630);
          match(EscriptParser::COLON);
          setState(631);
          expression(4);
          break;
        }

        case 11: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(633);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(634);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 113) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 113)) & 7) != 0))) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(635);
          expression(2);
          break;
        }

        case 12: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(636);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(637);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 94) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 94)) & 262175) != 0))) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(638);
          expression(1);
          break;
        }

        case 13: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(639);

          if (!(precpred(_ctx, 16))) throw FailedPredicateException(this, "precpred(_ctx, 16)");
          setState(640);
          expressionSuffix();
          break;
        }

        case 14: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(641);

          if (!(precpred(_ctx, 15))) throw FailedPredicateException(this, "precpred(_ctx, 15)");
          setState(642);
          antlrcpp::downCast<ExpressionContext *>(_localctx)->postfix = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::INC

          || _la == EscriptParser::DEC)) {
            antlrcpp::downCast<ExpressionContext *>(_localctx)->postfix = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          break;
        }

        default:
          break;
        } 
      }
      setState(647);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 54, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- PrimaryContext ------------------------------------------------------------------

EscriptParser::PrimaryContext::PrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::LiteralContext* EscriptParser::PrimaryContext::literal() {
  return getRuleContext<EscriptParser::LiteralContext>(0);
}

EscriptParser::ParExpressionContext* EscriptParser::PrimaryContext::parExpression() {
  return getRuleContext<EscriptParser::ParExpressionContext>(0);
}

EscriptParser::FunctionCallContext* EscriptParser::PrimaryContext::functionCall() {
  return getRuleContext<EscriptParser::FunctionCallContext>(0);
}

EscriptParser::ScopedFunctionCallContext* EscriptParser::PrimaryContext::scopedFunctionCall() {
  return getRuleContext<EscriptParser::ScopedFunctionCallContext>(0);
}

EscriptParser::ScopedIdentifierContext* EscriptParser::PrimaryContext::scopedIdentifier() {
  return getRuleContext<EscriptParser::ScopedIdentifierContext>(0);
}

tree::TerminalNode* EscriptParser::PrimaryContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::FunctionReferenceContext* EscriptParser::PrimaryContext::functionReference() {
  return getRuleContext<EscriptParser::FunctionReferenceContext>(0);
}

EscriptParser::FunctionExpressionContext* EscriptParser::PrimaryContext::functionExpression() {
  return getRuleContext<EscriptParser::FunctionExpressionContext>(0);
}

EscriptParser::ExplicitArrayInitializerContext* EscriptParser::PrimaryContext::explicitArrayInitializer() {
  return getRuleContext<EscriptParser::ExplicitArrayInitializerContext>(0);
}

EscriptParser::ExplicitStructInitializerContext* EscriptParser::PrimaryContext::explicitStructInitializer() {
  return getRuleContext<EscriptParser::ExplicitStructInitializerContext>(0);
}

EscriptParser::ExplicitDictInitializerContext* EscriptParser::PrimaryContext::explicitDictInitializer() {
  return getRuleContext<EscriptParser::ExplicitDictInitializerContext>(0);
}

EscriptParser::ExplicitErrorInitializerContext* EscriptParser::PrimaryContext::explicitErrorInitializer() {
  return getRuleContext<EscriptParser::ExplicitErrorInitializerContext>(0);
}

EscriptParser::BareArrayInitializerContext* EscriptParser::PrimaryContext::bareArrayInitializer() {
  return getRuleContext<EscriptParser::BareArrayInitializerContext>(0);
}

EscriptParser::InterpolatedStringContext* EscriptParser::PrimaryContext::interpolatedString() {
  return getRuleContext<EscriptParser::InterpolatedStringContext>(0);
}


size_t EscriptParser::PrimaryContext::getRuleIndex() const {
  return EscriptParser::RulePrimary;
}

void EscriptParser::PrimaryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrimary(this);
}

void EscriptParser::PrimaryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrimary(this);
}


std::any EscriptParser::PrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPrimary(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PrimaryContext* EscriptParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 116, EscriptParser::RulePrimary);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(662);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 55, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(648);
      literal();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(649);
      parExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(650);
      functionCall();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(651);
      scopedFunctionCall();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(652);
      scopedIdentifier();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(653);
      match(EscriptParser::IDENTIFIER);
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(654);
      functionReference();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(655);
      functionExpression();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(656);
      explicitArrayInitializer();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(657);
      explicitStructInitializer();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(658);
      explicitDictInitializer();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(659);
      explicitErrorInitializer();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(660);
      bareArrayInitializer();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(661);
      interpolatedString();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ScopedIdentifierContext ------------------------------------------------------------------

EscriptParser::ScopedIdentifierContext::ScopedIdentifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ScopedIdentifierContext::COLONCOLON() {
  return getToken(EscriptParser::COLONCOLON, 0);
}

std::vector<tree::TerminalNode *> EscriptParser::ScopedIdentifierContext::IDENTIFIER() {
  return getTokens(EscriptParser::IDENTIFIER);
}

tree::TerminalNode* EscriptParser::ScopedIdentifierContext::IDENTIFIER(size_t i) {
  return getToken(EscriptParser::IDENTIFIER, i);
}


size_t EscriptParser::ScopedIdentifierContext::getRuleIndex() const {
  return EscriptParser::RuleScopedIdentifier;
}

void EscriptParser::ScopedIdentifierContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterScopedIdentifier(this);
}

void EscriptParser::ScopedIdentifierContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitScopedIdentifier(this);
}


std::any EscriptParser::ScopedIdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitScopedIdentifier(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ScopedIdentifierContext* EscriptParser::scopedIdentifier() {
  ScopedIdentifierContext *_localctx = _tracker.createInstance<ScopedIdentifierContext>(_ctx, getState());
  enterRule(_localctx, 118, EscriptParser::RuleScopedIdentifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(664);
    antlrcpp::downCast<ScopedIdentifierContext *>(_localctx)->scope = match(EscriptParser::IDENTIFIER);
    setState(665);
    match(EscriptParser::COLONCOLON);
    setState(666);
    antlrcpp::downCast<ScopedIdentifierContext *>(_localctx)->identifier = match(EscriptParser::IDENTIFIER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionExpressionContext ------------------------------------------------------------------

EscriptParser::FunctionExpressionContext::FunctionExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionExpressionContext::AT() {
  return getToken(EscriptParser::AT, 0);
}

tree::TerminalNode* EscriptParser::FunctionExpressionContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

EscriptParser::BlockContext* EscriptParser::FunctionExpressionContext::block() {
  return getRuleContext<EscriptParser::BlockContext>(0);
}

tree::TerminalNode* EscriptParser::FunctionExpressionContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::FunctionParametersContext* EscriptParser::FunctionExpressionContext::functionParameters() {
  return getRuleContext<EscriptParser::FunctionParametersContext>(0);
}


size_t EscriptParser::FunctionExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionExpression;
}

void EscriptParser::FunctionExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionExpression(this);
}

void EscriptParser::FunctionExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionExpression(this);
}


std::any EscriptParser::FunctionExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionExpressionContext* EscriptParser::functionExpression() {
  FunctionExpressionContext *_localctx = _tracker.createInstance<FunctionExpressionContext>(_ctx, getState());
  enterRule(_localctx, 120, EscriptParser::RuleFunctionExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(668);
    match(EscriptParser::AT);
    setState(670);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::LPAREN) {
      setState(669);
      functionParameters();
    }
    setState(672);
    match(EscriptParser::LBRACE);
    setState(673);
    block();
    setState(674);
    match(EscriptParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExplicitArrayInitializerContext ------------------------------------------------------------------

EscriptParser::ExplicitArrayInitializerContext::ExplicitArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ExplicitArrayInitializerContext::ARRAY() {
  return getToken(EscriptParser::ARRAY, 0);
}

EscriptParser::ArrayInitializerContext* EscriptParser::ExplicitArrayInitializerContext::arrayInitializer() {
  return getRuleContext<EscriptParser::ArrayInitializerContext>(0);
}


size_t EscriptParser::ExplicitArrayInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleExplicitArrayInitializer;
}

void EscriptParser::ExplicitArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExplicitArrayInitializer(this);
}

void EscriptParser::ExplicitArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExplicitArrayInitializer(this);
}


std::any EscriptParser::ExplicitArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExplicitArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExplicitArrayInitializerContext* EscriptParser::explicitArrayInitializer() {
  ExplicitArrayInitializerContext *_localctx = _tracker.createInstance<ExplicitArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 122, EscriptParser::RuleExplicitArrayInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(676);
    match(EscriptParser::ARRAY);
    setState(678);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 57, _ctx)) {
    case 1: {
      setState(677);
      arrayInitializer();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExplicitStructInitializerContext ------------------------------------------------------------------

EscriptParser::ExplicitStructInitializerContext::ExplicitStructInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ExplicitStructInitializerContext::STRUCT() {
  return getToken(EscriptParser::STRUCT, 0);
}

EscriptParser::StructInitializerContext* EscriptParser::ExplicitStructInitializerContext::structInitializer() {
  return getRuleContext<EscriptParser::StructInitializerContext>(0);
}


size_t EscriptParser::ExplicitStructInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleExplicitStructInitializer;
}

void EscriptParser::ExplicitStructInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExplicitStructInitializer(this);
}

void EscriptParser::ExplicitStructInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExplicitStructInitializer(this);
}


std::any EscriptParser::ExplicitStructInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExplicitStructInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExplicitStructInitializerContext* EscriptParser::explicitStructInitializer() {
  ExplicitStructInitializerContext *_localctx = _tracker.createInstance<ExplicitStructInitializerContext>(_ctx, getState());
  enterRule(_localctx, 124, EscriptParser::RuleExplicitStructInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(680);
    match(EscriptParser::STRUCT);
    setState(682);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 58, _ctx)) {
    case 1: {
      setState(681);
      structInitializer();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExplicitDictInitializerContext ------------------------------------------------------------------

EscriptParser::ExplicitDictInitializerContext::ExplicitDictInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ExplicitDictInitializerContext::DICTIONARY() {
  return getToken(EscriptParser::DICTIONARY, 0);
}

EscriptParser::DictInitializerContext* EscriptParser::ExplicitDictInitializerContext::dictInitializer() {
  return getRuleContext<EscriptParser::DictInitializerContext>(0);
}


size_t EscriptParser::ExplicitDictInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleExplicitDictInitializer;
}

void EscriptParser::ExplicitDictInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExplicitDictInitializer(this);
}

void EscriptParser::ExplicitDictInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExplicitDictInitializer(this);
}


std::any EscriptParser::ExplicitDictInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExplicitDictInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExplicitDictInitializerContext* EscriptParser::explicitDictInitializer() {
  ExplicitDictInitializerContext *_localctx = _tracker.createInstance<ExplicitDictInitializerContext>(_ctx, getState());
  enterRule(_localctx, 126, EscriptParser::RuleExplicitDictInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(684);
    match(EscriptParser::DICTIONARY);
    setState(686);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 59, _ctx)) {
    case 1: {
      setState(685);
      dictInitializer();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExplicitErrorInitializerContext ------------------------------------------------------------------

EscriptParser::ExplicitErrorInitializerContext::ExplicitErrorInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ExplicitErrorInitializerContext::TOK_ERROR() {
  return getToken(EscriptParser::TOK_ERROR, 0);
}

EscriptParser::StructInitializerContext* EscriptParser::ExplicitErrorInitializerContext::structInitializer() {
  return getRuleContext<EscriptParser::StructInitializerContext>(0);
}


size_t EscriptParser::ExplicitErrorInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleExplicitErrorInitializer;
}

void EscriptParser::ExplicitErrorInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExplicitErrorInitializer(this);
}

void EscriptParser::ExplicitErrorInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExplicitErrorInitializer(this);
}


std::any EscriptParser::ExplicitErrorInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExplicitErrorInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExplicitErrorInitializerContext* EscriptParser::explicitErrorInitializer() {
  ExplicitErrorInitializerContext *_localctx = _tracker.createInstance<ExplicitErrorInitializerContext>(_ctx, getState());
  enterRule(_localctx, 128, EscriptParser::RuleExplicitErrorInitializer);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(688);
    match(EscriptParser::TOK_ERROR);
    setState(690);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 60, _ctx)) {
    case 1: {
      setState(689);
      structInitializer();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BareArrayInitializerContext ------------------------------------------------------------------

EscriptParser::BareArrayInitializerContext::BareArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::BareArrayInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::BareArrayInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::BareArrayInitializerContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::BareArrayInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::BareArrayInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleBareArrayInitializer;
}

void EscriptParser::BareArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBareArrayInitializer(this);
}

void EscriptParser::BareArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBareArrayInitializer(this);
}


std::any EscriptParser::BareArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBareArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BareArrayInitializerContext* EscriptParser::bareArrayInitializer() {
  BareArrayInitializerContext *_localctx = _tracker.createInstance<BareArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 130, EscriptParser::RuleBareArrayInitializer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(704);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 63, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(692);
      match(EscriptParser::LBRACE);
      setState(694);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(693);
        expressionList();
      }
      setState(696);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(697);
      match(EscriptParser::LBRACE);
      setState(699);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(698);
        expressionList();
      }
      setState(701);
      match(EscriptParser::COMMA);
      setState(702);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParExpressionContext ------------------------------------------------------------------

EscriptParser::ParExpressionContext::ParExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ParExpressionContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::ParExpressionContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::ParExpressionContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}


size_t EscriptParser::ParExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleParExpression;
}

void EscriptParser::ParExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParExpression(this);
}

void EscriptParser::ParExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParExpression(this);
}


std::any EscriptParser::ParExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitParExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ParExpressionContext* EscriptParser::parExpression() {
  ParExpressionContext *_localctx = _tracker.createInstance<ParExpressionContext>(_ctx, getState());
  enterRule(_localctx, 132, EscriptParser::RuleParExpression);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(706);
    match(EscriptParser::LPAREN);
    setState(707);
    expression(0);
    setState(708);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionListContext ------------------------------------------------------------------

EscriptParser::ExpressionListContext::ExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ExpressionListEntryContext *> EscriptParser::ExpressionListContext::expressionListEntry() {
  return getRuleContexts<EscriptParser::ExpressionListEntryContext>();
}

EscriptParser::ExpressionListEntryContext* EscriptParser::ExpressionListContext::expressionListEntry(size_t i) {
  return getRuleContext<EscriptParser::ExpressionListEntryContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::ExpressionListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::ExpressionListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::ExpressionListContext::getRuleIndex() const {
  return EscriptParser::RuleExpressionList;
}

void EscriptParser::ExpressionListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExpressionList(this);
}

void EscriptParser::ExpressionListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExpressionList(this);
}


std::any EscriptParser::ExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExpressionListContext* EscriptParser::expressionList() {
  ExpressionListContext *_localctx = _tracker.createInstance<ExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 134, EscriptParser::RuleExpressionList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(710);
    expressionListEntry();
    setState(715);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(711);
        match(EscriptParser::COMMA);
        setState(712);
        expressionListEntry(); 
      }
      setState(717);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 64, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionListEntryContext ------------------------------------------------------------------

EscriptParser::ExpressionListEntryContext::ExpressionListEntryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::ExpressionContext* EscriptParser::ExpressionListEntryContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::ExpressionListEntryContext::ELLIPSIS() {
  return getToken(EscriptParser::ELLIPSIS, 0);
}


size_t EscriptParser::ExpressionListEntryContext::getRuleIndex() const {
  return EscriptParser::RuleExpressionListEntry;
}

void EscriptParser::ExpressionListEntryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExpressionListEntry(this);
}

void EscriptParser::ExpressionListEntryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExpressionListEntry(this);
}


std::any EscriptParser::ExpressionListEntryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExpressionListEntry(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExpressionListEntryContext* EscriptParser::expressionListEntry() {
  ExpressionListEntryContext *_localctx = _tracker.createInstance<ExpressionListEntryContext>(_ctx, getState());
  enterRule(_localctx, 136, EscriptParser::RuleExpressionListEntry);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(718);
    expression(0);
    setState(720);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ELLIPSIS) {
      setState(719);
      match(EscriptParser::ELLIPSIS);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionSuffixContext ------------------------------------------------------------------

EscriptParser::ExpressionSuffixContext::ExpressionSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::IndexingSuffixContext* EscriptParser::ExpressionSuffixContext::indexingSuffix() {
  return getRuleContext<EscriptParser::IndexingSuffixContext>(0);
}

EscriptParser::MethodCallSuffixContext* EscriptParser::ExpressionSuffixContext::methodCallSuffix() {
  return getRuleContext<EscriptParser::MethodCallSuffixContext>(0);
}

EscriptParser::NavigationSuffixContext* EscriptParser::ExpressionSuffixContext::navigationSuffix() {
  return getRuleContext<EscriptParser::NavigationSuffixContext>(0);
}

EscriptParser::FunctionCallSuffixContext* EscriptParser::ExpressionSuffixContext::functionCallSuffix() {
  return getRuleContext<EscriptParser::FunctionCallSuffixContext>(0);
}


size_t EscriptParser::ExpressionSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleExpressionSuffix;
}

void EscriptParser::ExpressionSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterExpressionSuffix(this);
}

void EscriptParser::ExpressionSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitExpressionSuffix(this);
}


std::any EscriptParser::ExpressionSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExpressionSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExpressionSuffixContext* EscriptParser::expressionSuffix() {
  ExpressionSuffixContext *_localctx = _tracker.createInstance<ExpressionSuffixContext>(_ctx, getState());
  enterRule(_localctx, 138, EscriptParser::RuleExpressionSuffix);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(726);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 66, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(722);
      indexingSuffix();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(723);
      methodCallSuffix();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(724);
      navigationSuffix();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(725);
      functionCallSuffix();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IndexingSuffixContext ------------------------------------------------------------------

EscriptParser::IndexingSuffixContext::IndexingSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::IndexingSuffixContext::LBRACK() {
  return getToken(EscriptParser::LBRACK, 0);
}

EscriptParser::IndexListContext* EscriptParser::IndexingSuffixContext::indexList() {
  return getRuleContext<EscriptParser::IndexListContext>(0);
}

tree::TerminalNode* EscriptParser::IndexingSuffixContext::RBRACK() {
  return getToken(EscriptParser::RBRACK, 0);
}


size_t EscriptParser::IndexingSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleIndexingSuffix;
}

void EscriptParser::IndexingSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIndexingSuffix(this);
}

void EscriptParser::IndexingSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIndexingSuffix(this);
}


std::any EscriptParser::IndexingSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIndexingSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IndexingSuffixContext* EscriptParser::indexingSuffix() {
  IndexingSuffixContext *_localctx = _tracker.createInstance<IndexingSuffixContext>(_ctx, getState());
  enterRule(_localctx, 140, EscriptParser::RuleIndexingSuffix);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(728);
    match(EscriptParser::LBRACK);
    setState(729);
    indexList();
    setState(730);
    match(EscriptParser::RBRACK);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IndexListContext ------------------------------------------------------------------

EscriptParser::IndexListContext::IndexListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ExpressionContext *> EscriptParser::IndexListContext::expression() {
  return getRuleContexts<EscriptParser::ExpressionContext>();
}

EscriptParser::ExpressionContext* EscriptParser::IndexListContext::expression(size_t i) {
  return getRuleContext<EscriptParser::ExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::IndexListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::IndexListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::IndexListContext::getRuleIndex() const {
  return EscriptParser::RuleIndexList;
}

void EscriptParser::IndexListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIndexList(this);
}

void EscriptParser::IndexListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIndexList(this);
}


std::any EscriptParser::IndexListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIndexList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IndexListContext* EscriptParser::indexList() {
  IndexListContext *_localctx = _tracker.createInstance<IndexListContext>(_ctx, getState());
  enterRule(_localctx, 142, EscriptParser::RuleIndexList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(732);
    expression(0);
    setState(737);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(733);
      match(EscriptParser::COMMA);
      setState(734);
      expression(0);
      setState(739);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NavigationSuffixContext ------------------------------------------------------------------

EscriptParser::NavigationSuffixContext::NavigationSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::NavigationSuffixContext::DOT() {
  return getToken(EscriptParser::DOT, 0);
}

tree::TerminalNode* EscriptParser::NavigationSuffixContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::NavigationSuffixContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}


size_t EscriptParser::NavigationSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleNavigationSuffix;
}

void EscriptParser::NavigationSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNavigationSuffix(this);
}

void EscriptParser::NavigationSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNavigationSuffix(this);
}


std::any EscriptParser::NavigationSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitNavigationSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::NavigationSuffixContext* EscriptParser::navigationSuffix() {
  NavigationSuffixContext *_localctx = _tracker.createInstance<NavigationSuffixContext>(_ctx, getState());
  enterRule(_localctx, 144, EscriptParser::RuleNavigationSuffix);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(740);
    match(EscriptParser::DOT);
    setState(741);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::STRING_LITERAL

    || _la == EscriptParser::IDENTIFIER)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MethodCallSuffixContext ------------------------------------------------------------------

EscriptParser::MethodCallSuffixContext::MethodCallSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::MethodCallSuffixContext::DOT() {
  return getToken(EscriptParser::DOT, 0);
}

tree::TerminalNode* EscriptParser::MethodCallSuffixContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::MethodCallSuffixContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::MethodCallSuffixContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::MethodCallSuffixContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}


size_t EscriptParser::MethodCallSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleMethodCallSuffix;
}

void EscriptParser::MethodCallSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMethodCallSuffix(this);
}

void EscriptParser::MethodCallSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMethodCallSuffix(this);
}


std::any EscriptParser::MethodCallSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMethodCallSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MethodCallSuffixContext* EscriptParser::methodCallSuffix() {
  MethodCallSuffixContext *_localctx = _tracker.createInstance<MethodCallSuffixContext>(_ctx, getState());
  enterRule(_localctx, 146, EscriptParser::RuleMethodCallSuffix);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(743);
    match(EscriptParser::DOT);
    setState(744);
    match(EscriptParser::IDENTIFIER);
    setState(745);
    match(EscriptParser::LPAREN);
    setState(747);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 59) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

    || _la == EscriptParser::IDENTIFIER) {
      setState(746);
      expressionList();
    }
    setState(749);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionCallSuffixContext ------------------------------------------------------------------

EscriptParser::FunctionCallSuffixContext::FunctionCallSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionCallSuffixContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::FunctionCallSuffixContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::FunctionCallSuffixContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}


size_t EscriptParser::FunctionCallSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionCallSuffix;
}

void EscriptParser::FunctionCallSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionCallSuffix(this);
}

void EscriptParser::FunctionCallSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionCallSuffix(this);
}


std::any EscriptParser::FunctionCallSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionCallSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionCallSuffixContext* EscriptParser::functionCallSuffix() {
  FunctionCallSuffixContext *_localctx = _tracker.createInstance<FunctionCallSuffixContext>(_ctx, getState());
  enterRule(_localctx, 148, EscriptParser::RuleFunctionCallSuffix);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(751);
    match(EscriptParser::LPAREN);
    setState(753);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 59) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

    || _la == EscriptParser::IDENTIFIER) {
      setState(752);
      expressionList();
    }
    setState(755);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionCallContext ------------------------------------------------------------------

EscriptParser::FunctionCallContext::FunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FunctionCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::FunctionCallContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::FunctionCallContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::FunctionCallContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}


size_t EscriptParser::FunctionCallContext::getRuleIndex() const {
  return EscriptParser::RuleFunctionCall;
}

void EscriptParser::FunctionCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunctionCall(this);
}

void EscriptParser::FunctionCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunctionCall(this);
}


std::any EscriptParser::FunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionCallContext* EscriptParser::functionCall() {
  FunctionCallContext *_localctx = _tracker.createInstance<FunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 150, EscriptParser::RuleFunctionCall);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(757);
    match(EscriptParser::IDENTIFIER);
    setState(758);
    match(EscriptParser::LPAREN);
    setState(760);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 59) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

    || _la == EscriptParser::IDENTIFIER) {
      setState(759);
      expressionList();
    }
    setState(762);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructInitializerExpressionContext ------------------------------------------------------------------

EscriptParser::StructInitializerExpressionContext::StructInitializerExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::StructInitializerExpressionContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::StructInitializerExpressionContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::StructInitializerExpressionContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::StructInitializerExpressionContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}


size_t EscriptParser::StructInitializerExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleStructInitializerExpression;
}

void EscriptParser::StructInitializerExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStructInitializerExpression(this);
}

void EscriptParser::StructInitializerExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStructInitializerExpression(this);
}


std::any EscriptParser::StructInitializerExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStructInitializerExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StructInitializerExpressionContext* EscriptParser::structInitializerExpression() {
  StructInitializerExpressionContext *_localctx = _tracker.createInstance<StructInitializerExpressionContext>(_ctx, getState());
  enterRule(_localctx, 152, EscriptParser::RuleStructInitializerExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(774);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(764);
        match(EscriptParser::IDENTIFIER);
        setState(767);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(765);
          match(EscriptParser::ASSIGN);
          setState(766);
          expression(0);
        }
        break;
      }

      case EscriptParser::STRING_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(769);
        match(EscriptParser::STRING_LITERAL);
        setState(772);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(770);
          match(EscriptParser::ASSIGN);
          setState(771);
          expression(0);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructInitializerExpressionListContext ------------------------------------------------------------------

EscriptParser::StructInitializerExpressionListContext::StructInitializerExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::StructInitializerExpressionContext *> EscriptParser::StructInitializerExpressionListContext::structInitializerExpression() {
  return getRuleContexts<EscriptParser::StructInitializerExpressionContext>();
}

EscriptParser::StructInitializerExpressionContext* EscriptParser::StructInitializerExpressionListContext::structInitializerExpression(size_t i) {
  return getRuleContext<EscriptParser::StructInitializerExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::StructInitializerExpressionListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::StructInitializerExpressionListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::StructInitializerExpressionListContext::getRuleIndex() const {
  return EscriptParser::RuleStructInitializerExpressionList;
}

void EscriptParser::StructInitializerExpressionListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStructInitializerExpressionList(this);
}

void EscriptParser::StructInitializerExpressionListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStructInitializerExpressionList(this);
}


std::any EscriptParser::StructInitializerExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStructInitializerExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StructInitializerExpressionListContext* EscriptParser::structInitializerExpressionList() {
  StructInitializerExpressionListContext *_localctx = _tracker.createInstance<StructInitializerExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 154, EscriptParser::RuleStructInitializerExpressionList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(776);
    structInitializerExpression();
    setState(781);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(777);
        match(EscriptParser::COMMA);
        setState(778);
        structInitializerExpression(); 
      }
      setState(783);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 74, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StructInitializerContext ------------------------------------------------------------------

EscriptParser::StructInitializerContext::StructInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::StructInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::StructInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::StructInitializerExpressionListContext* EscriptParser::StructInitializerContext::structInitializerExpressionList() {
  return getRuleContext<EscriptParser::StructInitializerExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::StructInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::StructInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleStructInitializer;
}

void EscriptParser::StructInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStructInitializer(this);
}

void EscriptParser::StructInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStructInitializer(this);
}


std::any EscriptParser::StructInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStructInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StructInitializerContext* EscriptParser::structInitializer() {
  StructInitializerContext *_localctx = _tracker.createInstance<StructInitializerContext>(_ctx, getState());
  enterRule(_localctx, 156, EscriptParser::RuleStructInitializer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(796);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 77, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(784);
      match(EscriptParser::LBRACE);
      setState(786);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::STRING_LITERAL

      || _la == EscriptParser::IDENTIFIER) {
        setState(785);
        structInitializerExpressionList();
      }
      setState(788);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(789);
      match(EscriptParser::LBRACE);
      setState(791);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::STRING_LITERAL

      || _la == EscriptParser::IDENTIFIER) {
        setState(790);
        structInitializerExpressionList();
      }
      setState(793);
      match(EscriptParser::COMMA);
      setState(794);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in struct initializer list");
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DictInitializerExpressionContext ------------------------------------------------------------------

EscriptParser::DictInitializerExpressionContext::DictInitializerExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ExpressionContext *> EscriptParser::DictInitializerExpressionContext::expression() {
  return getRuleContexts<EscriptParser::ExpressionContext>();
}

EscriptParser::ExpressionContext* EscriptParser::DictInitializerExpressionContext::expression(size_t i) {
  return getRuleContext<EscriptParser::ExpressionContext>(i);
}

tree::TerminalNode* EscriptParser::DictInitializerExpressionContext::ARROW() {
  return getToken(EscriptParser::ARROW, 0);
}


size_t EscriptParser::DictInitializerExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleDictInitializerExpression;
}

void EscriptParser::DictInitializerExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDictInitializerExpression(this);
}

void EscriptParser::DictInitializerExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDictInitializerExpression(this);
}


std::any EscriptParser::DictInitializerExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDictInitializerExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DictInitializerExpressionContext* EscriptParser::dictInitializerExpression() {
  DictInitializerExpressionContext *_localctx = _tracker.createInstance<DictInitializerExpressionContext>(_ctx, getState());
  enterRule(_localctx, 158, EscriptParser::RuleDictInitializerExpression);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(798);
    expression(0);
    setState(801);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ARROW) {
      setState(799);
      match(EscriptParser::ARROW);
      setState(800);
      expression(0);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DictInitializerExpressionListContext ------------------------------------------------------------------

EscriptParser::DictInitializerExpressionListContext::DictInitializerExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::DictInitializerExpressionContext *> EscriptParser::DictInitializerExpressionListContext::dictInitializerExpression() {
  return getRuleContexts<EscriptParser::DictInitializerExpressionContext>();
}

EscriptParser::DictInitializerExpressionContext* EscriptParser::DictInitializerExpressionListContext::dictInitializerExpression(size_t i) {
  return getRuleContext<EscriptParser::DictInitializerExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::DictInitializerExpressionListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::DictInitializerExpressionListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::DictInitializerExpressionListContext::getRuleIndex() const {
  return EscriptParser::RuleDictInitializerExpressionList;
}

void EscriptParser::DictInitializerExpressionListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDictInitializerExpressionList(this);
}

void EscriptParser::DictInitializerExpressionListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDictInitializerExpressionList(this);
}


std::any EscriptParser::DictInitializerExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDictInitializerExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DictInitializerExpressionListContext* EscriptParser::dictInitializerExpressionList() {
  DictInitializerExpressionListContext *_localctx = _tracker.createInstance<DictInitializerExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 160, EscriptParser::RuleDictInitializerExpressionList);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(803);
    dictInitializerExpression();
    setState(808);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 79, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(804);
        match(EscriptParser::COMMA);
        setState(805);
        dictInitializerExpression(); 
      }
      setState(810);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 79, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DictInitializerContext ------------------------------------------------------------------

EscriptParser::DictInitializerContext::DictInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::DictInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::DictInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::DictInitializerExpressionListContext* EscriptParser::DictInitializerContext::dictInitializerExpressionList() {
  return getRuleContext<EscriptParser::DictInitializerExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::DictInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::DictInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleDictInitializer;
}

void EscriptParser::DictInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDictInitializer(this);
}

void EscriptParser::DictInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDictInitializer(this);
}


std::any EscriptParser::DictInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDictInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DictInitializerContext* EscriptParser::dictInitializer() {
  DictInitializerContext *_localctx = _tracker.createInstance<DictInitializerContext>(_ctx, getState());
  enterRule(_localctx, 162, EscriptParser::RuleDictInitializer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(823);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 82, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(811);
      match(EscriptParser::LBRACE);
      setState(813);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(812);
        dictInitializerExpressionList();
      }
      setState(815);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(816);
      match(EscriptParser::LBRACE);
      setState(818);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(817);
        dictInitializerExpressionList();
      }
      setState(820);
      match(EscriptParser::COMMA);
      setState(821);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in dictionary initializer list");
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArrayInitializerContext ------------------------------------------------------------------

EscriptParser::ArrayInitializerContext::ArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ArrayInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::ArrayInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::ArrayInitializerContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::ArrayInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}

tree::TerminalNode* EscriptParser::ArrayInitializerContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::ArrayInitializerContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}


size_t EscriptParser::ArrayInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleArrayInitializer;
}

void EscriptParser::ArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArrayInitializer(this);
}

void EscriptParser::ArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArrayInitializer(this);
}


std::any EscriptParser::ArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ArrayInitializerContext* EscriptParser::arrayInitializer() {
  ArrayInitializerContext *_localctx = _tracker.createInstance<ArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 164, EscriptParser::RuleArrayInitializer);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(849);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 87, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(825);
      match(EscriptParser::LBRACE);
      setState(827);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(826);
        expressionList();
      }
      setState(829);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(830);
      match(EscriptParser::LBRACE);
      setState(832);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(831);
        expressionList();
      }
      setState(834);
      match(EscriptParser::COMMA);
      setState(835);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(837);
      match(EscriptParser::LPAREN);
      setState(839);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(838);
        expressionList();
      }
      setState(841);
      match(EscriptParser::RPAREN);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(842);
      match(EscriptParser::LPAREN);
      setState(844);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (((((_la - 59) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 59)) & -7493989754099205677) != 0) || _la == EscriptParser::DEC

      || _la == EscriptParser::IDENTIFIER) {
        setState(843);
        expressionList();
      }
      setState(846);
      match(EscriptParser::COMMA);
      setState(847);
      match(EscriptParser::RPAREN);
      notifyErrorListeners("Expected expression following comma before right-paren in array initializer list");
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LiteralContext ------------------------------------------------------------------

EscriptParser::LiteralContext::LiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::IntegerLiteralContext* EscriptParser::LiteralContext::integerLiteral() {
  return getRuleContext<EscriptParser::IntegerLiteralContext>(0);
}

EscriptParser::FloatLiteralContext* EscriptParser::LiteralContext::floatLiteral() {
  return getRuleContext<EscriptParser::FloatLiteralContext>(0);
}

EscriptParser::BoolLiteralContext* EscriptParser::LiteralContext::boolLiteral() {
  return getRuleContext<EscriptParser::BoolLiteralContext>(0);
}

tree::TerminalNode* EscriptParser::LiteralContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::LiteralContext::UNINIT() {
  return getToken(EscriptParser::UNINIT, 0);
}


size_t EscriptParser::LiteralContext::getRuleIndex() const {
  return EscriptParser::RuleLiteral;
}

void EscriptParser::LiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterLiteral(this);
}

void EscriptParser::LiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitLiteral(this);
}


std::any EscriptParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::LiteralContext* EscriptParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 166, EscriptParser::RuleLiteral);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(856);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL: {
        enterOuterAlt(_localctx, 1);
        setState(851);
        integerLiteral();
        break;
      }

      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(852);
        floatLiteral();
        break;
      }

      case EscriptParser::BOOL_TRUE:
      case EscriptParser::BOOL_FALSE: {
        enterOuterAlt(_localctx, 3);
        setState(853);
        boolLiteral();
        break;
      }

      case EscriptParser::STRING_LITERAL: {
        enterOuterAlt(_localctx, 4);
        setState(854);
        match(EscriptParser::STRING_LITERAL);
        break;
      }

      case EscriptParser::UNINIT: {
        enterOuterAlt(_localctx, 5);
        setState(855);
        match(EscriptParser::UNINIT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InterpolatedStringContext ------------------------------------------------------------------

EscriptParser::InterpolatedStringContext::InterpolatedStringContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::InterpolatedStringContext::INTERPOLATED_STRING_START() {
  return getToken(EscriptParser::INTERPOLATED_STRING_START, 0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringContext::DOUBLE_QUOTE_INSIDE() {
  return getToken(EscriptParser::DOUBLE_QUOTE_INSIDE, 0);
}

std::vector<EscriptParser::InterpolatedStringPartContext *> EscriptParser::InterpolatedStringContext::interpolatedStringPart() {
  return getRuleContexts<EscriptParser::InterpolatedStringPartContext>();
}

EscriptParser::InterpolatedStringPartContext* EscriptParser::InterpolatedStringContext::interpolatedStringPart(size_t i) {
  return getRuleContext<EscriptParser::InterpolatedStringPartContext>(i);
}


size_t EscriptParser::InterpolatedStringContext::getRuleIndex() const {
  return EscriptParser::RuleInterpolatedString;
}

void EscriptParser::InterpolatedStringContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInterpolatedString(this);
}

void EscriptParser::InterpolatedStringContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInterpolatedString(this);
}


std::any EscriptParser::InterpolatedStringContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitInterpolatedString(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::InterpolatedStringContext* EscriptParser::interpolatedString() {
  InterpolatedStringContext *_localctx = _tracker.createInstance<InterpolatedStringContext>(_ctx, getState());
  enterRule(_localctx, 168, EscriptParser::RuleInterpolatedString);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(858);
    match(EscriptParser::INTERPOLATED_STRING_START);
    setState(862);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 130) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 130)) & 55) != 0)) {
      setState(859);
      interpolatedStringPart();
      setState(864);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(865);
    match(EscriptParser::DOUBLE_QUOTE_INSIDE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InterpolatedStringPartContext ------------------------------------------------------------------

EscriptParser::InterpolatedStringPartContext::InterpolatedStringPartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::LBRACE_INSIDE() {
  return getToken(EscriptParser::LBRACE_INSIDE, 0);
}

EscriptParser::ExpressionContext* EscriptParser::InterpolatedStringPartContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::COLON() {
  return getToken(EscriptParser::COLON, 0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::FORMAT_STRING() {
  return getToken(EscriptParser::FORMAT_STRING, 0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::DOUBLE_LBRACE_INSIDE() {
  return getToken(EscriptParser::DOUBLE_LBRACE_INSIDE, 0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::REGULAR_CHAR_INSIDE() {
  return getToken(EscriptParser::REGULAR_CHAR_INSIDE, 0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::DOUBLE_RBRACE() {
  return getToken(EscriptParser::DOUBLE_RBRACE, 0);
}

tree::TerminalNode* EscriptParser::InterpolatedStringPartContext::STRING_LITERAL_INSIDE() {
  return getToken(EscriptParser::STRING_LITERAL_INSIDE, 0);
}


size_t EscriptParser::InterpolatedStringPartContext::getRuleIndex() const {
  return EscriptParser::RuleInterpolatedStringPart;
}

void EscriptParser::InterpolatedStringPartContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInterpolatedStringPart(this);
}

void EscriptParser::InterpolatedStringPartContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInterpolatedStringPart(this);
}


std::any EscriptParser::InterpolatedStringPartContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitInterpolatedStringPart(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::InterpolatedStringPartContext* EscriptParser::interpolatedStringPart() {
  InterpolatedStringPartContext *_localctx = _tracker.createInstance<InterpolatedStringPartContext>(_ctx, getState());
  enterRule(_localctx, 170, EscriptParser::RuleInterpolatedStringPart);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(883);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 92, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(867);
      match(EscriptParser::LBRACE_INSIDE);
      setState(868);
      expression(0);
      setState(871);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::COLON) {
        setState(869);
        match(EscriptParser::COLON);
        setState(870);
        match(EscriptParser::FORMAT_STRING);
      }
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(873);
      match(EscriptParser::LBRACE_INSIDE);
      setState(876);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::COLON) {
        setState(874);
        match(EscriptParser::COLON);
        setState(875);
        match(EscriptParser::FORMAT_STRING);
      }
      notifyErrorListeners("Expected expression following interpolated string part start");
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(879);
      match(EscriptParser::DOUBLE_LBRACE_INSIDE);
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(880);
      match(EscriptParser::REGULAR_CHAR_INSIDE);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(881);
      match(EscriptParser::DOUBLE_RBRACE);
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(882);
      match(EscriptParser::STRING_LITERAL_INSIDE);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IntegerLiteralContext ------------------------------------------------------------------

EscriptParser::IntegerLiteralContext::IntegerLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::IntegerLiteralContext::DECIMAL_LITERAL() {
  return getToken(EscriptParser::DECIMAL_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::IntegerLiteralContext::HEX_LITERAL() {
  return getToken(EscriptParser::HEX_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::IntegerLiteralContext::OCT_LITERAL() {
  return getToken(EscriptParser::OCT_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::IntegerLiteralContext::BINARY_LITERAL() {
  return getToken(EscriptParser::BINARY_LITERAL, 0);
}


size_t EscriptParser::IntegerLiteralContext::getRuleIndex() const {
  return EscriptParser::RuleIntegerLiteral;
}

void EscriptParser::IntegerLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIntegerLiteral(this);
}

void EscriptParser::IntegerLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIntegerLiteral(this);
}


std::any EscriptParser::IntegerLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIntegerLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IntegerLiteralContext* EscriptParser::integerLiteral() {
  IntegerLiteralContext *_localctx = _tracker.createInstance<IntegerLiteralContext>(_ctx, getState());
  enterRule(_localctx, 172, EscriptParser::RuleIntegerLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(885);
    _la = _input->LA(1);
    if (!(((((_la - 73) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 73)) & 15) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FloatLiteralContext ------------------------------------------------------------------

EscriptParser::FloatLiteralContext::FloatLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::FloatLiteralContext::FLOAT_LITERAL() {
  return getToken(EscriptParser::FLOAT_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::FloatLiteralContext::HEX_FLOAT_LITERAL() {
  return getToken(EscriptParser::HEX_FLOAT_LITERAL, 0);
}


size_t EscriptParser::FloatLiteralContext::getRuleIndex() const {
  return EscriptParser::RuleFloatLiteral;
}

void EscriptParser::FloatLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFloatLiteral(this);
}

void EscriptParser::FloatLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFloatLiteral(this);
}


std::any EscriptParser::FloatLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFloatLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FloatLiteralContext* EscriptParser::floatLiteral() {
  FloatLiteralContext *_localctx = _tracker.createInstance<FloatLiteralContext>(_ctx, getState());
  enterRule(_localctx, 174, EscriptParser::RuleFloatLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(887);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::FLOAT_LITERAL

    || _la == EscriptParser::HEX_FLOAT_LITERAL)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- BoolLiteralContext ------------------------------------------------------------------

EscriptParser::BoolLiteralContext::BoolLiteralContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::BoolLiteralContext::BOOL_TRUE() {
  return getToken(EscriptParser::BOOL_TRUE, 0);
}

tree::TerminalNode* EscriptParser::BoolLiteralContext::BOOL_FALSE() {
  return getToken(EscriptParser::BOOL_FALSE, 0);
}


size_t EscriptParser::BoolLiteralContext::getRuleIndex() const {
  return EscriptParser::RuleBoolLiteral;
}

void EscriptParser::BoolLiteralContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBoolLiteral(this);
}

void EscriptParser::BoolLiteralContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBoolLiteral(this);
}


std::any EscriptParser::BoolLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBoolLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BoolLiteralContext* EscriptParser::boolLiteral() {
  BoolLiteralContext *_localctx = _tracker.createInstance<BoolLiteralContext>(_ctx, getState());
  enterRule(_localctx, 176, EscriptParser::RuleBoolLiteral);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(889);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::BOOL_TRUE

    || _la == EscriptParser::BOOL_FALSE)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool EscriptParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 57: return expressionSempred(antlrcpp::downCast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool EscriptParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 12);
    case 1: return precpred(_ctx, 11);
    case 2: return precpred(_ctx, 10);
    case 3: return precpred(_ctx, 9);
    case 4: return precpred(_ctx, 8);
    case 5: return precpred(_ctx, 7);
    case 6: return precpred(_ctx, 6);
    case 7: return precpred(_ctx, 5);
    case 8: return precpred(_ctx, 4);
    case 9: return precpred(_ctx, 3);
    case 10: return precpred(_ctx, 2);
    case 11: return precpred(_ctx, 1);
    case 12: return precpred(_ctx, 16);
    case 13: return precpred(_ctx, 15);

  default:
    break;
  }
  return true;
}

void EscriptParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  escriptparserParserInitialize();
#else
  ::antlr4::internal::call_once(escriptparserParserOnceFlag, escriptparserParserInitialize);
#endif
}
