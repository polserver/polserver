#pragma once

#include "antlr4-runtime.h"

namespace EscriptGrammar
{
class EscriptParserRuleContext : public antlr4::ParserRuleContext
{
public:
  EscriptParserRuleContext() : antlr4::ParserRuleContext() {}
  EscriptParserRuleContext( antlr4::ParserRuleContext* parent, size_t invokingStateNumber )
      : ParserRuleContext( parent, invokingStateNumber )
  {
  }
  virtual ~EscriptParserRuleContext() {}

  bool has_parse_errors = false;
};
}  // namespace EscriptGrammar
