


// Generated from lib/EscriptGrammar/EscriptParser.g4 by ANTLR 4.8


#include "EscriptParserListener.h"
#include "EscriptParserVisitor.h"

#include "EscriptParser.h"


using namespace antlrcpp;
using namespace EscriptGrammar;
using namespace antlr4;

EscriptParser::EscriptParser(antlr4::TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

EscriptParser::~EscriptParser() {
  delete _interpreter;
}

std::string EscriptParser::getGrammarFileName() const {
  return "EscriptParser.g4";
}

const std::vector<std::string>& EscriptParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& EscriptParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- UnambiguousCompilationUnitContext ------------------------------------------------------------------

EscriptParser::UnambiguousCompilationUnitContext::UnambiguousCompilationUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousCompilationUnitContext::EOF() {
  return getToken(EscriptParser::EOF, 0);
}

std::vector<EscriptParser::UnambiguousTopLevelDeclarationContext *> EscriptParser::UnambiguousCompilationUnitContext::unambiguousTopLevelDeclaration() {
  return getRuleContexts<EscriptParser::UnambiguousTopLevelDeclarationContext>();
}

EscriptParser::UnambiguousTopLevelDeclarationContext* EscriptParser::UnambiguousCompilationUnitContext::unambiguousTopLevelDeclaration(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousTopLevelDeclarationContext>(i);
}


size_t EscriptParser::UnambiguousCompilationUnitContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousCompilationUnit;
}

void EscriptParser::UnambiguousCompilationUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousCompilationUnit(this);
}

void EscriptParser::UnambiguousCompilationUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousCompilationUnit(this);
}


antlrcpp::Any EscriptParser::UnambiguousCompilationUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousCompilationUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousCompilationUnitContext* EscriptParser::unambiguousCompilationUnit() {
  UnambiguousCompilationUnitContext *_localctx = _tracker.createInstance<UnambiguousCompilationUnitContext>(_ctx, getState());
  enterRule(_localctx, 0, EscriptParser::RuleUnambiguousCompilationUnit);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(327);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::IF)
      | (1ULL << EscriptParser::RETURN)
      | (1ULL << EscriptParser::TOK_CONST)
      | (1ULL << EscriptParser::VAR)
      | (1ULL << EscriptParser::DO)
      | (1ULL << EscriptParser::WHILE)
      | (1ULL << EscriptParser::EXIT)
      | (1ULL << EscriptParser::FUNCTION)
      | (1ULL << EscriptParser::EXPORTED)
      | (1ULL << EscriptParser::USE)
      | (1ULL << EscriptParser::INCLUDE)
      | (1ULL << EscriptParser::BREAK)
      | (1ULL << EscriptParser::CONTINUE)
      | (1ULL << EscriptParser::FOR)
      | (1ULL << EscriptParser::FOREACH)
      | (1ULL << EscriptParser::REPEAT)
      | (1ULL << EscriptParser::PROGRAM)
      | (1ULL << EscriptParser::CASE)
      | (1ULL << EscriptParser::ENUM)
      | (1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::SEMI - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(324);
      unambiguousTopLevelDeclaration();
      setState(329);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(330);
    match(EscriptParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
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


antlrcpp::Any EscriptParser::CompilationUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCompilationUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CompilationUnitContext* EscriptParser::compilationUnit() {
  CompilationUnitContext *_localctx = _tracker.createInstance<CompilationUnitContext>(_ctx, getState());
  enterRule(_localctx, 2, EscriptParser::RuleCompilationUnit);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(335);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::IF)
      | (1ULL << EscriptParser::GOTO)
      | (1ULL << EscriptParser::RETURN)
      | (1ULL << EscriptParser::TOK_CONST)
      | (1ULL << EscriptParser::VAR)
      | (1ULL << EscriptParser::DO)
      | (1ULL << EscriptParser::WHILE)
      | (1ULL << EscriptParser::EXIT)
      | (1ULL << EscriptParser::DECLARE)
      | (1ULL << EscriptParser::FUNCTION)
      | (1ULL << EscriptParser::EXPORTED)
      | (1ULL << EscriptParser::USE)
      | (1ULL << EscriptParser::INCLUDE)
      | (1ULL << EscriptParser::BREAK)
      | (1ULL << EscriptParser::CONTINUE)
      | (1ULL << EscriptParser::FOR)
      | (1ULL << EscriptParser::FOREACH)
      | (1ULL << EscriptParser::REPEAT)
      | (1ULL << EscriptParser::PROGRAM)
      | (1ULL << EscriptParser::CASE)
      | (1ULL << EscriptParser::ENUM)
      | (1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::SEMI - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(332);
      topLevelDeclaration();
      setState(337);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(338);
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


antlrcpp::Any EscriptParser::ModuleUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleUnitContext* EscriptParser::moduleUnit() {
  ModuleUnitContext *_localctx = _tracker.createInstance<ModuleUnitContext>(_ctx, getState());
  enterRule(_localctx, 4, EscriptParser::RuleModuleUnit);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(343);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::TOK_CONST || _la == EscriptParser::IDENTIFIER) {
      setState(340);
      moduleDeclarationStatement();
      setState(345);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(346);
    match(EscriptParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousModuleUnitContext ------------------------------------------------------------------

EscriptParser::UnambiguousModuleUnitContext::UnambiguousModuleUnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousModuleUnitContext::EOF() {
  return getToken(EscriptParser::EOF, 0);
}

std::vector<EscriptParser::UnambiguousModuleDeclarationStatementContext *> EscriptParser::UnambiguousModuleUnitContext::unambiguousModuleDeclarationStatement() {
  return getRuleContexts<EscriptParser::UnambiguousModuleDeclarationStatementContext>();
}

EscriptParser::UnambiguousModuleDeclarationStatementContext* EscriptParser::UnambiguousModuleUnitContext::unambiguousModuleDeclarationStatement(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousModuleDeclarationStatementContext>(i);
}


size_t EscriptParser::UnambiguousModuleUnitContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousModuleUnit;
}

void EscriptParser::UnambiguousModuleUnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousModuleUnit(this);
}

void EscriptParser::UnambiguousModuleUnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousModuleUnit(this);
}


antlrcpp::Any EscriptParser::UnambiguousModuleUnitContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousModuleUnit(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousModuleUnitContext* EscriptParser::unambiguousModuleUnit() {
  UnambiguousModuleUnitContext *_localctx = _tracker.createInstance<UnambiguousModuleUnitContext>(_ctx, getState());
  enterRule(_localctx, 6, EscriptParser::RuleUnambiguousModuleUnit);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(351);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::TOK_CONST || _la == EscriptParser::IDENTIFIER) {
      setState(348);
      unambiguousModuleDeclarationStatement();
      setState(353);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(354);
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


antlrcpp::Any EscriptParser::ModuleDeclarationStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleDeclarationStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleDeclarationStatementContext* EscriptParser::moduleDeclarationStatement() {
  ModuleDeclarationStatementContext *_localctx = _tracker.createInstance<ModuleDeclarationStatementContext>(_ctx, getState());
  enterRule(_localctx, 8, EscriptParser::RuleModuleDeclarationStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(358);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(356);
        moduleFunctionDeclaration();
        break;
      }

      case EscriptParser::TOK_CONST: {
        enterOuterAlt(_localctx, 2);
        setState(357);
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

//----------------- UnambiguousModuleDeclarationStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousModuleDeclarationStatementContext::UnambiguousModuleDeclarationStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::UnambiguousModuleFunctionDeclarationContext* EscriptParser::UnambiguousModuleDeclarationStatementContext::unambiguousModuleFunctionDeclaration() {
  return getRuleContext<EscriptParser::UnambiguousModuleFunctionDeclarationContext>(0);
}

EscriptParser::UnambiguousConstStatementContext* EscriptParser::UnambiguousModuleDeclarationStatementContext::unambiguousConstStatement() {
  return getRuleContext<EscriptParser::UnambiguousConstStatementContext>(0);
}


size_t EscriptParser::UnambiguousModuleDeclarationStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousModuleDeclarationStatement;
}

void EscriptParser::UnambiguousModuleDeclarationStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousModuleDeclarationStatement(this);
}

void EscriptParser::UnambiguousModuleDeclarationStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousModuleDeclarationStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousModuleDeclarationStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousModuleDeclarationStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousModuleDeclarationStatementContext* EscriptParser::unambiguousModuleDeclarationStatement() {
  UnambiguousModuleDeclarationStatementContext *_localctx = _tracker.createInstance<UnambiguousModuleDeclarationStatementContext>(_ctx, getState());
  enterRule(_localctx, 10, EscriptParser::RuleUnambiguousModuleDeclarationStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(362);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(360);
        unambiguousModuleFunctionDeclaration();
        break;
      }

      case EscriptParser::TOK_CONST: {
        enterOuterAlt(_localctx, 2);
        setState(361);
        unambiguousConstStatement();
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


antlrcpp::Any EscriptParser::ModuleFunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleFunctionDeclarationContext* EscriptParser::moduleFunctionDeclaration() {
  ModuleFunctionDeclarationContext *_localctx = _tracker.createInstance<ModuleFunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 12, EscriptParser::RuleModuleFunctionDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(364);
    match(EscriptParser::IDENTIFIER);
    setState(365);
    match(EscriptParser::LPAREN);
    setState(367);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(366);
      moduleFunctionParameterList();
    }
    setState(369);
    match(EscriptParser::RPAREN);
    setState(370);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousModuleFunctionDeclarationContext ------------------------------------------------------------------

EscriptParser::UnambiguousModuleFunctionDeclarationContext::UnambiguousModuleFunctionDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionDeclarationContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionDeclarationContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionDeclarationContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::UnambiguousModuleFunctionParameterListContext* EscriptParser::UnambiguousModuleFunctionDeclarationContext::unambiguousModuleFunctionParameterList() {
  return getRuleContext<EscriptParser::UnambiguousModuleFunctionParameterListContext>(0);
}


size_t EscriptParser::UnambiguousModuleFunctionDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousModuleFunctionDeclaration;
}

void EscriptParser::UnambiguousModuleFunctionDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousModuleFunctionDeclaration(this);
}

void EscriptParser::UnambiguousModuleFunctionDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousModuleFunctionDeclaration(this);
}


antlrcpp::Any EscriptParser::UnambiguousModuleFunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousModuleFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousModuleFunctionDeclarationContext* EscriptParser::unambiguousModuleFunctionDeclaration() {
  UnambiguousModuleFunctionDeclarationContext *_localctx = _tracker.createInstance<UnambiguousModuleFunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 14, EscriptParser::RuleUnambiguousModuleFunctionDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(372);
    match(EscriptParser::IDENTIFIER);
    setState(373);
    match(EscriptParser::LPAREN);
    setState(375);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(374);
      unambiguousModuleFunctionParameterList();
    }
    setState(377);
    match(EscriptParser::RPAREN);
    setState(378);
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


antlrcpp::Any EscriptParser::ModuleFunctionParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleFunctionParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleFunctionParameterListContext* EscriptParser::moduleFunctionParameterList() {
  ModuleFunctionParameterListContext *_localctx = _tracker.createInstance<ModuleFunctionParameterListContext>(_ctx, getState());
  enterRule(_localctx, 16, EscriptParser::RuleModuleFunctionParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(380);
    moduleFunctionParameter();
    setState(385);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(381);
      match(EscriptParser::COMMA);
      setState(382);
      moduleFunctionParameter();
      setState(387);
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

//----------------- UnambiguousModuleFunctionParameterListContext ------------------------------------------------------------------

EscriptParser::UnambiguousModuleFunctionParameterListContext::UnambiguousModuleFunctionParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousModuleFunctionParameterContext *> EscriptParser::UnambiguousModuleFunctionParameterListContext::unambiguousModuleFunctionParameter() {
  return getRuleContexts<EscriptParser::UnambiguousModuleFunctionParameterContext>();
}

EscriptParser::UnambiguousModuleFunctionParameterContext* EscriptParser::UnambiguousModuleFunctionParameterListContext::unambiguousModuleFunctionParameter(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousModuleFunctionParameterContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousModuleFunctionParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousModuleFunctionParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousModuleFunctionParameterList;
}

void EscriptParser::UnambiguousModuleFunctionParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousModuleFunctionParameterList(this);
}

void EscriptParser::UnambiguousModuleFunctionParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousModuleFunctionParameterList(this);
}


antlrcpp::Any EscriptParser::UnambiguousModuleFunctionParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousModuleFunctionParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousModuleFunctionParameterListContext* EscriptParser::unambiguousModuleFunctionParameterList() {
  UnambiguousModuleFunctionParameterListContext *_localctx = _tracker.createInstance<UnambiguousModuleFunctionParameterListContext>(_ctx, getState());
  enterRule(_localctx, 18, EscriptParser::RuleUnambiguousModuleFunctionParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(388);
    unambiguousModuleFunctionParameter();
    setState(393);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(389);
      match(EscriptParser::COMMA);
      setState(390);
      unambiguousModuleFunctionParameter();
      setState(395);
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


antlrcpp::Any EscriptParser::ModuleFunctionParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitModuleFunctionParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ModuleFunctionParameterContext* EscriptParser::moduleFunctionParameter() {
  ModuleFunctionParameterContext *_localctx = _tracker.createInstance<ModuleFunctionParameterContext>(_ctx, getState());
  enterRule(_localctx, 20, EscriptParser::RuleModuleFunctionParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(396);
    match(EscriptParser::IDENTIFIER);
    setState(399);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(397);
      match(EscriptParser::ASSIGN);
      setState(398);
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

//----------------- UnambiguousModuleFunctionParameterContext ------------------------------------------------------------------

EscriptParser::UnambiguousModuleFunctionParameterContext::UnambiguousModuleFunctionParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionParameterContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousModuleFunctionParameterContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousModuleFunctionParameterContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousModuleFunctionParameterContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousModuleFunctionParameter;
}

void EscriptParser::UnambiguousModuleFunctionParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousModuleFunctionParameter(this);
}

void EscriptParser::UnambiguousModuleFunctionParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousModuleFunctionParameter(this);
}


antlrcpp::Any EscriptParser::UnambiguousModuleFunctionParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousModuleFunctionParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousModuleFunctionParameterContext* EscriptParser::unambiguousModuleFunctionParameter() {
  UnambiguousModuleFunctionParameterContext *_localctx = _tracker.createInstance<UnambiguousModuleFunctionParameterContext>(_ctx, getState());
  enterRule(_localctx, 22, EscriptParser::RuleUnambiguousModuleFunctionParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(401);
    match(EscriptParser::IDENTIFIER);
    setState(404);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(402);
      match(EscriptParser::ASSIGN);
      setState(403);
      unambiguousExpression();
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


antlrcpp::Any EscriptParser::TopLevelDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitTopLevelDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::TopLevelDeclarationContext* EscriptParser::topLevelDeclaration() {
  TopLevelDeclarationContext *_localctx = _tracker.createInstance<TopLevelDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 24, EscriptParser::RuleTopLevelDeclaration);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(411);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::USE: {
        enterOuterAlt(_localctx, 1);
        setState(406);
        useDeclaration();
        break;
      }

      case EscriptParser::INCLUDE: {
        enterOuterAlt(_localctx, 2);
        setState(407);
        includeDeclaration();
        break;
      }

      case EscriptParser::PROGRAM: {
        enterOuterAlt(_localctx, 3);
        setState(408);
        programDeclaration();
        break;
      }

      case EscriptParser::FUNCTION:
      case EscriptParser::EXPORTED: {
        enterOuterAlt(_localctx, 4);
        setState(409);
        functionDeclaration();
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
      case EscriptParser::DECLARE:
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
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL:
      case EscriptParser::CHAR_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::NULL_LITERAL:
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
        enterOuterAlt(_localctx, 5);
        setState(410);
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

//----------------- UnambiguousTopLevelDeclarationContext ------------------------------------------------------------------

EscriptParser::UnambiguousTopLevelDeclarationContext::UnambiguousTopLevelDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::UseDeclarationContext* EscriptParser::UnambiguousTopLevelDeclarationContext::useDeclaration() {
  return getRuleContext<EscriptParser::UseDeclarationContext>(0);
}

EscriptParser::IncludeDeclarationContext* EscriptParser::UnambiguousTopLevelDeclarationContext::includeDeclaration() {
  return getRuleContext<EscriptParser::IncludeDeclarationContext>(0);
}

EscriptParser::UnambiguousProgramDeclarationContext* EscriptParser::UnambiguousTopLevelDeclarationContext::unambiguousProgramDeclaration() {
  return getRuleContext<EscriptParser::UnambiguousProgramDeclarationContext>(0);
}

EscriptParser::UnambiguousFunctionDeclarationContext* EscriptParser::UnambiguousTopLevelDeclarationContext::unambiguousFunctionDeclaration() {
  return getRuleContext<EscriptParser::UnambiguousFunctionDeclarationContext>(0);
}

EscriptParser::UnambiguousStatementContext* EscriptParser::UnambiguousTopLevelDeclarationContext::unambiguousStatement() {
  return getRuleContext<EscriptParser::UnambiguousStatementContext>(0);
}


size_t EscriptParser::UnambiguousTopLevelDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousTopLevelDeclaration;
}

void EscriptParser::UnambiguousTopLevelDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousTopLevelDeclaration(this);
}

void EscriptParser::UnambiguousTopLevelDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousTopLevelDeclaration(this);
}


antlrcpp::Any EscriptParser::UnambiguousTopLevelDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousTopLevelDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousTopLevelDeclarationContext* EscriptParser::unambiguousTopLevelDeclaration() {
  UnambiguousTopLevelDeclarationContext *_localctx = _tracker.createInstance<UnambiguousTopLevelDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 26, EscriptParser::RuleUnambiguousTopLevelDeclaration);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(418);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::USE: {
        enterOuterAlt(_localctx, 1);
        setState(413);
        useDeclaration();
        break;
      }

      case EscriptParser::INCLUDE: {
        enterOuterAlt(_localctx, 2);
        setState(414);
        includeDeclaration();
        break;
      }

      case EscriptParser::PROGRAM: {
        enterOuterAlt(_localctx, 3);
        setState(415);
        unambiguousProgramDeclaration();
        break;
      }

      case EscriptParser::FUNCTION:
      case EscriptParser::EXPORTED: {
        enterOuterAlt(_localctx, 4);
        setState(416);
        unambiguousFunctionDeclaration();
        break;
      }

      case EscriptParser::IF:
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
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL:
      case EscriptParser::CHAR_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::NULL_LITERAL:
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
        enterOuterAlt(_localctx, 5);
        setState(417);
        unambiguousStatement();
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


antlrcpp::Any EscriptParser::FunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionDeclarationContext* EscriptParser::functionDeclaration() {
  FunctionDeclarationContext *_localctx = _tracker.createInstance<FunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 28, EscriptParser::RuleFunctionDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(421);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::EXPORTED) {
      setState(420);
      match(EscriptParser::EXPORTED);
    }
    setState(423);
    match(EscriptParser::FUNCTION);
    setState(424);
    match(EscriptParser::IDENTIFIER);
    setState(425);
    functionParameters();
    setState(426);
    block();
    setState(427);
    match(EscriptParser::ENDFUNCTION);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousFunctionDeclarationContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionDeclarationContext::UnambiguousFunctionDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionDeclarationContext::FUNCTION() {
  return getToken(EscriptParser::FUNCTION, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::UnambiguousFunctionParametersContext* EscriptParser::UnambiguousFunctionDeclarationContext::unambiguousFunctionParameters() {
  return getRuleContext<EscriptParser::UnambiguousFunctionParametersContext>(0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousFunctionDeclarationContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionDeclarationContext::ENDFUNCTION() {
  return getToken(EscriptParser::ENDFUNCTION, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionDeclarationContext::EXPORTED() {
  return getToken(EscriptParser::EXPORTED, 0);
}


size_t EscriptParser::UnambiguousFunctionDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionDeclaration;
}

void EscriptParser::UnambiguousFunctionDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionDeclaration(this);
}

void EscriptParser::UnambiguousFunctionDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionDeclaration(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionDeclarationContext* EscriptParser::unambiguousFunctionDeclaration() {
  UnambiguousFunctionDeclarationContext *_localctx = _tracker.createInstance<UnambiguousFunctionDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 30, EscriptParser::RuleUnambiguousFunctionDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(430);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::EXPORTED) {
      setState(429);
      match(EscriptParser::EXPORTED);
    }
    setState(432);
    match(EscriptParser::FUNCTION);
    setState(433);
    match(EscriptParser::IDENTIFIER);
    setState(434);
    unambiguousFunctionParameters();
    setState(435);
    unambiguousBlock();
    setState(436);
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


antlrcpp::Any EscriptParser::StringIdentifierContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStringIdentifier(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StringIdentifierContext* EscriptParser::stringIdentifier() {
  StringIdentifierContext *_localctx = _tracker.createInstance<StringIdentifierContext>(_ctx, getState());
  enterRule(_localctx, 32, EscriptParser::RuleStringIdentifier);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(438);
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


antlrcpp::Any EscriptParser::UseDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUseDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UseDeclarationContext* EscriptParser::useDeclaration() {
  UseDeclarationContext *_localctx = _tracker.createInstance<UseDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 34, EscriptParser::RuleUseDeclaration);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(440);
    match(EscriptParser::USE);
    setState(441);
    stringIdentifier();
    setState(442);
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


antlrcpp::Any EscriptParser::IncludeDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIncludeDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IncludeDeclarationContext* EscriptParser::includeDeclaration() {
  IncludeDeclarationContext *_localctx = _tracker.createInstance<IncludeDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 36, EscriptParser::RuleIncludeDeclaration);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(444);
    match(EscriptParser::INCLUDE);
    setState(445);
    stringIdentifier();
    setState(446);
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


antlrcpp::Any EscriptParser::ProgramDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramDeclarationContext* EscriptParser::programDeclaration() {
  ProgramDeclarationContext *_localctx = _tracker.createInstance<ProgramDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 38, EscriptParser::RuleProgramDeclaration);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(448);
    match(EscriptParser::PROGRAM);
    setState(449);
    match(EscriptParser::IDENTIFIER);
    setState(450);
    programParameters();
    setState(451);
    block();
    setState(452);
    match(EscriptParser::ENDPROGRAM);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousProgramDeclarationContext ------------------------------------------------------------------

EscriptParser::UnambiguousProgramDeclarationContext::UnambiguousProgramDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousProgramDeclarationContext::PROGRAM() {
  return getToken(EscriptParser::PROGRAM, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousProgramDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::UnambiguousProgramParametersContext* EscriptParser::UnambiguousProgramDeclarationContext::unambiguousProgramParameters() {
  return getRuleContext<EscriptParser::UnambiguousProgramParametersContext>(0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousProgramDeclarationContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousProgramDeclarationContext::ENDPROGRAM() {
  return getToken(EscriptParser::ENDPROGRAM, 0);
}


size_t EscriptParser::UnambiguousProgramDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousProgramDeclaration;
}

void EscriptParser::UnambiguousProgramDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousProgramDeclaration(this);
}

void EscriptParser::UnambiguousProgramDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousProgramDeclaration(this);
}


antlrcpp::Any EscriptParser::UnambiguousProgramDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousProgramDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousProgramDeclarationContext* EscriptParser::unambiguousProgramDeclaration() {
  UnambiguousProgramDeclarationContext *_localctx = _tracker.createInstance<UnambiguousProgramDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 40, EscriptParser::RuleUnambiguousProgramDeclaration);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(454);
    match(EscriptParser::PROGRAM);
    setState(455);
    match(EscriptParser::IDENTIFIER);
    setState(456);
    unambiguousProgramParameters();
    setState(457);
    unambiguousBlock();
    setState(458);
    match(EscriptParser::ENDPROGRAM);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousStatementContext::UnambiguousStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::UnambiguousIfStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousIfStatement() {
  return getRuleContext<EscriptParser::UnambiguousIfStatementContext>(0);
}

EscriptParser::UnambiguousReturnStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousReturnStatement() {
  return getRuleContext<EscriptParser::UnambiguousReturnStatementContext>(0);
}

EscriptParser::UnambiguousConstStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousConstStatement() {
  return getRuleContext<EscriptParser::UnambiguousConstStatementContext>(0);
}

EscriptParser::UnambiguousVarStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousVarStatement() {
  return getRuleContext<EscriptParser::UnambiguousVarStatementContext>(0);
}

EscriptParser::UnambiguousDoStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousDoStatement() {
  return getRuleContext<EscriptParser::UnambiguousDoStatementContext>(0);
}

EscriptParser::UnambiguousWhileStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousWhileStatement() {
  return getRuleContext<EscriptParser::UnambiguousWhileStatementContext>(0);
}

EscriptParser::ExitStatementContext* EscriptParser::UnambiguousStatementContext::exitStatement() {
  return getRuleContext<EscriptParser::ExitStatementContext>(0);
}

EscriptParser::BreakStatementContext* EscriptParser::UnambiguousStatementContext::breakStatement() {
  return getRuleContext<EscriptParser::BreakStatementContext>(0);
}

EscriptParser::ContinueStatementContext* EscriptParser::UnambiguousStatementContext::continueStatement() {
  return getRuleContext<EscriptParser::ContinueStatementContext>(0);
}

EscriptParser::UnambiguousForStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousForStatement() {
  return getRuleContext<EscriptParser::UnambiguousForStatementContext>(0);
}

EscriptParser::UnambiguousForeachStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousForeachStatement() {
  return getRuleContext<EscriptParser::UnambiguousForeachStatementContext>(0);
}

EscriptParser::UnambiguousRepeatStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousRepeatStatement() {
  return getRuleContext<EscriptParser::UnambiguousRepeatStatementContext>(0);
}

EscriptParser::UnambiguousCaseStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousCaseStatement() {
  return getRuleContext<EscriptParser::UnambiguousCaseStatementContext>(0);
}

EscriptParser::UnambiguousEnumStatementContext* EscriptParser::UnambiguousStatementContext::unambiguousEnumStatement() {
  return getRuleContext<EscriptParser::UnambiguousEnumStatementContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousStatementContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousStatement;
}

void EscriptParser::UnambiguousStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousStatement(this);
}

void EscriptParser::UnambiguousStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousStatementContext* EscriptParser::unambiguousStatement() {
  UnambiguousStatementContext *_localctx = _tracker.createInstance<UnambiguousStatementContext>(_ctx, getState());
  enterRule(_localctx, 42, EscriptParser::RuleUnambiguousStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(478);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 16, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(460);
      unambiguousIfStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(461);
      unambiguousReturnStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(462);
      unambiguousConstStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(463);
      unambiguousVarStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(464);
      unambiguousDoStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(465);
      unambiguousWhileStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(466);
      exitStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(467);
      breakStatement();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(468);
      continueStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(469);
      unambiguousForStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(470);
      unambiguousForeachStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(471);
      unambiguousRepeatStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(472);
      unambiguousCaseStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(473);
      unambiguousEnumStatement();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(474);
      match(EscriptParser::SEMI);
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(475);
      dynamic_cast<UnambiguousStatementContext *>(_localctx)->statementUnambiguousExpression = unambiguousExpression();
      setState(476);
      match(EscriptParser::SEMI);
      break;
    }

    }
   
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

EscriptParser::DeclareStatementContext* EscriptParser::StatementContext::declareStatement() {
  return getRuleContext<EscriptParser::DeclareStatementContext>(0);
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


antlrcpp::Any EscriptParser::StatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StatementContext* EscriptParser::statement() {
  StatementContext *_localctx = _tracker.createInstance<StatementContext>(_ctx, getState());
  enterRule(_localctx, 44, EscriptParser::RuleStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(500);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 17, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(480);
      ifStatement();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(481);
      gotoStatement();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(482);
      returnStatement();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(483);
      constStatement();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(484);
      varStatement();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(485);
      doStatement();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(486);
      whileStatement();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(487);
      exitStatement();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(488);
      declareStatement();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(489);
      breakStatement();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(490);
      continueStatement();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(491);
      forStatement();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(492);
      foreachStatement();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(493);
      repeatStatement();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(494);
      caseStatement();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(495);
      enumStatement();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(496);
      match(EscriptParser::SEMI);
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(497);
      dynamic_cast<StatementContext *>(_localctx)->statementExpression = expression(0);
      setState(498);
      match(EscriptParser::SEMI);
      break;
    }

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


antlrcpp::Any EscriptParser::StatementLabelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStatementLabel(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StatementLabelContext* EscriptParser::statementLabel() {
  StatementLabelContext *_localctx = _tracker.createInstance<StatementLabelContext>(_ctx, getState());
  enterRule(_localctx, 46, EscriptParser::RuleStatementLabel);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(502);
    match(EscriptParser::IDENTIFIER);
    setState(503);
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


antlrcpp::Any EscriptParser::IfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIfStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IfStatementContext* EscriptParser::ifStatement() {
  IfStatementContext *_localctx = _tracker.createInstance<IfStatementContext>(_ctx, getState());
  enterRule(_localctx, 48, EscriptParser::RuleIfStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(505);
    match(EscriptParser::IF);
    setState(506);
    parExpression();
    setState(508);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::THEN) {
      setState(507);
      match(EscriptParser::THEN);
    }
    setState(510);
    block();
    setState(517);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::ELSEIF) {
      setState(511);
      match(EscriptParser::ELSEIF);
      setState(512);
      parExpression();
      setState(513);
      block();
      setState(519);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(522);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ELSE) {
      setState(520);
      match(EscriptParser::ELSE);
      setState(521);
      block();
    }
    setState(524);
    match(EscriptParser::ENDIF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousIfStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousIfStatementContext::UnambiguousIfStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousIfStatementContext::IF() {
  return getToken(EscriptParser::IF, 0);
}

std::vector<EscriptParser::ParenthesizedExpressionContext *> EscriptParser::UnambiguousIfStatementContext::parenthesizedExpression() {
  return getRuleContexts<EscriptParser::ParenthesizedExpressionContext>();
}

EscriptParser::ParenthesizedExpressionContext* EscriptParser::UnambiguousIfStatementContext::parenthesizedExpression(size_t i) {
  return getRuleContext<EscriptParser::ParenthesizedExpressionContext>(i);
}

std::vector<EscriptParser::UnambiguousBlockContext *> EscriptParser::UnambiguousIfStatementContext::unambiguousBlock() {
  return getRuleContexts<EscriptParser::UnambiguousBlockContext>();
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousIfStatementContext::unambiguousBlock(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(i);
}

tree::TerminalNode* EscriptParser::UnambiguousIfStatementContext::ENDIF() {
  return getToken(EscriptParser::ENDIF, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousIfStatementContext::THEN() {
  return getToken(EscriptParser::THEN, 0);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousIfStatementContext::ELSEIF() {
  return getTokens(EscriptParser::ELSEIF);
}

tree::TerminalNode* EscriptParser::UnambiguousIfStatementContext::ELSEIF(size_t i) {
  return getToken(EscriptParser::ELSEIF, i);
}

tree::TerminalNode* EscriptParser::UnambiguousIfStatementContext::ELSE() {
  return getToken(EscriptParser::ELSE, 0);
}


size_t EscriptParser::UnambiguousIfStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousIfStatement;
}

void EscriptParser::UnambiguousIfStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousIfStatement(this);
}

void EscriptParser::UnambiguousIfStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousIfStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousIfStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousIfStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousIfStatementContext* EscriptParser::unambiguousIfStatement() {
  UnambiguousIfStatementContext *_localctx = _tracker.createInstance<UnambiguousIfStatementContext>(_ctx, getState());
  enterRule(_localctx, 50, EscriptParser::RuleUnambiguousIfStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(526);
    match(EscriptParser::IF);
    setState(527);
    parenthesizedExpression();
    setState(529);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::THEN) {
      setState(528);
      match(EscriptParser::THEN);
    }
    setState(531);
    unambiguousBlock();
    setState(538);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::ELSEIF) {
      setState(532);
      match(EscriptParser::ELSEIF);
      setState(533);
      parenthesizedExpression();
      setState(534);
      unambiguousBlock();
      setState(540);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(543);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ELSE) {
      setState(541);
      match(EscriptParser::ELSE);
      setState(542);
      unambiguousBlock();
    }
    setState(545);
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


antlrcpp::Any EscriptParser::GotoStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitGotoStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::GotoStatementContext* EscriptParser::gotoStatement() {
  GotoStatementContext *_localctx = _tracker.createInstance<GotoStatementContext>(_ctx, getState());
  enterRule(_localctx, 52, EscriptParser::RuleGotoStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(547);
    match(EscriptParser::GOTO);
    setState(548);
    match(EscriptParser::IDENTIFIER);
    setState(549);
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


antlrcpp::Any EscriptParser::ReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ReturnStatementContext* EscriptParser::returnStatement() {
  ReturnStatementContext *_localctx = _tracker.createInstance<ReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 54, EscriptParser::RuleReturnStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(551);
    match(EscriptParser::RETURN);
    setState(553);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(552);
      expression(0);
    }
    setState(555);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousReturnStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousReturnStatementContext::UnambiguousReturnStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousReturnStatementContext::RETURN() {
  return getToken(EscriptParser::RETURN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousReturnStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousReturnStatementContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousReturnStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousReturnStatement;
}

void EscriptParser::UnambiguousReturnStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousReturnStatement(this);
}

void EscriptParser::UnambiguousReturnStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousReturnStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousReturnStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousReturnStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousReturnStatementContext* EscriptParser::unambiguousReturnStatement() {
  UnambiguousReturnStatementContext *_localctx = _tracker.createInstance<UnambiguousReturnStatementContext>(_ctx, getState());
  enterRule(_localctx, 56, EscriptParser::RuleUnambiguousReturnStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(557);
    match(EscriptParser::RETURN);
    setState(559);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(558);
      unambiguousExpression();
    }
    setState(561);
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

EscriptParser::VariableDeclarationContext* EscriptParser::ConstStatementContext::variableDeclaration() {
  return getRuleContext<EscriptParser::VariableDeclarationContext>(0);
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


antlrcpp::Any EscriptParser::ConstStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitConstStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ConstStatementContext* EscriptParser::constStatement() {
  ConstStatementContext *_localctx = _tracker.createInstance<ConstStatementContext>(_ctx, getState());
  enterRule(_localctx, 58, EscriptParser::RuleConstStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(563);
    match(EscriptParser::TOK_CONST);
    setState(564);
    variableDeclaration();
    setState(565);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousConstStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousConstStatementContext::UnambiguousConstStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousConstStatementContext::TOK_CONST() {
  return getToken(EscriptParser::TOK_CONST, 0);
}

EscriptParser::UnambiguousVariableDeclarationContext* EscriptParser::UnambiguousConstStatementContext::unambiguousVariableDeclaration() {
  return getRuleContext<EscriptParser::UnambiguousVariableDeclarationContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousConstStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::UnambiguousConstStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousConstStatement;
}

void EscriptParser::UnambiguousConstStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousConstStatement(this);
}

void EscriptParser::UnambiguousConstStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousConstStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousConstStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousConstStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousConstStatementContext* EscriptParser::unambiguousConstStatement() {
  UnambiguousConstStatementContext *_localctx = _tracker.createInstance<UnambiguousConstStatementContext>(_ctx, getState());
  enterRule(_localctx, 60, EscriptParser::RuleUnambiguousConstStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(567);
    match(EscriptParser::TOK_CONST);
    setState(568);
    unambiguousVariableDeclaration();
    setState(569);
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


antlrcpp::Any EscriptParser::VarStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVarStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VarStatementContext* EscriptParser::varStatement() {
  VarStatementContext *_localctx = _tracker.createInstance<VarStatementContext>(_ctx, getState());
  enterRule(_localctx, 62, EscriptParser::RuleVarStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(571);
    match(EscriptParser::VAR);
    setState(572);
    variableDeclarationList();
    setState(573);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousVarStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousVarStatementContext::UnambiguousVarStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousVarStatementContext::VAR() {
  return getToken(EscriptParser::VAR, 0);
}

EscriptParser::UnambiguousVariableDeclarationListContext* EscriptParser::UnambiguousVarStatementContext::unambiguousVariableDeclarationList() {
  return getRuleContext<EscriptParser::UnambiguousVariableDeclarationListContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousVarStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::UnambiguousVarStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousVarStatement;
}

void EscriptParser::UnambiguousVarStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousVarStatement(this);
}

void EscriptParser::UnambiguousVarStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousVarStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousVarStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousVarStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousVarStatementContext* EscriptParser::unambiguousVarStatement() {
  UnambiguousVarStatementContext *_localctx = _tracker.createInstance<UnambiguousVarStatementContext>(_ctx, getState());
  enterRule(_localctx, 64, EscriptParser::RuleUnambiguousVarStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(575);
    match(EscriptParser::VAR);
    setState(576);
    unambiguousVariableDeclarationList();
    setState(577);
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


antlrcpp::Any EscriptParser::DoStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDoStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DoStatementContext* EscriptParser::doStatement() {
  DoStatementContext *_localctx = _tracker.createInstance<DoStatementContext>(_ctx, getState());
  enterRule(_localctx, 66, EscriptParser::RuleDoStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(580);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(579);
      statementLabel();
    }
    setState(582);
    match(EscriptParser::DO);
    setState(583);
    block();
    setState(584);
    match(EscriptParser::DOWHILE);
    setState(585);
    parExpression();
    setState(586);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousDoStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousDoStatementContext::UnambiguousDoStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousDoStatementContext::DO() {
  return getToken(EscriptParser::DO, 0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousDoStatementContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousDoStatementContext::DOWHILE() {
  return getToken(EscriptParser::DOWHILE, 0);
}

EscriptParser::ParenthesizedExpressionContext* EscriptParser::UnambiguousDoStatementContext::parenthesizedExpression() {
  return getRuleContext<EscriptParser::ParenthesizedExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousDoStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::UnambiguousDoStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::UnambiguousDoStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousDoStatement;
}

void EscriptParser::UnambiguousDoStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousDoStatement(this);
}

void EscriptParser::UnambiguousDoStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousDoStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousDoStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousDoStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousDoStatementContext* EscriptParser::unambiguousDoStatement() {
  UnambiguousDoStatementContext *_localctx = _tracker.createInstance<UnambiguousDoStatementContext>(_ctx, getState());
  enterRule(_localctx, 68, EscriptParser::RuleUnambiguousDoStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(589);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(588);
      statementLabel();
    }
    setState(591);
    match(EscriptParser::DO);
    setState(592);
    unambiguousBlock();
    setState(593);
    match(EscriptParser::DOWHILE);
    setState(594);
    parenthesizedExpression();
    setState(595);
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


antlrcpp::Any EscriptParser::WhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::WhileStatementContext* EscriptParser::whileStatement() {
  WhileStatementContext *_localctx = _tracker.createInstance<WhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 70, EscriptParser::RuleWhileStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(598);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(597);
      statementLabel();
    }
    setState(600);
    match(EscriptParser::WHILE);
    setState(601);
    parExpression();
    setState(602);
    block();
    setState(603);
    match(EscriptParser::ENDWHILE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousWhileStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousWhileStatementContext::UnambiguousWhileStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousWhileStatementContext::WHILE() {
  return getToken(EscriptParser::WHILE, 0);
}

EscriptParser::ParenthesizedExpressionContext* EscriptParser::UnambiguousWhileStatementContext::parenthesizedExpression() {
  return getRuleContext<EscriptParser::ParenthesizedExpressionContext>(0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousWhileStatementContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousWhileStatementContext::ENDWHILE() {
  return getToken(EscriptParser::ENDWHILE, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::UnambiguousWhileStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::UnambiguousWhileStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousWhileStatement;
}

void EscriptParser::UnambiguousWhileStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousWhileStatement(this);
}

void EscriptParser::UnambiguousWhileStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousWhileStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousWhileStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousWhileStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousWhileStatementContext* EscriptParser::unambiguousWhileStatement() {
  UnambiguousWhileStatementContext *_localctx = _tracker.createInstance<UnambiguousWhileStatementContext>(_ctx, getState());
  enterRule(_localctx, 72, EscriptParser::RuleUnambiguousWhileStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(606);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(605);
      statementLabel();
    }
    setState(608);
    match(EscriptParser::WHILE);
    setState(609);
    parenthesizedExpression();
    setState(610);
    unambiguousBlock();
    setState(611);
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


antlrcpp::Any EscriptParser::ExitStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExitStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExitStatementContext* EscriptParser::exitStatement() {
  ExitStatementContext *_localctx = _tracker.createInstance<ExitStatementContext>(_ctx, getState());
  enterRule(_localctx, 74, EscriptParser::RuleExitStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(613);
    match(EscriptParser::EXIT);
    setState(614);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DeclareStatementContext ------------------------------------------------------------------

EscriptParser::DeclareStatementContext::DeclareStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::DeclareStatementContext::DECLARE() {
  return getToken(EscriptParser::DECLARE, 0);
}

tree::TerminalNode* EscriptParser::DeclareStatementContext::FUNCTION() {
  return getToken(EscriptParser::FUNCTION, 0);
}

EscriptParser::IdentifierListContext* EscriptParser::DeclareStatementContext::identifierList() {
  return getRuleContext<EscriptParser::IdentifierListContext>(0);
}

tree::TerminalNode* EscriptParser::DeclareStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}


size_t EscriptParser::DeclareStatementContext::getRuleIndex() const {
  return EscriptParser::RuleDeclareStatement;
}

void EscriptParser::DeclareStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDeclareStatement(this);
}

void EscriptParser::DeclareStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDeclareStatement(this);
}


antlrcpp::Any EscriptParser::DeclareStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDeclareStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DeclareStatementContext* EscriptParser::declareStatement() {
  DeclareStatementContext *_localctx = _tracker.createInstance<DeclareStatementContext>(_ctx, getState());
  enterRule(_localctx, 76, EscriptParser::RuleDeclareStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(616);
    match(EscriptParser::DECLARE);
    setState(617);
    match(EscriptParser::FUNCTION);
    setState(618);
    identifierList();
    setState(619);
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


antlrcpp::Any EscriptParser::BreakStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBreakStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BreakStatementContext* EscriptParser::breakStatement() {
  BreakStatementContext *_localctx = _tracker.createInstance<BreakStatementContext>(_ctx, getState());
  enterRule(_localctx, 78, EscriptParser::RuleBreakStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(621);
    match(EscriptParser::BREAK);
    setState(623);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(622);
      match(EscriptParser::IDENTIFIER);
    }
    setState(625);
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


antlrcpp::Any EscriptParser::ContinueStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitContinueStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ContinueStatementContext* EscriptParser::continueStatement() {
  ContinueStatementContext *_localctx = _tracker.createInstance<ContinueStatementContext>(_ctx, getState());
  enterRule(_localctx, 80, EscriptParser::RuleContinueStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(627);
    match(EscriptParser::CONTINUE);
    setState(629);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(628);
      match(EscriptParser::IDENTIFIER);
    }
    setState(631);
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


antlrcpp::Any EscriptParser::ForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForStatementContext* EscriptParser::forStatement() {
  ForStatementContext *_localctx = _tracker.createInstance<ForStatementContext>(_ctx, getState());
  enterRule(_localctx, 82, EscriptParser::RuleForStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(634);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(633);
      statementLabel();
    }
    setState(636);
    match(EscriptParser::FOR);
    setState(637);
    forGroup();
    setState(638);
    match(EscriptParser::ENDFOR);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousForStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousForStatementContext::UnambiguousForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousForStatementContext::FOR() {
  return getToken(EscriptParser::FOR, 0);
}

EscriptParser::UnambiguousForGroupContext* EscriptParser::UnambiguousForStatementContext::unambiguousForGroup() {
  return getRuleContext<EscriptParser::UnambiguousForGroupContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousForStatementContext::ENDFOR() {
  return getToken(EscriptParser::ENDFOR, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::UnambiguousForStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::UnambiguousForStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousForStatement;
}

void EscriptParser::UnambiguousForStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousForStatement(this);
}

void EscriptParser::UnambiguousForStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousForStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousForStatementContext* EscriptParser::unambiguousForStatement() {
  UnambiguousForStatementContext *_localctx = _tracker.createInstance<UnambiguousForStatementContext>(_ctx, getState());
  enterRule(_localctx, 84, EscriptParser::RuleUnambiguousForStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(641);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(640);
      statementLabel();
    }
    setState(643);
    match(EscriptParser::FOR);
    setState(644);
    unambiguousForGroup();
    setState(645);
    match(EscriptParser::ENDFOR);
   
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

EscriptParser::ExpressionContext* EscriptParser::ForeachStatementContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
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


antlrcpp::Any EscriptParser::ForeachStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForeachStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForeachStatementContext* EscriptParser::foreachStatement() {
  ForeachStatementContext *_localctx = _tracker.createInstance<ForeachStatementContext>(_ctx, getState());
  enterRule(_localctx, 86, EscriptParser::RuleForeachStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(648);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(647);
      statementLabel();
    }
    setState(650);
    match(EscriptParser::FOREACH);
    setState(651);
    match(EscriptParser::IDENTIFIER);
    setState(652);
    match(EscriptParser::TOK_IN);
    setState(653);
    expression(0);
    setState(654);
    block();
    setState(655);
    match(EscriptParser::ENDFOREACH);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousForeachStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousForeachStatementContext::UnambiguousForeachStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousForeachStatementContext::FOREACH() {
  return getToken(EscriptParser::FOREACH, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousForeachStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousForeachStatementContext::TOK_IN() {
  return getToken(EscriptParser::TOK_IN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousForeachStatementContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousForeachStatementContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousForeachStatementContext::ENDFOREACH() {
  return getToken(EscriptParser::ENDFOREACH, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::UnambiguousForeachStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::UnambiguousForeachStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousForeachStatement;
}

void EscriptParser::UnambiguousForeachStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousForeachStatement(this);
}

void EscriptParser::UnambiguousForeachStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousForeachStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousForeachStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousForeachStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousForeachStatementContext* EscriptParser::unambiguousForeachStatement() {
  UnambiguousForeachStatementContext *_localctx = _tracker.createInstance<UnambiguousForeachStatementContext>(_ctx, getState());
  enterRule(_localctx, 88, EscriptParser::RuleUnambiguousForeachStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(658);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(657);
      statementLabel();
    }
    setState(660);
    match(EscriptParser::FOREACH);
    setState(661);
    match(EscriptParser::IDENTIFIER);
    setState(662);
    match(EscriptParser::TOK_IN);
    setState(663);
    unambiguousExpression();
    setState(664);
    unambiguousBlock();
    setState(665);
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


antlrcpp::Any EscriptParser::RepeatStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitRepeatStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::RepeatStatementContext* EscriptParser::repeatStatement() {
  RepeatStatementContext *_localctx = _tracker.createInstance<RepeatStatementContext>(_ctx, getState());
  enterRule(_localctx, 90, EscriptParser::RuleRepeatStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(668);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(667);
      statementLabel();
    }
    setState(670);
    match(EscriptParser::REPEAT);
    setState(671);
    block();
    setState(672);
    match(EscriptParser::UNTIL);
    setState(673);
    expression(0);
    setState(674);
    match(EscriptParser::SEMI);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousRepeatStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousRepeatStatementContext::UnambiguousRepeatStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousRepeatStatementContext::REPEAT() {
  return getToken(EscriptParser::REPEAT, 0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousRepeatStatementContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousRepeatStatementContext::UNTIL() {
  return getToken(EscriptParser::UNTIL, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousRepeatStatementContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousRepeatStatementContext::SEMI() {
  return getToken(EscriptParser::SEMI, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::UnambiguousRepeatStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}


size_t EscriptParser::UnambiguousRepeatStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousRepeatStatement;
}

void EscriptParser::UnambiguousRepeatStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousRepeatStatement(this);
}

void EscriptParser::UnambiguousRepeatStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousRepeatStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousRepeatStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousRepeatStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousRepeatStatementContext* EscriptParser::unambiguousRepeatStatement() {
  UnambiguousRepeatStatementContext *_localctx = _tracker.createInstance<UnambiguousRepeatStatementContext>(_ctx, getState());
  enterRule(_localctx, 92, EscriptParser::RuleUnambiguousRepeatStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(677);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(676);
      statementLabel();
    }
    setState(679);
    match(EscriptParser::REPEAT);
    setState(680);
    unambiguousBlock();
    setState(681);
    match(EscriptParser::UNTIL);
    setState(682);
    unambiguousExpression();
    setState(683);
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


antlrcpp::Any EscriptParser::CaseStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCaseStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CaseStatementContext* EscriptParser::caseStatement() {
  CaseStatementContext *_localctx = _tracker.createInstance<CaseStatementContext>(_ctx, getState());
  enterRule(_localctx, 94, EscriptParser::RuleCaseStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(686);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(685);
      statementLabel();
    }
    setState(688);
    match(EscriptParser::CASE);
    setState(689);
    match(EscriptParser::LPAREN);
    setState(690);
    expression(0);
    setState(691);
    match(EscriptParser::RPAREN);
    setState(693); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(692);
      switchBlockStatementGroup();
      setState(695); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == EscriptParser::DEFAULT || ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & ((1ULL << (EscriptParser::DECIMAL_LITERAL - 68))
      | (1ULL << (EscriptParser::HEX_LITERAL - 68))
      | (1ULL << (EscriptParser::OCT_LITERAL - 68))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 68))
      | (1ULL << (EscriptParser::STRING_LITERAL - 68))
      | (1ULL << (EscriptParser::IDENTIFIER - 68)))) != 0));
    setState(697);
    match(EscriptParser::ENDCASE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousCaseStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousCaseStatementContext::UnambiguousCaseStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousCaseStatementContext::CASE() {
  return getToken(EscriptParser::CASE, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousCaseStatementContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousCaseStatementContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousCaseStatementContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousCaseStatementContext::ENDCASE() {
  return getToken(EscriptParser::ENDCASE, 0);
}

EscriptParser::StatementLabelContext* EscriptParser::UnambiguousCaseStatementContext::statementLabel() {
  return getRuleContext<EscriptParser::StatementLabelContext>(0);
}

std::vector<EscriptParser::UnambiguousSwitchBlockStatementGroupContext *> EscriptParser::UnambiguousCaseStatementContext::unambiguousSwitchBlockStatementGroup() {
  return getRuleContexts<EscriptParser::UnambiguousSwitchBlockStatementGroupContext>();
}

EscriptParser::UnambiguousSwitchBlockStatementGroupContext* EscriptParser::UnambiguousCaseStatementContext::unambiguousSwitchBlockStatementGroup(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousSwitchBlockStatementGroupContext>(i);
}


size_t EscriptParser::UnambiguousCaseStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousCaseStatement;
}

void EscriptParser::UnambiguousCaseStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousCaseStatement(this);
}

void EscriptParser::UnambiguousCaseStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousCaseStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousCaseStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousCaseStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousCaseStatementContext* EscriptParser::unambiguousCaseStatement() {
  UnambiguousCaseStatementContext *_localctx = _tracker.createInstance<UnambiguousCaseStatementContext>(_ctx, getState());
  enterRule(_localctx, 96, EscriptParser::RuleUnambiguousCaseStatement);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(700);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::IDENTIFIER) {
      setState(699);
      statementLabel();
    }
    setState(702);
    match(EscriptParser::CASE);
    setState(703);
    match(EscriptParser::LPAREN);
    setState(704);
    unambiguousExpression();
    setState(705);
    match(EscriptParser::RPAREN);
    setState(707); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(706);
      unambiguousSwitchBlockStatementGroup();
      setState(709); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while (_la == EscriptParser::DEFAULT || ((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & ((1ULL << (EscriptParser::DECIMAL_LITERAL - 68))
      | (1ULL << (EscriptParser::HEX_LITERAL - 68))
      | (1ULL << (EscriptParser::OCT_LITERAL - 68))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 68))
      | (1ULL << (EscriptParser::STRING_LITERAL - 68))
      | (1ULL << (EscriptParser::IDENTIFIER - 68)))) != 0));
    setState(711);
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


antlrcpp::Any EscriptParser::EnumStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEnumStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EnumStatementContext* EscriptParser::enumStatement() {
  EnumStatementContext *_localctx = _tracker.createInstance<EnumStatementContext>(_ctx, getState());
  enterRule(_localctx, 98, EscriptParser::RuleEnumStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(713);
    match(EscriptParser::ENUM);
    setState(714);
    match(EscriptParser::IDENTIFIER);
    setState(715);
    enumList();
    setState(716);
    match(EscriptParser::ENDENUM);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousEnumStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousEnumStatementContext::UnambiguousEnumStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousEnumStatementContext::ENUM() {
  return getToken(EscriptParser::ENUM, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousEnumStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::UnambiguousEnumListContext* EscriptParser::UnambiguousEnumStatementContext::unambiguousEnumList() {
  return getRuleContext<EscriptParser::UnambiguousEnumListContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousEnumStatementContext::ENDENUM() {
  return getToken(EscriptParser::ENDENUM, 0);
}


size_t EscriptParser::UnambiguousEnumStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousEnumStatement;
}

void EscriptParser::UnambiguousEnumStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousEnumStatement(this);
}

void EscriptParser::UnambiguousEnumStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousEnumStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousEnumStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousEnumStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousEnumStatementContext* EscriptParser::unambiguousEnumStatement() {
  UnambiguousEnumStatementContext *_localctx = _tracker.createInstance<UnambiguousEnumStatementContext>(_ctx, getState());
  enterRule(_localctx, 100, EscriptParser::RuleUnambiguousEnumStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(718);
    match(EscriptParser::ENUM);
    setState(719);
    match(EscriptParser::IDENTIFIER);
    setState(720);
    unambiguousEnumList();
    setState(721);
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


antlrcpp::Any EscriptParser::BlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBlock(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BlockContext* EscriptParser::block() {
  BlockContext *_localctx = _tracker.createInstance<BlockContext>(_ctx, getState());
  enterRule(_localctx, 102, EscriptParser::RuleBlock);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(726);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(723);
        statement(); 
      }
      setState(728);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 42, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousBlockContext ------------------------------------------------------------------

EscriptParser::UnambiguousBlockContext::UnambiguousBlockContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousStatementContext *> EscriptParser::UnambiguousBlockContext::unambiguousStatement() {
  return getRuleContexts<EscriptParser::UnambiguousStatementContext>();
}

EscriptParser::UnambiguousStatementContext* EscriptParser::UnambiguousBlockContext::unambiguousStatement(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousStatementContext>(i);
}


size_t EscriptParser::UnambiguousBlockContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousBlock;
}

void EscriptParser::UnambiguousBlockContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousBlock(this);
}

void EscriptParser::UnambiguousBlockContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousBlock(this);
}


antlrcpp::Any EscriptParser::UnambiguousBlockContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousBlock(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::unambiguousBlock() {
  UnambiguousBlockContext *_localctx = _tracker.createInstance<UnambiguousBlockContext>(_ctx, getState());
  enterRule(_localctx, 104, EscriptParser::RuleUnambiguousBlock);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(732);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(729);
        unambiguousStatement(); 
      }
      setState(734);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 43, _ctx);
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


antlrcpp::Any EscriptParser::VariableDeclarationInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VariableDeclarationInitializerContext* EscriptParser::variableDeclarationInitializer() {
  VariableDeclarationInitializerContext *_localctx = _tracker.createInstance<VariableDeclarationInitializerContext>(_ctx, getState());
  enterRule(_localctx, 106, EscriptParser::RuleVariableDeclarationInitializer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(738);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::ASSIGN: {
        enterOuterAlt(_localctx, 1);
        setState(735);
        match(EscriptParser::ASSIGN);
        setState(736);
        expression(0);
        break;
      }

      case EscriptParser::ARRAY: {
        enterOuterAlt(_localctx, 2);
        setState(737);
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

//----------------- UnambiguousVariableDeclarationInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousVariableDeclarationInitializerContext::UnambiguousVariableDeclarationInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousVariableDeclarationInitializerContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousVariableDeclarationInitializerContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousVariableDeclarationInitializerContext::EQUAL_DEPRECATED() {
  return getToken(EscriptParser::EQUAL_DEPRECATED, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousVariableDeclarationInitializerContext::ARRAY() {
  return getToken(EscriptParser::ARRAY, 0);
}


size_t EscriptParser::UnambiguousVariableDeclarationInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousVariableDeclarationInitializer;
}

void EscriptParser::UnambiguousVariableDeclarationInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousVariableDeclarationInitializer(this);
}

void EscriptParser::UnambiguousVariableDeclarationInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousVariableDeclarationInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousVariableDeclarationInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousVariableDeclarationInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousVariableDeclarationInitializerContext* EscriptParser::unambiguousVariableDeclarationInitializer() {
  UnambiguousVariableDeclarationInitializerContext *_localctx = _tracker.createInstance<UnambiguousVariableDeclarationInitializerContext>(_ctx, getState());
  enterRule(_localctx, 108, EscriptParser::RuleUnambiguousVariableDeclarationInitializer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(747);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::ASSIGN: {
        enterOuterAlt(_localctx, 1);
        setState(740);
        match(EscriptParser::ASSIGN);
        setState(741);
        unambiguousExpression();
        break;
      }

      case EscriptParser::EQUAL_DEPRECATED: {
        enterOuterAlt(_localctx, 2);
        setState(742);
        match(EscriptParser::EQUAL_DEPRECATED);
        setState(743);
        unambiguousExpression();
        notifyErrorListeners("Unexpected token: '='. Did you mean := for assign?");
        break;
      }

      case EscriptParser::ARRAY: {
        enterOuterAlt(_localctx, 3);
        setState(746);
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


antlrcpp::Any EscriptParser::EnumListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEnumList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EnumListContext* EscriptParser::enumList() {
  EnumListContext *_localctx = _tracker.createInstance<EnumListContext>(_ctx, getState());
  enterRule(_localctx, 110, EscriptParser::RuleEnumList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(749);
    enumListEntry();
    setState(754);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(750);
        match(EscriptParser::COMMA);
        setState(751);
        enumListEntry(); 
      }
      setState(756);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 46, _ctx);
    }
    setState(758);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::COMMA) {
      setState(757);
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

//----------------- UnambiguousEnumListContext ------------------------------------------------------------------

EscriptParser::UnambiguousEnumListContext::UnambiguousEnumListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousEnumListEntryContext *> EscriptParser::UnambiguousEnumListContext::unambiguousEnumListEntry() {
  return getRuleContexts<EscriptParser::UnambiguousEnumListEntryContext>();
}

EscriptParser::UnambiguousEnumListEntryContext* EscriptParser::UnambiguousEnumListContext::unambiguousEnumListEntry(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousEnumListEntryContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousEnumListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousEnumListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousEnumListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousEnumList;
}

void EscriptParser::UnambiguousEnumListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousEnumList(this);
}

void EscriptParser::UnambiguousEnumListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousEnumList(this);
}


antlrcpp::Any EscriptParser::UnambiguousEnumListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousEnumList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousEnumListContext* EscriptParser::unambiguousEnumList() {
  UnambiguousEnumListContext *_localctx = _tracker.createInstance<UnambiguousEnumListContext>(_ctx, getState());
  enterRule(_localctx, 112, EscriptParser::RuleUnambiguousEnumList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(760);
    unambiguousEnumListEntry();
    setState(765);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(761);
        match(EscriptParser::COMMA);
        setState(762);
        unambiguousEnumListEntry(); 
      }
      setState(767);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 48, _ctx);
    }
    setState(769);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::COMMA) {
      setState(768);
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


antlrcpp::Any EscriptParser::EnumListEntryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEnumListEntry(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EnumListEntryContext* EscriptParser::enumListEntry() {
  EnumListEntryContext *_localctx = _tracker.createInstance<EnumListEntryContext>(_ctx, getState());
  enterRule(_localctx, 114, EscriptParser::RuleEnumListEntry);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(771);
    match(EscriptParser::IDENTIFIER);
    setState(774);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(772);
      match(EscriptParser::ASSIGN);
      setState(773);
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

//----------------- UnambiguousEnumListEntryContext ------------------------------------------------------------------

EscriptParser::UnambiguousEnumListEntryContext::UnambiguousEnumListEntryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousEnumListEntryContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousEnumListEntryContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousEnumListEntryContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousEnumListEntryContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousEnumListEntry;
}

void EscriptParser::UnambiguousEnumListEntryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousEnumListEntry(this);
}

void EscriptParser::UnambiguousEnumListEntryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousEnumListEntry(this);
}


antlrcpp::Any EscriptParser::UnambiguousEnumListEntryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousEnumListEntry(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousEnumListEntryContext* EscriptParser::unambiguousEnumListEntry() {
  UnambiguousEnumListEntryContext *_localctx = _tracker.createInstance<UnambiguousEnumListEntryContext>(_ctx, getState());
  enterRule(_localctx, 116, EscriptParser::RuleUnambiguousEnumListEntry);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(776);
    match(EscriptParser::IDENTIFIER);
    setState(779);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(777);
      match(EscriptParser::ASSIGN);
      setState(778);
      unambiguousExpression();
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


antlrcpp::Any EscriptParser::SwitchBlockStatementGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitSwitchBlockStatementGroup(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::SwitchBlockStatementGroupContext* EscriptParser::switchBlockStatementGroup() {
  SwitchBlockStatementGroupContext *_localctx = _tracker.createInstance<SwitchBlockStatementGroupContext>(_ctx, getState());
  enterRule(_localctx, 118, EscriptParser::RuleSwitchBlockStatementGroup);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(782); 
    _errHandler->sync(this);
    alt = 1;
    do {
      switch (alt) {
        case 1: {
              setState(781);
              switchLabel();
              break;
            }

      default:
        throw NoViableAltException(this);
      }
      setState(784); 
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 52, _ctx);
    } while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER);
    setState(786);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousSwitchBlockStatementGroupContext ------------------------------------------------------------------

EscriptParser::UnambiguousSwitchBlockStatementGroupContext::UnambiguousSwitchBlockStatementGroupContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::SwitchLabelContext *> EscriptParser::UnambiguousSwitchBlockStatementGroupContext::switchLabel() {
  return getRuleContexts<EscriptParser::SwitchLabelContext>();
}

EscriptParser::SwitchLabelContext* EscriptParser::UnambiguousSwitchBlockStatementGroupContext::switchLabel(size_t i) {
  return getRuleContext<EscriptParser::SwitchLabelContext>(i);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousSwitchBlockStatementGroupContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}


size_t EscriptParser::UnambiguousSwitchBlockStatementGroupContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousSwitchBlockStatementGroup;
}

void EscriptParser::UnambiguousSwitchBlockStatementGroupContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousSwitchBlockStatementGroup(this);
}

void EscriptParser::UnambiguousSwitchBlockStatementGroupContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousSwitchBlockStatementGroup(this);
}


antlrcpp::Any EscriptParser::UnambiguousSwitchBlockStatementGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousSwitchBlockStatementGroup(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousSwitchBlockStatementGroupContext* EscriptParser::unambiguousSwitchBlockStatementGroup() {
  UnambiguousSwitchBlockStatementGroupContext *_localctx = _tracker.createInstance<UnambiguousSwitchBlockStatementGroupContext>(_ctx, getState());
  enterRule(_localctx, 120, EscriptParser::RuleUnambiguousSwitchBlockStatementGroup);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(788);
    switchLabel();
    setState(792);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(789);
        switchLabel(); 
      }
      setState(794);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 53, _ctx);
    }
    setState(795);
    unambiguousBlock();
   
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


antlrcpp::Any EscriptParser::SwitchLabelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitSwitchLabel(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::SwitchLabelContext* EscriptParser::switchLabel() {
  SwitchLabelContext *_localctx = _tracker.createInstance<SwitchLabelContext>(_ctx, getState());
  enterRule(_localctx, 122, EscriptParser::RuleSwitchLabel);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(805);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(800);
        _errHandler->sync(this);
        switch (_input->LA(1)) {
          case EscriptParser::DECIMAL_LITERAL:
          case EscriptParser::HEX_LITERAL:
          case EscriptParser::OCT_LITERAL:
          case EscriptParser::BINARY_LITERAL: {
            setState(797);
            integerLiteral();
            break;
          }

          case EscriptParser::IDENTIFIER: {
            setState(798);
            match(EscriptParser::IDENTIFIER);
            break;
          }

          case EscriptParser::STRING_LITERAL: {
            setState(799);
            match(EscriptParser::STRING_LITERAL);
            break;
          }

        default:
          throw NoViableAltException(this);
        }
        setState(802);
        match(EscriptParser::COLON);
        break;
      }

      case EscriptParser::DEFAULT: {
        enterOuterAlt(_localctx, 2);
        setState(803);
        match(EscriptParser::DEFAULT);
        setState(804);
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


antlrcpp::Any EscriptParser::ForGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitForGroup(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ForGroupContext* EscriptParser::forGroup() {
  ForGroupContext *_localctx = _tracker.createInstance<ForGroupContext>(_ctx, getState());
  enterRule(_localctx, 124, EscriptParser::RuleForGroup);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(809);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::LPAREN: {
        enterOuterAlt(_localctx, 1);
        setState(807);
        cstyleForStatement();
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(808);
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

//----------------- UnambiguousForGroupContext ------------------------------------------------------------------

EscriptParser::UnambiguousForGroupContext::UnambiguousForGroupContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::UnambiguousCstyleForStatementContext* EscriptParser::UnambiguousForGroupContext::unambiguousCstyleForStatement() {
  return getRuleContext<EscriptParser::UnambiguousCstyleForStatementContext>(0);
}

EscriptParser::UnambiguousBasicForStatementContext* EscriptParser::UnambiguousForGroupContext::unambiguousBasicForStatement() {
  return getRuleContext<EscriptParser::UnambiguousBasicForStatementContext>(0);
}


size_t EscriptParser::UnambiguousForGroupContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousForGroup;
}

void EscriptParser::UnambiguousForGroupContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousForGroup(this);
}

void EscriptParser::UnambiguousForGroupContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousForGroup(this);
}


antlrcpp::Any EscriptParser::UnambiguousForGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousForGroup(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousForGroupContext* EscriptParser::unambiguousForGroup() {
  UnambiguousForGroupContext *_localctx = _tracker.createInstance<UnambiguousForGroupContext>(_ctx, getState());
  enterRule(_localctx, 126, EscriptParser::RuleUnambiguousForGroup);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(813);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::LPAREN: {
        enterOuterAlt(_localctx, 1);
        setState(811);
        unambiguousCstyleForStatement();
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(812);
        unambiguousBasicForStatement();
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


antlrcpp::Any EscriptParser::BasicForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBasicForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BasicForStatementContext* EscriptParser::basicForStatement() {
  BasicForStatementContext *_localctx = _tracker.createInstance<BasicForStatementContext>(_ctx, getState());
  enterRule(_localctx, 128, EscriptParser::RuleBasicForStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(815);
    match(EscriptParser::IDENTIFIER);
    setState(816);
    match(EscriptParser::ASSIGN);
    setState(817);
    expression(0);
    setState(818);
    match(EscriptParser::TO);
    setState(819);
    expression(0);
    setState(820);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousBasicForStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousBasicForStatementContext::UnambiguousBasicForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousBasicForStatementContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousBasicForStatementContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

std::vector<EscriptParser::UnambiguousExpressionContext *> EscriptParser::UnambiguousBasicForStatementContext::unambiguousExpression() {
  return getRuleContexts<EscriptParser::UnambiguousExpressionContext>();
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousBasicForStatementContext::unambiguousExpression(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(i);
}

tree::TerminalNode* EscriptParser::UnambiguousBasicForStatementContext::TO() {
  return getToken(EscriptParser::TO, 0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousBasicForStatementContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}


size_t EscriptParser::UnambiguousBasicForStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousBasicForStatement;
}

void EscriptParser::UnambiguousBasicForStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousBasicForStatement(this);
}

void EscriptParser::UnambiguousBasicForStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousBasicForStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousBasicForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousBasicForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousBasicForStatementContext* EscriptParser::unambiguousBasicForStatement() {
  UnambiguousBasicForStatementContext *_localctx = _tracker.createInstance<UnambiguousBasicForStatementContext>(_ctx, getState());
  enterRule(_localctx, 130, EscriptParser::RuleUnambiguousBasicForStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(822);
    match(EscriptParser::IDENTIFIER);
    setState(823);
    match(EscriptParser::ASSIGN);
    setState(824);
    unambiguousExpression();
    setState(825);
    match(EscriptParser::TO);
    setState(826);
    unambiguousExpression();
    setState(827);
    unambiguousBlock();
   
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


antlrcpp::Any EscriptParser::CstyleForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCstyleForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CstyleForStatementContext* EscriptParser::cstyleForStatement() {
  CstyleForStatementContext *_localctx = _tracker.createInstance<CstyleForStatementContext>(_ctx, getState());
  enterRule(_localctx, 132, EscriptParser::RuleCstyleForStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(829);
    match(EscriptParser::LPAREN);
    setState(830);
    expression(0);
    setState(831);
    match(EscriptParser::SEMI);
    setState(832);
    expression(0);
    setState(833);
    match(EscriptParser::SEMI);
    setState(834);
    expression(0);
    setState(835);
    match(EscriptParser::RPAREN);
    setState(836);
    block();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousCstyleForStatementContext ------------------------------------------------------------------

EscriptParser::UnambiguousCstyleForStatementContext::UnambiguousCstyleForStatementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousCstyleForStatementContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

std::vector<EscriptParser::UnambiguousExpressionContext *> EscriptParser::UnambiguousCstyleForStatementContext::unambiguousExpression() {
  return getRuleContexts<EscriptParser::UnambiguousExpressionContext>();
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousCstyleForStatementContext::unambiguousExpression(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousCstyleForStatementContext::SEMI() {
  return getTokens(EscriptParser::SEMI);
}

tree::TerminalNode* EscriptParser::UnambiguousCstyleForStatementContext::SEMI(size_t i) {
  return getToken(EscriptParser::SEMI, i);
}

tree::TerminalNode* EscriptParser::UnambiguousCstyleForStatementContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::UnambiguousBlockContext* EscriptParser::UnambiguousCstyleForStatementContext::unambiguousBlock() {
  return getRuleContext<EscriptParser::UnambiguousBlockContext>(0);
}


size_t EscriptParser::UnambiguousCstyleForStatementContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousCstyleForStatement;
}

void EscriptParser::UnambiguousCstyleForStatementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousCstyleForStatement(this);
}

void EscriptParser::UnambiguousCstyleForStatementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousCstyleForStatement(this);
}


antlrcpp::Any EscriptParser::UnambiguousCstyleForStatementContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousCstyleForStatement(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousCstyleForStatementContext* EscriptParser::unambiguousCstyleForStatement() {
  UnambiguousCstyleForStatementContext *_localctx = _tracker.createInstance<UnambiguousCstyleForStatementContext>(_ctx, getState());
  enterRule(_localctx, 134, EscriptParser::RuleUnambiguousCstyleForStatement);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(838);
    match(EscriptParser::LPAREN);
    setState(839);
    unambiguousExpression();
    setState(840);
    match(EscriptParser::SEMI);
    setState(841);
    unambiguousExpression();
    setState(842);
    match(EscriptParser::SEMI);
    setState(843);
    unambiguousExpression();
    setState(844);
    match(EscriptParser::RPAREN);
    setState(845);
    unambiguousBlock();
   
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


antlrcpp::Any EscriptParser::IdentifierListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIdentifierList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IdentifierListContext* EscriptParser::identifierList() {
  IdentifierListContext *_localctx = _tracker.createInstance<IdentifierListContext>(_ctx, getState());
  enterRule(_localctx, 136, EscriptParser::RuleIdentifierList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(847);
    match(EscriptParser::IDENTIFIER);
    setState(850);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::COMMA) {
      setState(848);
      match(EscriptParser::COMMA);
      setState(849);
      identifierList();
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


antlrcpp::Any EscriptParser::VariableDeclarationListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVariableDeclarationList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VariableDeclarationListContext* EscriptParser::variableDeclarationList() {
  VariableDeclarationListContext *_localctx = _tracker.createInstance<VariableDeclarationListContext>(_ctx, getState());
  enterRule(_localctx, 138, EscriptParser::RuleVariableDeclarationList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(852);
    variableDeclaration();
    setState(857);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(853);
      match(EscriptParser::COMMA);
      setState(854);
      variableDeclaration();
      setState(859);
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

//----------------- UnambiguousVariableDeclarationListContext ------------------------------------------------------------------

EscriptParser::UnambiguousVariableDeclarationListContext::UnambiguousVariableDeclarationListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousVariableDeclarationContext *> EscriptParser::UnambiguousVariableDeclarationListContext::unambiguousVariableDeclaration() {
  return getRuleContexts<EscriptParser::UnambiguousVariableDeclarationContext>();
}

EscriptParser::UnambiguousVariableDeclarationContext* EscriptParser::UnambiguousVariableDeclarationListContext::unambiguousVariableDeclaration(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousVariableDeclarationContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousVariableDeclarationListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousVariableDeclarationListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousVariableDeclarationListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousVariableDeclarationList;
}

void EscriptParser::UnambiguousVariableDeclarationListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousVariableDeclarationList(this);
}

void EscriptParser::UnambiguousVariableDeclarationListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousVariableDeclarationList(this);
}


antlrcpp::Any EscriptParser::UnambiguousVariableDeclarationListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousVariableDeclarationList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousVariableDeclarationListContext* EscriptParser::unambiguousVariableDeclarationList() {
  UnambiguousVariableDeclarationListContext *_localctx = _tracker.createInstance<UnambiguousVariableDeclarationListContext>(_ctx, getState());
  enterRule(_localctx, 140, EscriptParser::RuleUnambiguousVariableDeclarationList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(860);
    unambiguousVariableDeclaration();
    setState(865);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(861);
      match(EscriptParser::COMMA);
      setState(862);
      unambiguousVariableDeclaration();
      setState(867);
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


antlrcpp::Any EscriptParser::VariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::VariableDeclarationContext* EscriptParser::variableDeclaration() {
  VariableDeclarationContext *_localctx = _tracker.createInstance<VariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 142, EscriptParser::RuleVariableDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(868);
    match(EscriptParser::IDENTIFIER);
    setState(870);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ARRAY

    || _la == EscriptParser::ASSIGN) {
      setState(869);
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

//----------------- UnambiguousVariableDeclarationContext ------------------------------------------------------------------

EscriptParser::UnambiguousVariableDeclarationContext::UnambiguousVariableDeclarationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousVariableDeclarationContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::UnambiguousVariableDeclarationInitializerContext* EscriptParser::UnambiguousVariableDeclarationContext::unambiguousVariableDeclarationInitializer() {
  return getRuleContext<EscriptParser::UnambiguousVariableDeclarationInitializerContext>(0);
}


size_t EscriptParser::UnambiguousVariableDeclarationContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousVariableDeclaration;
}

void EscriptParser::UnambiguousVariableDeclarationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousVariableDeclaration(this);
}

void EscriptParser::UnambiguousVariableDeclarationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousVariableDeclaration(this);
}


antlrcpp::Any EscriptParser::UnambiguousVariableDeclarationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousVariableDeclaration(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousVariableDeclarationContext* EscriptParser::unambiguousVariableDeclaration() {
  UnambiguousVariableDeclarationContext *_localctx = _tracker.createInstance<UnambiguousVariableDeclarationContext>(_ctx, getState());
  enterRule(_localctx, 144, EscriptParser::RuleUnambiguousVariableDeclaration);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(872);
    match(EscriptParser::IDENTIFIER);
    setState(874);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (((((_la - 65) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 65)) & ((1ULL << (EscriptParser::ARRAY - 65))
      | (1ULL << (EscriptParser::EQUAL_DEPRECATED - 65))
      | (1ULL << (EscriptParser::ASSIGN - 65)))) != 0)) {
      setState(873);
      unambiguousVariableDeclarationInitializer();
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


antlrcpp::Any EscriptParser::ProgramParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramParametersContext* EscriptParser::programParameters() {
  ProgramParametersContext *_localctx = _tracker.createInstance<ProgramParametersContext>(_ctx, getState());
  enterRule(_localctx, 146, EscriptParser::RuleProgramParameters);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(876);
    match(EscriptParser::LPAREN);
    setState(878);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::UNUSED || _la == EscriptParser::IDENTIFIER) {
      setState(877);
      programParameterList();
    }
    setState(880);
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


antlrcpp::Any EscriptParser::ProgramParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramParameterListContext* EscriptParser::programParameterList() {
  ProgramParameterListContext *_localctx = _tracker.createInstance<ProgramParameterListContext>(_ctx, getState());
  enterRule(_localctx, 148, EscriptParser::RuleProgramParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(882);
    programParameter();
    setState(889);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::UNUSED || _la == EscriptParser::COMMA

    || _la == EscriptParser::IDENTIFIER) {
      setState(884);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::COMMA) {
        setState(883);
        match(EscriptParser::COMMA);
      }
      setState(886);
      programParameter();
      setState(891);
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


antlrcpp::Any EscriptParser::ProgramParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitProgramParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ProgramParameterContext* EscriptParser::programParameter() {
  ProgramParameterContext *_localctx = _tracker.createInstance<ProgramParameterContext>(_ctx, getState());
  enterRule(_localctx, 150, EscriptParser::RuleProgramParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(899);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::UNUSED: {
        enterOuterAlt(_localctx, 1);
        setState(892);
        match(EscriptParser::UNUSED);
        setState(893);
        match(EscriptParser::IDENTIFIER);
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(894);
        match(EscriptParser::IDENTIFIER);
        setState(897);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(895);
          match(EscriptParser::ASSIGN);
          setState(896);
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

//----------------- UnambiguousProgramParametersContext ------------------------------------------------------------------

EscriptParser::UnambiguousProgramParametersContext::UnambiguousProgramParametersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousProgramParametersContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousProgramParametersContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::UnambiguousProgramParameterListContext* EscriptParser::UnambiguousProgramParametersContext::unambiguousProgramParameterList() {
  return getRuleContext<EscriptParser::UnambiguousProgramParameterListContext>(0);
}


size_t EscriptParser::UnambiguousProgramParametersContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousProgramParameters;
}

void EscriptParser::UnambiguousProgramParametersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousProgramParameters(this);
}

void EscriptParser::UnambiguousProgramParametersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousProgramParameters(this);
}


antlrcpp::Any EscriptParser::UnambiguousProgramParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousProgramParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousProgramParametersContext* EscriptParser::unambiguousProgramParameters() {
  UnambiguousProgramParametersContext *_localctx = _tracker.createInstance<UnambiguousProgramParametersContext>(_ctx, getState());
  enterRule(_localctx, 152, EscriptParser::RuleUnambiguousProgramParameters);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(901);
    match(EscriptParser::LPAREN);
    setState(903);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::UNUSED || _la == EscriptParser::IDENTIFIER) {
      setState(902);
      unambiguousProgramParameterList();
    }
    setState(905);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousProgramParameterListContext ------------------------------------------------------------------

EscriptParser::UnambiguousProgramParameterListContext::UnambiguousProgramParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousProgramParameterContext *> EscriptParser::UnambiguousProgramParameterListContext::unambiguousProgramParameter() {
  return getRuleContexts<EscriptParser::UnambiguousProgramParameterContext>();
}

EscriptParser::UnambiguousProgramParameterContext* EscriptParser::UnambiguousProgramParameterListContext::unambiguousProgramParameter(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousProgramParameterContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousProgramParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousProgramParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousProgramParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousProgramParameterList;
}

void EscriptParser::UnambiguousProgramParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousProgramParameterList(this);
}

void EscriptParser::UnambiguousProgramParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousProgramParameterList(this);
}


antlrcpp::Any EscriptParser::UnambiguousProgramParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousProgramParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousProgramParameterListContext* EscriptParser::unambiguousProgramParameterList() {
  UnambiguousProgramParameterListContext *_localctx = _tracker.createInstance<UnambiguousProgramParameterListContext>(_ctx, getState());
  enterRule(_localctx, 154, EscriptParser::RuleUnambiguousProgramParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(907);
    unambiguousProgramParameter();
    setState(914);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::UNUSED || _la == EscriptParser::COMMA

    || _la == EscriptParser::IDENTIFIER) {
      setState(909);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::COMMA) {
        setState(908);
        match(EscriptParser::COMMA);
      }
      setState(911);
      unambiguousProgramParameter();
      setState(916);
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

//----------------- UnambiguousProgramParameterContext ------------------------------------------------------------------

EscriptParser::UnambiguousProgramParameterContext::UnambiguousProgramParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousProgramParameterContext::UNUSED() {
  return getToken(EscriptParser::UNUSED, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousProgramParameterContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousProgramParameterContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousProgramParameterContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousProgramParameterContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousProgramParameter;
}

void EscriptParser::UnambiguousProgramParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousProgramParameter(this);
}

void EscriptParser::UnambiguousProgramParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousProgramParameter(this);
}


antlrcpp::Any EscriptParser::UnambiguousProgramParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousProgramParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousProgramParameterContext* EscriptParser::unambiguousProgramParameter() {
  UnambiguousProgramParameterContext *_localctx = _tracker.createInstance<UnambiguousProgramParameterContext>(_ctx, getState());
  enterRule(_localctx, 156, EscriptParser::RuleUnambiguousProgramParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(924);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::UNUSED: {
        enterOuterAlt(_localctx, 1);
        setState(917);
        match(EscriptParser::UNUSED);
        setState(918);
        match(EscriptParser::IDENTIFIER);
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 2);
        setState(919);
        match(EscriptParser::IDENTIFIER);
        setState(922);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(920);
          match(EscriptParser::ASSIGN);
          setState(921);
          unambiguousExpression();
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


antlrcpp::Any EscriptParser::FunctionParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionParametersContext* EscriptParser::functionParameters() {
  FunctionParametersContext *_localctx = _tracker.createInstance<FunctionParametersContext>(_ctx, getState());
  enterRule(_localctx, 158, EscriptParser::RuleFunctionParameters);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(926);
    match(EscriptParser::LPAREN);
    setState(928);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::BYREF

    || _la == EscriptParser::UNUSED || _la == EscriptParser::IDENTIFIER) {
      setState(927);
      functionParameterList();
    }
    setState(930);
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


antlrcpp::Any EscriptParser::FunctionParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionParameterListContext* EscriptParser::functionParameterList() {
  FunctionParameterListContext *_localctx = _tracker.createInstance<FunctionParameterListContext>(_ctx, getState());
  enterRule(_localctx, 160, EscriptParser::RuleFunctionParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(932);
    functionParameter();
    setState(937);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(933);
      match(EscriptParser::COMMA);
      setState(934);
      functionParameter();
      setState(939);
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


antlrcpp::Any EscriptParser::FunctionParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionParameterContext* EscriptParser::functionParameter() {
  FunctionParameterContext *_localctx = _tracker.createInstance<FunctionParameterContext>(_ctx, getState());
  enterRule(_localctx, 162, EscriptParser::RuleFunctionParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(941);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::BYREF) {
      setState(940);
      match(EscriptParser::BYREF);
    }
    setState(944);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::UNUSED) {
      setState(943);
      match(EscriptParser::UNUSED);
    }
    setState(946);
    match(EscriptParser::IDENTIFIER);
    setState(949);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(947);
      match(EscriptParser::ASSIGN);
      setState(948);
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

//----------------- UnambiguousFunctionParametersContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionParametersContext::UnambiguousFunctionParametersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParametersContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParametersContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::UnambiguousFunctionParameterListContext* EscriptParser::UnambiguousFunctionParametersContext::unambiguousFunctionParameterList() {
  return getRuleContext<EscriptParser::UnambiguousFunctionParameterListContext>(0);
}


size_t EscriptParser::UnambiguousFunctionParametersContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionParameters;
}

void EscriptParser::UnambiguousFunctionParametersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionParameters(this);
}

void EscriptParser::UnambiguousFunctionParametersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionParameters(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionParametersContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionParameters(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionParametersContext* EscriptParser::unambiguousFunctionParameters() {
  UnambiguousFunctionParametersContext *_localctx = _tracker.createInstance<UnambiguousFunctionParametersContext>(_ctx, getState());
  enterRule(_localctx, 164, EscriptParser::RuleUnambiguousFunctionParameters);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(951);
    match(EscriptParser::LPAREN);
    setState(953);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::BYREF

    || _la == EscriptParser::UNUSED || _la == EscriptParser::IDENTIFIER) {
      setState(952);
      unambiguousFunctionParameterList();
    }
    setState(955);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousFunctionParameterListContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionParameterListContext::UnambiguousFunctionParameterListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousFunctionParameterContext *> EscriptParser::UnambiguousFunctionParameterListContext::unambiguousFunctionParameter() {
  return getRuleContexts<EscriptParser::UnambiguousFunctionParameterContext>();
}

EscriptParser::UnambiguousFunctionParameterContext* EscriptParser::UnambiguousFunctionParameterListContext::unambiguousFunctionParameter(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousFunctionParameterContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousFunctionParameterListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParameterListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousFunctionParameterListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionParameterList;
}

void EscriptParser::UnambiguousFunctionParameterListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionParameterList(this);
}

void EscriptParser::UnambiguousFunctionParameterListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionParameterList(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionParameterListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionParameterList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionParameterListContext* EscriptParser::unambiguousFunctionParameterList() {
  UnambiguousFunctionParameterListContext *_localctx = _tracker.createInstance<UnambiguousFunctionParameterListContext>(_ctx, getState());
  enterRule(_localctx, 166, EscriptParser::RuleUnambiguousFunctionParameterList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(957);
    unambiguousFunctionParameter();
    setState(962);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(958);
      match(EscriptParser::COMMA);
      setState(959);
      unambiguousFunctionParameter();
      setState(964);
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

//----------------- UnambiguousFunctionParameterContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionParameterContext::UnambiguousFunctionParameterContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParameterContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParameterContext::BYREF() {
  return getToken(EscriptParser::BYREF, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParameterContext::UNUSED() {
  return getToken(EscriptParser::UNUSED, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionParameterContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousFunctionParameterContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousFunctionParameterContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionParameter;
}

void EscriptParser::UnambiguousFunctionParameterContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionParameter(this);
}

void EscriptParser::UnambiguousFunctionParameterContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionParameter(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionParameterContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionParameter(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionParameterContext* EscriptParser::unambiguousFunctionParameter() {
  UnambiguousFunctionParameterContext *_localctx = _tracker.createInstance<UnambiguousFunctionParameterContext>(_ctx, getState());
  enterRule(_localctx, 168, EscriptParser::RuleUnambiguousFunctionParameter);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(966);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::BYREF) {
      setState(965);
      match(EscriptParser::BYREF);
    }
    setState(969);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::UNUSED) {
      setState(968);
      match(EscriptParser::UNUSED);
    }
    setState(971);
    match(EscriptParser::IDENTIFIER);
    setState(974);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ASSIGN) {
      setState(972);
      match(EscriptParser::ASSIGN);
      setState(973);
      unambiguousExpression();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ScopedMethodCallContext ------------------------------------------------------------------

EscriptParser::ScopedMethodCallContext::ScopedMethodCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ScopedMethodCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ScopedMethodCallContext::COLONCOLON() {
  return getToken(EscriptParser::COLONCOLON, 0);
}

EscriptParser::MethodCallContext* EscriptParser::ScopedMethodCallContext::methodCall() {
  return getRuleContext<EscriptParser::MethodCallContext>(0);
}


size_t EscriptParser::ScopedMethodCallContext::getRuleIndex() const {
  return EscriptParser::RuleScopedMethodCall;
}

void EscriptParser::ScopedMethodCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterScopedMethodCall(this);
}

void EscriptParser::ScopedMethodCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitScopedMethodCall(this);
}


antlrcpp::Any EscriptParser::ScopedMethodCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitScopedMethodCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ScopedMethodCallContext* EscriptParser::scopedMethodCall() {
  ScopedMethodCallContext *_localctx = _tracker.createInstance<ScopedMethodCallContext>(_ctx, getState());
  enterRule(_localctx, 170, EscriptParser::RuleScopedMethodCall);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(976);
    match(EscriptParser::IDENTIFIER);
    setState(977);
    match(EscriptParser::COLONCOLON);
    setState(978);
    methodCall();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousExpressionContext ------------------------------------------------------------------

EscriptParser::UnambiguousExpressionContext::UnambiguousExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::MembershipContext *> EscriptParser::UnambiguousExpressionContext::membership() {
  return getRuleContexts<EscriptParser::MembershipContext>();
}

EscriptParser::MembershipContext* EscriptParser::UnambiguousExpressionContext::membership(size_t i) {
  return getRuleContext<EscriptParser::MembershipContext>(i);
}

std::vector<EscriptParser::AssignmentOperatorContext *> EscriptParser::UnambiguousExpressionContext::assignmentOperator() {
  return getRuleContexts<EscriptParser::AssignmentOperatorContext>();
}

EscriptParser::AssignmentOperatorContext* EscriptParser::UnambiguousExpressionContext::assignmentOperator(size_t i) {
  return getRuleContext<EscriptParser::AssignmentOperatorContext>(i);
}


size_t EscriptParser::UnambiguousExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousExpression;
}

void EscriptParser::UnambiguousExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousExpression(this);
}

void EscriptParser::UnambiguousExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousExpression(this);
}


antlrcpp::Any EscriptParser::UnambiguousExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::unambiguousExpression() {
  UnambiguousExpressionContext *_localctx = _tracker.createInstance<UnambiguousExpressionContext>(_ctx, getState());
  enterRule(_localctx, 172, EscriptParser::RuleUnambiguousExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(980);
    membership();
    setState(986);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 90) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 90)) & ((1ULL << (EscriptParser::ADD_ASSIGN - 90))
      | (1ULL << (EscriptParser::SUB_ASSIGN - 90))
      | (1ULL << (EscriptParser::MUL_ASSIGN - 90))
      | (1ULL << (EscriptParser::DIV_ASSIGN - 90))
      | (1ULL << (EscriptParser::MOD_ASSIGN - 90))
      | (1ULL << (EscriptParser::ASSIGN - 90)))) != 0)) {
      setState(981);
      assignmentOperator();
      setState(982);
      membership();
      setState(988);
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

//----------------- AssignmentOperatorContext ------------------------------------------------------------------

EscriptParser::AssignmentOperatorContext::AssignmentOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::AssignmentOperatorContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::AssignmentOperatorContext::ADD_ASSIGN() {
  return getToken(EscriptParser::ADD_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::AssignmentOperatorContext::SUB_ASSIGN() {
  return getToken(EscriptParser::SUB_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::AssignmentOperatorContext::MUL_ASSIGN() {
  return getToken(EscriptParser::MUL_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::AssignmentOperatorContext::DIV_ASSIGN() {
  return getToken(EscriptParser::DIV_ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::AssignmentOperatorContext::MOD_ASSIGN() {
  return getToken(EscriptParser::MOD_ASSIGN, 0);
}


size_t EscriptParser::AssignmentOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleAssignmentOperator;
}

void EscriptParser::AssignmentOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAssignmentOperator(this);
}

void EscriptParser::AssignmentOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAssignmentOperator(this);
}


antlrcpp::Any EscriptParser::AssignmentOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitAssignmentOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::AssignmentOperatorContext* EscriptParser::assignmentOperator() {
  AssignmentOperatorContext *_localctx = _tracker.createInstance<AssignmentOperatorContext>(_ctx, getState());
  enterRule(_localctx, 174, EscriptParser::RuleAssignmentOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(989);
    _la = _input->LA(1);
    if (!(((((_la - 90) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 90)) & ((1ULL << (EscriptParser::ADD_ASSIGN - 90))
      | (1ULL << (EscriptParser::SUB_ASSIGN - 90))
      | (1ULL << (EscriptParser::MUL_ASSIGN - 90))
      | (1ULL << (EscriptParser::DIV_ASSIGN - 90))
      | (1ULL << (EscriptParser::MOD_ASSIGN - 90))
      | (1ULL << (EscriptParser::ASSIGN - 90)))) != 0))) {
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

//----------------- MembershipContext ------------------------------------------------------------------

EscriptParser::MembershipContext::MembershipContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::DisjunctionContext *> EscriptParser::MembershipContext::disjunction() {
  return getRuleContexts<EscriptParser::DisjunctionContext>();
}

EscriptParser::DisjunctionContext* EscriptParser::MembershipContext::disjunction(size_t i) {
  return getRuleContext<EscriptParser::DisjunctionContext>(i);
}

std::vector<EscriptParser::MembershipOperatorContext *> EscriptParser::MembershipContext::membershipOperator() {
  return getRuleContexts<EscriptParser::MembershipOperatorContext>();
}

EscriptParser::MembershipOperatorContext* EscriptParser::MembershipContext::membershipOperator(size_t i) {
  return getRuleContext<EscriptParser::MembershipOperatorContext>(i);
}


size_t EscriptParser::MembershipContext::getRuleIndex() const {
  return EscriptParser::RuleMembership;
}

void EscriptParser::MembershipContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMembership(this);
}

void EscriptParser::MembershipContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMembership(this);
}


antlrcpp::Any EscriptParser::MembershipContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMembership(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MembershipContext* EscriptParser::membership() {
  MembershipContext *_localctx = _tracker.createInstance<MembershipContext>(_ctx, getState());
  enterRule(_localctx, 176, EscriptParser::RuleMembership);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(991);
    disjunction();
    setState(997);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 109) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 109)) & ((1ULL << (EscriptParser::ADDMEMBER - 109))
      | (1ULL << (EscriptParser::DELMEMBER - 109))
      | (1ULL << (EscriptParser::CHKMEMBER - 109)))) != 0)) {
      setState(992);
      membershipOperator();
      setState(993);
      disjunction();
      setState(999);
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

//----------------- MembershipOperatorContext ------------------------------------------------------------------

EscriptParser::MembershipOperatorContext::MembershipOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::MembershipOperatorContext::ADDMEMBER() {
  return getToken(EscriptParser::ADDMEMBER, 0);
}

tree::TerminalNode* EscriptParser::MembershipOperatorContext::DELMEMBER() {
  return getToken(EscriptParser::DELMEMBER, 0);
}

tree::TerminalNode* EscriptParser::MembershipOperatorContext::CHKMEMBER() {
  return getToken(EscriptParser::CHKMEMBER, 0);
}


size_t EscriptParser::MembershipOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleMembershipOperator;
}

void EscriptParser::MembershipOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMembershipOperator(this);
}

void EscriptParser::MembershipOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMembershipOperator(this);
}


antlrcpp::Any EscriptParser::MembershipOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMembershipOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MembershipOperatorContext* EscriptParser::membershipOperator() {
  MembershipOperatorContext *_localctx = _tracker.createInstance<MembershipOperatorContext>(_ctx, getState());
  enterRule(_localctx, 178, EscriptParser::RuleMembershipOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1000);
    _la = _input->LA(1);
    if (!(((((_la - 109) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 109)) & ((1ULL << (EscriptParser::ADDMEMBER - 109))
      | (1ULL << (EscriptParser::DELMEMBER - 109))
      | (1ULL << (EscriptParser::CHKMEMBER - 109)))) != 0))) {
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

//----------------- DisjunctionContext ------------------------------------------------------------------

EscriptParser::DisjunctionContext::DisjunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ConjunctionContext *> EscriptParser::DisjunctionContext::conjunction() {
  return getRuleContexts<EscriptParser::ConjunctionContext>();
}

EscriptParser::ConjunctionContext* EscriptParser::DisjunctionContext::conjunction(size_t i) {
  return getRuleContext<EscriptParser::ConjunctionContext>(i);
}

std::vector<EscriptParser::DisjunctionOperatorContext *> EscriptParser::DisjunctionContext::disjunctionOperator() {
  return getRuleContexts<EscriptParser::DisjunctionOperatorContext>();
}

EscriptParser::DisjunctionOperatorContext* EscriptParser::DisjunctionContext::disjunctionOperator(size_t i) {
  return getRuleContext<EscriptParser::DisjunctionOperatorContext>(i);
}


size_t EscriptParser::DisjunctionContext::getRuleIndex() const {
  return EscriptParser::RuleDisjunction;
}

void EscriptParser::DisjunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDisjunction(this);
}

void EscriptParser::DisjunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDisjunction(this);
}


antlrcpp::Any EscriptParser::DisjunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDisjunction(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DisjunctionContext* EscriptParser::disjunction() {
  DisjunctionContext *_localctx = _tracker.createInstance<DisjunctionContext>(_ctx, getState());
  enterRule(_localctx, 180, EscriptParser::RuleDisjunction);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1002);
    conjunction();
    setState(1008);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::OR_A

    || _la == EscriptParser::OR_B) {
      setState(1003);
      disjunctionOperator();
      setState(1004);
      conjunction();
      setState(1010);
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

//----------------- DisjunctionOperatorContext ------------------------------------------------------------------

EscriptParser::DisjunctionOperatorContext::DisjunctionOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::DisjunctionOperatorContext::OR_A() {
  return getToken(EscriptParser::OR_A, 0);
}

tree::TerminalNode* EscriptParser::DisjunctionOperatorContext::OR_B() {
  return getToken(EscriptParser::OR_B, 0);
}


size_t EscriptParser::DisjunctionOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleDisjunctionOperator;
}

void EscriptParser::DisjunctionOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDisjunctionOperator(this);
}

void EscriptParser::DisjunctionOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDisjunctionOperator(this);
}


antlrcpp::Any EscriptParser::DisjunctionOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDisjunctionOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DisjunctionOperatorContext* EscriptParser::disjunctionOperator() {
  DisjunctionOperatorContext *_localctx = _tracker.createInstance<DisjunctionOperatorContext>(_ctx, getState());
  enterRule(_localctx, 182, EscriptParser::RuleDisjunctionOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1011);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::OR_A

    || _la == EscriptParser::OR_B)) {
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

//----------------- ConjunctionContext ------------------------------------------------------------------

EscriptParser::ConjunctionContext::ConjunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::EqualityContext *> EscriptParser::ConjunctionContext::equality() {
  return getRuleContexts<EscriptParser::EqualityContext>();
}

EscriptParser::EqualityContext* EscriptParser::ConjunctionContext::equality(size_t i) {
  return getRuleContext<EscriptParser::EqualityContext>(i);
}

std::vector<EscriptParser::ConjunctionOperatorContext *> EscriptParser::ConjunctionContext::conjunctionOperator() {
  return getRuleContexts<EscriptParser::ConjunctionOperatorContext>();
}

EscriptParser::ConjunctionOperatorContext* EscriptParser::ConjunctionContext::conjunctionOperator(size_t i) {
  return getRuleContext<EscriptParser::ConjunctionOperatorContext>(i);
}


size_t EscriptParser::ConjunctionContext::getRuleIndex() const {
  return EscriptParser::RuleConjunction;
}

void EscriptParser::ConjunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConjunction(this);
}

void EscriptParser::ConjunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConjunction(this);
}


antlrcpp::Any EscriptParser::ConjunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitConjunction(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ConjunctionContext* EscriptParser::conjunction() {
  ConjunctionContext *_localctx = _tracker.createInstance<ConjunctionContext>(_ctx, getState());
  enterRule(_localctx, 184, EscriptParser::RuleConjunction);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1013);
    equality();
    setState(1019);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::AND_A

    || _la == EscriptParser::AND_B) {
      setState(1014);
      conjunctionOperator();
      setState(1015);
      equality();
      setState(1021);
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

//----------------- ConjunctionOperatorContext ------------------------------------------------------------------

EscriptParser::ConjunctionOperatorContext::ConjunctionOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ConjunctionOperatorContext::AND_A() {
  return getToken(EscriptParser::AND_A, 0);
}

tree::TerminalNode* EscriptParser::ConjunctionOperatorContext::AND_B() {
  return getToken(EscriptParser::AND_B, 0);
}


size_t EscriptParser::ConjunctionOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleConjunctionOperator;
}

void EscriptParser::ConjunctionOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConjunctionOperator(this);
}

void EscriptParser::ConjunctionOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConjunctionOperator(this);
}


antlrcpp::Any EscriptParser::ConjunctionOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitConjunctionOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ConjunctionOperatorContext* EscriptParser::conjunctionOperator() {
  ConjunctionOperatorContext *_localctx = _tracker.createInstance<ConjunctionOperatorContext>(_ctx, getState());
  enterRule(_localctx, 186, EscriptParser::RuleConjunctionOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1022);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::AND_A

    || _la == EscriptParser::AND_B)) {
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

//----------------- EqualityContext ------------------------------------------------------------------

EscriptParser::EqualityContext::EqualityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::BitwiseDisjunctionContext *> EscriptParser::EqualityContext::bitwiseDisjunction() {
  return getRuleContexts<EscriptParser::BitwiseDisjunctionContext>();
}

EscriptParser::BitwiseDisjunctionContext* EscriptParser::EqualityContext::bitwiseDisjunction(size_t i) {
  return getRuleContext<EscriptParser::BitwiseDisjunctionContext>(i);
}

std::vector<EscriptParser::EqualityOperatorContext *> EscriptParser::EqualityContext::equalityOperator() {
  return getRuleContexts<EscriptParser::EqualityOperatorContext>();
}

EscriptParser::EqualityOperatorContext* EscriptParser::EqualityContext::equalityOperator(size_t i) {
  return getRuleContext<EscriptParser::EqualityOperatorContext>(i);
}


size_t EscriptParser::EqualityContext::getRuleIndex() const {
  return EscriptParser::RuleEquality;
}

void EscriptParser::EqualityContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEquality(this);
}

void EscriptParser::EqualityContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEquality(this);
}


antlrcpp::Any EscriptParser::EqualityContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEquality(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EqualityContext* EscriptParser::equality() {
  EqualityContext *_localctx = _tracker.createInstance<EqualityContext>(_ctx, getState());
  enterRule(_localctx, 188, EscriptParser::RuleEquality);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1024);
    bitwiseDisjunction();
    setState(1030);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 104) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 104)) & ((1ULL << (EscriptParser::NOTEQUAL_A - 104))
      | (1ULL << (EscriptParser::NOTEQUAL_B - 104))
      | (1ULL << (EscriptParser::EQUAL_DEPRECATED - 104))
      | (1ULL << (EscriptParser::EQUAL - 104)))) != 0)) {
      setState(1025);
      equalityOperator();
      setState(1026);
      bitwiseDisjunction();
      setState(1032);
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

//----------------- EqualityOperatorContext ------------------------------------------------------------------

EscriptParser::EqualityOperatorContext::EqualityOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::EqualityOperatorContext::EQUAL() {
  return getToken(EscriptParser::EQUAL, 0);
}

tree::TerminalNode* EscriptParser::EqualityOperatorContext::EQUAL_DEPRECATED() {
  return getToken(EscriptParser::EQUAL_DEPRECATED, 0);
}

tree::TerminalNode* EscriptParser::EqualityOperatorContext::NOTEQUAL_A() {
  return getToken(EscriptParser::NOTEQUAL_A, 0);
}

tree::TerminalNode* EscriptParser::EqualityOperatorContext::NOTEQUAL_B() {
  return getToken(EscriptParser::NOTEQUAL_B, 0);
}


size_t EscriptParser::EqualityOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleEqualityOperator;
}

void EscriptParser::EqualityOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterEqualityOperator(this);
}

void EscriptParser::EqualityOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitEqualityOperator(this);
}


antlrcpp::Any EscriptParser::EqualityOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitEqualityOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::EqualityOperatorContext* EscriptParser::equalityOperator() {
  EqualityOperatorContext *_localctx = _tracker.createInstance<EqualityOperatorContext>(_ctx, getState());
  enterRule(_localctx, 190, EscriptParser::RuleEqualityOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1033);
    _la = _input->LA(1);
    if (!(((((_la - 104) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 104)) & ((1ULL << (EscriptParser::NOTEQUAL_A - 104))
      | (1ULL << (EscriptParser::NOTEQUAL_B - 104))
      | (1ULL << (EscriptParser::EQUAL_DEPRECATED - 104))
      | (1ULL << (EscriptParser::EQUAL - 104)))) != 0))) {
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

//----------------- BitwiseDisjunctionContext ------------------------------------------------------------------

EscriptParser::BitwiseDisjunctionContext::BitwiseDisjunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::BitwiseXorContext *> EscriptParser::BitwiseDisjunctionContext::bitwiseXor() {
  return getRuleContexts<EscriptParser::BitwiseXorContext>();
}

EscriptParser::BitwiseXorContext* EscriptParser::BitwiseDisjunctionContext::bitwiseXor(size_t i) {
  return getRuleContext<EscriptParser::BitwiseXorContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::BitwiseDisjunctionContext::BITOR() {
  return getTokens(EscriptParser::BITOR);
}

tree::TerminalNode* EscriptParser::BitwiseDisjunctionContext::BITOR(size_t i) {
  return getToken(EscriptParser::BITOR, i);
}


size_t EscriptParser::BitwiseDisjunctionContext::getRuleIndex() const {
  return EscriptParser::RuleBitwiseDisjunction;
}

void EscriptParser::BitwiseDisjunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBitwiseDisjunction(this);
}

void EscriptParser::BitwiseDisjunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBitwiseDisjunction(this);
}


antlrcpp::Any EscriptParser::BitwiseDisjunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBitwiseDisjunction(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BitwiseDisjunctionContext* EscriptParser::bitwiseDisjunction() {
  BitwiseDisjunctionContext *_localctx = _tracker.createInstance<BitwiseDisjunctionContext>(_ctx, getState());
  enterRule(_localctx, 192, EscriptParser::RuleBitwiseDisjunction);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1035);
    bitwiseXor();
    setState(1040);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::BITOR) {
      setState(1036);
      match(EscriptParser::BITOR);
      setState(1037);
      bitwiseXor();
      setState(1042);
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

//----------------- BitwiseXorContext ------------------------------------------------------------------

EscriptParser::BitwiseXorContext::BitwiseXorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::BitwiseConjunctionContext *> EscriptParser::BitwiseXorContext::bitwiseConjunction() {
  return getRuleContexts<EscriptParser::BitwiseConjunctionContext>();
}

EscriptParser::BitwiseConjunctionContext* EscriptParser::BitwiseXorContext::bitwiseConjunction(size_t i) {
  return getRuleContext<EscriptParser::BitwiseConjunctionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::BitwiseXorContext::CARET() {
  return getTokens(EscriptParser::CARET);
}

tree::TerminalNode* EscriptParser::BitwiseXorContext::CARET(size_t i) {
  return getToken(EscriptParser::CARET, i);
}


size_t EscriptParser::BitwiseXorContext::getRuleIndex() const {
  return EscriptParser::RuleBitwiseXor;
}

void EscriptParser::BitwiseXorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBitwiseXor(this);
}

void EscriptParser::BitwiseXorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBitwiseXor(this);
}


antlrcpp::Any EscriptParser::BitwiseXorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBitwiseXor(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BitwiseXorContext* EscriptParser::bitwiseXor() {
  BitwiseXorContext *_localctx = _tracker.createInstance<BitwiseXorContext>(_ctx, getState());
  enterRule(_localctx, 194, EscriptParser::RuleBitwiseXor);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1043);
    bitwiseConjunction();
    setState(1048);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::CARET) {
      setState(1044);
      match(EscriptParser::CARET);
      setState(1045);
      bitwiseConjunction();
      setState(1050);
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

//----------------- BitwiseConjunctionContext ------------------------------------------------------------------

EscriptParser::BitwiseConjunctionContext::BitwiseConjunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::BitshiftRightContext *> EscriptParser::BitwiseConjunctionContext::bitshiftRight() {
  return getRuleContexts<EscriptParser::BitshiftRightContext>();
}

EscriptParser::BitshiftRightContext* EscriptParser::BitwiseConjunctionContext::bitshiftRight(size_t i) {
  return getRuleContext<EscriptParser::BitshiftRightContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::BitwiseConjunctionContext::BITAND() {
  return getTokens(EscriptParser::BITAND);
}

tree::TerminalNode* EscriptParser::BitwiseConjunctionContext::BITAND(size_t i) {
  return getToken(EscriptParser::BITAND, i);
}


size_t EscriptParser::BitwiseConjunctionContext::getRuleIndex() const {
  return EscriptParser::RuleBitwiseConjunction;
}

void EscriptParser::BitwiseConjunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBitwiseConjunction(this);
}

void EscriptParser::BitwiseConjunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBitwiseConjunction(this);
}


antlrcpp::Any EscriptParser::BitwiseConjunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBitwiseConjunction(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BitwiseConjunctionContext* EscriptParser::bitwiseConjunction() {
  BitwiseConjunctionContext *_localctx = _tracker.createInstance<BitwiseConjunctionContext>(_ctx, getState());
  enterRule(_localctx, 196, EscriptParser::RuleBitwiseConjunction);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1051);
    bitshiftRight();
    setState(1056);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::BITAND) {
      setState(1052);
      match(EscriptParser::BITAND);
      setState(1053);
      bitshiftRight();
      setState(1058);
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

//----------------- BitshiftRightContext ------------------------------------------------------------------

EscriptParser::BitshiftRightContext::BitshiftRightContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::BitshiftLeftContext *> EscriptParser::BitshiftRightContext::bitshiftLeft() {
  return getRuleContexts<EscriptParser::BitshiftLeftContext>();
}

EscriptParser::BitshiftLeftContext* EscriptParser::BitshiftRightContext::bitshiftLeft(size_t i) {
  return getRuleContext<EscriptParser::BitshiftLeftContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::BitshiftRightContext::RSHIFT() {
  return getTokens(EscriptParser::RSHIFT);
}

tree::TerminalNode* EscriptParser::BitshiftRightContext::RSHIFT(size_t i) {
  return getToken(EscriptParser::RSHIFT, i);
}


size_t EscriptParser::BitshiftRightContext::getRuleIndex() const {
  return EscriptParser::RuleBitshiftRight;
}

void EscriptParser::BitshiftRightContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBitshiftRight(this);
}

void EscriptParser::BitshiftRightContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBitshiftRight(this);
}


antlrcpp::Any EscriptParser::BitshiftRightContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBitshiftRight(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BitshiftRightContext* EscriptParser::bitshiftRight() {
  BitshiftRightContext *_localctx = _tracker.createInstance<BitshiftRightContext>(_ctx, getState());
  enterRule(_localctx, 198, EscriptParser::RuleBitshiftRight);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1059);
    bitshiftLeft();
    setState(1064);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::RSHIFT) {
      setState(1060);
      match(EscriptParser::RSHIFT);
      setState(1061);
      bitshiftLeft();
      setState(1066);
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

//----------------- BitshiftLeftContext ------------------------------------------------------------------

EscriptParser::BitshiftLeftContext::BitshiftLeftContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ComparisonContext *> EscriptParser::BitshiftLeftContext::comparison() {
  return getRuleContexts<EscriptParser::ComparisonContext>();
}

EscriptParser::ComparisonContext* EscriptParser::BitshiftLeftContext::comparison(size_t i) {
  return getRuleContext<EscriptParser::ComparisonContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::BitshiftLeftContext::LSHIFT() {
  return getTokens(EscriptParser::LSHIFT);
}

tree::TerminalNode* EscriptParser::BitshiftLeftContext::LSHIFT(size_t i) {
  return getToken(EscriptParser::LSHIFT, i);
}


size_t EscriptParser::BitshiftLeftContext::getRuleIndex() const {
  return EscriptParser::RuleBitshiftLeft;
}

void EscriptParser::BitshiftLeftContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBitshiftLeft(this);
}

void EscriptParser::BitshiftLeftContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBitshiftLeft(this);
}


antlrcpp::Any EscriptParser::BitshiftLeftContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitBitshiftLeft(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::BitshiftLeftContext* EscriptParser::bitshiftLeft() {
  BitshiftLeftContext *_localctx = _tracker.createInstance<BitshiftLeftContext>(_ctx, getState());
  enterRule(_localctx, 200, EscriptParser::RuleBitshiftLeft);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1067);
    comparison();
    setState(1072);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::LSHIFT) {
      setState(1068);
      match(EscriptParser::LSHIFT);
      setState(1069);
      comparison();
      setState(1074);
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

//----------------- ComparisonContext ------------------------------------------------------------------

EscriptParser::ComparisonContext::ComparisonContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::InfixOperationContext *> EscriptParser::ComparisonContext::infixOperation() {
  return getRuleContexts<EscriptParser::InfixOperationContext>();
}

EscriptParser::InfixOperationContext* EscriptParser::ComparisonContext::infixOperation(size_t i) {
  return getRuleContext<EscriptParser::InfixOperationContext>(i);
}

std::vector<EscriptParser::ComparisonOperatorContext *> EscriptParser::ComparisonContext::comparisonOperator() {
  return getRuleContexts<EscriptParser::ComparisonOperatorContext>();
}

EscriptParser::ComparisonOperatorContext* EscriptParser::ComparisonContext::comparisonOperator(size_t i) {
  return getRuleContext<EscriptParser::ComparisonOperatorContext>(i);
}


size_t EscriptParser::ComparisonContext::getRuleIndex() const {
  return EscriptParser::RuleComparison;
}

void EscriptParser::ComparisonContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComparison(this);
}

void EscriptParser::ComparisonContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComparison(this);
}


antlrcpp::Any EscriptParser::ComparisonContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitComparison(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ComparisonContext* EscriptParser::comparison() {
  ComparisonContext *_localctx = _tracker.createInstance<ComparisonContext>(_ctx, getState());
  enterRule(_localctx, 202, EscriptParser::RuleComparison);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1075);
    infixOperation();
    setState(1081);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 95) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 95)) & ((1ULL << (EscriptParser::LE - 95))
      | (1ULL << (EscriptParser::LT - 95))
      | (1ULL << (EscriptParser::GE - 95))
      | (1ULL << (EscriptParser::GT - 95)))) != 0)) {
      setState(1076);
      comparisonOperator();
      setState(1077);
      infixOperation();
      setState(1083);
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

//----------------- ComparisonOperatorContext ------------------------------------------------------------------

EscriptParser::ComparisonOperatorContext::ComparisonOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ComparisonOperatorContext::LE() {
  return getToken(EscriptParser::LE, 0);
}

tree::TerminalNode* EscriptParser::ComparisonOperatorContext::LT() {
  return getToken(EscriptParser::LT, 0);
}

tree::TerminalNode* EscriptParser::ComparisonOperatorContext::GE() {
  return getToken(EscriptParser::GE, 0);
}

tree::TerminalNode* EscriptParser::ComparisonOperatorContext::GT() {
  return getToken(EscriptParser::GT, 0);
}


size_t EscriptParser::ComparisonOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleComparisonOperator;
}

void EscriptParser::ComparisonOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComparisonOperator(this);
}

void EscriptParser::ComparisonOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComparisonOperator(this);
}


antlrcpp::Any EscriptParser::ComparisonOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitComparisonOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ComparisonOperatorContext* EscriptParser::comparisonOperator() {
  ComparisonOperatorContext *_localctx = _tracker.createInstance<ComparisonOperatorContext>(_ctx, getState());
  enterRule(_localctx, 204, EscriptParser::RuleComparisonOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1084);
    _la = _input->LA(1);
    if (!(((((_la - 95) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 95)) & ((1ULL << (EscriptParser::LE - 95))
      | (1ULL << (EscriptParser::LT - 95))
      | (1ULL << (EscriptParser::GE - 95))
      | (1ULL << (EscriptParser::GT - 95)))) != 0))) {
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

//----------------- InfixOperationContext ------------------------------------------------------------------

EscriptParser::InfixOperationContext::InfixOperationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::ElvisExpressionContext *> EscriptParser::InfixOperationContext::elvisExpression() {
  return getRuleContexts<EscriptParser::ElvisExpressionContext>();
}

EscriptParser::ElvisExpressionContext* EscriptParser::InfixOperationContext::elvisExpression(size_t i) {
  return getRuleContext<EscriptParser::ElvisExpressionContext>(i);
}

std::vector<EscriptParser::InfixOperatorContext *> EscriptParser::InfixOperationContext::infixOperator() {
  return getRuleContexts<EscriptParser::InfixOperatorContext>();
}

EscriptParser::InfixOperatorContext* EscriptParser::InfixOperationContext::infixOperator(size_t i) {
  return getRuleContext<EscriptParser::InfixOperatorContext>(i);
}


size_t EscriptParser::InfixOperationContext::getRuleIndex() const {
  return EscriptParser::RuleInfixOperation;
}

void EscriptParser::InfixOperationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInfixOperation(this);
}

void EscriptParser::InfixOperationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInfixOperation(this);
}


antlrcpp::Any EscriptParser::InfixOperationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitInfixOperation(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::InfixOperationContext* EscriptParser::infixOperation() {
  InfixOperationContext *_localctx = _tracker.createInstance<InfixOperationContext>(_ctx, getState());
  enterRule(_localctx, 206, EscriptParser::RuleInfixOperation);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1086);
    elvisExpression();
    setState(1092);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::TOK_IN) {
      setState(1087);
      infixOperator();
      setState(1088);
      elvisExpression();
      setState(1094);
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

//----------------- InfixOperatorContext ------------------------------------------------------------------

EscriptParser::InfixOperatorContext::InfixOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::InfixOperatorContext::TOK_IN() {
  return getToken(EscriptParser::TOK_IN, 0);
}


size_t EscriptParser::InfixOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleInfixOperator;
}

void EscriptParser::InfixOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInfixOperator(this);
}

void EscriptParser::InfixOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInfixOperator(this);
}


antlrcpp::Any EscriptParser::InfixOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitInfixOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::InfixOperatorContext* EscriptParser::infixOperator() {
  InfixOperatorContext *_localctx = _tracker.createInstance<InfixOperatorContext>(_ctx, getState());
  enterRule(_localctx, 208, EscriptParser::RuleInfixOperator);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1095);
    match(EscriptParser::TOK_IN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ElvisExpressionContext ------------------------------------------------------------------

EscriptParser::ElvisExpressionContext::ElvisExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::AdditiveExpressionContext *> EscriptParser::ElvisExpressionContext::additiveExpression() {
  return getRuleContexts<EscriptParser::AdditiveExpressionContext>();
}

EscriptParser::AdditiveExpressionContext* EscriptParser::ElvisExpressionContext::additiveExpression(size_t i) {
  return getRuleContext<EscriptParser::AdditiveExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::ElvisExpressionContext::ELVIS() {
  return getTokens(EscriptParser::ELVIS);
}

tree::TerminalNode* EscriptParser::ElvisExpressionContext::ELVIS(size_t i) {
  return getToken(EscriptParser::ELVIS, i);
}


size_t EscriptParser::ElvisExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleElvisExpression;
}

void EscriptParser::ElvisExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterElvisExpression(this);
}

void EscriptParser::ElvisExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitElvisExpression(this);
}


antlrcpp::Any EscriptParser::ElvisExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitElvisExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ElvisExpressionContext* EscriptParser::elvisExpression() {
  ElvisExpressionContext *_localctx = _tracker.createInstance<ElvisExpressionContext>(_ctx, getState());
  enterRule(_localctx, 210, EscriptParser::RuleElvisExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1097);
    additiveExpression();
    setState(1102);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::ELVIS) {
      setState(1098);
      match(EscriptParser::ELVIS);
      setState(1099);
      additiveExpression();
      setState(1104);
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

//----------------- AdditiveExpressionContext ------------------------------------------------------------------

EscriptParser::AdditiveExpressionContext::AdditiveExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::MultiplicativeExpressionContext *> EscriptParser::AdditiveExpressionContext::multiplicativeExpression() {
  return getRuleContexts<EscriptParser::MultiplicativeExpressionContext>();
}

EscriptParser::MultiplicativeExpressionContext* EscriptParser::AdditiveExpressionContext::multiplicativeExpression(size_t i) {
  return getRuleContext<EscriptParser::MultiplicativeExpressionContext>(i);
}

std::vector<EscriptParser::AdditiveOperatorContext *> EscriptParser::AdditiveExpressionContext::additiveOperator() {
  return getRuleContexts<EscriptParser::AdditiveOperatorContext>();
}

EscriptParser::AdditiveOperatorContext* EscriptParser::AdditiveExpressionContext::additiveOperator(size_t i) {
  return getRuleContext<EscriptParser::AdditiveOperatorContext>(i);
}


size_t EscriptParser::AdditiveExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleAdditiveExpression;
}

void EscriptParser::AdditiveExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAdditiveExpression(this);
}

void EscriptParser::AdditiveExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAdditiveExpression(this);
}


antlrcpp::Any EscriptParser::AdditiveExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitAdditiveExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::AdditiveExpressionContext* EscriptParser::additiveExpression() {
  AdditiveExpressionContext *_localctx = _tracker.createInstance<AdditiveExpressionContext>(_ctx, getState());
  enterRule(_localctx, 212, EscriptParser::RuleAdditiveExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1105);
    multiplicativeExpression();
    setState(1111);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 96, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1106);
        additiveOperator();
        setState(1107);
        multiplicativeExpression(); 
      }
      setState(1113);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 96, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AdditiveOperatorContext ------------------------------------------------------------------

EscriptParser::AdditiveOperatorContext::AdditiveOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::AdditiveOperatorContext::ADD() {
  return getToken(EscriptParser::ADD, 0);
}

tree::TerminalNode* EscriptParser::AdditiveOperatorContext::SUB() {
  return getToken(EscriptParser::SUB, 0);
}


size_t EscriptParser::AdditiveOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleAdditiveOperator;
}

void EscriptParser::AdditiveOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAdditiveOperator(this);
}

void EscriptParser::AdditiveOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAdditiveOperator(this);
}


antlrcpp::Any EscriptParser::AdditiveOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitAdditiveOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::AdditiveOperatorContext* EscriptParser::additiveOperator() {
  AdditiveOperatorContext *_localctx = _tracker.createInstance<AdditiveOperatorContext>(_ctx, getState());
  enterRule(_localctx, 214, EscriptParser::RuleAdditiveOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1114);
    _la = _input->LA(1);
    if (!(_la == EscriptParser::ADD

    || _la == EscriptParser::SUB)) {
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

//----------------- MultiplicativeExpressionContext ------------------------------------------------------------------

EscriptParser::MultiplicativeExpressionContext::MultiplicativeExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::PrefixUnaryInversionExpressionContext *> EscriptParser::MultiplicativeExpressionContext::prefixUnaryInversionExpression() {
  return getRuleContexts<EscriptParser::PrefixUnaryInversionExpressionContext>();
}

EscriptParser::PrefixUnaryInversionExpressionContext* EscriptParser::MultiplicativeExpressionContext::prefixUnaryInversionExpression(size_t i) {
  return getRuleContext<EscriptParser::PrefixUnaryInversionExpressionContext>(i);
}

std::vector<EscriptParser::MultiplicativeOperatorContext *> EscriptParser::MultiplicativeExpressionContext::multiplicativeOperator() {
  return getRuleContexts<EscriptParser::MultiplicativeOperatorContext>();
}

EscriptParser::MultiplicativeOperatorContext* EscriptParser::MultiplicativeExpressionContext::multiplicativeOperator(size_t i) {
  return getRuleContext<EscriptParser::MultiplicativeOperatorContext>(i);
}


size_t EscriptParser::MultiplicativeExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleMultiplicativeExpression;
}

void EscriptParser::MultiplicativeExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMultiplicativeExpression(this);
}

void EscriptParser::MultiplicativeExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMultiplicativeExpression(this);
}


antlrcpp::Any EscriptParser::MultiplicativeExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MultiplicativeExpressionContext* EscriptParser::multiplicativeExpression() {
  MultiplicativeExpressionContext *_localctx = _tracker.createInstance<MultiplicativeExpressionContext>(_ctx, getState());
  enterRule(_localctx, 216, EscriptParser::RuleMultiplicativeExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1116);
    prefixUnaryInversionExpression();
    setState(1122);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 85) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 85)) & ((1ULL << (EscriptParser::MUL - 85))
      | (1ULL << (EscriptParser::DIV - 85))
      | (1ULL << (EscriptParser::MOD - 85)))) != 0)) {
      setState(1117);
      multiplicativeOperator();
      setState(1118);
      prefixUnaryInversionExpression();
      setState(1124);
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

//----------------- MultiplicativeOperatorContext ------------------------------------------------------------------

EscriptParser::MultiplicativeOperatorContext::MultiplicativeOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::MultiplicativeOperatorContext::MUL() {
  return getToken(EscriptParser::MUL, 0);
}

tree::TerminalNode* EscriptParser::MultiplicativeOperatorContext::DIV() {
  return getToken(EscriptParser::DIV, 0);
}

tree::TerminalNode* EscriptParser::MultiplicativeOperatorContext::MOD() {
  return getToken(EscriptParser::MOD, 0);
}


size_t EscriptParser::MultiplicativeOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleMultiplicativeOperator;
}

void EscriptParser::MultiplicativeOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMultiplicativeOperator(this);
}

void EscriptParser::MultiplicativeOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMultiplicativeOperator(this);
}


antlrcpp::Any EscriptParser::MultiplicativeOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMultiplicativeOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MultiplicativeOperatorContext* EscriptParser::multiplicativeOperator() {
  MultiplicativeOperatorContext *_localctx = _tracker.createInstance<MultiplicativeOperatorContext>(_ctx, getState());
  enterRule(_localctx, 218, EscriptParser::RuleMultiplicativeOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1125);
    _la = _input->LA(1);
    if (!(((((_la - 85) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 85)) & ((1ULL << (EscriptParser::MUL - 85))
      | (1ULL << (EscriptParser::DIV - 85))
      | (1ULL << (EscriptParser::MOD - 85)))) != 0))) {
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

//----------------- PrefixUnaryInversionExpressionContext ------------------------------------------------------------------

EscriptParser::PrefixUnaryInversionExpressionContext::PrefixUnaryInversionExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::PrefixUnaryArithmeticExpressionContext* EscriptParser::PrefixUnaryInversionExpressionContext::prefixUnaryArithmeticExpression() {
  return getRuleContext<EscriptParser::PrefixUnaryArithmeticExpressionContext>(0);
}

std::vector<EscriptParser::PrefixUnaryInversionOperatorContext *> EscriptParser::PrefixUnaryInversionExpressionContext::prefixUnaryInversionOperator() {
  return getRuleContexts<EscriptParser::PrefixUnaryInversionOperatorContext>();
}

EscriptParser::PrefixUnaryInversionOperatorContext* EscriptParser::PrefixUnaryInversionExpressionContext::prefixUnaryInversionOperator(size_t i) {
  return getRuleContext<EscriptParser::PrefixUnaryInversionOperatorContext>(i);
}


size_t EscriptParser::PrefixUnaryInversionExpressionContext::getRuleIndex() const {
  return EscriptParser::RulePrefixUnaryInversionExpression;
}

void EscriptParser::PrefixUnaryInversionExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrefixUnaryInversionExpression(this);
}

void EscriptParser::PrefixUnaryInversionExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrefixUnaryInversionExpression(this);
}


antlrcpp::Any EscriptParser::PrefixUnaryInversionExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPrefixUnaryInversionExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PrefixUnaryInversionExpressionContext* EscriptParser::prefixUnaryInversionExpression() {
  PrefixUnaryInversionExpressionContext *_localctx = _tracker.createInstance<PrefixUnaryInversionExpressionContext>(_ctx, getState());
  enterRule(_localctx, 220, EscriptParser::RulePrefixUnaryInversionExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1130);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 57) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 57)) & ((1ULL << (EscriptParser::BANG_A - 57))
      | (1ULL << (EscriptParser::BANG_B - 57))
      | (1ULL << (EscriptParser::TILDE - 57)))) != 0)) {
      setState(1127);
      prefixUnaryInversionOperator();
      setState(1132);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(1133);
    prefixUnaryArithmeticExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrefixUnaryInversionOperatorContext ------------------------------------------------------------------

EscriptParser::PrefixUnaryInversionOperatorContext::PrefixUnaryInversionOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::PrefixUnaryInversionOperatorContext::TILDE() {
  return getToken(EscriptParser::TILDE, 0);
}

tree::TerminalNode* EscriptParser::PrefixUnaryInversionOperatorContext::BANG_A() {
  return getToken(EscriptParser::BANG_A, 0);
}

tree::TerminalNode* EscriptParser::PrefixUnaryInversionOperatorContext::BANG_B() {
  return getToken(EscriptParser::BANG_B, 0);
}


size_t EscriptParser::PrefixUnaryInversionOperatorContext::getRuleIndex() const {
  return EscriptParser::RulePrefixUnaryInversionOperator;
}

void EscriptParser::PrefixUnaryInversionOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrefixUnaryInversionOperator(this);
}

void EscriptParser::PrefixUnaryInversionOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrefixUnaryInversionOperator(this);
}


antlrcpp::Any EscriptParser::PrefixUnaryInversionOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPrefixUnaryInversionOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PrefixUnaryInversionOperatorContext* EscriptParser::prefixUnaryInversionOperator() {
  PrefixUnaryInversionOperatorContext *_localctx = _tracker.createInstance<PrefixUnaryInversionOperatorContext>(_ctx, getState());
  enterRule(_localctx, 222, EscriptParser::RulePrefixUnaryInversionOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1135);
    _la = _input->LA(1);
    if (!(((((_la - 57) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 57)) & ((1ULL << (EscriptParser::BANG_A - 57))
      | (1ULL << (EscriptParser::BANG_B - 57))
      | (1ULL << (EscriptParser::TILDE - 57)))) != 0))) {
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

//----------------- PrefixUnaryArithmeticExpressionContext ------------------------------------------------------------------

EscriptParser::PrefixUnaryArithmeticExpressionContext::PrefixUnaryArithmeticExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::PostfixUnaryExpressionContext* EscriptParser::PrefixUnaryArithmeticExpressionContext::postfixUnaryExpression() {
  return getRuleContext<EscriptParser::PostfixUnaryExpressionContext>(0);
}

std::vector<EscriptParser::PrefixUnaryArithmeticOperatorContext *> EscriptParser::PrefixUnaryArithmeticExpressionContext::prefixUnaryArithmeticOperator() {
  return getRuleContexts<EscriptParser::PrefixUnaryArithmeticOperatorContext>();
}

EscriptParser::PrefixUnaryArithmeticOperatorContext* EscriptParser::PrefixUnaryArithmeticExpressionContext::prefixUnaryArithmeticOperator(size_t i) {
  return getRuleContext<EscriptParser::PrefixUnaryArithmeticOperatorContext>(i);
}


size_t EscriptParser::PrefixUnaryArithmeticExpressionContext::getRuleIndex() const {
  return EscriptParser::RulePrefixUnaryArithmeticExpression;
}

void EscriptParser::PrefixUnaryArithmeticExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrefixUnaryArithmeticExpression(this);
}

void EscriptParser::PrefixUnaryArithmeticExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrefixUnaryArithmeticExpression(this);
}


antlrcpp::Any EscriptParser::PrefixUnaryArithmeticExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPrefixUnaryArithmeticExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PrefixUnaryArithmeticExpressionContext* EscriptParser::prefixUnaryArithmeticExpression() {
  PrefixUnaryArithmeticExpressionContext *_localctx = _tracker.createInstance<PrefixUnaryArithmeticExpressionContext>(_ctx, getState());
  enterRule(_localctx, 224, EscriptParser::RulePrefixUnaryArithmeticExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1140);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (((((_la - 88) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 88)) & ((1ULL << (EscriptParser::ADD - 88))
      | (1ULL << (EscriptParser::SUB - 88))
      | (1ULL << (EscriptParser::INC - 88))
      | (1ULL << (EscriptParser::DEC - 88)))) != 0)) {
      setState(1137);
      prefixUnaryArithmeticOperator();
      setState(1142);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(1143);
    postfixUnaryExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PrefixUnaryArithmeticOperatorContext ------------------------------------------------------------------

EscriptParser::PrefixUnaryArithmeticOperatorContext::PrefixUnaryArithmeticOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::PrefixUnaryArithmeticOperatorContext::INC() {
  return getToken(EscriptParser::INC, 0);
}

tree::TerminalNode* EscriptParser::PrefixUnaryArithmeticOperatorContext::DEC() {
  return getToken(EscriptParser::DEC, 0);
}

tree::TerminalNode* EscriptParser::PrefixUnaryArithmeticOperatorContext::ADD() {
  return getToken(EscriptParser::ADD, 0);
}

tree::TerminalNode* EscriptParser::PrefixUnaryArithmeticOperatorContext::SUB() {
  return getToken(EscriptParser::SUB, 0);
}


size_t EscriptParser::PrefixUnaryArithmeticOperatorContext::getRuleIndex() const {
  return EscriptParser::RulePrefixUnaryArithmeticOperator;
}

void EscriptParser::PrefixUnaryArithmeticOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPrefixUnaryArithmeticOperator(this);
}

void EscriptParser::PrefixUnaryArithmeticOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPrefixUnaryArithmeticOperator(this);
}


antlrcpp::Any EscriptParser::PrefixUnaryArithmeticOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPrefixUnaryArithmeticOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PrefixUnaryArithmeticOperatorContext* EscriptParser::prefixUnaryArithmeticOperator() {
  PrefixUnaryArithmeticOperatorContext *_localctx = _tracker.createInstance<PrefixUnaryArithmeticOperatorContext>(_ctx, getState());
  enterRule(_localctx, 226, EscriptParser::RulePrefixUnaryArithmeticOperator);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1145);
    _la = _input->LA(1);
    if (!(((((_la - 88) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 88)) & ((1ULL << (EscriptParser::ADD - 88))
      | (1ULL << (EscriptParser::SUB - 88))
      | (1ULL << (EscriptParser::INC - 88))
      | (1ULL << (EscriptParser::DEC - 88)))) != 0))) {
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

//----------------- PostfixUnaryExpressionContext ------------------------------------------------------------------

EscriptParser::PostfixUnaryExpressionContext::PostfixUnaryExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::AtomicExpressionContext* EscriptParser::PostfixUnaryExpressionContext::atomicExpression() {
  return getRuleContext<EscriptParser::AtomicExpressionContext>(0);
}

std::vector<EscriptParser::PostfixUnaryOperatorContext *> EscriptParser::PostfixUnaryExpressionContext::postfixUnaryOperator() {
  return getRuleContexts<EscriptParser::PostfixUnaryOperatorContext>();
}

EscriptParser::PostfixUnaryOperatorContext* EscriptParser::PostfixUnaryExpressionContext::postfixUnaryOperator(size_t i) {
  return getRuleContext<EscriptParser::PostfixUnaryOperatorContext>(i);
}


size_t EscriptParser::PostfixUnaryExpressionContext::getRuleIndex() const {
  return EscriptParser::RulePostfixUnaryExpression;
}

void EscriptParser::PostfixUnaryExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPostfixUnaryExpression(this);
}

void EscriptParser::PostfixUnaryExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPostfixUnaryExpression(this);
}


antlrcpp::Any EscriptParser::PostfixUnaryExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPostfixUnaryExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PostfixUnaryExpressionContext* EscriptParser::postfixUnaryExpression() {
  PostfixUnaryExpressionContext *_localctx = _tracker.createInstance<PostfixUnaryExpressionContext>(_ctx, getState());
  enterRule(_localctx, 228, EscriptParser::RulePostfixUnaryExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1147);
    atomicExpression();
    setState(1151);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 100, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1148);
        postfixUnaryOperator(); 
      }
      setState(1153);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 100, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PostfixUnaryOperatorContext ------------------------------------------------------------------

EscriptParser::PostfixUnaryOperatorContext::PostfixUnaryOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::PostfixUnaryOperatorContext::INC() {
  return getToken(EscriptParser::INC, 0);
}

tree::TerminalNode* EscriptParser::PostfixUnaryOperatorContext::DEC() {
  return getToken(EscriptParser::DEC, 0);
}

EscriptParser::IndexingSuffixContext* EscriptParser::PostfixUnaryOperatorContext::indexingSuffix() {
  return getRuleContext<EscriptParser::IndexingSuffixContext>(0);
}

EscriptParser::NavigationSuffixContext* EscriptParser::PostfixUnaryOperatorContext::navigationSuffix() {
  return getRuleContext<EscriptParser::NavigationSuffixContext>(0);
}

EscriptParser::MemberCallSuffixContext* EscriptParser::PostfixUnaryOperatorContext::memberCallSuffix() {
  return getRuleContext<EscriptParser::MemberCallSuffixContext>(0);
}


size_t EscriptParser::PostfixUnaryOperatorContext::getRuleIndex() const {
  return EscriptParser::RulePostfixUnaryOperator;
}

void EscriptParser::PostfixUnaryOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPostfixUnaryOperator(this);
}

void EscriptParser::PostfixUnaryOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPostfixUnaryOperator(this);
}


antlrcpp::Any EscriptParser::PostfixUnaryOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPostfixUnaryOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PostfixUnaryOperatorContext* EscriptParser::postfixUnaryOperator() {
  PostfixUnaryOperatorContext *_localctx = _tracker.createInstance<PostfixUnaryOperatorContext>(_ctx, getState());
  enterRule(_localctx, 230, EscriptParser::RulePostfixUnaryOperator);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1159);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 101, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1154);
      match(EscriptParser::INC);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1155);
      match(EscriptParser::DEC);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(1156);
      indexingSuffix();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(1157);
      navigationSuffix();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(1158);
      memberCallSuffix();
      break;
    }

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

EscriptParser::UnambiguousExpressionListContext* EscriptParser::IndexingSuffixContext::unambiguousExpressionList() {
  return getRuleContext<EscriptParser::UnambiguousExpressionListContext>(0);
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


antlrcpp::Any EscriptParser::IndexingSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIndexingSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IndexingSuffixContext* EscriptParser::indexingSuffix() {
  IndexingSuffixContext *_localctx = _tracker.createInstance<IndexingSuffixContext>(_ctx, getState());
  enterRule(_localctx, 232, EscriptParser::RuleIndexingSuffix);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1161);
    match(EscriptParser::LBRACK);
    setState(1162);
    unambiguousExpressionList();
    setState(1163);
    match(EscriptParser::RBRACK);
   
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

EscriptParser::MemberAccessOperatorContext* EscriptParser::NavigationSuffixContext::memberAccessOperator() {
  return getRuleContext<EscriptParser::MemberAccessOperatorContext>(0);
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


antlrcpp::Any EscriptParser::NavigationSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitNavigationSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::NavigationSuffixContext* EscriptParser::navigationSuffix() {
  NavigationSuffixContext *_localctx = _tracker.createInstance<NavigationSuffixContext>(_ctx, getState());
  enterRule(_localctx, 234, EscriptParser::RuleNavigationSuffix);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1165);
    memberAccessOperator();
    setState(1166);
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

//----------------- MembershipSuffixContext ------------------------------------------------------------------

EscriptParser::MembershipSuffixContext::MembershipSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::MembershipOperatorContext* EscriptParser::MembershipSuffixContext::membershipOperator() {
  return getRuleContext<EscriptParser::MembershipOperatorContext>(0);
}

tree::TerminalNode* EscriptParser::MembershipSuffixContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::MembershipSuffixContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::MembershipSuffixContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::MembershipSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleMembershipSuffix;
}

void EscriptParser::MembershipSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMembershipSuffix(this);
}

void EscriptParser::MembershipSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMembershipSuffix(this);
}


antlrcpp::Any EscriptParser::MembershipSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMembershipSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MembershipSuffixContext* EscriptParser::membershipSuffix() {
  MembershipSuffixContext *_localctx = _tracker.createInstance<MembershipSuffixContext>(_ctx, getState());
  enterRule(_localctx, 236, EscriptParser::RuleMembershipSuffix);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1168);
    membershipOperator();
    setState(1172);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 102, _ctx)) {
    case 1: {
      setState(1169);
      match(EscriptParser::IDENTIFIER);
      break;
    }

    case 2: {
      setState(1170);
      match(EscriptParser::STRING_LITERAL);
      break;
    }

    case 3: {
      setState(1171);
      unambiguousExpression();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MemberCallSuffixContext ------------------------------------------------------------------

EscriptParser::MemberCallSuffixContext::MemberCallSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::MemberAccessOperatorContext* EscriptParser::MemberCallSuffixContext::memberAccessOperator() {
  return getRuleContext<EscriptParser::MemberAccessOperatorContext>(0);
}

tree::TerminalNode* EscriptParser::MemberCallSuffixContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::MemberCallSuffixContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::MemberCallSuffixContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::UnambiguousExpressionListContext* EscriptParser::MemberCallSuffixContext::unambiguousExpressionList() {
  return getRuleContext<EscriptParser::UnambiguousExpressionListContext>(0);
}


size_t EscriptParser::MemberCallSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleMemberCallSuffix;
}

void EscriptParser::MemberCallSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMemberCallSuffix(this);
}

void EscriptParser::MemberCallSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMemberCallSuffix(this);
}


antlrcpp::Any EscriptParser::MemberCallSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMemberCallSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MemberCallSuffixContext* EscriptParser::memberCallSuffix() {
  MemberCallSuffixContext *_localctx = _tracker.createInstance<MemberCallSuffixContext>(_ctx, getState());
  enterRule(_localctx, 238, EscriptParser::RuleMemberCallSuffix);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1174);
    memberAccessOperator();
    setState(1175);
    match(EscriptParser::IDENTIFIER);
    setState(1176);
    match(EscriptParser::LPAREN);
    setState(1178);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(1177);
      unambiguousExpressionList();
    }
    setState(1180);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MemberAccessOperatorContext ------------------------------------------------------------------

EscriptParser::MemberAccessOperatorContext::MemberAccessOperatorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::MemberAccessOperatorContext::DOT() {
  return getToken(EscriptParser::DOT, 0);
}


size_t EscriptParser::MemberAccessOperatorContext::getRuleIndex() const {
  return EscriptParser::RuleMemberAccessOperator;
}

void EscriptParser::MemberAccessOperatorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMemberAccessOperator(this);
}

void EscriptParser::MemberAccessOperatorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMemberAccessOperator(this);
}


antlrcpp::Any EscriptParser::MemberAccessOperatorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMemberAccessOperator(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MemberAccessOperatorContext* EscriptParser::memberAccessOperator() {
  MemberAccessOperatorContext *_localctx = _tracker.createInstance<MemberAccessOperatorContext>(_ctx, getState());
  enterRule(_localctx, 240, EscriptParser::RuleMemberAccessOperator);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1182);
    match(EscriptParser::DOT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CallSuffixContext ------------------------------------------------------------------

EscriptParser::CallSuffixContext::CallSuffixContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::ValueArgumentsContext* EscriptParser::CallSuffixContext::valueArguments() {
  return getRuleContext<EscriptParser::ValueArgumentsContext>(0);
}


size_t EscriptParser::CallSuffixContext::getRuleIndex() const {
  return EscriptParser::RuleCallSuffix;
}

void EscriptParser::CallSuffixContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCallSuffix(this);
}

void EscriptParser::CallSuffixContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCallSuffix(this);
}


antlrcpp::Any EscriptParser::CallSuffixContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitCallSuffix(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::CallSuffixContext* EscriptParser::callSuffix() {
  CallSuffixContext *_localctx = _tracker.createInstance<CallSuffixContext>(_ctx, getState());
  enterRule(_localctx, 242, EscriptParser::RuleCallSuffix);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1184);
    valueArguments();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AtomicExpressionContext ------------------------------------------------------------------

EscriptParser::AtomicExpressionContext::AtomicExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::LiteralContext* EscriptParser::AtomicExpressionContext::literal() {
  return getRuleContext<EscriptParser::LiteralContext>(0);
}

EscriptParser::ParenthesizedExpressionContext* EscriptParser::AtomicExpressionContext::parenthesizedExpression() {
  return getRuleContext<EscriptParser::ParenthesizedExpressionContext>(0);
}

EscriptParser::UnambiguousFunctionCallContext* EscriptParser::AtomicExpressionContext::unambiguousFunctionCall() {
  return getRuleContext<EscriptParser::UnambiguousFunctionCallContext>(0);
}

tree::TerminalNode* EscriptParser::AtomicExpressionContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::FunctionReferenceContext* EscriptParser::AtomicExpressionContext::functionReference() {
  return getRuleContext<EscriptParser::FunctionReferenceContext>(0);
}

EscriptParser::UnambiguousScopedFunctionCallContext* EscriptParser::AtomicExpressionContext::unambiguousScopedFunctionCall() {
  return getRuleContext<EscriptParser::UnambiguousScopedFunctionCallContext>(0);
}

EscriptParser::UnambiguousExplicitArrayInitializerContext* EscriptParser::AtomicExpressionContext::unambiguousExplicitArrayInitializer() {
  return getRuleContext<EscriptParser::UnambiguousExplicitArrayInitializerContext>(0);
}

EscriptParser::UnambiguousExplicitStructInitializerContext* EscriptParser::AtomicExpressionContext::unambiguousExplicitStructInitializer() {
  return getRuleContext<EscriptParser::UnambiguousExplicitStructInitializerContext>(0);
}

EscriptParser::UnambiguousExplicitDictInitializerContext* EscriptParser::AtomicExpressionContext::unambiguousExplicitDictInitializer() {
  return getRuleContext<EscriptParser::UnambiguousExplicitDictInitializerContext>(0);
}

EscriptParser::UnambiguousExplicitErrorInitializerContext* EscriptParser::AtomicExpressionContext::unambiguousExplicitErrorInitializer() {
  return getRuleContext<EscriptParser::UnambiguousExplicitErrorInitializerContext>(0);
}

EscriptParser::UnambiguousBareArrayInitializerContext* EscriptParser::AtomicExpressionContext::unambiguousBareArrayInitializer() {
  return getRuleContext<EscriptParser::UnambiguousBareArrayInitializerContext>(0);
}


size_t EscriptParser::AtomicExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleAtomicExpression;
}

void EscriptParser::AtomicExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAtomicExpression(this);
}

void EscriptParser::AtomicExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAtomicExpression(this);
}


antlrcpp::Any EscriptParser::AtomicExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitAtomicExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::AtomicExpressionContext* EscriptParser::atomicExpression() {
  AtomicExpressionContext *_localctx = _tracker.createInstance<AtomicExpressionContext>(_ctx, getState());
  enterRule(_localctx, 244, EscriptParser::RuleAtomicExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1197);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 104, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1186);
      literal();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1187);
      parenthesizedExpression();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(1188);
      unambiguousFunctionCall();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(1189);
      match(EscriptParser::IDENTIFIER);
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(1190);
      functionReference();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(1191);
      unambiguousScopedFunctionCall();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(1192);
      unambiguousExplicitArrayInitializer();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(1193);
      unambiguousExplicitStructInitializer();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(1194);
      unambiguousExplicitDictInitializer();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(1195);
      unambiguousExplicitErrorInitializer();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(1196);
      unambiguousBareArrayInitializer();
      break;
    }

    }
   
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

tree::TerminalNode* EscriptParser::FunctionReferenceContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
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


antlrcpp::Any EscriptParser::FunctionReferenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFunctionReference(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FunctionReferenceContext* EscriptParser::functionReference() {
  FunctionReferenceContext *_localctx = _tracker.createInstance<FunctionReferenceContext>(_ctx, getState());
  enterRule(_localctx, 246, EscriptParser::RuleFunctionReference);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1199);
    match(EscriptParser::AT);
    setState(1200);
    match(EscriptParser::IDENTIFIER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousExplicitArrayInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousExplicitArrayInitializerContext::UnambiguousExplicitArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousExplicitArrayInitializerContext::ARRAY() {
  return getToken(EscriptParser::ARRAY, 0);
}

EscriptParser::UnambiguousArrayInitializerContext* EscriptParser::UnambiguousExplicitArrayInitializerContext::unambiguousArrayInitializer() {
  return getRuleContext<EscriptParser::UnambiguousArrayInitializerContext>(0);
}


size_t EscriptParser::UnambiguousExplicitArrayInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousExplicitArrayInitializer;
}

void EscriptParser::UnambiguousExplicitArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousExplicitArrayInitializer(this);
}

void EscriptParser::UnambiguousExplicitArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousExplicitArrayInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousExplicitArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousExplicitArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousExplicitArrayInitializerContext* EscriptParser::unambiguousExplicitArrayInitializer() {
  UnambiguousExplicitArrayInitializerContext *_localctx = _tracker.createInstance<UnambiguousExplicitArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 248, EscriptParser::RuleUnambiguousExplicitArrayInitializer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1202);
    match(EscriptParser::ARRAY);
    setState(1204);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 105, _ctx)) {
    case 1: {
      setState(1203);
      unambiguousArrayInitializer();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousExplicitStructInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousExplicitStructInitializerContext::UnambiguousExplicitStructInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousExplicitStructInitializerContext::STRUCT() {
  return getToken(EscriptParser::STRUCT, 0);
}

EscriptParser::UnambiguousStructInitializerContext* EscriptParser::UnambiguousExplicitStructInitializerContext::unambiguousStructInitializer() {
  return getRuleContext<EscriptParser::UnambiguousStructInitializerContext>(0);
}


size_t EscriptParser::UnambiguousExplicitStructInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousExplicitStructInitializer;
}

void EscriptParser::UnambiguousExplicitStructInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousExplicitStructInitializer(this);
}

void EscriptParser::UnambiguousExplicitStructInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousExplicitStructInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousExplicitStructInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousExplicitStructInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousExplicitStructInitializerContext* EscriptParser::unambiguousExplicitStructInitializer() {
  UnambiguousExplicitStructInitializerContext *_localctx = _tracker.createInstance<UnambiguousExplicitStructInitializerContext>(_ctx, getState());
  enterRule(_localctx, 250, EscriptParser::RuleUnambiguousExplicitStructInitializer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1206);
    match(EscriptParser::STRUCT);
    setState(1208);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 106, _ctx)) {
    case 1: {
      setState(1207);
      unambiguousStructInitializer();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousExplicitDictInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousExplicitDictInitializerContext::UnambiguousExplicitDictInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousExplicitDictInitializerContext::DICTIONARY() {
  return getToken(EscriptParser::DICTIONARY, 0);
}

EscriptParser::UnambiguousDictInitializerContext* EscriptParser::UnambiguousExplicitDictInitializerContext::unambiguousDictInitializer() {
  return getRuleContext<EscriptParser::UnambiguousDictInitializerContext>(0);
}


size_t EscriptParser::UnambiguousExplicitDictInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousExplicitDictInitializer;
}

void EscriptParser::UnambiguousExplicitDictInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousExplicitDictInitializer(this);
}

void EscriptParser::UnambiguousExplicitDictInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousExplicitDictInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousExplicitDictInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousExplicitDictInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousExplicitDictInitializerContext* EscriptParser::unambiguousExplicitDictInitializer() {
  UnambiguousExplicitDictInitializerContext *_localctx = _tracker.createInstance<UnambiguousExplicitDictInitializerContext>(_ctx, getState());
  enterRule(_localctx, 252, EscriptParser::RuleUnambiguousExplicitDictInitializer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1210);
    match(EscriptParser::DICTIONARY);
    setState(1212);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 107, _ctx)) {
    case 1: {
      setState(1211);
      unambiguousDictInitializer();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousExplicitErrorInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousExplicitErrorInitializerContext::UnambiguousExplicitErrorInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousExplicitErrorInitializerContext::TOK_ERROR() {
  return getToken(EscriptParser::TOK_ERROR, 0);
}

EscriptParser::UnambiguousStructInitializerContext* EscriptParser::UnambiguousExplicitErrorInitializerContext::unambiguousStructInitializer() {
  return getRuleContext<EscriptParser::UnambiguousStructInitializerContext>(0);
}


size_t EscriptParser::UnambiguousExplicitErrorInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousExplicitErrorInitializer;
}

void EscriptParser::UnambiguousExplicitErrorInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousExplicitErrorInitializer(this);
}

void EscriptParser::UnambiguousExplicitErrorInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousExplicitErrorInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousExplicitErrorInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousExplicitErrorInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousExplicitErrorInitializerContext* EscriptParser::unambiguousExplicitErrorInitializer() {
  UnambiguousExplicitErrorInitializerContext *_localctx = _tracker.createInstance<UnambiguousExplicitErrorInitializerContext>(_ctx, getState());
  enterRule(_localctx, 254, EscriptParser::RuleUnambiguousExplicitErrorInitializer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1214);
    match(EscriptParser::TOK_ERROR);
    setState(1216);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 108, _ctx)) {
    case 1: {
      setState(1215);
      unambiguousStructInitializer();
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousBareArrayInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousBareArrayInitializerContext::UnambiguousBareArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousBareArrayInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousBareArrayInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::UnambiguousExpressionListContext* EscriptParser::UnambiguousBareArrayInitializerContext::unambiguousExpressionList() {
  return getRuleContext<EscriptParser::UnambiguousExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousBareArrayInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::UnambiguousBareArrayInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousBareArrayInitializer;
}

void EscriptParser::UnambiguousBareArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousBareArrayInitializer(this);
}

void EscriptParser::UnambiguousBareArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousBareArrayInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousBareArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousBareArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousBareArrayInitializerContext* EscriptParser::unambiguousBareArrayInitializer() {
  UnambiguousBareArrayInitializerContext *_localctx = _tracker.createInstance<UnambiguousBareArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 256, EscriptParser::RuleUnambiguousBareArrayInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1230);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 111, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1218);
      match(EscriptParser::LBRACE);
      setState(1220);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1219);
        unambiguousExpressionList();
      }
      setState(1222);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1223);
      match(EscriptParser::LBRACE);
      setState(1225);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1224);
        unambiguousExpressionList();
      }
      setState(1227);
      match(EscriptParser::COMMA);
      setState(1228);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ParenthesizedExpressionContext ------------------------------------------------------------------

EscriptParser::ParenthesizedExpressionContext::ParenthesizedExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ParenthesizedExpressionContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::ParenthesizedExpressionContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::ParenthesizedExpressionContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}


size_t EscriptParser::ParenthesizedExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleParenthesizedExpression;
}

void EscriptParser::ParenthesizedExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterParenthesizedExpression(this);
}

void EscriptParser::ParenthesizedExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitParenthesizedExpression(this);
}


antlrcpp::Any EscriptParser::ParenthesizedExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitParenthesizedExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ParenthesizedExpressionContext* EscriptParser::parenthesizedExpression() {
  ParenthesizedExpressionContext *_localctx = _tracker.createInstance<ParenthesizedExpressionContext>(_ctx, getState());
  enterRule(_localctx, 258, EscriptParser::RuleParenthesizedExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1232);
    match(EscriptParser::LPAREN);
    setState(1233);
    unambiguousExpression();
    setState(1234);
    match(EscriptParser::RPAREN);
   
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

EscriptParser::MethodCallContext* EscriptParser::ExpressionContext::methodCall() {
  return getRuleContext<EscriptParser::MethodCallContext>(0);
}

EscriptParser::ScopedMethodCallContext* EscriptParser::ExpressionContext::scopedMethodCall() {
  return getRuleContext<EscriptParser::ScopedMethodCallContext>(0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::ARRAY() {
  return getToken(EscriptParser::ARRAY, 0);
}

EscriptParser::ArrayInitializerContext* EscriptParser::ExpressionContext::arrayInitializer() {
  return getRuleContext<EscriptParser::ArrayInitializerContext>(0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::STRUCT() {
  return getToken(EscriptParser::STRUCT, 0);
}

EscriptParser::StructInitializerContext* EscriptParser::ExpressionContext::structInitializer() {
  return getRuleContext<EscriptParser::StructInitializerContext>(0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::DICTIONARY() {
  return getToken(EscriptParser::DICTIONARY, 0);
}

EscriptParser::DictInitializerContext* EscriptParser::ExpressionContext::dictInitializer() {
  return getRuleContext<EscriptParser::DictInitializerContext>(0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::TOK_ERROR() {
  return getToken(EscriptParser::TOK_ERROR, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::ExpressionContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}

EscriptParser::FunctionReferenceContext* EscriptParser::ExpressionContext::functionReference() {
  return getRuleContext<EscriptParser::FunctionReferenceContext>(0);
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

tree::TerminalNode* EscriptParser::ExpressionContext::ELVIS() {
  return getToken(EscriptParser::ELVIS, 0);
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

tree::TerminalNode* EscriptParser::ExpressionContext::EQUAL() {
  return getToken(EscriptParser::EQUAL, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::EQUAL_DEPRECATED() {
  return getToken(EscriptParser::EQUAL_DEPRECATED, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::NOTEQUAL_B() {
  return getToken(EscriptParser::NOTEQUAL_B, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::NOTEQUAL_A() {
  return getToken(EscriptParser::NOTEQUAL_A, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::BITAND() {
  return getToken(EscriptParser::BITAND, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::CARET() {
  return getToken(EscriptParser::CARET, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::BITOR() {
  return getToken(EscriptParser::BITOR, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::TOK_IN() {
  return getToken(EscriptParser::TOK_IN, 0);
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

tree::TerminalNode* EscriptParser::ExpressionContext::DOT() {
  return getToken(EscriptParser::DOT, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}

EscriptParser::MemberCallContext* EscriptParser::ExpressionContext::memberCall() {
  return getRuleContext<EscriptParser::MemberCallContext>(0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::LBRACK() {
  return getToken(EscriptParser::LBRACK, 0);
}

tree::TerminalNode* EscriptParser::ExpressionContext::RBRACK() {
  return getToken(EscriptParser::RBRACK, 0);
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


antlrcpp::Any EscriptParser::ExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
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
  size_t startState = 260;
  enterRecursionRule(_localctx, 260, EscriptParser::RuleExpression, precedence);

    size_t _la = 0;

  auto onExit = finally([=] {
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1266);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 117, _ctx)) {
    case 1: {
      setState(1237);
      primary();
      break;
    }

    case 2: {
      setState(1238);
      methodCall();
      break;
    }

    case 3: {
      setState(1239);
      scopedMethodCall();
      break;
    }

    case 4: {
      setState(1240);
      match(EscriptParser::ARRAY);
      setState(1242);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 112, _ctx)) {
      case 1: {
        setState(1241);
        arrayInitializer();
        break;
      }

      }
      break;
    }

    case 5: {
      setState(1244);
      match(EscriptParser::STRUCT);
      setState(1246);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 113, _ctx)) {
      case 1: {
        setState(1245);
        structInitializer();
        break;
      }

      }
      break;
    }

    case 6: {
      setState(1248);
      match(EscriptParser::DICTIONARY);
      setState(1250);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 114, _ctx)) {
      case 1: {
        setState(1249);
        dictInitializer();
        break;
      }

      }
      break;
    }

    case 7: {
      setState(1252);
      match(EscriptParser::TOK_ERROR);
      setState(1254);
      _errHandler->sync(this);

      switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 115, _ctx)) {
      case 1: {
        setState(1253);
        structInitializer();
        break;
      }

      }
      break;
    }

    case 8: {
      setState(1256);
      match(EscriptParser::LBRACE);
      setState(1258);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1257);
        expressionList();
      }
      setState(1260);
      match(EscriptParser::RBRACE);
      break;
    }

    case 9: {
      setState(1261);
      functionReference();
      break;
    }

    case 10: {
      setState(1262);
      dynamic_cast<ExpressionContext *>(_localctx)->prefix = _input->LT(1);
      _la = _input->LA(1);
      if (!(((((_la - 88) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 88)) & ((1ULL << (EscriptParser::ADD - 88))
        | (1ULL << (EscriptParser::SUB - 88))
        | (1ULL << (EscriptParser::INC - 88))
        | (1ULL << (EscriptParser::DEC - 88)))) != 0))) {
        dynamic_cast<ExpressionContext *>(_localctx)->prefix = _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(1263);
      expression(16);
      break;
    }

    case 11: {
      setState(1264);
      dynamic_cast<ExpressionContext *>(_localctx)->prefix = _input->LT(1);
      _la = _input->LA(1);
      if (!(((((_la - 57) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 57)) & ((1ULL << (EscriptParser::BANG_A - 57))
        | (1ULL << (EscriptParser::BANG_B - 57))
        | (1ULL << (EscriptParser::TILDE - 57)))) != 0))) {
        dynamic_cast<ExpressionContext *>(_localctx)->prefix = _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(1265);
      expression(15);
      break;
    }

    }
    _ctx->stop = _input->LT(-1);
    setState(1326);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 120, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(1324);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 119, _ctx)) {
        case 1: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1268);

          if (!(precpred(_ctx, 14))) throw FailedPredicateException(this, "precpred(_ctx, 14)");
          setState(1269);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 85) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 85)) & ((1ULL << (EscriptParser::MUL - 85))
            | (1ULL << (EscriptParser::DIV - 85))
            | (1ULL << (EscriptParser::MOD - 85)))) != 0))) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1270);
          expression(15);
          break;
        }

        case 2: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1271);

          if (!(precpred(_ctx, 13))) throw FailedPredicateException(this, "precpred(_ctx, 13)");
          setState(1272);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::ADD

          || _la == EscriptParser::SUB)) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1273);
          expression(14);
          break;
        }

        case 3: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1274);

          if (!(precpred(_ctx, 12))) throw FailedPredicateException(this, "precpred(_ctx, 12)");
          setState(1275);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::RSHIFT

          || _la == EscriptParser::LSHIFT)) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1276);
          expression(13);
          break;
        }

        case 4: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1277);

          if (!(precpred(_ctx, 11))) throw FailedPredicateException(this, "precpred(_ctx, 11)");
          setState(1278);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::ELVIS);
          setState(1279);
          expression(12);
          break;
        }

        case 5: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1280);

          if (!(precpred(_ctx, 10))) throw FailedPredicateException(this, "precpred(_ctx, 10)");
          setState(1281);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 95) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 95)) & ((1ULL << (EscriptParser::LE - 95))
            | (1ULL << (EscriptParser::LT - 95))
            | (1ULL << (EscriptParser::GE - 95))
            | (1ULL << (EscriptParser::GT - 95)))) != 0))) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1282);
          expression(11);
          break;
        }

        case 6: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1283);

          if (!(precpred(_ctx, 9))) throw FailedPredicateException(this, "precpred(_ctx, 9)");
          setState(1284);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 104) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 104)) & ((1ULL << (EscriptParser::NOTEQUAL_A - 104))
            | (1ULL << (EscriptParser::NOTEQUAL_B - 104))
            | (1ULL << (EscriptParser::EQUAL_DEPRECATED - 104))
            | (1ULL << (EscriptParser::EQUAL - 104)))) != 0))) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1285);
          expression(10);
          break;
        }

        case 7: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1286);

          if (!(precpred(_ctx, 8))) throw FailedPredicateException(this, "precpred(_ctx, 8)");
          setState(1287);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::BITAND);
          setState(1288);
          expression(9);
          break;
        }

        case 8: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1289);

          if (!(precpred(_ctx, 7))) throw FailedPredicateException(this, "precpred(_ctx, 7)");
          setState(1290);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::CARET);
          setState(1291);
          expression(8);
          break;
        }

        case 9: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1292);

          if (!(precpred(_ctx, 6))) throw FailedPredicateException(this, "precpred(_ctx, 6)");
          setState(1293);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::BITOR);
          setState(1294);
          expression(7);
          break;
        }

        case 10: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1295);

          if (!(precpred(_ctx, 5))) throw FailedPredicateException(this, "precpred(_ctx, 5)");
          setState(1296);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::TOK_IN);
          setState(1297);
          expression(6);
          break;
        }

        case 11: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1298);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(1299);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::AND_A

          || _la == EscriptParser::AND_B)) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1300);
          expression(5);
          break;
        }

        case 12: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1301);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(1302);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::OR_A

          || _la == EscriptParser::OR_B)) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1303);
          expression(4);
          break;
        }

        case 13: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1304);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(1305);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 109) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 109)) & ((1ULL << (EscriptParser::ADDMEMBER - 109))
            | (1ULL << (EscriptParser::DELMEMBER - 109))
            | (1ULL << (EscriptParser::CHKMEMBER - 109)))) != 0))) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1306);
          expression(2);
          break;
        }

        case 14: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1307);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(1308);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = _input->LT(1);
          _la = _input->LA(1);
          if (!(((((_la - 90) & ~ 0x3fULL) == 0) &&
            ((1ULL << (_la - 90)) & ((1ULL << (EscriptParser::ADD_ASSIGN - 90))
            | (1ULL << (EscriptParser::SUB_ASSIGN - 90))
            | (1ULL << (EscriptParser::MUL_ASSIGN - 90))
            | (1ULL << (EscriptParser::DIV_ASSIGN - 90))
            | (1ULL << (EscriptParser::MOD_ASSIGN - 90))
            | (1ULL << (EscriptParser::ASSIGN - 90)))) != 0))) {
            dynamic_cast<ExpressionContext *>(_localctx)->bop = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          setState(1309);
          expression(1);
          break;
        }

        case 15: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1310);

          if (!(precpred(_ctx, 27))) throw FailedPredicateException(this, "precpred(_ctx, 27)");
          setState(1311);
          dynamic_cast<ExpressionContext *>(_localctx)->bop = match(EscriptParser::DOT);
          setState(1315);
          _errHandler->sync(this);
          switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 118, _ctx)) {
          case 1: {
            setState(1312);
            match(EscriptParser::IDENTIFIER);
            break;
          }

          case 2: {
            setState(1313);
            match(EscriptParser::STRING_LITERAL);
            break;
          }

          case 3: {
            setState(1314);
            memberCall();
            break;
          }

          }
          break;
        }

        case 16: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1317);

          if (!(precpred(_ctx, 26))) throw FailedPredicateException(this, "precpred(_ctx, 26)");
          setState(1318);
          match(EscriptParser::LBRACK);
          setState(1319);
          expressionList();
          setState(1320);
          match(EscriptParser::RBRACK);
          break;
        }

        case 17: {
          _localctx = _tracker.createInstance<ExpressionContext>(parentContext, parentState);
          pushNewRecursionContext(_localctx, startState, RuleExpression);
          setState(1322);

          if (!(precpred(_ctx, 17))) throw FailedPredicateException(this, "precpred(_ctx, 17)");
          setState(1323);
          dynamic_cast<ExpressionContext *>(_localctx)->postfix = _input->LT(1);
          _la = _input->LA(1);
          if (!(_la == EscriptParser::INC

          || _la == EscriptParser::DEC)) {
            dynamic_cast<ExpressionContext *>(_localctx)->postfix = _errHandler->recoverInline(this);
          }
          else {
            _errHandler->reportMatch(this);
            consume();
          }
          break;
        }

        } 
      }
      setState(1328);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 120, _ctx);
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

tree::TerminalNode* EscriptParser::PrimaryContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

EscriptParser::ExpressionContext* EscriptParser::PrimaryContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::PrimaryContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::LiteralContext* EscriptParser::PrimaryContext::literal() {
  return getRuleContext<EscriptParser::LiteralContext>(0);
}

tree::TerminalNode* EscriptParser::PrimaryContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
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


antlrcpp::Any EscriptParser::PrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitPrimary(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::PrimaryContext* EscriptParser::primary() {
  PrimaryContext *_localctx = _tracker.createInstance<PrimaryContext>(_ctx, getState());
  enterRule(_localctx, 262, EscriptParser::RulePrimary);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1335);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::LPAREN: {
        enterOuterAlt(_localctx, 1);
        setState(1329);
        match(EscriptParser::LPAREN);
        setState(1330);
        expression(0);
        setState(1331);
        match(EscriptParser::RPAREN);
        break;
      }

      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL:
      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL:
      case EscriptParser::CHAR_LITERAL:
      case EscriptParser::STRING_LITERAL:
      case EscriptParser::NULL_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(1333);
        literal();
        break;
      }

      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 3);
        setState(1334);
        match(EscriptParser::IDENTIFIER);
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


antlrcpp::Any EscriptParser::ParExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitParExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ParExpressionContext* EscriptParser::parExpression() {
  ParExpressionContext *_localctx = _tracker.createInstance<ParExpressionContext>(_ctx, getState());
  enterRule(_localctx, 264, EscriptParser::RuleParExpression);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1337);
    match(EscriptParser::LPAREN);
    setState(1338);
    expression(0);
    setState(1339);
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

std::vector<EscriptParser::ExpressionContext *> EscriptParser::ExpressionListContext::expression() {
  return getRuleContexts<EscriptParser::ExpressionContext>();
}

EscriptParser::ExpressionContext* EscriptParser::ExpressionListContext::expression(size_t i) {
  return getRuleContext<EscriptParser::ExpressionContext>(i);
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


antlrcpp::Any EscriptParser::ExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ExpressionListContext* EscriptParser::expressionList() {
  ExpressionListContext *_localctx = _tracker.createInstance<ExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 266, EscriptParser::RuleExpressionList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1341);
    expression(0);
    setState(1346);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(1342);
      match(EscriptParser::COMMA);
      setState(1343);
      expression(0);
      setState(1348);
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

//----------------- UnambiguousExpressionListContext ------------------------------------------------------------------

EscriptParser::UnambiguousExpressionListContext::UnambiguousExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousExpressionContext *> EscriptParser::UnambiguousExpressionListContext::unambiguousExpression() {
  return getRuleContexts<EscriptParser::UnambiguousExpressionContext>();
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousExpressionListContext::unambiguousExpression(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousExpressionListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousExpressionListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousExpressionListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousExpressionList;
}

void EscriptParser::UnambiguousExpressionListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousExpressionList(this);
}

void EscriptParser::UnambiguousExpressionListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousExpressionList(this);
}


antlrcpp::Any EscriptParser::UnambiguousExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousExpressionListContext* EscriptParser::unambiguousExpressionList() {
  UnambiguousExpressionListContext *_localctx = _tracker.createInstance<UnambiguousExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 268, EscriptParser::RuleUnambiguousExpressionList);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1349);
    unambiguousExpression();
    setState(1354);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 123, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1350);
        match(EscriptParser::COMMA);
        setState(1351);
        unambiguousExpression(); 
      }
      setState(1356);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 123, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MethodCallArgumentContext ------------------------------------------------------------------

EscriptParser::MethodCallArgumentContext::MethodCallArgumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::ExpressionContext* EscriptParser::MethodCallArgumentContext::expression() {
  return getRuleContext<EscriptParser::ExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::MethodCallArgumentContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

tree::TerminalNode* EscriptParser::MethodCallArgumentContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}


size_t EscriptParser::MethodCallArgumentContext::getRuleIndex() const {
  return EscriptParser::RuleMethodCallArgument;
}

void EscriptParser::MethodCallArgumentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMethodCallArgument(this);
}

void EscriptParser::MethodCallArgumentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMethodCallArgument(this);
}


antlrcpp::Any EscriptParser::MethodCallArgumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMethodCallArgument(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MethodCallArgumentContext* EscriptParser::methodCallArgument() {
  MethodCallArgumentContext *_localctx = _tracker.createInstance<MethodCallArgumentContext>(_ctx, getState());
  enterRule(_localctx, 270, EscriptParser::RuleMethodCallArgument);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1359);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 124, _ctx)) {
    case 1: {
      setState(1357);
      dynamic_cast<MethodCallArgumentContext *>(_localctx)->parameter = match(EscriptParser::IDENTIFIER);
      setState(1358);
      match(EscriptParser::ASSIGN);
      break;
    }

    }
    setState(1361);
    expression(0);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MethodCallArgumentListContext ------------------------------------------------------------------

EscriptParser::MethodCallArgumentListContext::MethodCallArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::MethodCallArgumentContext *> EscriptParser::MethodCallArgumentListContext::methodCallArgument() {
  return getRuleContexts<EscriptParser::MethodCallArgumentContext>();
}

EscriptParser::MethodCallArgumentContext* EscriptParser::MethodCallArgumentListContext::methodCallArgument(size_t i) {
  return getRuleContext<EscriptParser::MethodCallArgumentContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::MethodCallArgumentListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::MethodCallArgumentListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::MethodCallArgumentListContext::getRuleIndex() const {
  return EscriptParser::RuleMethodCallArgumentList;
}

void EscriptParser::MethodCallArgumentListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMethodCallArgumentList(this);
}

void EscriptParser::MethodCallArgumentListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMethodCallArgumentList(this);
}


antlrcpp::Any EscriptParser::MethodCallArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMethodCallArgumentList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MethodCallArgumentListContext* EscriptParser::methodCallArgumentList() {
  MethodCallArgumentListContext *_localctx = _tracker.createInstance<MethodCallArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 272, EscriptParser::RuleMethodCallArgumentList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1363);
    methodCallArgument();
    setState(1368);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(1364);
      match(EscriptParser::COMMA);
      setState(1365);
      methodCallArgument();
      setState(1370);
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

//----------------- MethodCallContext ------------------------------------------------------------------

EscriptParser::MethodCallContext::MethodCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::MethodCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::MethodCallContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::MethodCallContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::MethodCallArgumentListContext* EscriptParser::MethodCallContext::methodCallArgumentList() {
  return getRuleContext<EscriptParser::MethodCallArgumentListContext>(0);
}


size_t EscriptParser::MethodCallContext::getRuleIndex() const {
  return EscriptParser::RuleMethodCall;
}

void EscriptParser::MethodCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMethodCall(this);
}

void EscriptParser::MethodCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMethodCall(this);
}


antlrcpp::Any EscriptParser::MethodCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMethodCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MethodCallContext* EscriptParser::methodCall() {
  MethodCallContext *_localctx = _tracker.createInstance<MethodCallContext>(_ctx, getState());
  enterRule(_localctx, 274, EscriptParser::RuleMethodCall);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1371);
    match(EscriptParser::IDENTIFIER);
    setState(1372);
    match(EscriptParser::LPAREN);
    setState(1374);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(1373);
      methodCallArgumentList();
    }
    setState(1376);
    match(EscriptParser::RPAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueArgumentListContext ------------------------------------------------------------------

EscriptParser::ValueArgumentListContext::ValueArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousFunctionCallArgumentContext *> EscriptParser::ValueArgumentListContext::unambiguousFunctionCallArgument() {
  return getRuleContexts<EscriptParser::UnambiguousFunctionCallArgumentContext>();
}

EscriptParser::UnambiguousFunctionCallArgumentContext* EscriptParser::ValueArgumentListContext::unambiguousFunctionCallArgument(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousFunctionCallArgumentContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::ValueArgumentListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::ValueArgumentListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::ValueArgumentListContext::getRuleIndex() const {
  return EscriptParser::RuleValueArgumentList;
}

void EscriptParser::ValueArgumentListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValueArgumentList(this);
}

void EscriptParser::ValueArgumentListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValueArgumentList(this);
}


antlrcpp::Any EscriptParser::ValueArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitValueArgumentList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ValueArgumentListContext* EscriptParser::valueArgumentList() {
  ValueArgumentListContext *_localctx = _tracker.createInstance<ValueArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 276, EscriptParser::RuleValueArgumentList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1378);
    unambiguousFunctionCallArgument();
    setState(1383);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(1379);
      match(EscriptParser::COMMA);
      setState(1380);
      unambiguousFunctionCallArgument();
      setState(1385);
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

//----------------- ValueArgumentsContext ------------------------------------------------------------------

EscriptParser::ValueArgumentsContext::ValueArgumentsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::ValueArgumentsContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

EscriptParser::UnambiguousFunctionCallArgumentListContext* EscriptParser::ValueArgumentsContext::unambiguousFunctionCallArgumentList() {
  return getRuleContext<EscriptParser::UnambiguousFunctionCallArgumentListContext>(0);
}

tree::TerminalNode* EscriptParser::ValueArgumentsContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}


size_t EscriptParser::ValueArgumentsContext::getRuleIndex() const {
  return EscriptParser::RuleValueArguments;
}

void EscriptParser::ValueArgumentsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterValueArguments(this);
}

void EscriptParser::ValueArgumentsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitValueArguments(this);
}


antlrcpp::Any EscriptParser::ValueArgumentsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitValueArguments(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ValueArgumentsContext* EscriptParser::valueArguments() {
  ValueArgumentsContext *_localctx = _tracker.createInstance<ValueArgumentsContext>(_ctx, getState());
  enterRule(_localctx, 278, EscriptParser::RuleValueArguments);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1392);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 128, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1386);
      match(EscriptParser::LPAREN);
      setState(1387);
      unambiguousFunctionCallArgumentList();
      setState(1388);
      match(EscriptParser::RPAREN);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1390);
      match(EscriptParser::LPAREN);
      setState(1391);
      match(EscriptParser::RPAREN);
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousFunctionCallArgumentContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionCallArgumentContext::UnambiguousFunctionCallArgumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousFunctionCallArgumentContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}


size_t EscriptParser::UnambiguousFunctionCallArgumentContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionCallArgument;
}

void EscriptParser::UnambiguousFunctionCallArgumentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionCallArgument(this);
}

void EscriptParser::UnambiguousFunctionCallArgumentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionCallArgument(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionCallArgumentContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionCallArgument(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionCallArgumentContext* EscriptParser::unambiguousFunctionCallArgument() {
  UnambiguousFunctionCallArgumentContext *_localctx = _tracker.createInstance<UnambiguousFunctionCallArgumentContext>(_ctx, getState());
  enterRule(_localctx, 280, EscriptParser::RuleUnambiguousFunctionCallArgument);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1394);
    unambiguousExpression();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousFunctionCallContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionCallContext::UnambiguousFunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

EscriptParser::ValueArgumentsContext* EscriptParser::UnambiguousFunctionCallContext::valueArguments() {
  return getRuleContext<EscriptParser::ValueArgumentsContext>(0);
}


size_t EscriptParser::UnambiguousFunctionCallContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionCall;
}

void EscriptParser::UnambiguousFunctionCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionCall(this);
}

void EscriptParser::UnambiguousFunctionCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionCall(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionCallContext* EscriptParser::unambiguousFunctionCall() {
  UnambiguousFunctionCallContext *_localctx = _tracker.createInstance<UnambiguousFunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 282, EscriptParser::RuleUnambiguousFunctionCall);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1396);
    match(EscriptParser::IDENTIFIER);
    setState(1397);
    valueArguments();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousScopedFunctionCallContext ------------------------------------------------------------------

EscriptParser::UnambiguousScopedFunctionCallContext::UnambiguousScopedFunctionCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousScopedFunctionCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousScopedFunctionCallContext::COLONCOLON() {
  return getToken(EscriptParser::COLONCOLON, 0);
}

EscriptParser::UnambiguousFunctionCallContext* EscriptParser::UnambiguousScopedFunctionCallContext::unambiguousFunctionCall() {
  return getRuleContext<EscriptParser::UnambiguousFunctionCallContext>(0);
}


size_t EscriptParser::UnambiguousScopedFunctionCallContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousScopedFunctionCall;
}

void EscriptParser::UnambiguousScopedFunctionCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousScopedFunctionCall(this);
}

void EscriptParser::UnambiguousScopedFunctionCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousScopedFunctionCall(this);
}


antlrcpp::Any EscriptParser::UnambiguousScopedFunctionCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousScopedFunctionCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousScopedFunctionCallContext* EscriptParser::unambiguousScopedFunctionCall() {
  UnambiguousScopedFunctionCallContext *_localctx = _tracker.createInstance<UnambiguousScopedFunctionCallContext>(_ctx, getState());
  enterRule(_localctx, 284, EscriptParser::RuleUnambiguousScopedFunctionCall);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1399);
    match(EscriptParser::IDENTIFIER);
    setState(1400);
    match(EscriptParser::COLONCOLON);
    setState(1401);
    unambiguousFunctionCall();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousFunctionCallArgumentListContext ------------------------------------------------------------------

EscriptParser::UnambiguousFunctionCallArgumentListContext::UnambiguousFunctionCallArgumentListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousFunctionCallArgumentContext *> EscriptParser::UnambiguousFunctionCallArgumentListContext::unambiguousFunctionCallArgument() {
  return getRuleContexts<EscriptParser::UnambiguousFunctionCallArgumentContext>();
}

EscriptParser::UnambiguousFunctionCallArgumentContext* EscriptParser::UnambiguousFunctionCallArgumentListContext::unambiguousFunctionCallArgument(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousFunctionCallArgumentContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousFunctionCallArgumentListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousFunctionCallArgumentListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousFunctionCallArgumentListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousFunctionCallArgumentList;
}

void EscriptParser::UnambiguousFunctionCallArgumentListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousFunctionCallArgumentList(this);
}

void EscriptParser::UnambiguousFunctionCallArgumentListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousFunctionCallArgumentList(this);
}


antlrcpp::Any EscriptParser::UnambiguousFunctionCallArgumentListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousFunctionCallArgumentList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousFunctionCallArgumentListContext* EscriptParser::unambiguousFunctionCallArgumentList() {
  UnambiguousFunctionCallArgumentListContext *_localctx = _tracker.createInstance<UnambiguousFunctionCallArgumentListContext>(_ctx, getState());
  enterRule(_localctx, 286, EscriptParser::RuleUnambiguousFunctionCallArgumentList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1403);
    unambiguousFunctionCallArgument();
    setState(1408);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(1404);
      match(EscriptParser::COMMA);
      setState(1405);
      unambiguousFunctionCallArgument();
      setState(1410);
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

//----------------- MemberCallContext ------------------------------------------------------------------

EscriptParser::MemberCallContext::MemberCallContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::MemberCallContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::MemberCallContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::MemberCallContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

EscriptParser::ExpressionListContext* EscriptParser::MemberCallContext::expressionList() {
  return getRuleContext<EscriptParser::ExpressionListContext>(0);
}


size_t EscriptParser::MemberCallContext::getRuleIndex() const {
  return EscriptParser::RuleMemberCall;
}

void EscriptParser::MemberCallContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMemberCall(this);
}

void EscriptParser::MemberCallContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMemberCall(this);
}


antlrcpp::Any EscriptParser::MemberCallContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitMemberCall(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::MemberCallContext* EscriptParser::memberCall() {
  MemberCallContext *_localctx = _tracker.createInstance<MemberCallContext>(_ctx, getState());
  enterRule(_localctx, 288, EscriptParser::RuleMemberCall);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1411);
    match(EscriptParser::IDENTIFIER);
    setState(1412);
    match(EscriptParser::LPAREN);
    setState(1414);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
      | (1ULL << EscriptParser::BANG_B)
      | (1ULL << EscriptParser::TOK_ERROR)
      | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
      | (1ULL << (EscriptParser::ARRAY - 64))
      | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_LITERAL - 64))
      | (1ULL << (EscriptParser::OCT_LITERAL - 64))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
      | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
      | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
      | (1ULL << (EscriptParser::STRING_LITERAL - 64))
      | (1ULL << (EscriptParser::NULL_LITERAL - 64))
      | (1ULL << (EscriptParser::LPAREN - 64))
      | (1ULL << (EscriptParser::LBRACE - 64))
      | (1ULL << (EscriptParser::ADD - 64))
      | (1ULL << (EscriptParser::SUB - 64))
      | (1ULL << (EscriptParser::TILDE - 64))
      | (1ULL << (EscriptParser::AT - 64))
      | (1ULL << (EscriptParser::INC - 64))
      | (1ULL << (EscriptParser::DEC - 64))
      | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
      setState(1413);
      expressionList();
    }
    setState(1416);
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


antlrcpp::Any EscriptParser::StructInitializerExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStructInitializerExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StructInitializerExpressionContext* EscriptParser::structInitializerExpression() {
  StructInitializerExpressionContext *_localctx = _tracker.createInstance<StructInitializerExpressionContext>(_ctx, getState());
  enterRule(_localctx, 290, EscriptParser::RuleStructInitializerExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1428);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(1418);
        match(EscriptParser::IDENTIFIER);
        setState(1421);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(1419);
          match(EscriptParser::ASSIGN);
          setState(1420);
          expression(0);
        }
        break;
      }

      case EscriptParser::STRING_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(1423);
        match(EscriptParser::STRING_LITERAL);
        setState(1426);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(1424);
          match(EscriptParser::ASSIGN);
          setState(1425);
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


antlrcpp::Any EscriptParser::StructInitializerExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStructInitializerExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StructInitializerExpressionListContext* EscriptParser::structInitializerExpressionList() {
  StructInitializerExpressionListContext *_localctx = _tracker.createInstance<StructInitializerExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 292, EscriptParser::RuleStructInitializerExpressionList);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1430);
    structInitializerExpression();
    setState(1435);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == EscriptParser::COMMA) {
      setState(1431);
      match(EscriptParser::COMMA);
      setState(1432);
      structInitializerExpression();
      setState(1437);
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


antlrcpp::Any EscriptParser::StructInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitStructInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::StructInitializerContext* EscriptParser::structInitializer() {
  StructInitializerContext *_localctx = _tracker.createInstance<StructInitializerContext>(_ctx, getState());
  enterRule(_localctx, 294, EscriptParser::RuleStructInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1438);
    match(EscriptParser::LBRACE);
    setState(1440);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::STRING_LITERAL

    || _la == EscriptParser::IDENTIFIER) {
      setState(1439);
      structInitializerExpressionList();
    }
    setState(1442);
    match(EscriptParser::RBRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousStructInitializerExpressionContext ------------------------------------------------------------------

EscriptParser::UnambiguousStructInitializerExpressionContext::UnambiguousStructInitializerExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerExpressionContext::IDENTIFIER() {
  return getToken(EscriptParser::IDENTIFIER, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerExpressionContext::ASSIGN() {
  return getToken(EscriptParser::ASSIGN, 0);
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousStructInitializerExpressionContext::unambiguousExpression() {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerExpressionContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}


size_t EscriptParser::UnambiguousStructInitializerExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousStructInitializerExpression;
}

void EscriptParser::UnambiguousStructInitializerExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousStructInitializerExpression(this);
}

void EscriptParser::UnambiguousStructInitializerExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousStructInitializerExpression(this);
}


antlrcpp::Any EscriptParser::UnambiguousStructInitializerExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousStructInitializerExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousStructInitializerExpressionContext* EscriptParser::unambiguousStructInitializerExpression() {
  UnambiguousStructInitializerExpressionContext *_localctx = _tracker.createInstance<UnambiguousStructInitializerExpressionContext>(_ctx, getState());
  enterRule(_localctx, 296, EscriptParser::RuleUnambiguousStructInitializerExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1454);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::IDENTIFIER: {
        enterOuterAlt(_localctx, 1);
        setState(1444);
        match(EscriptParser::IDENTIFIER);
        setState(1447);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(1445);
          match(EscriptParser::ASSIGN);
          setState(1446);
          unambiguousExpression();
        }
        break;
      }

      case EscriptParser::STRING_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(1449);
        match(EscriptParser::STRING_LITERAL);
        setState(1452);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == EscriptParser::ASSIGN) {
          setState(1450);
          match(EscriptParser::ASSIGN);
          setState(1451);
          unambiguousExpression();
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

//----------------- UnambiguousStructInitializerExpressionListContext ------------------------------------------------------------------

EscriptParser::UnambiguousStructInitializerExpressionListContext::UnambiguousStructInitializerExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousStructInitializerExpressionContext *> EscriptParser::UnambiguousStructInitializerExpressionListContext::unambiguousStructInitializerExpression() {
  return getRuleContexts<EscriptParser::UnambiguousStructInitializerExpressionContext>();
}

EscriptParser::UnambiguousStructInitializerExpressionContext* EscriptParser::UnambiguousStructInitializerExpressionListContext::unambiguousStructInitializerExpression(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousStructInitializerExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousStructInitializerExpressionListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerExpressionListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousStructInitializerExpressionListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousStructInitializerExpressionList;
}

void EscriptParser::UnambiguousStructInitializerExpressionListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousStructInitializerExpressionList(this);
}

void EscriptParser::UnambiguousStructInitializerExpressionListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousStructInitializerExpressionList(this);
}


antlrcpp::Any EscriptParser::UnambiguousStructInitializerExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousStructInitializerExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousStructInitializerExpressionListContext* EscriptParser::unambiguousStructInitializerExpressionList() {
  UnambiguousStructInitializerExpressionListContext *_localctx = _tracker.createInstance<UnambiguousStructInitializerExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 298, EscriptParser::RuleUnambiguousStructInitializerExpressionList);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1456);
    unambiguousStructInitializerExpression();
    setState(1461);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 139, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1457);
        match(EscriptParser::COMMA);
        setState(1458);
        unambiguousStructInitializerExpression(); 
      }
      setState(1463);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 139, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousStructInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousStructInitializerContext::UnambiguousStructInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::UnambiguousStructInitializerExpressionListContext* EscriptParser::UnambiguousStructInitializerContext::unambiguousStructInitializerExpressionList() {
  return getRuleContext<EscriptParser::UnambiguousStructInitializerExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousStructInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::UnambiguousStructInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousStructInitializer;
}

void EscriptParser::UnambiguousStructInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousStructInitializer(this);
}

void EscriptParser::UnambiguousStructInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousStructInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousStructInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousStructInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousStructInitializerContext* EscriptParser::unambiguousStructInitializer() {
  UnambiguousStructInitializerContext *_localctx = _tracker.createInstance<UnambiguousStructInitializerContext>(_ctx, getState());
  enterRule(_localctx, 300, EscriptParser::RuleUnambiguousStructInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1476);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 142, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1464);
      match(EscriptParser::LBRACE);
      setState(1466);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::STRING_LITERAL

      || _la == EscriptParser::IDENTIFIER) {
        setState(1465);
        unambiguousStructInitializerExpressionList();
      }
      setState(1468);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1469);
      match(EscriptParser::LBRACE);
      setState(1471);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if (_la == EscriptParser::STRING_LITERAL

      || _la == EscriptParser::IDENTIFIER) {
        setState(1470);
        unambiguousStructInitializerExpressionList();
      }
      setState(1473);
      match(EscriptParser::COMMA);
      setState(1474);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in struct initializer list");
      break;
    }

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


antlrcpp::Any EscriptParser::DictInitializerExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDictInitializerExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DictInitializerExpressionContext* EscriptParser::dictInitializerExpression() {
  DictInitializerExpressionContext *_localctx = _tracker.createInstance<DictInitializerExpressionContext>(_ctx, getState());
  enterRule(_localctx, 302, EscriptParser::RuleDictInitializerExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1478);
    expression(0);
    setState(1481);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ARROW) {
      setState(1479);
      match(EscriptParser::ARROW);
      setState(1480);
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

//----------------- UnambiguousDictInitializerExpressionContext ------------------------------------------------------------------

EscriptParser::UnambiguousDictInitializerExpressionContext::UnambiguousDictInitializerExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousExpressionContext *> EscriptParser::UnambiguousDictInitializerExpressionContext::unambiguousExpression() {
  return getRuleContexts<EscriptParser::UnambiguousExpressionContext>();
}

EscriptParser::UnambiguousExpressionContext* EscriptParser::UnambiguousDictInitializerExpressionContext::unambiguousExpression(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousExpressionContext>(i);
}

tree::TerminalNode* EscriptParser::UnambiguousDictInitializerExpressionContext::ARROW() {
  return getToken(EscriptParser::ARROW, 0);
}


size_t EscriptParser::UnambiguousDictInitializerExpressionContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousDictInitializerExpression;
}

void EscriptParser::UnambiguousDictInitializerExpressionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousDictInitializerExpression(this);
}

void EscriptParser::UnambiguousDictInitializerExpressionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousDictInitializerExpression(this);
}


antlrcpp::Any EscriptParser::UnambiguousDictInitializerExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousDictInitializerExpression(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousDictInitializerExpressionContext* EscriptParser::unambiguousDictInitializerExpression() {
  UnambiguousDictInitializerExpressionContext *_localctx = _tracker.createInstance<UnambiguousDictInitializerExpressionContext>(_ctx, getState());
  enterRule(_localctx, 304, EscriptParser::RuleUnambiguousDictInitializerExpression);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1483);
    unambiguousExpression();
    setState(1486);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == EscriptParser::ARROW) {
      setState(1484);
      match(EscriptParser::ARROW);
      setState(1485);
      unambiguousExpression();
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


antlrcpp::Any EscriptParser::DictInitializerExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDictInitializerExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DictInitializerExpressionListContext* EscriptParser::dictInitializerExpressionList() {
  DictInitializerExpressionListContext *_localctx = _tracker.createInstance<DictInitializerExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 306, EscriptParser::RuleDictInitializerExpressionList);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1488);
    dictInitializerExpression();
    setState(1493);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 145, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1489);
        match(EscriptParser::COMMA);
        setState(1490);
        dictInitializerExpression(); 
      }
      setState(1495);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 145, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousDictInitializerExpressionListContext ------------------------------------------------------------------

EscriptParser::UnambiguousDictInitializerExpressionListContext::UnambiguousDictInitializerExpressionListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<EscriptParser::UnambiguousDictInitializerExpressionContext *> EscriptParser::UnambiguousDictInitializerExpressionListContext::unambiguousDictInitializerExpression() {
  return getRuleContexts<EscriptParser::UnambiguousDictInitializerExpressionContext>();
}

EscriptParser::UnambiguousDictInitializerExpressionContext* EscriptParser::UnambiguousDictInitializerExpressionListContext::unambiguousDictInitializerExpression(size_t i) {
  return getRuleContext<EscriptParser::UnambiguousDictInitializerExpressionContext>(i);
}

std::vector<tree::TerminalNode *> EscriptParser::UnambiguousDictInitializerExpressionListContext::COMMA() {
  return getTokens(EscriptParser::COMMA);
}

tree::TerminalNode* EscriptParser::UnambiguousDictInitializerExpressionListContext::COMMA(size_t i) {
  return getToken(EscriptParser::COMMA, i);
}


size_t EscriptParser::UnambiguousDictInitializerExpressionListContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousDictInitializerExpressionList;
}

void EscriptParser::UnambiguousDictInitializerExpressionListContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousDictInitializerExpressionList(this);
}

void EscriptParser::UnambiguousDictInitializerExpressionListContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousDictInitializerExpressionList(this);
}


antlrcpp::Any EscriptParser::UnambiguousDictInitializerExpressionListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousDictInitializerExpressionList(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousDictInitializerExpressionListContext* EscriptParser::unambiguousDictInitializerExpressionList() {
  UnambiguousDictInitializerExpressionListContext *_localctx = _tracker.createInstance<UnambiguousDictInitializerExpressionListContext>(_ctx, getState());
  enterRule(_localctx, 308, EscriptParser::RuleUnambiguousDictInitializerExpressionList);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(1496);
    unambiguousDictInitializerExpression();
    setState(1501);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 146, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(1497);
        match(EscriptParser::COMMA);
        setState(1498);
        unambiguousDictInitializerExpression(); 
      }
      setState(1503);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 146, _ctx);
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


antlrcpp::Any EscriptParser::DictInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitDictInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::DictInitializerContext* EscriptParser::dictInitializer() {
  DictInitializerContext *_localctx = _tracker.createInstance<DictInitializerContext>(_ctx, getState());
  enterRule(_localctx, 310, EscriptParser::RuleDictInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1516);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 149, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1504);
      match(EscriptParser::LBRACE);
      setState(1506);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1505);
        dictInitializerExpressionList();
      }
      setState(1508);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1509);
      match(EscriptParser::LBRACE);
      setState(1511);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1510);
        dictInitializerExpressionList();
      }
      setState(1513);
      match(EscriptParser::COMMA);
      setState(1514);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in dictionary initializer list");
      break;
    }

    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnambiguousDictInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousDictInitializerContext::UnambiguousDictInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousDictInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousDictInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::UnambiguousDictInitializerExpressionListContext* EscriptParser::UnambiguousDictInitializerContext::unambiguousDictInitializerExpressionList() {
  return getRuleContext<EscriptParser::UnambiguousDictInitializerExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousDictInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::UnambiguousDictInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousDictInitializer;
}

void EscriptParser::UnambiguousDictInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousDictInitializer(this);
}

void EscriptParser::UnambiguousDictInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousDictInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousDictInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousDictInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousDictInitializerContext* EscriptParser::unambiguousDictInitializer() {
  UnambiguousDictInitializerContext *_localctx = _tracker.createInstance<UnambiguousDictInitializerContext>(_ctx, getState());
  enterRule(_localctx, 312, EscriptParser::RuleUnambiguousDictInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1530);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 152, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1518);
      match(EscriptParser::LBRACE);
      setState(1520);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1519);
        unambiguousDictInitializerExpressionList();
      }
      setState(1522);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1523);
      match(EscriptParser::LBRACE);
      setState(1525);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1524);
        unambiguousDictInitializerExpressionList();
      }
      setState(1527);
      match(EscriptParser::COMMA);
      setState(1528);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in dictionary initializer list");
      break;
    }

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


antlrcpp::Any EscriptParser::ArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::ArrayInitializerContext* EscriptParser::arrayInitializer() {
  ArrayInitializerContext *_localctx = _tracker.createInstance<ArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 314, EscriptParser::RuleArrayInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1542);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::LBRACE: {
        enterOuterAlt(_localctx, 1);
        setState(1532);
        match(EscriptParser::LBRACE);
        setState(1534);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
          | (1ULL << EscriptParser::BANG_B)
          | (1ULL << EscriptParser::TOK_ERROR)
          | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
          | (1ULL << (EscriptParser::ARRAY - 64))
          | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
          | (1ULL << (EscriptParser::HEX_LITERAL - 64))
          | (1ULL << (EscriptParser::OCT_LITERAL - 64))
          | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
          | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
          | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
          | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
          | (1ULL << (EscriptParser::STRING_LITERAL - 64))
          | (1ULL << (EscriptParser::NULL_LITERAL - 64))
          | (1ULL << (EscriptParser::LPAREN - 64))
          | (1ULL << (EscriptParser::LBRACE - 64))
          | (1ULL << (EscriptParser::ADD - 64))
          | (1ULL << (EscriptParser::SUB - 64))
          | (1ULL << (EscriptParser::TILDE - 64))
          | (1ULL << (EscriptParser::AT - 64))
          | (1ULL << (EscriptParser::INC - 64))
          | (1ULL << (EscriptParser::DEC - 64))
          | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
          setState(1533);
          expressionList();
        }
        setState(1536);
        match(EscriptParser::RBRACE);
        break;
      }

      case EscriptParser::LPAREN: {
        enterOuterAlt(_localctx, 2);
        setState(1537);
        match(EscriptParser::LPAREN);
        setState(1539);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if ((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
          | (1ULL << EscriptParser::BANG_B)
          | (1ULL << EscriptParser::TOK_ERROR)
          | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
          ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
          | (1ULL << (EscriptParser::ARRAY - 64))
          | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
          | (1ULL << (EscriptParser::HEX_LITERAL - 64))
          | (1ULL << (EscriptParser::OCT_LITERAL - 64))
          | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
          | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
          | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
          | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
          | (1ULL << (EscriptParser::STRING_LITERAL - 64))
          | (1ULL << (EscriptParser::NULL_LITERAL - 64))
          | (1ULL << (EscriptParser::LPAREN - 64))
          | (1ULL << (EscriptParser::LBRACE - 64))
          | (1ULL << (EscriptParser::ADD - 64))
          | (1ULL << (EscriptParser::SUB - 64))
          | (1ULL << (EscriptParser::TILDE - 64))
          | (1ULL << (EscriptParser::AT - 64))
          | (1ULL << (EscriptParser::INC - 64))
          | (1ULL << (EscriptParser::DEC - 64))
          | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
          setState(1538);
          expressionList();
        }
        setState(1541);
        match(EscriptParser::RPAREN);
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

//----------------- UnambiguousArrayInitializerContext ------------------------------------------------------------------

EscriptParser::UnambiguousArrayInitializerContext::UnambiguousArrayInitializerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* EscriptParser::UnambiguousArrayInitializerContext::LBRACE() {
  return getToken(EscriptParser::LBRACE, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousArrayInitializerContext::RBRACE() {
  return getToken(EscriptParser::RBRACE, 0);
}

EscriptParser::UnambiguousExpressionListContext* EscriptParser::UnambiguousArrayInitializerContext::unambiguousExpressionList() {
  return getRuleContext<EscriptParser::UnambiguousExpressionListContext>(0);
}

tree::TerminalNode* EscriptParser::UnambiguousArrayInitializerContext::LPAREN() {
  return getToken(EscriptParser::LPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousArrayInitializerContext::RPAREN() {
  return getToken(EscriptParser::RPAREN, 0);
}

tree::TerminalNode* EscriptParser::UnambiguousArrayInitializerContext::COMMA() {
  return getToken(EscriptParser::COMMA, 0);
}


size_t EscriptParser::UnambiguousArrayInitializerContext::getRuleIndex() const {
  return EscriptParser::RuleUnambiguousArrayInitializer;
}

void EscriptParser::UnambiguousArrayInitializerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnambiguousArrayInitializer(this);
}

void EscriptParser::UnambiguousArrayInitializerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<EscriptParserListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnambiguousArrayInitializer(this);
}


antlrcpp::Any EscriptParser::UnambiguousArrayInitializerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitUnambiguousArrayInitializer(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::UnambiguousArrayInitializerContext* EscriptParser::unambiguousArrayInitializer() {
  UnambiguousArrayInitializerContext *_localctx = _tracker.createInstance<UnambiguousArrayInitializerContext>(_ctx, getState());
  enterRule(_localctx, 316, EscriptParser::RuleUnambiguousArrayInitializer);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1568);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 160, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(1544);
      match(EscriptParser::LBRACE);
      setState(1546);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1545);
        unambiguousExpressionList();
      }
      setState(1548);
      match(EscriptParser::RBRACE);
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(1549);
      match(EscriptParser::LPAREN);
      setState(1551);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1550);
        unambiguousExpressionList();
      }
      setState(1553);
      match(EscriptParser::RPAREN);
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(1554);
      match(EscriptParser::LBRACE);
      setState(1556);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1555);
        unambiguousExpressionList();
      }
      setState(1558);
      match(EscriptParser::COMMA);
      setState(1559);
      match(EscriptParser::RBRACE);
      notifyErrorListeners("Expected expression following comma before right-brace in array initializer list");
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(1561);
      match(EscriptParser::LPAREN);
      setState(1563);
      _errHandler->sync(this);

      _la = _input->LA(1);
      if ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << EscriptParser::BANG_A)
        | (1ULL << EscriptParser::BANG_B)
        | (1ULL << EscriptParser::TOK_ERROR)
        | (1ULL << EscriptParser::DICTIONARY))) != 0) || ((((_la - 64) & ~ 0x3fULL) == 0) &&
        ((1ULL << (_la - 64)) & ((1ULL << (EscriptParser::STRUCT - 64))
        | (1ULL << (EscriptParser::ARRAY - 64))
        | (1ULL << (EscriptParser::DECIMAL_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_LITERAL - 64))
        | (1ULL << (EscriptParser::OCT_LITERAL - 64))
        | (1ULL << (EscriptParser::BINARY_LITERAL - 64))
        | (1ULL << (EscriptParser::FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::HEX_FLOAT_LITERAL - 64))
        | (1ULL << (EscriptParser::CHAR_LITERAL - 64))
        | (1ULL << (EscriptParser::STRING_LITERAL - 64))
        | (1ULL << (EscriptParser::NULL_LITERAL - 64))
        | (1ULL << (EscriptParser::LPAREN - 64))
        | (1ULL << (EscriptParser::LBRACE - 64))
        | (1ULL << (EscriptParser::ADD - 64))
        | (1ULL << (EscriptParser::SUB - 64))
        | (1ULL << (EscriptParser::TILDE - 64))
        | (1ULL << (EscriptParser::AT - 64))
        | (1ULL << (EscriptParser::INC - 64))
        | (1ULL << (EscriptParser::DEC - 64))
        | (1ULL << (EscriptParser::IDENTIFIER - 64)))) != 0)) {
        setState(1562);
        unambiguousExpressionList();
      }
      setState(1565);
      match(EscriptParser::COMMA);
      setState(1566);
      match(EscriptParser::RPAREN);
      notifyErrorListeners("Expected expression following comma before right-paren in array initializer list");
      break;
    }

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

tree::TerminalNode* EscriptParser::LiteralContext::CHAR_LITERAL() {
  return getToken(EscriptParser::CHAR_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::LiteralContext::STRING_LITERAL() {
  return getToken(EscriptParser::STRING_LITERAL, 0);
}

tree::TerminalNode* EscriptParser::LiteralContext::NULL_LITERAL() {
  return getToken(EscriptParser::NULL_LITERAL, 0);
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


antlrcpp::Any EscriptParser::LiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::LiteralContext* EscriptParser::literal() {
  LiteralContext *_localctx = _tracker.createInstance<LiteralContext>(_ctx, getState());
  enterRule(_localctx, 318, EscriptParser::RuleLiteral);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(1575);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case EscriptParser::DECIMAL_LITERAL:
      case EscriptParser::HEX_LITERAL:
      case EscriptParser::OCT_LITERAL:
      case EscriptParser::BINARY_LITERAL: {
        enterOuterAlt(_localctx, 1);
        setState(1570);
        integerLiteral();
        break;
      }

      case EscriptParser::FLOAT_LITERAL:
      case EscriptParser::HEX_FLOAT_LITERAL: {
        enterOuterAlt(_localctx, 2);
        setState(1571);
        floatLiteral();
        break;
      }

      case EscriptParser::CHAR_LITERAL: {
        enterOuterAlt(_localctx, 3);
        setState(1572);
        match(EscriptParser::CHAR_LITERAL);
        break;
      }

      case EscriptParser::STRING_LITERAL: {
        enterOuterAlt(_localctx, 4);
        setState(1573);
        match(EscriptParser::STRING_LITERAL);
        break;
      }

      case EscriptParser::NULL_LITERAL: {
        enterOuterAlt(_localctx, 5);
        setState(1574);
        match(EscriptParser::NULL_LITERAL);
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


antlrcpp::Any EscriptParser::IntegerLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitIntegerLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::IntegerLiteralContext* EscriptParser::integerLiteral() {
  IntegerLiteralContext *_localctx = _tracker.createInstance<IntegerLiteralContext>(_ctx, getState());
  enterRule(_localctx, 320, EscriptParser::RuleIntegerLiteral);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1577);
    _la = _input->LA(1);
    if (!(((((_la - 68) & ~ 0x3fULL) == 0) &&
      ((1ULL << (_la - 68)) & ((1ULL << (EscriptParser::DECIMAL_LITERAL - 68))
      | (1ULL << (EscriptParser::HEX_LITERAL - 68))
      | (1ULL << (EscriptParser::OCT_LITERAL - 68))
      | (1ULL << (EscriptParser::BINARY_LITERAL - 68)))) != 0))) {
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


antlrcpp::Any EscriptParser::FloatLiteralContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<EscriptParserVisitor*>(visitor))
    return parserVisitor->visitFloatLiteral(this);
  else
    return visitor->visitChildren(this);
}

EscriptParser::FloatLiteralContext* EscriptParser::floatLiteral() {
  FloatLiteralContext *_localctx = _tracker.createInstance<FloatLiteralContext>(_ctx, getState());
  enterRule(_localctx, 322, EscriptParser::RuleFloatLiteral);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(1579);
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

bool EscriptParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 130: return expressionSempred(dynamic_cast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool EscriptParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 14);
    case 1: return precpred(_ctx, 13);
    case 2: return precpred(_ctx, 12);
    case 3: return precpred(_ctx, 11);
    case 4: return precpred(_ctx, 10);
    case 5: return precpred(_ctx, 9);
    case 6: return precpred(_ctx, 8);
    case 7: return precpred(_ctx, 7);
    case 8: return precpred(_ctx, 6);
    case 9: return precpred(_ctx, 5);
    case 10: return precpred(_ctx, 4);
    case 11: return precpred(_ctx, 3);
    case 12: return precpred(_ctx, 2);
    case 13: return precpred(_ctx, 1);
    case 14: return precpred(_ctx, 27);
    case 15: return precpred(_ctx, 26);
    case 16: return precpred(_ctx, 17);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> EscriptParser::_decisionToDFA;
atn::PredictionContextCache EscriptParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN EscriptParser::_atn;
std::vector<uint16_t> EscriptParser::_serializedATN;

std::vector<std::string> EscriptParser::_ruleNames = {
  "unambiguousCompilationUnit", "compilationUnit", "moduleUnit", "unambiguousModuleUnit", 
  "moduleDeclarationStatement", "unambiguousModuleDeclarationStatement", 
  "moduleFunctionDeclaration", "unambiguousModuleFunctionDeclaration", "moduleFunctionParameterList", 
  "unambiguousModuleFunctionParameterList", "moduleFunctionParameter", "unambiguousModuleFunctionParameter", 
  "topLevelDeclaration", "unambiguousTopLevelDeclaration", "functionDeclaration", 
  "unambiguousFunctionDeclaration", "stringIdentifier", "useDeclaration", 
  "includeDeclaration", "programDeclaration", "unambiguousProgramDeclaration", 
  "unambiguousStatement", "statement", "statementLabel", "ifStatement", 
  "unambiguousIfStatement", "gotoStatement", "returnStatement", "unambiguousReturnStatement", 
  "constStatement", "unambiguousConstStatement", "varStatement", "unambiguousVarStatement", 
  "doStatement", "unambiguousDoStatement", "whileStatement", "unambiguousWhileStatement", 
  "exitStatement", "declareStatement", "breakStatement", "continueStatement", 
  "forStatement", "unambiguousForStatement", "foreachStatement", "unambiguousForeachStatement", 
  "repeatStatement", "unambiguousRepeatStatement", "caseStatement", "unambiguousCaseStatement", 
  "enumStatement", "unambiguousEnumStatement", "block", "unambiguousBlock", 
  "variableDeclarationInitializer", "unambiguousVariableDeclarationInitializer", 
  "enumList", "unambiguousEnumList", "enumListEntry", "unambiguousEnumListEntry", 
  "switchBlockStatementGroup", "unambiguousSwitchBlockStatementGroup", "switchLabel", 
  "forGroup", "unambiguousForGroup", "basicForStatement", "unambiguousBasicForStatement", 
  "cstyleForStatement", "unambiguousCstyleForStatement", "identifierList", 
  "variableDeclarationList", "unambiguousVariableDeclarationList", "variableDeclaration", 
  "unambiguousVariableDeclaration", "programParameters", "programParameterList", 
  "programParameter", "unambiguousProgramParameters", "unambiguousProgramParameterList", 
  "unambiguousProgramParameter", "functionParameters", "functionParameterList", 
  "functionParameter", "unambiguousFunctionParameters", "unambiguousFunctionParameterList", 
  "unambiguousFunctionParameter", "scopedMethodCall", "unambiguousExpression", 
  "assignmentOperator", "membership", "membershipOperator", "disjunction", 
  "disjunctionOperator", "conjunction", "conjunctionOperator", "equality", 
  "equalityOperator", "bitwiseDisjunction", "bitwiseXor", "bitwiseConjunction", 
  "bitshiftRight", "bitshiftLeft", "comparison", "comparisonOperator", "infixOperation", 
  "infixOperator", "elvisExpression", "additiveExpression", "additiveOperator", 
  "multiplicativeExpression", "multiplicativeOperator", "prefixUnaryInversionExpression", 
  "prefixUnaryInversionOperator", "prefixUnaryArithmeticExpression", "prefixUnaryArithmeticOperator", 
  "postfixUnaryExpression", "postfixUnaryOperator", "indexingSuffix", "navigationSuffix", 
  "membershipSuffix", "memberCallSuffix", "memberAccessOperator", "callSuffix", 
  "atomicExpression", "functionReference", "unambiguousExplicitArrayInitializer", 
  "unambiguousExplicitStructInitializer", "unambiguousExplicitDictInitializer", 
  "unambiguousExplicitErrorInitializer", "unambiguousBareArrayInitializer", 
  "parenthesizedExpression", "expression", "primary", "parExpression", "expressionList", 
  "unambiguousExpressionList", "methodCallArgument", "methodCallArgumentList", 
  "methodCall", "valueArgumentList", "valueArguments", "unambiguousFunctionCallArgument", 
  "unambiguousFunctionCall", "unambiguousScopedFunctionCall", "unambiguousFunctionCallArgumentList", 
  "memberCall", "structInitializerExpression", "structInitializerExpressionList", 
  "structInitializer", "unambiguousStructInitializerExpression", "unambiguousStructInitializerExpressionList", 
  "unambiguousStructInitializer", "dictInitializerExpression", "unambiguousDictInitializerExpression", 
  "dictInitializerExpressionList", "unambiguousDictInitializerExpressionList", 
  "dictInitializer", "unambiguousDictInitializer", "arrayInitializer", "unambiguousArrayInitializer", 
  "literal", "integerLiteral", "floatLiteral"
};

std::vector<std::string> EscriptParser::_literalNames = {
  "", "'if'", "'then'", "'elseif'", "'endif'", "'else'", "'goto'", "'return'", 
  "'const'", "'var'", "'do'", "'dowhile'", "'while'", "'endwhile'", "'exit'", 
  "'declare'", "'function'", "'endfunction'", "'exported'", "'use'", "'include'", 
  "'break'", "'continue'", "'for'", "'endfor'", "'to'", "'foreach'", "'endforeach'", 
  "'repeat'", "'until'", "'program'", "'endprogram'", "'case'", "'default'", 
  "'endcase'", "'enum'", "'endenum'", "'downto'", "'step'", "'reference'", 
  "'out'", "'inout'", "'ByVal'", "'string'", "'long'", "'integer'", "'unsigned'", 
  "'signed'", "'real'", "'float'", "'double'", "'as'", "'is'", "'&&'", "'and'", 
  "'||'", "'or'", "'!'", "'not'", "'byref'", "'unused'", "'error'", "'hash'", 
  "'dictionary'", "'struct'", "'array'", "'stack'", "'in'", "", "", "", 
  "", "", "", "", "", "'null'", "'('", "')'", "'['", "']'", "'{'", "'}'", 
  "'.'", "'->'", "'*'", "'/'", "'%'", "'+'", "'-'", "'+='", "'-='", "'*='", 
  "'/='", "'%='", "'<='", "'<'", "'>='", "'>'", "'>>'", "'<<'", "'&'", "'^'", 
  "'|'", "'<>'", "'!='", "'='", "'=='", "':='", "'.+'", "'.-'", "'.?'", 
  "';'", "','", "'~'", "'@'", "'::'", "':'", "'++'", "'--'", "'?:'"
};

std::vector<std::string> EscriptParser::_symbolicNames = {
  "", "IF", "THEN", "ELSEIF", "ENDIF", "ELSE", "GOTO", "RETURN", "TOK_CONST", 
  "VAR", "DO", "DOWHILE", "WHILE", "ENDWHILE", "EXIT", "DECLARE", "FUNCTION", 
  "ENDFUNCTION", "EXPORTED", "USE", "INCLUDE", "BREAK", "CONTINUE", "FOR", 
  "ENDFOR", "TO", "FOREACH", "ENDFOREACH", "REPEAT", "UNTIL", "PROGRAM", 
  "ENDPROGRAM", "CASE", "DEFAULT", "ENDCASE", "ENUM", "ENDENUM", "DOWNTO", 
  "STEP", "REFERENCE", "TOK_OUT", "INOUT", "BYVAL", "STRING", "TOK_LONG", 
  "INTEGER", "UNSIGNED", "SIGNED", "REAL", "FLOAT", "DOUBLE", "AS", "IS", 
  "AND_A", "AND_B", "OR_A", "OR_B", "BANG_A", "BANG_B", "BYREF", "UNUSED", 
  "TOK_ERROR", "HASH", "DICTIONARY", "STRUCT", "ARRAY", "STACK", "TOK_IN", 
  "DECIMAL_LITERAL", "HEX_LITERAL", "OCT_LITERAL", "BINARY_LITERAL", "FLOAT_LITERAL", 
  "HEX_FLOAT_LITERAL", "CHAR_LITERAL", "STRING_LITERAL", "NULL_LITERAL", 
  "LPAREN", "RPAREN", "LBRACK", "RBRACK", "LBRACE", "RBRACE", "DOT", "ARROW", 
  "MUL", "DIV", "MOD", "ADD", "SUB", "ADD_ASSIGN", "SUB_ASSIGN", "MUL_ASSIGN", 
  "DIV_ASSIGN", "MOD_ASSIGN", "LE", "LT", "GE", "GT", "RSHIFT", "LSHIFT", 
  "BITAND", "CARET", "BITOR", "NOTEQUAL_A", "NOTEQUAL_B", "EQUAL_DEPRECATED", 
  "EQUAL", "ASSIGN", "ADDMEMBER", "DELMEMBER", "CHKMEMBER", "SEMI", "COMMA", 
  "TILDE", "AT", "COLONCOLON", "COLON", "INC", "DEC", "ELVIS", "WS", "COMMENT", 
  "LINE_COMMENT", "IDENTIFIER"
};

dfa::Vocabulary EscriptParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> EscriptParser::_tokenNames;

EscriptParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0x7e, 0x630, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 
    0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 
    0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 0xb, 
    0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 0xe, 
    0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 0x9, 0x11, 0x4, 
    0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 0x9, 0x14, 0x4, 0x15, 
    0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 0x9, 0x17, 0x4, 0x18, 0x9, 
    0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 0x9, 0x1a, 0x4, 0x1b, 0x9, 0x1b, 
    0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 0x9, 0x1d, 0x4, 0x1e, 0x9, 0x1e, 0x4, 
    0x1f, 0x9, 0x1f, 0x4, 0x20, 0x9, 0x20, 0x4, 0x21, 0x9, 0x21, 0x4, 0x22, 
    0x9, 0x22, 0x4, 0x23, 0x9, 0x23, 0x4, 0x24, 0x9, 0x24, 0x4, 0x25, 0x9, 
    0x25, 0x4, 0x26, 0x9, 0x26, 0x4, 0x27, 0x9, 0x27, 0x4, 0x28, 0x9, 0x28, 
    0x4, 0x29, 0x9, 0x29, 0x4, 0x2a, 0x9, 0x2a, 0x4, 0x2b, 0x9, 0x2b, 0x4, 
    0x2c, 0x9, 0x2c, 0x4, 0x2d, 0x9, 0x2d, 0x4, 0x2e, 0x9, 0x2e, 0x4, 0x2f, 
    0x9, 0x2f, 0x4, 0x30, 0x9, 0x30, 0x4, 0x31, 0x9, 0x31, 0x4, 0x32, 0x9, 
    0x32, 0x4, 0x33, 0x9, 0x33, 0x4, 0x34, 0x9, 0x34, 0x4, 0x35, 0x9, 0x35, 
    0x4, 0x36, 0x9, 0x36, 0x4, 0x37, 0x9, 0x37, 0x4, 0x38, 0x9, 0x38, 0x4, 
    0x39, 0x9, 0x39, 0x4, 0x3a, 0x9, 0x3a, 0x4, 0x3b, 0x9, 0x3b, 0x4, 0x3c, 
    0x9, 0x3c, 0x4, 0x3d, 0x9, 0x3d, 0x4, 0x3e, 0x9, 0x3e, 0x4, 0x3f, 0x9, 
    0x3f, 0x4, 0x40, 0x9, 0x40, 0x4, 0x41, 0x9, 0x41, 0x4, 0x42, 0x9, 0x42, 
    0x4, 0x43, 0x9, 0x43, 0x4, 0x44, 0x9, 0x44, 0x4, 0x45, 0x9, 0x45, 0x4, 
    0x46, 0x9, 0x46, 0x4, 0x47, 0x9, 0x47, 0x4, 0x48, 0x9, 0x48, 0x4, 0x49, 
    0x9, 0x49, 0x4, 0x4a, 0x9, 0x4a, 0x4, 0x4b, 0x9, 0x4b, 0x4, 0x4c, 0x9, 
    0x4c, 0x4, 0x4d, 0x9, 0x4d, 0x4, 0x4e, 0x9, 0x4e, 0x4, 0x4f, 0x9, 0x4f, 
    0x4, 0x50, 0x9, 0x50, 0x4, 0x51, 0x9, 0x51, 0x4, 0x52, 0x9, 0x52, 0x4, 
    0x53, 0x9, 0x53, 0x4, 0x54, 0x9, 0x54, 0x4, 0x55, 0x9, 0x55, 0x4, 0x56, 
    0x9, 0x56, 0x4, 0x57, 0x9, 0x57, 0x4, 0x58, 0x9, 0x58, 0x4, 0x59, 0x9, 
    0x59, 0x4, 0x5a, 0x9, 0x5a, 0x4, 0x5b, 0x9, 0x5b, 0x4, 0x5c, 0x9, 0x5c, 
    0x4, 0x5d, 0x9, 0x5d, 0x4, 0x5e, 0x9, 0x5e, 0x4, 0x5f, 0x9, 0x5f, 0x4, 
    0x60, 0x9, 0x60, 0x4, 0x61, 0x9, 0x61, 0x4, 0x62, 0x9, 0x62, 0x4, 0x63, 
    0x9, 0x63, 0x4, 0x64, 0x9, 0x64, 0x4, 0x65, 0x9, 0x65, 0x4, 0x66, 0x9, 
    0x66, 0x4, 0x67, 0x9, 0x67, 0x4, 0x68, 0x9, 0x68, 0x4, 0x69, 0x9, 0x69, 
    0x4, 0x6a, 0x9, 0x6a, 0x4, 0x6b, 0x9, 0x6b, 0x4, 0x6c, 0x9, 0x6c, 0x4, 
    0x6d, 0x9, 0x6d, 0x4, 0x6e, 0x9, 0x6e, 0x4, 0x6f, 0x9, 0x6f, 0x4, 0x70, 
    0x9, 0x70, 0x4, 0x71, 0x9, 0x71, 0x4, 0x72, 0x9, 0x72, 0x4, 0x73, 0x9, 
    0x73, 0x4, 0x74, 0x9, 0x74, 0x4, 0x75, 0x9, 0x75, 0x4, 0x76, 0x9, 0x76, 
    0x4, 0x77, 0x9, 0x77, 0x4, 0x78, 0x9, 0x78, 0x4, 0x79, 0x9, 0x79, 0x4, 
    0x7a, 0x9, 0x7a, 0x4, 0x7b, 0x9, 0x7b, 0x4, 0x7c, 0x9, 0x7c, 0x4, 0x7d, 
    0x9, 0x7d, 0x4, 0x7e, 0x9, 0x7e, 0x4, 0x7f, 0x9, 0x7f, 0x4, 0x80, 0x9, 
    0x80, 0x4, 0x81, 0x9, 0x81, 0x4, 0x82, 0x9, 0x82, 0x4, 0x83, 0x9, 0x83, 
    0x4, 0x84, 0x9, 0x84, 0x4, 0x85, 0x9, 0x85, 0x4, 0x86, 0x9, 0x86, 0x4, 
    0x87, 0x9, 0x87, 0x4, 0x88, 0x9, 0x88, 0x4, 0x89, 0x9, 0x89, 0x4, 0x8a, 
    0x9, 0x8a, 0x4, 0x8b, 0x9, 0x8b, 0x4, 0x8c, 0x9, 0x8c, 0x4, 0x8d, 0x9, 
    0x8d, 0x4, 0x8e, 0x9, 0x8e, 0x4, 0x8f, 0x9, 0x8f, 0x4, 0x90, 0x9, 0x90, 
    0x4, 0x91, 0x9, 0x91, 0x4, 0x92, 0x9, 0x92, 0x4, 0x93, 0x9, 0x93, 0x4, 
    0x94, 0x9, 0x94, 0x4, 0x95, 0x9, 0x95, 0x4, 0x96, 0x9, 0x96, 0x4, 0x97, 
    0x9, 0x97, 0x4, 0x98, 0x9, 0x98, 0x4, 0x99, 0x9, 0x99, 0x4, 0x9a, 0x9, 
    0x9a, 0x4, 0x9b, 0x9, 0x9b, 0x4, 0x9c, 0x9, 0x9c, 0x4, 0x9d, 0x9, 0x9d, 
    0x4, 0x9e, 0x9, 0x9e, 0x4, 0x9f, 0x9, 0x9f, 0x4, 0xa0, 0x9, 0xa0, 0x4, 
    0xa1, 0x9, 0xa1, 0x4, 0xa2, 0x9, 0xa2, 0x4, 0xa3, 0x9, 0xa3, 0x3, 0x2, 
    0x7, 0x2, 0x148, 0xa, 0x2, 0xc, 0x2, 0xe, 0x2, 0x14b, 0xb, 0x2, 0x3, 
    0x2, 0x3, 0x2, 0x3, 0x3, 0x7, 0x3, 0x150, 0xa, 0x3, 0xc, 0x3, 0xe, 0x3, 
    0x153, 0xb, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x7, 0x4, 0x158, 0xa, 
    0x4, 0xc, 0x4, 0xe, 0x4, 0x15b, 0xb, 0x4, 0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 
    0x7, 0x5, 0x160, 0xa, 0x5, 0xc, 0x5, 0xe, 0x5, 0x163, 0xb, 0x5, 0x3, 
    0x5, 0x3, 0x5, 0x3, 0x6, 0x3, 0x6, 0x5, 0x6, 0x169, 0xa, 0x6, 0x3, 0x7, 
    0x3, 0x7, 0x5, 0x7, 0x16d, 0xa, 0x7, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x5, 
    0x8, 0x172, 0xa, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 0x9, 
    0x3, 0x9, 0x5, 0x9, 0x17a, 0xa, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 
    0xa, 0x3, 0xa, 0x3, 0xa, 0x7, 0xa, 0x182, 0xa, 0xa, 0xc, 0xa, 0xe, 0xa, 
    0x185, 0xb, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xb, 0x7, 0xb, 0x18a, 0xa, 
    0xb, 0xc, 0xb, 0xe, 0xb, 0x18d, 0xb, 0xb, 0x3, 0xc, 0x3, 0xc, 0x3, 0xc, 
    0x5, 0xc, 0x192, 0xa, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x5, 0xd, 0x197, 
    0xa, 0xd, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 
    0x19e, 0xa, 0xe, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x3, 0xf, 0x5, 
    0xf, 0x1a5, 0xa, 0xf, 0x3, 0x10, 0x5, 0x10, 0x1a8, 0xa, 0x10, 0x3, 0x10, 
    0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 0x11, 0x5, 
    0x11, 0x1b1, 0xa, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 
    0x3, 0x11, 0x3, 0x11, 0x3, 0x12, 0x3, 0x12, 0x3, 0x13, 0x3, 0x13, 0x3, 
    0x13, 0x3, 0x13, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 0x15, 
    0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x16, 0x3, 
    0x16, 0x3, 0x16, 0x3, 0x16, 0x3, 0x16, 0x3, 0x16, 0x3, 0x17, 0x3, 0x17, 
    0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 
    0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 
    0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x5, 0x17, 0x1e1, 0xa, 0x17, 0x3, 0x18, 
    0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 
    0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
    0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x5, 
    0x18, 0x1f7, 0xa, 0x18, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x1a, 
    0x3, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x1ff, 0xa, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 
    0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x7, 0x1a, 0x206, 0xa, 0x1a, 0xc, 0x1a, 
    0xe, 0x1a, 0x209, 0xb, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x20d, 
    0xa, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x5, 
    0x1b, 0x214, 0xa, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 
    0x3, 0x1b, 0x7, 0x1b, 0x21b, 0xa, 0x1b, 0xc, 0x1b, 0xe, 0x1b, 0x21e, 
    0xb, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x5, 0x1b, 0x222, 0xa, 0x1b, 0x3, 0x1b, 
    0x3, 0x1b, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1d, 0x3, 
    0x1d, 0x5, 0x1d, 0x22c, 0xa, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1e, 
    0x3, 0x1e, 0x5, 0x1e, 0x232, 0xa, 0x1e, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1f, 
    0x3, 0x1f, 0x3, 0x1f, 0x3, 0x1f, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 
    0x20, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 0x3, 0x21, 0x3, 0x22, 0x3, 0x22, 
    0x3, 0x22, 0x3, 0x22, 0x3, 0x23, 0x5, 0x23, 0x247, 0xa, 0x23, 0x3, 0x23, 
    0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x23, 0x3, 0x24, 0x5, 
    0x24, 0x250, 0xa, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 0x3, 0x24, 
    0x3, 0x24, 0x3, 0x24, 0x3, 0x25, 0x5, 0x25, 0x259, 0xa, 0x25, 0x3, 0x25, 
    0x3, 0x25, 0x3, 0x25, 0x3, 0x25, 0x3, 0x25, 0x3, 0x26, 0x5, 0x26, 0x261, 
    0xa, 0x26, 0x3, 0x26, 0x3, 0x26, 0x3, 0x26, 0x3, 0x26, 0x3, 0x26, 0x3, 
    0x27, 0x3, 0x27, 0x3, 0x27, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 0x3, 0x28, 
    0x3, 0x28, 0x3, 0x29, 0x3, 0x29, 0x5, 0x29, 0x272, 0xa, 0x29, 0x3, 0x29, 
    0x3, 0x29, 0x3, 0x2a, 0x3, 0x2a, 0x5, 0x2a, 0x278, 0xa, 0x2a, 0x3, 0x2a, 
    0x3, 0x2a, 0x3, 0x2b, 0x5, 0x2b, 0x27d, 0xa, 0x2b, 0x3, 0x2b, 0x3, 0x2b, 
    0x3, 0x2b, 0x3, 0x2b, 0x3, 0x2c, 0x5, 0x2c, 0x284, 0xa, 0x2c, 0x3, 0x2c, 
    0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2c, 0x3, 0x2d, 0x5, 0x2d, 0x28b, 0xa, 0x2d, 
    0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 0x2d, 0x3, 
    0x2d, 0x3, 0x2e, 0x5, 0x2e, 0x295, 0xa, 0x2e, 0x3, 0x2e, 0x3, 0x2e, 
    0x3, 0x2e, 0x3, 0x2e, 0x3, 0x2e, 0x3, 0x2e, 0x3, 0x2e, 0x3, 0x2f, 0x5, 
    0x2f, 0x29f, 0xa, 0x2f, 0x3, 0x2f, 0x3, 0x2f, 0x3, 0x2f, 0x3, 0x2f, 
    0x3, 0x2f, 0x3, 0x2f, 0x3, 0x30, 0x5, 0x30, 0x2a8, 0xa, 0x30, 0x3, 0x30, 
    0x3, 0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x30, 0x3, 0x31, 0x5, 
    0x31, 0x2b1, 0xa, 0x31, 0x3, 0x31, 0x3, 0x31, 0x3, 0x31, 0x3, 0x31, 
    0x3, 0x31, 0x6, 0x31, 0x2b8, 0xa, 0x31, 0xd, 0x31, 0xe, 0x31, 0x2b9, 
    0x3, 0x31, 0x3, 0x31, 0x3, 0x32, 0x5, 0x32, 0x2bf, 0xa, 0x32, 0x3, 0x32, 
    0x3, 0x32, 0x3, 0x32, 0x3, 0x32, 0x3, 0x32, 0x6, 0x32, 0x2c6, 0xa, 0x32, 
    0xd, 0x32, 0xe, 0x32, 0x2c7, 0x3, 0x32, 0x3, 0x32, 0x3, 0x33, 0x3, 0x33, 
    0x3, 0x33, 0x3, 0x33, 0x3, 0x33, 0x3, 0x34, 0x3, 0x34, 0x3, 0x34, 0x3, 
    0x34, 0x3, 0x34, 0x3, 0x35, 0x7, 0x35, 0x2d7, 0xa, 0x35, 0xc, 0x35, 
    0xe, 0x35, 0x2da, 0xb, 0x35, 0x3, 0x36, 0x7, 0x36, 0x2dd, 0xa, 0x36, 
    0xc, 0x36, 0xe, 0x36, 0x2e0, 0xb, 0x36, 0x3, 0x37, 0x3, 0x37, 0x3, 0x37, 
    0x5, 0x37, 0x2e5, 0xa, 0x37, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 
    0x3, 0x38, 0x3, 0x38, 0x3, 0x38, 0x5, 0x38, 0x2ee, 0xa, 0x38, 0x3, 0x39, 
    0x3, 0x39, 0x3, 0x39, 0x7, 0x39, 0x2f3, 0xa, 0x39, 0xc, 0x39, 0xe, 0x39, 
    0x2f6, 0xb, 0x39, 0x3, 0x39, 0x5, 0x39, 0x2f9, 0xa, 0x39, 0x3, 0x3a, 
    0x3, 0x3a, 0x3, 0x3a, 0x7, 0x3a, 0x2fe, 0xa, 0x3a, 0xc, 0x3a, 0xe, 0x3a, 
    0x301, 0xb, 0x3a, 0x3, 0x3a, 0x5, 0x3a, 0x304, 0xa, 0x3a, 0x3, 0x3b, 
    0x3, 0x3b, 0x3, 0x3b, 0x5, 0x3b, 0x309, 0xa, 0x3b, 0x3, 0x3c, 0x3, 0x3c, 
    0x3, 0x3c, 0x5, 0x3c, 0x30e, 0xa, 0x3c, 0x3, 0x3d, 0x6, 0x3d, 0x311, 
    0xa, 0x3d, 0xd, 0x3d, 0xe, 0x3d, 0x312, 0x3, 0x3d, 0x3, 0x3d, 0x3, 0x3e, 
    0x3, 0x3e, 0x7, 0x3e, 0x319, 0xa, 0x3e, 0xc, 0x3e, 0xe, 0x3e, 0x31c, 
    0xb, 0x3e, 0x3, 0x3e, 0x3, 0x3e, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 
    0x3f, 0x323, 0xa, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x3, 0x3f, 0x5, 0x3f, 
    0x328, 0xa, 0x3f, 0x3, 0x40, 0x3, 0x40, 0x5, 0x40, 0x32c, 0xa, 0x40, 
    0x3, 0x41, 0x3, 0x41, 0x5, 0x41, 0x330, 0xa, 0x41, 0x3, 0x42, 0x3, 0x42, 
    0x3, 0x42, 0x3, 0x42, 0x3, 0x42, 0x3, 0x42, 0x3, 0x42, 0x3, 0x43, 0x3, 
    0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x43, 0x3, 0x44, 
    0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 0x44, 0x3, 
    0x44, 0x3, 0x44, 0x3, 0x45, 0x3, 0x45, 0x3, 0x45, 0x3, 0x45, 0x3, 0x45, 
    0x3, 0x45, 0x3, 0x45, 0x3, 0x45, 0x3, 0x45, 0x3, 0x46, 0x3, 0x46, 0x3, 
    0x46, 0x5, 0x46, 0x355, 0xa, 0x46, 0x3, 0x47, 0x3, 0x47, 0x3, 0x47, 
    0x7, 0x47, 0x35a, 0xa, 0x47, 0xc, 0x47, 0xe, 0x47, 0x35d, 0xb, 0x47, 
    0x3, 0x48, 0x3, 0x48, 0x3, 0x48, 0x7, 0x48, 0x362, 0xa, 0x48, 0xc, 0x48, 
    0xe, 0x48, 0x365, 0xb, 0x48, 0x3, 0x49, 0x3, 0x49, 0x5, 0x49, 0x369, 
    0xa, 0x49, 0x3, 0x4a, 0x3, 0x4a, 0x5, 0x4a, 0x36d, 0xa, 0x4a, 0x3, 0x4b, 
    0x3, 0x4b, 0x5, 0x4b, 0x371, 0xa, 0x4b, 0x3, 0x4b, 0x3, 0x4b, 0x3, 0x4c, 
    0x3, 0x4c, 0x5, 0x4c, 0x377, 0xa, 0x4c, 0x3, 0x4c, 0x7, 0x4c, 0x37a, 
    0xa, 0x4c, 0xc, 0x4c, 0xe, 0x4c, 0x37d, 0xb, 0x4c, 0x3, 0x4d, 0x3, 0x4d, 
    0x3, 0x4d, 0x3, 0x4d, 0x3, 0x4d, 0x5, 0x4d, 0x384, 0xa, 0x4d, 0x5, 0x4d, 
    0x386, 0xa, 0x4d, 0x3, 0x4e, 0x3, 0x4e, 0x5, 0x4e, 0x38a, 0xa, 0x4e, 
    0x3, 0x4e, 0x3, 0x4e, 0x3, 0x4f, 0x3, 0x4f, 0x5, 0x4f, 0x390, 0xa, 0x4f, 
    0x3, 0x4f, 0x7, 0x4f, 0x393, 0xa, 0x4f, 0xc, 0x4f, 0xe, 0x4f, 0x396, 
    0xb, 0x4f, 0x3, 0x50, 0x3, 0x50, 0x3, 0x50, 0x3, 0x50, 0x3, 0x50, 0x5, 
    0x50, 0x39d, 0xa, 0x50, 0x5, 0x50, 0x39f, 0xa, 0x50, 0x3, 0x51, 0x3, 
    0x51, 0x5, 0x51, 0x3a3, 0xa, 0x51, 0x3, 0x51, 0x3, 0x51, 0x3, 0x52, 
    0x3, 0x52, 0x3, 0x52, 0x7, 0x52, 0x3aa, 0xa, 0x52, 0xc, 0x52, 0xe, 0x52, 
    0x3ad, 0xb, 0x52, 0x3, 0x53, 0x5, 0x53, 0x3b0, 0xa, 0x53, 0x3, 0x53, 
    0x5, 0x53, 0x3b3, 0xa, 0x53, 0x3, 0x53, 0x3, 0x53, 0x3, 0x53, 0x5, 0x53, 
    0x3b8, 0xa, 0x53, 0x3, 0x54, 0x3, 0x54, 0x5, 0x54, 0x3bc, 0xa, 0x54, 
    0x3, 0x54, 0x3, 0x54, 0x3, 0x55, 0x3, 0x55, 0x3, 0x55, 0x7, 0x55, 0x3c3, 
    0xa, 0x55, 0xc, 0x55, 0xe, 0x55, 0x3c6, 0xb, 0x55, 0x3, 0x56, 0x5, 0x56, 
    0x3c9, 0xa, 0x56, 0x3, 0x56, 0x5, 0x56, 0x3cc, 0xa, 0x56, 0x3, 0x56, 
    0x3, 0x56, 0x3, 0x56, 0x5, 0x56, 0x3d1, 0xa, 0x56, 0x3, 0x57, 0x3, 0x57, 
    0x3, 0x57, 0x3, 0x57, 0x3, 0x58, 0x3, 0x58, 0x3, 0x58, 0x3, 0x58, 0x7, 
    0x58, 0x3db, 0xa, 0x58, 0xc, 0x58, 0xe, 0x58, 0x3de, 0xb, 0x58, 0x3, 
    0x59, 0x3, 0x59, 0x3, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x3, 0x5a, 0x7, 0x5a, 
    0x3e6, 0xa, 0x5a, 0xc, 0x5a, 0xe, 0x5a, 0x3e9, 0xb, 0x5a, 0x3, 0x5b, 
    0x3, 0x5b, 0x3, 0x5c, 0x3, 0x5c, 0x3, 0x5c, 0x3, 0x5c, 0x7, 0x5c, 0x3f1, 
    0xa, 0x5c, 0xc, 0x5c, 0xe, 0x5c, 0x3f4, 0xb, 0x5c, 0x3, 0x5d, 0x3, 0x5d, 
    0x3, 0x5e, 0x3, 0x5e, 0x3, 0x5e, 0x3, 0x5e, 0x7, 0x5e, 0x3fc, 0xa, 0x5e, 
    0xc, 0x5e, 0xe, 0x5e, 0x3ff, 0xb, 0x5e, 0x3, 0x5f, 0x3, 0x5f, 0x3, 0x60, 
    0x3, 0x60, 0x3, 0x60, 0x3, 0x60, 0x7, 0x60, 0x407, 0xa, 0x60, 0xc, 0x60, 
    0xe, 0x60, 0x40a, 0xb, 0x60, 0x3, 0x61, 0x3, 0x61, 0x3, 0x62, 0x3, 0x62, 
    0x3, 0x62, 0x7, 0x62, 0x411, 0xa, 0x62, 0xc, 0x62, 0xe, 0x62, 0x414, 
    0xb, 0x62, 0x3, 0x63, 0x3, 0x63, 0x3, 0x63, 0x7, 0x63, 0x419, 0xa, 0x63, 
    0xc, 0x63, 0xe, 0x63, 0x41c, 0xb, 0x63, 0x3, 0x64, 0x3, 0x64, 0x3, 0x64, 
    0x7, 0x64, 0x421, 0xa, 0x64, 0xc, 0x64, 0xe, 0x64, 0x424, 0xb, 0x64, 
    0x3, 0x65, 0x3, 0x65, 0x3, 0x65, 0x7, 0x65, 0x429, 0xa, 0x65, 0xc, 0x65, 
    0xe, 0x65, 0x42c, 0xb, 0x65, 0x3, 0x66, 0x3, 0x66, 0x3, 0x66, 0x7, 0x66, 
    0x431, 0xa, 0x66, 0xc, 0x66, 0xe, 0x66, 0x434, 0xb, 0x66, 0x3, 0x67, 
    0x3, 0x67, 0x3, 0x67, 0x3, 0x67, 0x7, 0x67, 0x43a, 0xa, 0x67, 0xc, 0x67, 
    0xe, 0x67, 0x43d, 0xb, 0x67, 0x3, 0x68, 0x3, 0x68, 0x3, 0x69, 0x3, 0x69, 
    0x3, 0x69, 0x3, 0x69, 0x7, 0x69, 0x445, 0xa, 0x69, 0xc, 0x69, 0xe, 0x69, 
    0x448, 0xb, 0x69, 0x3, 0x6a, 0x3, 0x6a, 0x3, 0x6b, 0x3, 0x6b, 0x3, 0x6b, 
    0x7, 0x6b, 0x44f, 0xa, 0x6b, 0xc, 0x6b, 0xe, 0x6b, 0x452, 0xb, 0x6b, 
    0x3, 0x6c, 0x3, 0x6c, 0x3, 0x6c, 0x3, 0x6c, 0x7, 0x6c, 0x458, 0xa, 0x6c, 
    0xc, 0x6c, 0xe, 0x6c, 0x45b, 0xb, 0x6c, 0x3, 0x6d, 0x3, 0x6d, 0x3, 0x6e, 
    0x3, 0x6e, 0x3, 0x6e, 0x3, 0x6e, 0x7, 0x6e, 0x463, 0xa, 0x6e, 0xc, 0x6e, 
    0xe, 0x6e, 0x466, 0xb, 0x6e, 0x3, 0x6f, 0x3, 0x6f, 0x3, 0x70, 0x7, 0x70, 
    0x46b, 0xa, 0x70, 0xc, 0x70, 0xe, 0x70, 0x46e, 0xb, 0x70, 0x3, 0x70, 
    0x3, 0x70, 0x3, 0x71, 0x3, 0x71, 0x3, 0x72, 0x7, 0x72, 0x475, 0xa, 0x72, 
    0xc, 0x72, 0xe, 0x72, 0x478, 0xb, 0x72, 0x3, 0x72, 0x3, 0x72, 0x3, 0x73, 
    0x3, 0x73, 0x3, 0x74, 0x3, 0x74, 0x7, 0x74, 0x480, 0xa, 0x74, 0xc, 0x74, 
    0xe, 0x74, 0x483, 0xb, 0x74, 0x3, 0x75, 0x3, 0x75, 0x3, 0x75, 0x3, 0x75, 
    0x3, 0x75, 0x5, 0x75, 0x48a, 0xa, 0x75, 0x3, 0x76, 0x3, 0x76, 0x3, 0x76, 
    0x3, 0x76, 0x3, 0x77, 0x3, 0x77, 0x3, 0x77, 0x3, 0x78, 0x3, 0x78, 0x3, 
    0x78, 0x3, 0x78, 0x5, 0x78, 0x497, 0xa, 0x78, 0x3, 0x79, 0x3, 0x79, 
    0x3, 0x79, 0x3, 0x79, 0x5, 0x79, 0x49d, 0xa, 0x79, 0x3, 0x79, 0x3, 0x79, 
    0x3, 0x7a, 0x3, 0x7a, 0x3, 0x7b, 0x3, 0x7b, 0x3, 0x7c, 0x3, 0x7c, 0x3, 
    0x7c, 0x3, 0x7c, 0x3, 0x7c, 0x3, 0x7c, 0x3, 0x7c, 0x3, 0x7c, 0x3, 0x7c, 
    0x3, 0x7c, 0x3, 0x7c, 0x5, 0x7c, 0x4b0, 0xa, 0x7c, 0x3, 0x7d, 0x3, 0x7d, 
    0x3, 0x7d, 0x3, 0x7e, 0x3, 0x7e, 0x5, 0x7e, 0x4b7, 0xa, 0x7e, 0x3, 0x7f, 
    0x3, 0x7f, 0x5, 0x7f, 0x4bb, 0xa, 0x7f, 0x3, 0x80, 0x3, 0x80, 0x5, 0x80, 
    0x4bf, 0xa, 0x80, 0x3, 0x81, 0x3, 0x81, 0x5, 0x81, 0x4c3, 0xa, 0x81, 
    0x3, 0x82, 0x3, 0x82, 0x5, 0x82, 0x4c7, 0xa, 0x82, 0x3, 0x82, 0x3, 0x82, 
    0x3, 0x82, 0x5, 0x82, 0x4cc, 0xa, 0x82, 0x3, 0x82, 0x3, 0x82, 0x3, 0x82, 
    0x5, 0x82, 0x4d1, 0xa, 0x82, 0x3, 0x83, 0x3, 0x83, 0x3, 0x83, 0x3, 0x83, 
    0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x5, 
    0x84, 0x4dd, 0xa, 0x84, 0x3, 0x84, 0x3, 0x84, 0x5, 0x84, 0x4e1, 0xa, 
    0x84, 0x3, 0x84, 0x3, 0x84, 0x5, 0x84, 0x4e5, 0xa, 0x84, 0x3, 0x84, 
    0x3, 0x84, 0x5, 0x84, 0x4e9, 0xa, 0x84, 0x3, 0x84, 0x3, 0x84, 0x5, 0x84, 
    0x4ed, 0xa, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 
    0x3, 0x84, 0x5, 0x84, 0x4f5, 0xa, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 
    0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 
    0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 
    0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 
    0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 
    0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 
    0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 
    0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x5, 0x84, 0x526, 
    0xa, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 0x84, 0x3, 
    0x84, 0x3, 0x84, 0x7, 0x84, 0x52f, 0xa, 0x84, 0xc, 0x84, 0xe, 0x84, 
    0x532, 0xb, 0x84, 0x3, 0x85, 0x3, 0x85, 0x3, 0x85, 0x3, 0x85, 0x3, 0x85, 
    0x3, 0x85, 0x5, 0x85, 0x53a, 0xa, 0x85, 0x3, 0x86, 0x3, 0x86, 0x3, 0x86, 
    0x3, 0x86, 0x3, 0x87, 0x3, 0x87, 0x3, 0x87, 0x7, 0x87, 0x543, 0xa, 0x87, 
    0xc, 0x87, 0xe, 0x87, 0x546, 0xb, 0x87, 0x3, 0x88, 0x3, 0x88, 0x3, 0x88, 
    0x7, 0x88, 0x54b, 0xa, 0x88, 0xc, 0x88, 0xe, 0x88, 0x54e, 0xb, 0x88, 
    0x3, 0x89, 0x3, 0x89, 0x5, 0x89, 0x552, 0xa, 0x89, 0x3, 0x89, 0x3, 0x89, 
    0x3, 0x8a, 0x3, 0x8a, 0x3, 0x8a, 0x7, 0x8a, 0x559, 0xa, 0x8a, 0xc, 0x8a, 
    0xe, 0x8a, 0x55c, 0xb, 0x8a, 0x3, 0x8b, 0x3, 0x8b, 0x3, 0x8b, 0x5, 0x8b, 
    0x561, 0xa, 0x8b, 0x3, 0x8b, 0x3, 0x8b, 0x3, 0x8c, 0x3, 0x8c, 0x3, 0x8c, 
    0x7, 0x8c, 0x568, 0xa, 0x8c, 0xc, 0x8c, 0xe, 0x8c, 0x56b, 0xb, 0x8c, 
    0x3, 0x8d, 0x3, 0x8d, 0x3, 0x8d, 0x3, 0x8d, 0x3, 0x8d, 0x3, 0x8d, 0x5, 
    0x8d, 0x573, 0xa, 0x8d, 0x3, 0x8e, 0x3, 0x8e, 0x3, 0x8f, 0x3, 0x8f, 
    0x3, 0x8f, 0x3, 0x90, 0x3, 0x90, 0x3, 0x90, 0x3, 0x90, 0x3, 0x91, 0x3, 
    0x91, 0x3, 0x91, 0x7, 0x91, 0x581, 0xa, 0x91, 0xc, 0x91, 0xe, 0x91, 
    0x584, 0xb, 0x91, 0x3, 0x92, 0x3, 0x92, 0x3, 0x92, 0x5, 0x92, 0x589, 
    0xa, 0x92, 0x3, 0x92, 0x3, 0x92, 0x3, 0x93, 0x3, 0x93, 0x3, 0x93, 0x5, 
    0x93, 0x590, 0xa, 0x93, 0x3, 0x93, 0x3, 0x93, 0x3, 0x93, 0x5, 0x93, 
    0x595, 0xa, 0x93, 0x5, 0x93, 0x597, 0xa, 0x93, 0x3, 0x94, 0x3, 0x94, 
    0x3, 0x94, 0x7, 0x94, 0x59c, 0xa, 0x94, 0xc, 0x94, 0xe, 0x94, 0x59f, 
    0xb, 0x94, 0x3, 0x95, 0x3, 0x95, 0x5, 0x95, 0x5a3, 0xa, 0x95, 0x3, 0x95, 
    0x3, 0x95, 0x3, 0x96, 0x3, 0x96, 0x3, 0x96, 0x5, 0x96, 0x5aa, 0xa, 0x96, 
    0x3, 0x96, 0x3, 0x96, 0x3, 0x96, 0x5, 0x96, 0x5af, 0xa, 0x96, 0x5, 0x96, 
    0x5b1, 0xa, 0x96, 0x3, 0x97, 0x3, 0x97, 0x3, 0x97, 0x7, 0x97, 0x5b6, 
    0xa, 0x97, 0xc, 0x97, 0xe, 0x97, 0x5b9, 0xb, 0x97, 0x3, 0x98, 0x3, 0x98, 
    0x5, 0x98, 0x5bd, 0xa, 0x98, 0x3, 0x98, 0x3, 0x98, 0x3, 0x98, 0x5, 0x98, 
    0x5c2, 0xa, 0x98, 0x3, 0x98, 0x3, 0x98, 0x3, 0x98, 0x5, 0x98, 0x5c7, 
    0xa, 0x98, 0x3, 0x99, 0x3, 0x99, 0x3, 0x99, 0x5, 0x99, 0x5cc, 0xa, 0x99, 
    0x3, 0x9a, 0x3, 0x9a, 0x3, 0x9a, 0x5, 0x9a, 0x5d1, 0xa, 0x9a, 0x3, 0x9b, 
    0x3, 0x9b, 0x3, 0x9b, 0x7, 0x9b, 0x5d6, 0xa, 0x9b, 0xc, 0x9b, 0xe, 0x9b, 
    0x5d9, 0xb, 0x9b, 0x3, 0x9c, 0x3, 0x9c, 0x3, 0x9c, 0x7, 0x9c, 0x5de, 
    0xa, 0x9c, 0xc, 0x9c, 0xe, 0x9c, 0x5e1, 0xb, 0x9c, 0x3, 0x9d, 0x3, 0x9d, 
    0x5, 0x9d, 0x5e5, 0xa, 0x9d, 0x3, 0x9d, 0x3, 0x9d, 0x3, 0x9d, 0x5, 0x9d, 
    0x5ea, 0xa, 0x9d, 0x3, 0x9d, 0x3, 0x9d, 0x3, 0x9d, 0x5, 0x9d, 0x5ef, 
    0xa, 0x9d, 0x3, 0x9e, 0x3, 0x9e, 0x5, 0x9e, 0x5f3, 0xa, 0x9e, 0x3, 0x9e, 
    0x3, 0x9e, 0x3, 0x9e, 0x5, 0x9e, 0x5f8, 0xa, 0x9e, 0x3, 0x9e, 0x3, 0x9e, 
    0x3, 0x9e, 0x5, 0x9e, 0x5fd, 0xa, 0x9e, 0x3, 0x9f, 0x3, 0x9f, 0x5, 0x9f, 
    0x601, 0xa, 0x9f, 0x3, 0x9f, 0x3, 0x9f, 0x3, 0x9f, 0x5, 0x9f, 0x606, 
    0xa, 0x9f, 0x3, 0x9f, 0x5, 0x9f, 0x609, 0xa, 0x9f, 0x3, 0xa0, 0x3, 0xa0, 
    0x5, 0xa0, 0x60d, 0xa, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x5, 0xa0, 
    0x612, 0xa, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x5, 0xa0, 0x617, 
    0xa, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x5, 
    0xa0, 0x61e, 0xa, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x3, 0xa0, 0x5, 0xa0, 
    0x623, 0xa, 0xa0, 0x3, 0xa1, 0x3, 0xa1, 0x3, 0xa1, 0x3, 0xa1, 0x3, 0xa1, 
    0x5, 0xa1, 0x62a, 0xa, 0xa1, 0x3, 0xa2, 0x3, 0xa2, 0x3, 0xa3, 0x3, 0xa3, 
    0x3, 0xa3, 0x2, 0x3, 0x106, 0xa4, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 
    0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 
    0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 
    0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 
    0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x84, 0x86, 
    0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, 
    0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 
    0xb8, 0xba, 0xbc, 0xbe, 0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 
    0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 
    0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe, 
    0x100, 0x102, 0x104, 0x106, 0x108, 0x10a, 0x10c, 0x10e, 0x110, 0x112, 
    0x114, 0x116, 0x118, 0x11a, 0x11c, 0x11e, 0x120, 0x122, 0x124, 0x126, 
    0x128, 0x12a, 0x12c, 0x12e, 0x130, 0x132, 0x134, 0x136, 0x138, 0x13a, 
    0x13c, 0x13e, 0x140, 0x142, 0x144, 0x2, 0x11, 0x4, 0x2, 0x4d, 0x4d, 
    0x7e, 0x7e, 0x4, 0x2, 0x5c, 0x60, 0x6e, 0x6e, 0x3, 0x2, 0x6f, 0x71, 
    0x3, 0x2, 0x39, 0x3a, 0x3, 0x2, 0x37, 0x38, 0x3, 0x2, 0x6a, 0x6d, 0x3, 
    0x2, 0x61, 0x64, 0x3, 0x2, 0x5a, 0x5b, 0x3, 0x2, 0x57, 0x59, 0x4, 0x2, 
    0x3b, 0x3c, 0x74, 0x74, 0x4, 0x2, 0x5a, 0x5b, 0x78, 0x79, 0x3, 0x2, 
    0x65, 0x66, 0x3, 0x2, 0x78, 0x79, 0x3, 0x2, 0x46, 0x49, 0x3, 0x2, 0x4a, 
    0x4b, 0x2, 0x681, 0x2, 0x149, 0x3, 0x2, 0x2, 0x2, 0x4, 0x151, 0x3, 0x2, 
    0x2, 0x2, 0x6, 0x159, 0x3, 0x2, 0x2, 0x2, 0x8, 0x161, 0x3, 0x2, 0x2, 
    0x2, 0xa, 0x168, 0x3, 0x2, 0x2, 0x2, 0xc, 0x16c, 0x3, 0x2, 0x2, 0x2, 
    0xe, 0x16e, 0x3, 0x2, 0x2, 0x2, 0x10, 0x176, 0x3, 0x2, 0x2, 0x2, 0x12, 
    0x17e, 0x3, 0x2, 0x2, 0x2, 0x14, 0x186, 0x3, 0x2, 0x2, 0x2, 0x16, 0x18e, 
    0x3, 0x2, 0x2, 0x2, 0x18, 0x193, 0x3, 0x2, 0x2, 0x2, 0x1a, 0x19d, 0x3, 
    0x2, 0x2, 0x2, 0x1c, 0x1a4, 0x3, 0x2, 0x2, 0x2, 0x1e, 0x1a7, 0x3, 0x2, 
    0x2, 0x2, 0x20, 0x1b0, 0x3, 0x2, 0x2, 0x2, 0x22, 0x1b8, 0x3, 0x2, 0x2, 
    0x2, 0x24, 0x1ba, 0x3, 0x2, 0x2, 0x2, 0x26, 0x1be, 0x3, 0x2, 0x2, 0x2, 
    0x28, 0x1c2, 0x3, 0x2, 0x2, 0x2, 0x2a, 0x1c8, 0x3, 0x2, 0x2, 0x2, 0x2c, 
    0x1e0, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x1f6, 0x3, 0x2, 0x2, 0x2, 0x30, 0x1f8, 
    0x3, 0x2, 0x2, 0x2, 0x32, 0x1fb, 0x3, 0x2, 0x2, 0x2, 0x34, 0x210, 0x3, 
    0x2, 0x2, 0x2, 0x36, 0x225, 0x3, 0x2, 0x2, 0x2, 0x38, 0x229, 0x3, 0x2, 
    0x2, 0x2, 0x3a, 0x22f, 0x3, 0x2, 0x2, 0x2, 0x3c, 0x235, 0x3, 0x2, 0x2, 
    0x2, 0x3e, 0x239, 0x3, 0x2, 0x2, 0x2, 0x40, 0x23d, 0x3, 0x2, 0x2, 0x2, 
    0x42, 0x241, 0x3, 0x2, 0x2, 0x2, 0x44, 0x246, 0x3, 0x2, 0x2, 0x2, 0x46, 
    0x24f, 0x3, 0x2, 0x2, 0x2, 0x48, 0x258, 0x3, 0x2, 0x2, 0x2, 0x4a, 0x260, 
    0x3, 0x2, 0x2, 0x2, 0x4c, 0x267, 0x3, 0x2, 0x2, 0x2, 0x4e, 0x26a, 0x3, 
    0x2, 0x2, 0x2, 0x50, 0x26f, 0x3, 0x2, 0x2, 0x2, 0x52, 0x275, 0x3, 0x2, 
    0x2, 0x2, 0x54, 0x27c, 0x3, 0x2, 0x2, 0x2, 0x56, 0x283, 0x3, 0x2, 0x2, 
    0x2, 0x58, 0x28a, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x294, 0x3, 0x2, 0x2, 0x2, 
    0x5c, 0x29e, 0x3, 0x2, 0x2, 0x2, 0x5e, 0x2a7, 0x3, 0x2, 0x2, 0x2, 0x60, 
    0x2b0, 0x3, 0x2, 0x2, 0x2, 0x62, 0x2be, 0x3, 0x2, 0x2, 0x2, 0x64, 0x2cb, 
    0x3, 0x2, 0x2, 0x2, 0x66, 0x2d0, 0x3, 0x2, 0x2, 0x2, 0x68, 0x2d8, 0x3, 
    0x2, 0x2, 0x2, 0x6a, 0x2de, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x2e4, 0x3, 0x2, 
    0x2, 0x2, 0x6e, 0x2ed, 0x3, 0x2, 0x2, 0x2, 0x70, 0x2ef, 0x3, 0x2, 0x2, 
    0x2, 0x72, 0x2fa, 0x3, 0x2, 0x2, 0x2, 0x74, 0x305, 0x3, 0x2, 0x2, 0x2, 
    0x76, 0x30a, 0x3, 0x2, 0x2, 0x2, 0x78, 0x310, 0x3, 0x2, 0x2, 0x2, 0x7a, 
    0x316, 0x3, 0x2, 0x2, 0x2, 0x7c, 0x327, 0x3, 0x2, 0x2, 0x2, 0x7e, 0x32b, 
    0x3, 0x2, 0x2, 0x2, 0x80, 0x32f, 0x3, 0x2, 0x2, 0x2, 0x82, 0x331, 0x3, 
    0x2, 0x2, 0x2, 0x84, 0x338, 0x3, 0x2, 0x2, 0x2, 0x86, 0x33f, 0x3, 0x2, 
    0x2, 0x2, 0x88, 0x348, 0x3, 0x2, 0x2, 0x2, 0x8a, 0x351, 0x3, 0x2, 0x2, 
    0x2, 0x8c, 0x356, 0x3, 0x2, 0x2, 0x2, 0x8e, 0x35e, 0x3, 0x2, 0x2, 0x2, 
    0x90, 0x366, 0x3, 0x2, 0x2, 0x2, 0x92, 0x36a, 0x3, 0x2, 0x2, 0x2, 0x94, 
    0x36e, 0x3, 0x2, 0x2, 0x2, 0x96, 0x374, 0x3, 0x2, 0x2, 0x2, 0x98, 0x385, 
    0x3, 0x2, 0x2, 0x2, 0x9a, 0x387, 0x3, 0x2, 0x2, 0x2, 0x9c, 0x38d, 0x3, 
    0x2, 0x2, 0x2, 0x9e, 0x39e, 0x3, 0x2, 0x2, 0x2, 0xa0, 0x3a0, 0x3, 0x2, 
    0x2, 0x2, 0xa2, 0x3a6, 0x3, 0x2, 0x2, 0x2, 0xa4, 0x3af, 0x3, 0x2, 0x2, 
    0x2, 0xa6, 0x3b9, 0x3, 0x2, 0x2, 0x2, 0xa8, 0x3bf, 0x3, 0x2, 0x2, 0x2, 
    0xaa, 0x3c8, 0x3, 0x2, 0x2, 0x2, 0xac, 0x3d2, 0x3, 0x2, 0x2, 0x2, 0xae, 
    0x3d6, 0x3, 0x2, 0x2, 0x2, 0xb0, 0x3df, 0x3, 0x2, 0x2, 0x2, 0xb2, 0x3e1, 
    0x3, 0x2, 0x2, 0x2, 0xb4, 0x3ea, 0x3, 0x2, 0x2, 0x2, 0xb6, 0x3ec, 0x3, 
    0x2, 0x2, 0x2, 0xb8, 0x3f5, 0x3, 0x2, 0x2, 0x2, 0xba, 0x3f7, 0x3, 0x2, 
    0x2, 0x2, 0xbc, 0x400, 0x3, 0x2, 0x2, 0x2, 0xbe, 0x402, 0x3, 0x2, 0x2, 
    0x2, 0xc0, 0x40b, 0x3, 0x2, 0x2, 0x2, 0xc2, 0x40d, 0x3, 0x2, 0x2, 0x2, 
    0xc4, 0x415, 0x3, 0x2, 0x2, 0x2, 0xc6, 0x41d, 0x3, 0x2, 0x2, 0x2, 0xc8, 
    0x425, 0x3, 0x2, 0x2, 0x2, 0xca, 0x42d, 0x3, 0x2, 0x2, 0x2, 0xcc, 0x435, 
    0x3, 0x2, 0x2, 0x2, 0xce, 0x43e, 0x3, 0x2, 0x2, 0x2, 0xd0, 0x440, 0x3, 
    0x2, 0x2, 0x2, 0xd2, 0x449, 0x3, 0x2, 0x2, 0x2, 0xd4, 0x44b, 0x3, 0x2, 
    0x2, 0x2, 0xd6, 0x453, 0x3, 0x2, 0x2, 0x2, 0xd8, 0x45c, 0x3, 0x2, 0x2, 
    0x2, 0xda, 0x45e, 0x3, 0x2, 0x2, 0x2, 0xdc, 0x467, 0x3, 0x2, 0x2, 0x2, 
    0xde, 0x46c, 0x3, 0x2, 0x2, 0x2, 0xe0, 0x471, 0x3, 0x2, 0x2, 0x2, 0xe2, 
    0x476, 0x3, 0x2, 0x2, 0x2, 0xe4, 0x47b, 0x3, 0x2, 0x2, 0x2, 0xe6, 0x47d, 
    0x3, 0x2, 0x2, 0x2, 0xe8, 0x489, 0x3, 0x2, 0x2, 0x2, 0xea, 0x48b, 0x3, 
    0x2, 0x2, 0x2, 0xec, 0x48f, 0x3, 0x2, 0x2, 0x2, 0xee, 0x492, 0x3, 0x2, 
    0x2, 0x2, 0xf0, 0x498, 0x3, 0x2, 0x2, 0x2, 0xf2, 0x4a0, 0x3, 0x2, 0x2, 
    0x2, 0xf4, 0x4a2, 0x3, 0x2, 0x2, 0x2, 0xf6, 0x4af, 0x3, 0x2, 0x2, 0x2, 
    0xf8, 0x4b1, 0x3, 0x2, 0x2, 0x2, 0xfa, 0x4b4, 0x3, 0x2, 0x2, 0x2, 0xfc, 
    0x4b8, 0x3, 0x2, 0x2, 0x2, 0xfe, 0x4bc, 0x3, 0x2, 0x2, 0x2, 0x100, 0x4c0, 
    0x3, 0x2, 0x2, 0x2, 0x102, 0x4d0, 0x3, 0x2, 0x2, 0x2, 0x104, 0x4d2, 
    0x3, 0x2, 0x2, 0x2, 0x106, 0x4f4, 0x3, 0x2, 0x2, 0x2, 0x108, 0x539, 
    0x3, 0x2, 0x2, 0x2, 0x10a, 0x53b, 0x3, 0x2, 0x2, 0x2, 0x10c, 0x53f, 
    0x3, 0x2, 0x2, 0x2, 0x10e, 0x547, 0x3, 0x2, 0x2, 0x2, 0x110, 0x551, 
    0x3, 0x2, 0x2, 0x2, 0x112, 0x555, 0x3, 0x2, 0x2, 0x2, 0x114, 0x55d, 
    0x3, 0x2, 0x2, 0x2, 0x116, 0x564, 0x3, 0x2, 0x2, 0x2, 0x118, 0x572, 
    0x3, 0x2, 0x2, 0x2, 0x11a, 0x574, 0x3, 0x2, 0x2, 0x2, 0x11c, 0x576, 
    0x3, 0x2, 0x2, 0x2, 0x11e, 0x579, 0x3, 0x2, 0x2, 0x2, 0x120, 0x57d, 
    0x3, 0x2, 0x2, 0x2, 0x122, 0x585, 0x3, 0x2, 0x2, 0x2, 0x124, 0x596, 
    0x3, 0x2, 0x2, 0x2, 0x126, 0x598, 0x3, 0x2, 0x2, 0x2, 0x128, 0x5a0, 
    0x3, 0x2, 0x2, 0x2, 0x12a, 0x5b0, 0x3, 0x2, 0x2, 0x2, 0x12c, 0x5b2, 
    0x3, 0x2, 0x2, 0x2, 0x12e, 0x5c6, 0x3, 0x2, 0x2, 0x2, 0x130, 0x5c8, 
    0x3, 0x2, 0x2, 0x2, 0x132, 0x5cd, 0x3, 0x2, 0x2, 0x2, 0x134, 0x5d2, 
    0x3, 0x2, 0x2, 0x2, 0x136, 0x5da, 0x3, 0x2, 0x2, 0x2, 0x138, 0x5ee, 
    0x3, 0x2, 0x2, 0x2, 0x13a, 0x5fc, 0x3, 0x2, 0x2, 0x2, 0x13c, 0x608, 
    0x3, 0x2, 0x2, 0x2, 0x13e, 0x622, 0x3, 0x2, 0x2, 0x2, 0x140, 0x629, 
    0x3, 0x2, 0x2, 0x2, 0x142, 0x62b, 0x3, 0x2, 0x2, 0x2, 0x144, 0x62d, 
    0x3, 0x2, 0x2, 0x2, 0x146, 0x148, 0x5, 0x1c, 0xf, 0x2, 0x147, 0x146, 
    0x3, 0x2, 0x2, 0x2, 0x148, 0x14b, 0x3, 0x2, 0x2, 0x2, 0x149, 0x147, 
    0x3, 0x2, 0x2, 0x2, 0x149, 0x14a, 0x3, 0x2, 0x2, 0x2, 0x14a, 0x14c, 
    0x3, 0x2, 0x2, 0x2, 0x14b, 0x149, 0x3, 0x2, 0x2, 0x2, 0x14c, 0x14d, 
    0x7, 0x2, 0x2, 0x3, 0x14d, 0x3, 0x3, 0x2, 0x2, 0x2, 0x14e, 0x150, 0x5, 
    0x1a, 0xe, 0x2, 0x14f, 0x14e, 0x3, 0x2, 0x2, 0x2, 0x150, 0x153, 0x3, 
    0x2, 0x2, 0x2, 0x151, 0x14f, 0x3, 0x2, 0x2, 0x2, 0x151, 0x152, 0x3, 
    0x2, 0x2, 0x2, 0x152, 0x154, 0x3, 0x2, 0x2, 0x2, 0x153, 0x151, 0x3, 
    0x2, 0x2, 0x2, 0x154, 0x155, 0x7, 0x2, 0x2, 0x3, 0x155, 0x5, 0x3, 0x2, 
    0x2, 0x2, 0x156, 0x158, 0x5, 0xa, 0x6, 0x2, 0x157, 0x156, 0x3, 0x2, 
    0x2, 0x2, 0x158, 0x15b, 0x3, 0x2, 0x2, 0x2, 0x159, 0x157, 0x3, 0x2, 
    0x2, 0x2, 0x159, 0x15a, 0x3, 0x2, 0x2, 0x2, 0x15a, 0x15c, 0x3, 0x2, 
    0x2, 0x2, 0x15b, 0x159, 0x3, 0x2, 0x2, 0x2, 0x15c, 0x15d, 0x7, 0x2, 
    0x2, 0x3, 0x15d, 0x7, 0x3, 0x2, 0x2, 0x2, 0x15e, 0x160, 0x5, 0xc, 0x7, 
    0x2, 0x15f, 0x15e, 0x3, 0x2, 0x2, 0x2, 0x160, 0x163, 0x3, 0x2, 0x2, 
    0x2, 0x161, 0x15f, 0x3, 0x2, 0x2, 0x2, 0x161, 0x162, 0x3, 0x2, 0x2, 
    0x2, 0x162, 0x164, 0x3, 0x2, 0x2, 0x2, 0x163, 0x161, 0x3, 0x2, 0x2, 
    0x2, 0x164, 0x165, 0x7, 0x2, 0x2, 0x3, 0x165, 0x9, 0x3, 0x2, 0x2, 0x2, 
    0x166, 0x169, 0x5, 0xe, 0x8, 0x2, 0x167, 0x169, 0x5, 0x3c, 0x1f, 0x2, 
    0x168, 0x166, 0x3, 0x2, 0x2, 0x2, 0x168, 0x167, 0x3, 0x2, 0x2, 0x2, 
    0x169, 0xb, 0x3, 0x2, 0x2, 0x2, 0x16a, 0x16d, 0x5, 0x10, 0x9, 0x2, 0x16b, 
    0x16d, 0x5, 0x3e, 0x20, 0x2, 0x16c, 0x16a, 0x3, 0x2, 0x2, 0x2, 0x16c, 
    0x16b, 0x3, 0x2, 0x2, 0x2, 0x16d, 0xd, 0x3, 0x2, 0x2, 0x2, 0x16e, 0x16f, 
    0x7, 0x7e, 0x2, 0x2, 0x16f, 0x171, 0x7, 0x4f, 0x2, 0x2, 0x170, 0x172, 
    0x5, 0x12, 0xa, 0x2, 0x171, 0x170, 0x3, 0x2, 0x2, 0x2, 0x171, 0x172, 
    0x3, 0x2, 0x2, 0x2, 0x172, 0x173, 0x3, 0x2, 0x2, 0x2, 0x173, 0x174, 
    0x7, 0x50, 0x2, 0x2, 0x174, 0x175, 0x7, 0x72, 0x2, 0x2, 0x175, 0xf, 
    0x3, 0x2, 0x2, 0x2, 0x176, 0x177, 0x7, 0x7e, 0x2, 0x2, 0x177, 0x179, 
    0x7, 0x4f, 0x2, 0x2, 0x178, 0x17a, 0x5, 0x14, 0xb, 0x2, 0x179, 0x178, 
    0x3, 0x2, 0x2, 0x2, 0x179, 0x17a, 0x3, 0x2, 0x2, 0x2, 0x17a, 0x17b, 
    0x3, 0x2, 0x2, 0x2, 0x17b, 0x17c, 0x7, 0x50, 0x2, 0x2, 0x17c, 0x17d, 
    0x7, 0x72, 0x2, 0x2, 0x17d, 0x11, 0x3, 0x2, 0x2, 0x2, 0x17e, 0x183, 
    0x5, 0x16, 0xc, 0x2, 0x17f, 0x180, 0x7, 0x73, 0x2, 0x2, 0x180, 0x182, 
    0x5, 0x16, 0xc, 0x2, 0x181, 0x17f, 0x3, 0x2, 0x2, 0x2, 0x182, 0x185, 
    0x3, 0x2, 0x2, 0x2, 0x183, 0x181, 0x3, 0x2, 0x2, 0x2, 0x183, 0x184, 
    0x3, 0x2, 0x2, 0x2, 0x184, 0x13, 0x3, 0x2, 0x2, 0x2, 0x185, 0x183, 0x3, 
    0x2, 0x2, 0x2, 0x186, 0x18b, 0x5, 0x18, 0xd, 0x2, 0x187, 0x188, 0x7, 
    0x73, 0x2, 0x2, 0x188, 0x18a, 0x5, 0x18, 0xd, 0x2, 0x189, 0x187, 0x3, 
    0x2, 0x2, 0x2, 0x18a, 0x18d, 0x3, 0x2, 0x2, 0x2, 0x18b, 0x189, 0x3, 
    0x2, 0x2, 0x2, 0x18b, 0x18c, 0x3, 0x2, 0x2, 0x2, 0x18c, 0x15, 0x3, 0x2, 
    0x2, 0x2, 0x18d, 0x18b, 0x3, 0x2, 0x2, 0x2, 0x18e, 0x191, 0x7, 0x7e, 
    0x2, 0x2, 0x18f, 0x190, 0x7, 0x6e, 0x2, 0x2, 0x190, 0x192, 0x5, 0x106, 
    0x84, 0x2, 0x191, 0x18f, 0x3, 0x2, 0x2, 0x2, 0x191, 0x192, 0x3, 0x2, 
    0x2, 0x2, 0x192, 0x17, 0x3, 0x2, 0x2, 0x2, 0x193, 0x196, 0x7, 0x7e, 
    0x2, 0x2, 0x194, 0x195, 0x7, 0x6e, 0x2, 0x2, 0x195, 0x197, 0x5, 0xae, 
    0x58, 0x2, 0x196, 0x194, 0x3, 0x2, 0x2, 0x2, 0x196, 0x197, 0x3, 0x2, 
    0x2, 0x2, 0x197, 0x19, 0x3, 0x2, 0x2, 0x2, 0x198, 0x19e, 0x5, 0x24, 
    0x13, 0x2, 0x199, 0x19e, 0x5, 0x26, 0x14, 0x2, 0x19a, 0x19e, 0x5, 0x28, 
    0x15, 0x2, 0x19b, 0x19e, 0x5, 0x1e, 0x10, 0x2, 0x19c, 0x19e, 0x5, 0x2e, 
    0x18, 0x2, 0x19d, 0x198, 0x3, 0x2, 0x2, 0x2, 0x19d, 0x199, 0x3, 0x2, 
    0x2, 0x2, 0x19d, 0x19a, 0x3, 0x2, 0x2, 0x2, 0x19d, 0x19b, 0x3, 0x2, 
    0x2, 0x2, 0x19d, 0x19c, 0x3, 0x2, 0x2, 0x2, 0x19e, 0x1b, 0x3, 0x2, 0x2, 
    0x2, 0x19f, 0x1a5, 0x5, 0x24, 0x13, 0x2, 0x1a0, 0x1a5, 0x5, 0x26, 0x14, 
    0x2, 0x1a1, 0x1a5, 0x5, 0x2a, 0x16, 0x2, 0x1a2, 0x1a5, 0x5, 0x20, 0x11, 
    0x2, 0x1a3, 0x1a5, 0x5, 0x2c, 0x17, 0x2, 0x1a4, 0x19f, 0x3, 0x2, 0x2, 
    0x2, 0x1a4, 0x1a0, 0x3, 0x2, 0x2, 0x2, 0x1a4, 0x1a1, 0x3, 0x2, 0x2, 
    0x2, 0x1a4, 0x1a2, 0x3, 0x2, 0x2, 0x2, 0x1a4, 0x1a3, 0x3, 0x2, 0x2, 
    0x2, 0x1a5, 0x1d, 0x3, 0x2, 0x2, 0x2, 0x1a6, 0x1a8, 0x7, 0x14, 0x2, 
    0x2, 0x1a7, 0x1a6, 0x3, 0x2, 0x2, 0x2, 0x1a7, 0x1a8, 0x3, 0x2, 0x2, 
    0x2, 0x1a8, 0x1a9, 0x3, 0x2, 0x2, 0x2, 0x1a9, 0x1aa, 0x7, 0x12, 0x2, 
    0x2, 0x1aa, 0x1ab, 0x7, 0x7e, 0x2, 0x2, 0x1ab, 0x1ac, 0x5, 0xa0, 0x51, 
    0x2, 0x1ac, 0x1ad, 0x5, 0x68, 0x35, 0x2, 0x1ad, 0x1ae, 0x7, 0x13, 0x2, 
    0x2, 0x1ae, 0x1f, 0x3, 0x2, 0x2, 0x2, 0x1af, 0x1b1, 0x7, 0x14, 0x2, 
    0x2, 0x1b0, 0x1af, 0x3, 0x2, 0x2, 0x2, 0x1b0, 0x1b1, 0x3, 0x2, 0x2, 
    0x2, 0x1b1, 0x1b2, 0x3, 0x2, 0x2, 0x2, 0x1b2, 0x1b3, 0x7, 0x12, 0x2, 
    0x2, 0x1b3, 0x1b4, 0x7, 0x7e, 0x2, 0x2, 0x1b4, 0x1b5, 0x5, 0xa6, 0x54, 
    0x2, 0x1b5, 0x1b6, 0x5, 0x6a, 0x36, 0x2, 0x1b6, 0x1b7, 0x7, 0x13, 0x2, 
    0x2, 0x1b7, 0x21, 0x3, 0x2, 0x2, 0x2, 0x1b8, 0x1b9, 0x9, 0x2, 0x2, 0x2, 
    0x1b9, 0x23, 0x3, 0x2, 0x2, 0x2, 0x1ba, 0x1bb, 0x7, 0x15, 0x2, 0x2, 
    0x1bb, 0x1bc, 0x5, 0x22, 0x12, 0x2, 0x1bc, 0x1bd, 0x7, 0x72, 0x2, 0x2, 
    0x1bd, 0x25, 0x3, 0x2, 0x2, 0x2, 0x1be, 0x1bf, 0x7, 0x16, 0x2, 0x2, 
    0x1bf, 0x1c0, 0x5, 0x22, 0x12, 0x2, 0x1c0, 0x1c1, 0x7, 0x72, 0x2, 0x2, 
    0x1c1, 0x27, 0x3, 0x2, 0x2, 0x2, 0x1c2, 0x1c3, 0x7, 0x20, 0x2, 0x2, 
    0x1c3, 0x1c4, 0x7, 0x7e, 0x2, 0x2, 0x1c4, 0x1c5, 0x5, 0x94, 0x4b, 0x2, 
    0x1c5, 0x1c6, 0x5, 0x68, 0x35, 0x2, 0x1c6, 0x1c7, 0x7, 0x21, 0x2, 0x2, 
    0x1c7, 0x29, 0x3, 0x2, 0x2, 0x2, 0x1c8, 0x1c9, 0x7, 0x20, 0x2, 0x2, 
    0x1c9, 0x1ca, 0x7, 0x7e, 0x2, 0x2, 0x1ca, 0x1cb, 0x5, 0x9a, 0x4e, 0x2, 
    0x1cb, 0x1cc, 0x5, 0x6a, 0x36, 0x2, 0x1cc, 0x1cd, 0x7, 0x21, 0x2, 0x2, 
    0x1cd, 0x2b, 0x3, 0x2, 0x2, 0x2, 0x1ce, 0x1e1, 0x5, 0x34, 0x1b, 0x2, 
    0x1cf, 0x1e1, 0x5, 0x3a, 0x1e, 0x2, 0x1d0, 0x1e1, 0x5, 0x3e, 0x20, 0x2, 
    0x1d1, 0x1e1, 0x5, 0x42, 0x22, 0x2, 0x1d2, 0x1e1, 0x5, 0x46, 0x24, 0x2, 
    0x1d3, 0x1e1, 0x5, 0x4a, 0x26, 0x2, 0x1d4, 0x1e1, 0x5, 0x4c, 0x27, 0x2, 
    0x1d5, 0x1e1, 0x5, 0x50, 0x29, 0x2, 0x1d6, 0x1e1, 0x5, 0x52, 0x2a, 0x2, 
    0x1d7, 0x1e1, 0x5, 0x56, 0x2c, 0x2, 0x1d8, 0x1e1, 0x5, 0x5a, 0x2e, 0x2, 
    0x1d9, 0x1e1, 0x5, 0x5e, 0x30, 0x2, 0x1da, 0x1e1, 0x5, 0x62, 0x32, 0x2, 
    0x1db, 0x1e1, 0x5, 0x66, 0x34, 0x2, 0x1dc, 0x1e1, 0x7, 0x72, 0x2, 0x2, 
    0x1dd, 0x1de, 0x5, 0xae, 0x58, 0x2, 0x1de, 0x1df, 0x7, 0x72, 0x2, 0x2, 
    0x1df, 0x1e1, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1ce, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1cf, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1d0, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1d1, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1d2, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1d3, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1d4, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1d5, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1d6, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1d7, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1d8, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1d9, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1da, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1db, 0x3, 0x2, 0x2, 0x2, 0x1e0, 0x1dc, 0x3, 0x2, 0x2, 0x2, 
    0x1e0, 0x1dd, 0x3, 0x2, 0x2, 0x2, 0x1e1, 0x2d, 0x3, 0x2, 0x2, 0x2, 0x1e2, 
    0x1f7, 0x5, 0x32, 0x1a, 0x2, 0x1e3, 0x1f7, 0x5, 0x36, 0x1c, 0x2, 0x1e4, 
    0x1f7, 0x5, 0x38, 0x1d, 0x2, 0x1e5, 0x1f7, 0x5, 0x3c, 0x1f, 0x2, 0x1e6, 
    0x1f7, 0x5, 0x40, 0x21, 0x2, 0x1e7, 0x1f7, 0x5, 0x44, 0x23, 0x2, 0x1e8, 
    0x1f7, 0x5, 0x48, 0x25, 0x2, 0x1e9, 0x1f7, 0x5, 0x4c, 0x27, 0x2, 0x1ea, 
    0x1f7, 0x5, 0x4e, 0x28, 0x2, 0x1eb, 0x1f7, 0x5, 0x50, 0x29, 0x2, 0x1ec, 
    0x1f7, 0x5, 0x52, 0x2a, 0x2, 0x1ed, 0x1f7, 0x5, 0x54, 0x2b, 0x2, 0x1ee, 
    0x1f7, 0x5, 0x58, 0x2d, 0x2, 0x1ef, 0x1f7, 0x5, 0x5c, 0x2f, 0x2, 0x1f0, 
    0x1f7, 0x5, 0x60, 0x31, 0x2, 0x1f1, 0x1f7, 0x5, 0x64, 0x33, 0x2, 0x1f2, 
    0x1f7, 0x7, 0x72, 0x2, 0x2, 0x1f3, 0x1f4, 0x5, 0x106, 0x84, 0x2, 0x1f4, 
    0x1f5, 0x7, 0x72, 0x2, 0x2, 0x1f5, 0x1f7, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1e2, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1e3, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1e4, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1e5, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1e6, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1e7, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1e8, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1e9, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1ea, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1eb, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1ec, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1ed, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1ee, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1ef, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1f0, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1f1, 0x3, 0x2, 0x2, 0x2, 0x1f6, 
    0x1f2, 0x3, 0x2, 0x2, 0x2, 0x1f6, 0x1f3, 0x3, 0x2, 0x2, 0x2, 0x1f7, 
    0x2f, 0x3, 0x2, 0x2, 0x2, 0x1f8, 0x1f9, 0x7, 0x7e, 0x2, 0x2, 0x1f9, 
    0x1fa, 0x7, 0x77, 0x2, 0x2, 0x1fa, 0x31, 0x3, 0x2, 0x2, 0x2, 0x1fb, 
    0x1fc, 0x7, 0x3, 0x2, 0x2, 0x1fc, 0x1fe, 0x5, 0x10a, 0x86, 0x2, 0x1fd, 
    0x1ff, 0x7, 0x4, 0x2, 0x2, 0x1fe, 0x1fd, 0x3, 0x2, 0x2, 0x2, 0x1fe, 
    0x1ff, 0x3, 0x2, 0x2, 0x2, 0x1ff, 0x200, 0x3, 0x2, 0x2, 0x2, 0x200, 
    0x207, 0x5, 0x68, 0x35, 0x2, 0x201, 0x202, 0x7, 0x5, 0x2, 0x2, 0x202, 
    0x203, 0x5, 0x10a, 0x86, 0x2, 0x203, 0x204, 0x5, 0x68, 0x35, 0x2, 0x204, 
    0x206, 0x3, 0x2, 0x2, 0x2, 0x205, 0x201, 0x3, 0x2, 0x2, 0x2, 0x206, 
    0x209, 0x3, 0x2, 0x2, 0x2, 0x207, 0x205, 0x3, 0x2, 0x2, 0x2, 0x207, 
    0x208, 0x3, 0x2, 0x2, 0x2, 0x208, 0x20c, 0x3, 0x2, 0x2, 0x2, 0x209, 
    0x207, 0x3, 0x2, 0x2, 0x2, 0x20a, 0x20b, 0x7, 0x7, 0x2, 0x2, 0x20b, 
    0x20d, 0x5, 0x68, 0x35, 0x2, 0x20c, 0x20a, 0x3, 0x2, 0x2, 0x2, 0x20c, 
    0x20d, 0x3, 0x2, 0x2, 0x2, 0x20d, 0x20e, 0x3, 0x2, 0x2, 0x2, 0x20e, 
    0x20f, 0x7, 0x6, 0x2, 0x2, 0x20f, 0x33, 0x3, 0x2, 0x2, 0x2, 0x210, 0x211, 
    0x7, 0x3, 0x2, 0x2, 0x211, 0x213, 0x5, 0x104, 0x83, 0x2, 0x212, 0x214, 
    0x7, 0x4, 0x2, 0x2, 0x213, 0x212, 0x3, 0x2, 0x2, 0x2, 0x213, 0x214, 
    0x3, 0x2, 0x2, 0x2, 0x214, 0x215, 0x3, 0x2, 0x2, 0x2, 0x215, 0x21c, 
    0x5, 0x6a, 0x36, 0x2, 0x216, 0x217, 0x7, 0x5, 0x2, 0x2, 0x217, 0x218, 
    0x5, 0x104, 0x83, 0x2, 0x218, 0x219, 0x5, 0x6a, 0x36, 0x2, 0x219, 0x21b, 
    0x3, 0x2, 0x2, 0x2, 0x21a, 0x216, 0x3, 0x2, 0x2, 0x2, 0x21b, 0x21e, 
    0x3, 0x2, 0x2, 0x2, 0x21c, 0x21a, 0x3, 0x2, 0x2, 0x2, 0x21c, 0x21d, 
    0x3, 0x2, 0x2, 0x2, 0x21d, 0x221, 0x3, 0x2, 0x2, 0x2, 0x21e, 0x21c, 
    0x3, 0x2, 0x2, 0x2, 0x21f, 0x220, 0x7, 0x7, 0x2, 0x2, 0x220, 0x222, 
    0x5, 0x6a, 0x36, 0x2, 0x221, 0x21f, 0x3, 0x2, 0x2, 0x2, 0x221, 0x222, 
    0x3, 0x2, 0x2, 0x2, 0x222, 0x223, 0x3, 0x2, 0x2, 0x2, 0x223, 0x224, 
    0x7, 0x6, 0x2, 0x2, 0x224, 0x35, 0x3, 0x2, 0x2, 0x2, 0x225, 0x226, 0x7, 
    0x8, 0x2, 0x2, 0x226, 0x227, 0x7, 0x7e, 0x2, 0x2, 0x227, 0x228, 0x7, 
    0x72, 0x2, 0x2, 0x228, 0x37, 0x3, 0x2, 0x2, 0x2, 0x229, 0x22b, 0x7, 
    0x9, 0x2, 0x2, 0x22a, 0x22c, 0x5, 0x106, 0x84, 0x2, 0x22b, 0x22a, 0x3, 
    0x2, 0x2, 0x2, 0x22b, 0x22c, 0x3, 0x2, 0x2, 0x2, 0x22c, 0x22d, 0x3, 
    0x2, 0x2, 0x2, 0x22d, 0x22e, 0x7, 0x72, 0x2, 0x2, 0x22e, 0x39, 0x3, 
    0x2, 0x2, 0x2, 0x22f, 0x231, 0x7, 0x9, 0x2, 0x2, 0x230, 0x232, 0x5, 
    0xae, 0x58, 0x2, 0x231, 0x230, 0x3, 0x2, 0x2, 0x2, 0x231, 0x232, 0x3, 
    0x2, 0x2, 0x2, 0x232, 0x233, 0x3, 0x2, 0x2, 0x2, 0x233, 0x234, 0x7, 
    0x72, 0x2, 0x2, 0x234, 0x3b, 0x3, 0x2, 0x2, 0x2, 0x235, 0x236, 0x7, 
    0xa, 0x2, 0x2, 0x236, 0x237, 0x5, 0x90, 0x49, 0x2, 0x237, 0x238, 0x7, 
    0x72, 0x2, 0x2, 0x238, 0x3d, 0x3, 0x2, 0x2, 0x2, 0x239, 0x23a, 0x7, 
    0xa, 0x2, 0x2, 0x23a, 0x23b, 0x5, 0x92, 0x4a, 0x2, 0x23b, 0x23c, 0x7, 
    0x72, 0x2, 0x2, 0x23c, 0x3f, 0x3, 0x2, 0x2, 0x2, 0x23d, 0x23e, 0x7, 
    0xb, 0x2, 0x2, 0x23e, 0x23f, 0x5, 0x8c, 0x47, 0x2, 0x23f, 0x240, 0x7, 
    0x72, 0x2, 0x2, 0x240, 0x41, 0x3, 0x2, 0x2, 0x2, 0x241, 0x242, 0x7, 
    0xb, 0x2, 0x2, 0x242, 0x243, 0x5, 0x8e, 0x48, 0x2, 0x243, 0x244, 0x7, 
    0x72, 0x2, 0x2, 0x244, 0x43, 0x3, 0x2, 0x2, 0x2, 0x245, 0x247, 0x5, 
    0x30, 0x19, 0x2, 0x246, 0x245, 0x3, 0x2, 0x2, 0x2, 0x246, 0x247, 0x3, 
    0x2, 0x2, 0x2, 0x247, 0x248, 0x3, 0x2, 0x2, 0x2, 0x248, 0x249, 0x7, 
    0xc, 0x2, 0x2, 0x249, 0x24a, 0x5, 0x68, 0x35, 0x2, 0x24a, 0x24b, 0x7, 
    0xd, 0x2, 0x2, 0x24b, 0x24c, 0x5, 0x10a, 0x86, 0x2, 0x24c, 0x24d, 0x7, 
    0x72, 0x2, 0x2, 0x24d, 0x45, 0x3, 0x2, 0x2, 0x2, 0x24e, 0x250, 0x5, 
    0x30, 0x19, 0x2, 0x24f, 0x24e, 0x3, 0x2, 0x2, 0x2, 0x24f, 0x250, 0x3, 
    0x2, 0x2, 0x2, 0x250, 0x251, 0x3, 0x2, 0x2, 0x2, 0x251, 0x252, 0x7, 
    0xc, 0x2, 0x2, 0x252, 0x253, 0x5, 0x6a, 0x36, 0x2, 0x253, 0x254, 0x7, 
    0xd, 0x2, 0x2, 0x254, 0x255, 0x5, 0x104, 0x83, 0x2, 0x255, 0x256, 0x7, 
    0x72, 0x2, 0x2, 0x256, 0x47, 0x3, 0x2, 0x2, 0x2, 0x257, 0x259, 0x5, 
    0x30, 0x19, 0x2, 0x258, 0x257, 0x3, 0x2, 0x2, 0x2, 0x258, 0x259, 0x3, 
    0x2, 0x2, 0x2, 0x259, 0x25a, 0x3, 0x2, 0x2, 0x2, 0x25a, 0x25b, 0x7, 
    0xe, 0x2, 0x2, 0x25b, 0x25c, 0x5, 0x10a, 0x86, 0x2, 0x25c, 0x25d, 0x5, 
    0x68, 0x35, 0x2, 0x25d, 0x25e, 0x7, 0xf, 0x2, 0x2, 0x25e, 0x49, 0x3, 
    0x2, 0x2, 0x2, 0x25f, 0x261, 0x5, 0x30, 0x19, 0x2, 0x260, 0x25f, 0x3, 
    0x2, 0x2, 0x2, 0x260, 0x261, 0x3, 0x2, 0x2, 0x2, 0x261, 0x262, 0x3, 
    0x2, 0x2, 0x2, 0x262, 0x263, 0x7, 0xe, 0x2, 0x2, 0x263, 0x264, 0x5, 
    0x104, 0x83, 0x2, 0x264, 0x265, 0x5, 0x6a, 0x36, 0x2, 0x265, 0x266, 
    0x7, 0xf, 0x2, 0x2, 0x266, 0x4b, 0x3, 0x2, 0x2, 0x2, 0x267, 0x268, 0x7, 
    0x10, 0x2, 0x2, 0x268, 0x269, 0x7, 0x72, 0x2, 0x2, 0x269, 0x4d, 0x3, 
    0x2, 0x2, 0x2, 0x26a, 0x26b, 0x7, 0x11, 0x2, 0x2, 0x26b, 0x26c, 0x7, 
    0x12, 0x2, 0x2, 0x26c, 0x26d, 0x5, 0x8a, 0x46, 0x2, 0x26d, 0x26e, 0x7, 
    0x72, 0x2, 0x2, 0x26e, 0x4f, 0x3, 0x2, 0x2, 0x2, 0x26f, 0x271, 0x7, 
    0x17, 0x2, 0x2, 0x270, 0x272, 0x7, 0x7e, 0x2, 0x2, 0x271, 0x270, 0x3, 
    0x2, 0x2, 0x2, 0x271, 0x272, 0x3, 0x2, 0x2, 0x2, 0x272, 0x273, 0x3, 
    0x2, 0x2, 0x2, 0x273, 0x274, 0x7, 0x72, 0x2, 0x2, 0x274, 0x51, 0x3, 
    0x2, 0x2, 0x2, 0x275, 0x277, 0x7, 0x18, 0x2, 0x2, 0x276, 0x278, 0x7, 
    0x7e, 0x2, 0x2, 0x277, 0x276, 0x3, 0x2, 0x2, 0x2, 0x277, 0x278, 0x3, 
    0x2, 0x2, 0x2, 0x278, 0x279, 0x3, 0x2, 0x2, 0x2, 0x279, 0x27a, 0x7, 
    0x72, 0x2, 0x2, 0x27a, 0x53, 0x3, 0x2, 0x2, 0x2, 0x27b, 0x27d, 0x5, 
    0x30, 0x19, 0x2, 0x27c, 0x27b, 0x3, 0x2, 0x2, 0x2, 0x27c, 0x27d, 0x3, 
    0x2, 0x2, 0x2, 0x27d, 0x27e, 0x3, 0x2, 0x2, 0x2, 0x27e, 0x27f, 0x7, 
    0x19, 0x2, 0x2, 0x27f, 0x280, 0x5, 0x7e, 0x40, 0x2, 0x280, 0x281, 0x7, 
    0x1a, 0x2, 0x2, 0x281, 0x55, 0x3, 0x2, 0x2, 0x2, 0x282, 0x284, 0x5, 
    0x30, 0x19, 0x2, 0x283, 0x282, 0x3, 0x2, 0x2, 0x2, 0x283, 0x284, 0x3, 
    0x2, 0x2, 0x2, 0x284, 0x285, 0x3, 0x2, 0x2, 0x2, 0x285, 0x286, 0x7, 
    0x19, 0x2, 0x2, 0x286, 0x287, 0x5, 0x80, 0x41, 0x2, 0x287, 0x288, 0x7, 
    0x1a, 0x2, 0x2, 0x288, 0x57, 0x3, 0x2, 0x2, 0x2, 0x289, 0x28b, 0x5, 
    0x30, 0x19, 0x2, 0x28a, 0x289, 0x3, 0x2, 0x2, 0x2, 0x28a, 0x28b, 0x3, 
    0x2, 0x2, 0x2, 0x28b, 0x28c, 0x3, 0x2, 0x2, 0x2, 0x28c, 0x28d, 0x7, 
    0x1c, 0x2, 0x2, 0x28d, 0x28e, 0x7, 0x7e, 0x2, 0x2, 0x28e, 0x28f, 0x7, 
    0x45, 0x2, 0x2, 0x28f, 0x290, 0x5, 0x106, 0x84, 0x2, 0x290, 0x291, 0x5, 
    0x68, 0x35, 0x2, 0x291, 0x292, 0x7, 0x1d, 0x2, 0x2, 0x292, 0x59, 0x3, 
    0x2, 0x2, 0x2, 0x293, 0x295, 0x5, 0x30, 0x19, 0x2, 0x294, 0x293, 0x3, 
    0x2, 0x2, 0x2, 0x294, 0x295, 0x3, 0x2, 0x2, 0x2, 0x295, 0x296, 0x3, 
    0x2, 0x2, 0x2, 0x296, 0x297, 0x7, 0x1c, 0x2, 0x2, 0x297, 0x298, 0x7, 
    0x7e, 0x2, 0x2, 0x298, 0x299, 0x7, 0x45, 0x2, 0x2, 0x299, 0x29a, 0x5, 
    0xae, 0x58, 0x2, 0x29a, 0x29b, 0x5, 0x6a, 0x36, 0x2, 0x29b, 0x29c, 0x7, 
    0x1d, 0x2, 0x2, 0x29c, 0x5b, 0x3, 0x2, 0x2, 0x2, 0x29d, 0x29f, 0x5, 
    0x30, 0x19, 0x2, 0x29e, 0x29d, 0x3, 0x2, 0x2, 0x2, 0x29e, 0x29f, 0x3, 
    0x2, 0x2, 0x2, 0x29f, 0x2a0, 0x3, 0x2, 0x2, 0x2, 0x2a0, 0x2a1, 0x7, 
    0x1e, 0x2, 0x2, 0x2a1, 0x2a2, 0x5, 0x68, 0x35, 0x2, 0x2a2, 0x2a3, 0x7, 
    0x1f, 0x2, 0x2, 0x2a3, 0x2a4, 0x5, 0x106, 0x84, 0x2, 0x2a4, 0x2a5, 0x7, 
    0x72, 0x2, 0x2, 0x2a5, 0x5d, 0x3, 0x2, 0x2, 0x2, 0x2a6, 0x2a8, 0x5, 
    0x30, 0x19, 0x2, 0x2a7, 0x2a6, 0x3, 0x2, 0x2, 0x2, 0x2a7, 0x2a8, 0x3, 
    0x2, 0x2, 0x2, 0x2a8, 0x2a9, 0x3, 0x2, 0x2, 0x2, 0x2a9, 0x2aa, 0x7, 
    0x1e, 0x2, 0x2, 0x2aa, 0x2ab, 0x5, 0x6a, 0x36, 0x2, 0x2ab, 0x2ac, 0x7, 
    0x1f, 0x2, 0x2, 0x2ac, 0x2ad, 0x5, 0xae, 0x58, 0x2, 0x2ad, 0x2ae, 0x7, 
    0x72, 0x2, 0x2, 0x2ae, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x2af, 0x2b1, 0x5, 
    0x30, 0x19, 0x2, 0x2b0, 0x2af, 0x3, 0x2, 0x2, 0x2, 0x2b0, 0x2b1, 0x3, 
    0x2, 0x2, 0x2, 0x2b1, 0x2b2, 0x3, 0x2, 0x2, 0x2, 0x2b2, 0x2b3, 0x7, 
    0x22, 0x2, 0x2, 0x2b3, 0x2b4, 0x7, 0x4f, 0x2, 0x2, 0x2b4, 0x2b5, 0x5, 
    0x106, 0x84, 0x2, 0x2b5, 0x2b7, 0x7, 0x50, 0x2, 0x2, 0x2b6, 0x2b8, 0x5, 
    0x78, 0x3d, 0x2, 0x2b7, 0x2b6, 0x3, 0x2, 0x2, 0x2, 0x2b8, 0x2b9, 0x3, 
    0x2, 0x2, 0x2, 0x2b9, 0x2b7, 0x3, 0x2, 0x2, 0x2, 0x2b9, 0x2ba, 0x3, 
    0x2, 0x2, 0x2, 0x2ba, 0x2bb, 0x3, 0x2, 0x2, 0x2, 0x2bb, 0x2bc, 0x7, 
    0x24, 0x2, 0x2, 0x2bc, 0x61, 0x3, 0x2, 0x2, 0x2, 0x2bd, 0x2bf, 0x5, 
    0x30, 0x19, 0x2, 0x2be, 0x2bd, 0x3, 0x2, 0x2, 0x2, 0x2be, 0x2bf, 0x3, 
    0x2, 0x2, 0x2, 0x2bf, 0x2c0, 0x3, 0x2, 0x2, 0x2, 0x2c0, 0x2c1, 0x7, 
    0x22, 0x2, 0x2, 0x2c1, 0x2c2, 0x7, 0x4f, 0x2, 0x2, 0x2c2, 0x2c3, 0x5, 
    0xae, 0x58, 0x2, 0x2c3, 0x2c5, 0x7, 0x50, 0x2, 0x2, 0x2c4, 0x2c6, 0x5, 
    0x7a, 0x3e, 0x2, 0x2c5, 0x2c4, 0x3, 0x2, 0x2, 0x2, 0x2c6, 0x2c7, 0x3, 
    0x2, 0x2, 0x2, 0x2c7, 0x2c5, 0x3, 0x2, 0x2, 0x2, 0x2c7, 0x2c8, 0x3, 
    0x2, 0x2, 0x2, 0x2c8, 0x2c9, 0x3, 0x2, 0x2, 0x2, 0x2c9, 0x2ca, 0x7, 
    0x24, 0x2, 0x2, 0x2ca, 0x63, 0x3, 0x2, 0x2, 0x2, 0x2cb, 0x2cc, 0x7, 
    0x25, 0x2, 0x2, 0x2cc, 0x2cd, 0x7, 0x7e, 0x2, 0x2, 0x2cd, 0x2ce, 0x5, 
    0x70, 0x39, 0x2, 0x2ce, 0x2cf, 0x7, 0x26, 0x2, 0x2, 0x2cf, 0x65, 0x3, 
    0x2, 0x2, 0x2, 0x2d0, 0x2d1, 0x7, 0x25, 0x2, 0x2, 0x2d1, 0x2d2, 0x7, 
    0x7e, 0x2, 0x2, 0x2d2, 0x2d3, 0x5, 0x72, 0x3a, 0x2, 0x2d3, 0x2d4, 0x7, 
    0x26, 0x2, 0x2, 0x2d4, 0x67, 0x3, 0x2, 0x2, 0x2, 0x2d5, 0x2d7, 0x5, 
    0x2e, 0x18, 0x2, 0x2d6, 0x2d5, 0x3, 0x2, 0x2, 0x2, 0x2d7, 0x2da, 0x3, 
    0x2, 0x2, 0x2, 0x2d8, 0x2d6, 0x3, 0x2, 0x2, 0x2, 0x2d8, 0x2d9, 0x3, 
    0x2, 0x2, 0x2, 0x2d9, 0x69, 0x3, 0x2, 0x2, 0x2, 0x2da, 0x2d8, 0x3, 0x2, 
    0x2, 0x2, 0x2db, 0x2dd, 0x5, 0x2c, 0x17, 0x2, 0x2dc, 0x2db, 0x3, 0x2, 
    0x2, 0x2, 0x2dd, 0x2e0, 0x3, 0x2, 0x2, 0x2, 0x2de, 0x2dc, 0x3, 0x2, 
    0x2, 0x2, 0x2de, 0x2df, 0x3, 0x2, 0x2, 0x2, 0x2df, 0x6b, 0x3, 0x2, 0x2, 
    0x2, 0x2e0, 0x2de, 0x3, 0x2, 0x2, 0x2, 0x2e1, 0x2e2, 0x7, 0x6e, 0x2, 
    0x2, 0x2e2, 0x2e5, 0x5, 0x106, 0x84, 0x2, 0x2e3, 0x2e5, 0x7, 0x43, 0x2, 
    0x2, 0x2e4, 0x2e1, 0x3, 0x2, 0x2, 0x2, 0x2e4, 0x2e3, 0x3, 0x2, 0x2, 
    0x2, 0x2e5, 0x6d, 0x3, 0x2, 0x2, 0x2, 0x2e6, 0x2e7, 0x7, 0x6e, 0x2, 
    0x2, 0x2e7, 0x2ee, 0x5, 0xae, 0x58, 0x2, 0x2e8, 0x2e9, 0x7, 0x6c, 0x2, 
    0x2, 0x2e9, 0x2ea, 0x5, 0xae, 0x58, 0x2, 0x2ea, 0x2eb, 0x8, 0x38, 0x1, 
    0x2, 0x2eb, 0x2ee, 0x3, 0x2, 0x2, 0x2, 0x2ec, 0x2ee, 0x7, 0x43, 0x2, 
    0x2, 0x2ed, 0x2e6, 0x3, 0x2, 0x2, 0x2, 0x2ed, 0x2e8, 0x3, 0x2, 0x2, 
    0x2, 0x2ed, 0x2ec, 0x3, 0x2, 0x2, 0x2, 0x2ee, 0x6f, 0x3, 0x2, 0x2, 0x2, 
    0x2ef, 0x2f4, 0x5, 0x74, 0x3b, 0x2, 0x2f0, 0x2f1, 0x7, 0x73, 0x2, 0x2, 
    0x2f1, 0x2f3, 0x5, 0x74, 0x3b, 0x2, 0x2f2, 0x2f0, 0x3, 0x2, 0x2, 0x2, 
    0x2f3, 0x2f6, 0x3, 0x2, 0x2, 0x2, 0x2f4, 0x2f2, 0x3, 0x2, 0x2, 0x2, 
    0x2f4, 0x2f5, 0x3, 0x2, 0x2, 0x2, 0x2f5, 0x2f8, 0x3, 0x2, 0x2, 0x2, 
    0x2f6, 0x2f4, 0x3, 0x2, 0x2, 0x2, 0x2f7, 0x2f9, 0x7, 0x73, 0x2, 0x2, 
    0x2f8, 0x2f7, 0x3, 0x2, 0x2, 0x2, 0x2f8, 0x2f9, 0x3, 0x2, 0x2, 0x2, 
    0x2f9, 0x71, 0x3, 0x2, 0x2, 0x2, 0x2fa, 0x2ff, 0x5, 0x76, 0x3c, 0x2, 
    0x2fb, 0x2fc, 0x7, 0x73, 0x2, 0x2, 0x2fc, 0x2fe, 0x5, 0x76, 0x3c, 0x2, 
    0x2fd, 0x2fb, 0x3, 0x2, 0x2, 0x2, 0x2fe, 0x301, 0x3, 0x2, 0x2, 0x2, 
    0x2ff, 0x2fd, 0x3, 0x2, 0x2, 0x2, 0x2ff, 0x300, 0x3, 0x2, 0x2, 0x2, 
    0x300, 0x303, 0x3, 0x2, 0x2, 0x2, 0x301, 0x2ff, 0x3, 0x2, 0x2, 0x2, 
    0x302, 0x304, 0x7, 0x73, 0x2, 0x2, 0x303, 0x302, 0x3, 0x2, 0x2, 0x2, 
    0x303, 0x304, 0x3, 0x2, 0x2, 0x2, 0x304, 0x73, 0x3, 0x2, 0x2, 0x2, 0x305, 
    0x308, 0x7, 0x7e, 0x2, 0x2, 0x306, 0x307, 0x7, 0x6e, 0x2, 0x2, 0x307, 
    0x309, 0x5, 0x106, 0x84, 0x2, 0x308, 0x306, 0x3, 0x2, 0x2, 0x2, 0x308, 
    0x309, 0x3, 0x2, 0x2, 0x2, 0x309, 0x75, 0x3, 0x2, 0x2, 0x2, 0x30a, 0x30d, 
    0x7, 0x7e, 0x2, 0x2, 0x30b, 0x30c, 0x7, 0x6e, 0x2, 0x2, 0x30c, 0x30e, 
    0x5, 0xae, 0x58, 0x2, 0x30d, 0x30b, 0x3, 0x2, 0x2, 0x2, 0x30d, 0x30e, 
    0x3, 0x2, 0x2, 0x2, 0x30e, 0x77, 0x3, 0x2, 0x2, 0x2, 0x30f, 0x311, 0x5, 
    0x7c, 0x3f, 0x2, 0x310, 0x30f, 0x3, 0x2, 0x2, 0x2, 0x311, 0x312, 0x3, 
    0x2, 0x2, 0x2, 0x312, 0x310, 0x3, 0x2, 0x2, 0x2, 0x312, 0x313, 0x3, 
    0x2, 0x2, 0x2, 0x313, 0x314, 0x3, 0x2, 0x2, 0x2, 0x314, 0x315, 0x5, 
    0x68, 0x35, 0x2, 0x315, 0x79, 0x3, 0x2, 0x2, 0x2, 0x316, 0x31a, 0x5, 
    0x7c, 0x3f, 0x2, 0x317, 0x319, 0x5, 0x7c, 0x3f, 0x2, 0x318, 0x317, 0x3, 
    0x2, 0x2, 0x2, 0x319, 0x31c, 0x3, 0x2, 0x2, 0x2, 0x31a, 0x318, 0x3, 
    0x2, 0x2, 0x2, 0x31a, 0x31b, 0x3, 0x2, 0x2, 0x2, 0x31b, 0x31d, 0x3, 
    0x2, 0x2, 0x2, 0x31c, 0x31a, 0x3, 0x2, 0x2, 0x2, 0x31d, 0x31e, 0x5, 
    0x6a, 0x36, 0x2, 0x31e, 0x7b, 0x3, 0x2, 0x2, 0x2, 0x31f, 0x323, 0x5, 
    0x142, 0xa2, 0x2, 0x320, 0x323, 0x7, 0x7e, 0x2, 0x2, 0x321, 0x323, 0x7, 
    0x4d, 0x2, 0x2, 0x322, 0x31f, 0x3, 0x2, 0x2, 0x2, 0x322, 0x320, 0x3, 
    0x2, 0x2, 0x2, 0x322, 0x321, 0x3, 0x2, 0x2, 0x2, 0x323, 0x324, 0x3, 
    0x2, 0x2, 0x2, 0x324, 0x328, 0x7, 0x77, 0x2, 0x2, 0x325, 0x326, 0x7, 
    0x23, 0x2, 0x2, 0x326, 0x328, 0x7, 0x77, 0x2, 0x2, 0x327, 0x322, 0x3, 
    0x2, 0x2, 0x2, 0x327, 0x325, 0x3, 0x2, 0x2, 0x2, 0x328, 0x7d, 0x3, 0x2, 
    0x2, 0x2, 0x329, 0x32c, 0x5, 0x86, 0x44, 0x2, 0x32a, 0x32c, 0x5, 0x82, 
    0x42, 0x2, 0x32b, 0x329, 0x3, 0x2, 0x2, 0x2, 0x32b, 0x32a, 0x3, 0x2, 
    0x2, 0x2, 0x32c, 0x7f, 0x3, 0x2, 0x2, 0x2, 0x32d, 0x330, 0x5, 0x88, 
    0x45, 0x2, 0x32e, 0x330, 0x5, 0x84, 0x43, 0x2, 0x32f, 0x32d, 0x3, 0x2, 
    0x2, 0x2, 0x32f, 0x32e, 0x3, 0x2, 0x2, 0x2, 0x330, 0x81, 0x3, 0x2, 0x2, 
    0x2, 0x331, 0x332, 0x7, 0x7e, 0x2, 0x2, 0x332, 0x333, 0x7, 0x6e, 0x2, 
    0x2, 0x333, 0x334, 0x5, 0x106, 0x84, 0x2, 0x334, 0x335, 0x7, 0x1b, 0x2, 
    0x2, 0x335, 0x336, 0x5, 0x106, 0x84, 0x2, 0x336, 0x337, 0x5, 0x68, 0x35, 
    0x2, 0x337, 0x83, 0x3, 0x2, 0x2, 0x2, 0x338, 0x339, 0x7, 0x7e, 0x2, 
    0x2, 0x339, 0x33a, 0x7, 0x6e, 0x2, 0x2, 0x33a, 0x33b, 0x5, 0xae, 0x58, 
    0x2, 0x33b, 0x33c, 0x7, 0x1b, 0x2, 0x2, 0x33c, 0x33d, 0x5, 0xae, 0x58, 
    0x2, 0x33d, 0x33e, 0x5, 0x6a, 0x36, 0x2, 0x33e, 0x85, 0x3, 0x2, 0x2, 
    0x2, 0x33f, 0x340, 0x7, 0x4f, 0x2, 0x2, 0x340, 0x341, 0x5, 0x106, 0x84, 
    0x2, 0x341, 0x342, 0x7, 0x72, 0x2, 0x2, 0x342, 0x343, 0x5, 0x106, 0x84, 
    0x2, 0x343, 0x344, 0x7, 0x72, 0x2, 0x2, 0x344, 0x345, 0x5, 0x106, 0x84, 
    0x2, 0x345, 0x346, 0x7, 0x50, 0x2, 0x2, 0x346, 0x347, 0x5, 0x68, 0x35, 
    0x2, 0x347, 0x87, 0x3, 0x2, 0x2, 0x2, 0x348, 0x349, 0x7, 0x4f, 0x2, 
    0x2, 0x349, 0x34a, 0x5, 0xae, 0x58, 0x2, 0x34a, 0x34b, 0x7, 0x72, 0x2, 
    0x2, 0x34b, 0x34c, 0x5, 0xae, 0x58, 0x2, 0x34c, 0x34d, 0x7, 0x72, 0x2, 
    0x2, 0x34d, 0x34e, 0x5, 0xae, 0x58, 0x2, 0x34e, 0x34f, 0x7, 0x50, 0x2, 
    0x2, 0x34f, 0x350, 0x5, 0x6a, 0x36, 0x2, 0x350, 0x89, 0x3, 0x2, 0x2, 
    0x2, 0x351, 0x354, 0x7, 0x7e, 0x2, 0x2, 0x352, 0x353, 0x7, 0x73, 0x2, 
    0x2, 0x353, 0x355, 0x5, 0x8a, 0x46, 0x2, 0x354, 0x352, 0x3, 0x2, 0x2, 
    0x2, 0x354, 0x355, 0x3, 0x2, 0x2, 0x2, 0x355, 0x8b, 0x3, 0x2, 0x2, 0x2, 
    0x356, 0x35b, 0x5, 0x90, 0x49, 0x2, 0x357, 0x358, 0x7, 0x73, 0x2, 0x2, 
    0x358, 0x35a, 0x5, 0x90, 0x49, 0x2, 0x359, 0x357, 0x3, 0x2, 0x2, 0x2, 
    0x35a, 0x35d, 0x3, 0x2, 0x2, 0x2, 0x35b, 0x359, 0x3, 0x2, 0x2, 0x2, 
    0x35b, 0x35c, 0x3, 0x2, 0x2, 0x2, 0x35c, 0x8d, 0x3, 0x2, 0x2, 0x2, 0x35d, 
    0x35b, 0x3, 0x2, 0x2, 0x2, 0x35e, 0x363, 0x5, 0x92, 0x4a, 0x2, 0x35f, 
    0x360, 0x7, 0x73, 0x2, 0x2, 0x360, 0x362, 0x5, 0x92, 0x4a, 0x2, 0x361, 
    0x35f, 0x3, 0x2, 0x2, 0x2, 0x362, 0x365, 0x3, 0x2, 0x2, 0x2, 0x363, 
    0x361, 0x3, 0x2, 0x2, 0x2, 0x363, 0x364, 0x3, 0x2, 0x2, 0x2, 0x364, 
    0x8f, 0x3, 0x2, 0x2, 0x2, 0x365, 0x363, 0x3, 0x2, 0x2, 0x2, 0x366, 0x368, 
    0x7, 0x7e, 0x2, 0x2, 0x367, 0x369, 0x5, 0x6c, 0x37, 0x2, 0x368, 0x367, 
    0x3, 0x2, 0x2, 0x2, 0x368, 0x369, 0x3, 0x2, 0x2, 0x2, 0x369, 0x91, 0x3, 
    0x2, 0x2, 0x2, 0x36a, 0x36c, 0x7, 0x7e, 0x2, 0x2, 0x36b, 0x36d, 0x5, 
    0x6e, 0x38, 0x2, 0x36c, 0x36b, 0x3, 0x2, 0x2, 0x2, 0x36c, 0x36d, 0x3, 
    0x2, 0x2, 0x2, 0x36d, 0x93, 0x3, 0x2, 0x2, 0x2, 0x36e, 0x370, 0x7, 0x4f, 
    0x2, 0x2, 0x36f, 0x371, 0x5, 0x96, 0x4c, 0x2, 0x370, 0x36f, 0x3, 0x2, 
    0x2, 0x2, 0x370, 0x371, 0x3, 0x2, 0x2, 0x2, 0x371, 0x372, 0x3, 0x2, 
    0x2, 0x2, 0x372, 0x373, 0x7, 0x50, 0x2, 0x2, 0x373, 0x95, 0x3, 0x2, 
    0x2, 0x2, 0x374, 0x37b, 0x5, 0x98, 0x4d, 0x2, 0x375, 0x377, 0x7, 0x73, 
    0x2, 0x2, 0x376, 0x375, 0x3, 0x2, 0x2, 0x2, 0x376, 0x377, 0x3, 0x2, 
    0x2, 0x2, 0x377, 0x378, 0x3, 0x2, 0x2, 0x2, 0x378, 0x37a, 0x5, 0x98, 
    0x4d, 0x2, 0x379, 0x376, 0x3, 0x2, 0x2, 0x2, 0x37a, 0x37d, 0x3, 0x2, 
    0x2, 0x2, 0x37b, 0x379, 0x3, 0x2, 0x2, 0x2, 0x37b, 0x37c, 0x3, 0x2, 
    0x2, 0x2, 0x37c, 0x97, 0x3, 0x2, 0x2, 0x2, 0x37d, 0x37b, 0x3, 0x2, 0x2, 
    0x2, 0x37e, 0x37f, 0x7, 0x3e, 0x2, 0x2, 0x37f, 0x386, 0x7, 0x7e, 0x2, 
    0x2, 0x380, 0x383, 0x7, 0x7e, 0x2, 0x2, 0x381, 0x382, 0x7, 0x6e, 0x2, 
    0x2, 0x382, 0x384, 0x5, 0x106, 0x84, 0x2, 0x383, 0x381, 0x3, 0x2, 0x2, 
    0x2, 0x383, 0x384, 0x3, 0x2, 0x2, 0x2, 0x384, 0x386, 0x3, 0x2, 0x2, 
    0x2, 0x385, 0x37e, 0x3, 0x2, 0x2, 0x2, 0x385, 0x380, 0x3, 0x2, 0x2, 
    0x2, 0x386, 0x99, 0x3, 0x2, 0x2, 0x2, 0x387, 0x389, 0x7, 0x4f, 0x2, 
    0x2, 0x388, 0x38a, 0x5, 0x9c, 0x4f, 0x2, 0x389, 0x388, 0x3, 0x2, 0x2, 
    0x2, 0x389, 0x38a, 0x3, 0x2, 0x2, 0x2, 0x38a, 0x38b, 0x3, 0x2, 0x2, 
    0x2, 0x38b, 0x38c, 0x7, 0x50, 0x2, 0x2, 0x38c, 0x9b, 0x3, 0x2, 0x2, 
    0x2, 0x38d, 0x394, 0x5, 0x9e, 0x50, 0x2, 0x38e, 0x390, 0x7, 0x73, 0x2, 
    0x2, 0x38f, 0x38e, 0x3, 0x2, 0x2, 0x2, 0x38f, 0x390, 0x3, 0x2, 0x2, 
    0x2, 0x390, 0x391, 0x3, 0x2, 0x2, 0x2, 0x391, 0x393, 0x5, 0x9e, 0x50, 
    0x2, 0x392, 0x38f, 0x3, 0x2, 0x2, 0x2, 0x393, 0x396, 0x3, 0x2, 0x2, 
    0x2, 0x394, 0x392, 0x3, 0x2, 0x2, 0x2, 0x394, 0x395, 0x3, 0x2, 0x2, 
    0x2, 0x395, 0x9d, 0x3, 0x2, 0x2, 0x2, 0x396, 0x394, 0x3, 0x2, 0x2, 0x2, 
    0x397, 0x398, 0x7, 0x3e, 0x2, 0x2, 0x398, 0x39f, 0x7, 0x7e, 0x2, 0x2, 
    0x399, 0x39c, 0x7, 0x7e, 0x2, 0x2, 0x39a, 0x39b, 0x7, 0x6e, 0x2, 0x2, 
    0x39b, 0x39d, 0x5, 0xae, 0x58, 0x2, 0x39c, 0x39a, 0x3, 0x2, 0x2, 0x2, 
    0x39c, 0x39d, 0x3, 0x2, 0x2, 0x2, 0x39d, 0x39f, 0x3, 0x2, 0x2, 0x2, 
    0x39e, 0x397, 0x3, 0x2, 0x2, 0x2, 0x39e, 0x399, 0x3, 0x2, 0x2, 0x2, 
    0x39f, 0x9f, 0x3, 0x2, 0x2, 0x2, 0x3a0, 0x3a2, 0x7, 0x4f, 0x2, 0x2, 
    0x3a1, 0x3a3, 0x5, 0xa2, 0x52, 0x2, 0x3a2, 0x3a1, 0x3, 0x2, 0x2, 0x2, 
    0x3a2, 0x3a3, 0x3, 0x2, 0x2, 0x2, 0x3a3, 0x3a4, 0x3, 0x2, 0x2, 0x2, 
    0x3a4, 0x3a5, 0x7, 0x50, 0x2, 0x2, 0x3a5, 0xa1, 0x3, 0x2, 0x2, 0x2, 
    0x3a6, 0x3ab, 0x5, 0xa4, 0x53, 0x2, 0x3a7, 0x3a8, 0x7, 0x73, 0x2, 0x2, 
    0x3a8, 0x3aa, 0x5, 0xa4, 0x53, 0x2, 0x3a9, 0x3a7, 0x3, 0x2, 0x2, 0x2, 
    0x3aa, 0x3ad, 0x3, 0x2, 0x2, 0x2, 0x3ab, 0x3a9, 0x3, 0x2, 0x2, 0x2, 
    0x3ab, 0x3ac, 0x3, 0x2, 0x2, 0x2, 0x3ac, 0xa3, 0x3, 0x2, 0x2, 0x2, 0x3ad, 
    0x3ab, 0x3, 0x2, 0x2, 0x2, 0x3ae, 0x3b0, 0x7, 0x3d, 0x2, 0x2, 0x3af, 
    0x3ae, 0x3, 0x2, 0x2, 0x2, 0x3af, 0x3b0, 0x3, 0x2, 0x2, 0x2, 0x3b0, 
    0x3b2, 0x3, 0x2, 0x2, 0x2, 0x3b1, 0x3b3, 0x7, 0x3e, 0x2, 0x2, 0x3b2, 
    0x3b1, 0x3, 0x2, 0x2, 0x2, 0x3b2, 0x3b3, 0x3, 0x2, 0x2, 0x2, 0x3b3, 
    0x3b4, 0x3, 0x2, 0x2, 0x2, 0x3b4, 0x3b7, 0x7, 0x7e, 0x2, 0x2, 0x3b5, 
    0x3b6, 0x7, 0x6e, 0x2, 0x2, 0x3b6, 0x3b8, 0x5, 0x106, 0x84, 0x2, 0x3b7, 
    0x3b5, 0x3, 0x2, 0x2, 0x2, 0x3b7, 0x3b8, 0x3, 0x2, 0x2, 0x2, 0x3b8, 
    0xa5, 0x3, 0x2, 0x2, 0x2, 0x3b9, 0x3bb, 0x7, 0x4f, 0x2, 0x2, 0x3ba, 
    0x3bc, 0x5, 0xa8, 0x55, 0x2, 0x3bb, 0x3ba, 0x3, 0x2, 0x2, 0x2, 0x3bb, 
    0x3bc, 0x3, 0x2, 0x2, 0x2, 0x3bc, 0x3bd, 0x3, 0x2, 0x2, 0x2, 0x3bd, 
    0x3be, 0x7, 0x50, 0x2, 0x2, 0x3be, 0xa7, 0x3, 0x2, 0x2, 0x2, 0x3bf, 
    0x3c4, 0x5, 0xaa, 0x56, 0x2, 0x3c0, 0x3c1, 0x7, 0x73, 0x2, 0x2, 0x3c1, 
    0x3c3, 0x5, 0xaa, 0x56, 0x2, 0x3c2, 0x3c0, 0x3, 0x2, 0x2, 0x2, 0x3c3, 
    0x3c6, 0x3, 0x2, 0x2, 0x2, 0x3c4, 0x3c2, 0x3, 0x2, 0x2, 0x2, 0x3c4, 
    0x3c5, 0x3, 0x2, 0x2, 0x2, 0x3c5, 0xa9, 0x3, 0x2, 0x2, 0x2, 0x3c6, 0x3c4, 
    0x3, 0x2, 0x2, 0x2, 0x3c7, 0x3c9, 0x7, 0x3d, 0x2, 0x2, 0x3c8, 0x3c7, 
    0x3, 0x2, 0x2, 0x2, 0x3c8, 0x3c9, 0x3, 0x2, 0x2, 0x2, 0x3c9, 0x3cb, 
    0x3, 0x2, 0x2, 0x2, 0x3ca, 0x3cc, 0x7, 0x3e, 0x2, 0x2, 0x3cb, 0x3ca, 
    0x3, 0x2, 0x2, 0x2, 0x3cb, 0x3cc, 0x3, 0x2, 0x2, 0x2, 0x3cc, 0x3cd, 
    0x3, 0x2, 0x2, 0x2, 0x3cd, 0x3d0, 0x7, 0x7e, 0x2, 0x2, 0x3ce, 0x3cf, 
    0x7, 0x6e, 0x2, 0x2, 0x3cf, 0x3d1, 0x5, 0xae, 0x58, 0x2, 0x3d0, 0x3ce, 
    0x3, 0x2, 0x2, 0x2, 0x3d0, 0x3d1, 0x3, 0x2, 0x2, 0x2, 0x3d1, 0xab, 0x3, 
    0x2, 0x2, 0x2, 0x3d2, 0x3d3, 0x7, 0x7e, 0x2, 0x2, 0x3d3, 0x3d4, 0x7, 
    0x76, 0x2, 0x2, 0x3d4, 0x3d5, 0x5, 0x114, 0x8b, 0x2, 0x3d5, 0xad, 0x3, 
    0x2, 0x2, 0x2, 0x3d6, 0x3dc, 0x5, 0xb2, 0x5a, 0x2, 0x3d7, 0x3d8, 0x5, 
    0xb0, 0x59, 0x2, 0x3d8, 0x3d9, 0x5, 0xb2, 0x5a, 0x2, 0x3d9, 0x3db, 0x3, 
    0x2, 0x2, 0x2, 0x3da, 0x3d7, 0x3, 0x2, 0x2, 0x2, 0x3db, 0x3de, 0x3, 
    0x2, 0x2, 0x2, 0x3dc, 0x3da, 0x3, 0x2, 0x2, 0x2, 0x3dc, 0x3dd, 0x3, 
    0x2, 0x2, 0x2, 0x3dd, 0xaf, 0x3, 0x2, 0x2, 0x2, 0x3de, 0x3dc, 0x3, 0x2, 
    0x2, 0x2, 0x3df, 0x3e0, 0x9, 0x3, 0x2, 0x2, 0x3e0, 0xb1, 0x3, 0x2, 0x2, 
    0x2, 0x3e1, 0x3e7, 0x5, 0xb6, 0x5c, 0x2, 0x3e2, 0x3e3, 0x5, 0xb4, 0x5b, 
    0x2, 0x3e3, 0x3e4, 0x5, 0xb6, 0x5c, 0x2, 0x3e4, 0x3e6, 0x3, 0x2, 0x2, 
    0x2, 0x3e5, 0x3e2, 0x3, 0x2, 0x2, 0x2, 0x3e6, 0x3e9, 0x3, 0x2, 0x2, 
    0x2, 0x3e7, 0x3e5, 0x3, 0x2, 0x2, 0x2, 0x3e7, 0x3e8, 0x3, 0x2, 0x2, 
    0x2, 0x3e8, 0xb3, 0x3, 0x2, 0x2, 0x2, 0x3e9, 0x3e7, 0x3, 0x2, 0x2, 0x2, 
    0x3ea, 0x3eb, 0x9, 0x4, 0x2, 0x2, 0x3eb, 0xb5, 0x3, 0x2, 0x2, 0x2, 0x3ec, 
    0x3f2, 0x5, 0xba, 0x5e, 0x2, 0x3ed, 0x3ee, 0x5, 0xb8, 0x5d, 0x2, 0x3ee, 
    0x3ef, 0x5, 0xba, 0x5e, 0x2, 0x3ef, 0x3f1, 0x3, 0x2, 0x2, 0x2, 0x3f0, 
    0x3ed, 0x3, 0x2, 0x2, 0x2, 0x3f1, 0x3f4, 0x3, 0x2, 0x2, 0x2, 0x3f2, 
    0x3f0, 0x3, 0x2, 0x2, 0x2, 0x3f2, 0x3f3, 0x3, 0x2, 0x2, 0x2, 0x3f3, 
    0xb7, 0x3, 0x2, 0x2, 0x2, 0x3f4, 0x3f2, 0x3, 0x2, 0x2, 0x2, 0x3f5, 0x3f6, 
    0x9, 0x5, 0x2, 0x2, 0x3f6, 0xb9, 0x3, 0x2, 0x2, 0x2, 0x3f7, 0x3fd, 0x5, 
    0xbe, 0x60, 0x2, 0x3f8, 0x3f9, 0x5, 0xbc, 0x5f, 0x2, 0x3f9, 0x3fa, 0x5, 
    0xbe, 0x60, 0x2, 0x3fa, 0x3fc, 0x3, 0x2, 0x2, 0x2, 0x3fb, 0x3f8, 0x3, 
    0x2, 0x2, 0x2, 0x3fc, 0x3ff, 0x3, 0x2, 0x2, 0x2, 0x3fd, 0x3fb, 0x3, 
    0x2, 0x2, 0x2, 0x3fd, 0x3fe, 0x3, 0x2, 0x2, 0x2, 0x3fe, 0xbb, 0x3, 0x2, 
    0x2, 0x2, 0x3ff, 0x3fd, 0x3, 0x2, 0x2, 0x2, 0x400, 0x401, 0x9, 0x6, 
    0x2, 0x2, 0x401, 0xbd, 0x3, 0x2, 0x2, 0x2, 0x402, 0x408, 0x5, 0xc2, 
    0x62, 0x2, 0x403, 0x404, 0x5, 0xc0, 0x61, 0x2, 0x404, 0x405, 0x5, 0xc2, 
    0x62, 0x2, 0x405, 0x407, 0x3, 0x2, 0x2, 0x2, 0x406, 0x403, 0x3, 0x2, 
    0x2, 0x2, 0x407, 0x40a, 0x3, 0x2, 0x2, 0x2, 0x408, 0x406, 0x3, 0x2, 
    0x2, 0x2, 0x408, 0x409, 0x3, 0x2, 0x2, 0x2, 0x409, 0xbf, 0x3, 0x2, 0x2, 
    0x2, 0x40a, 0x408, 0x3, 0x2, 0x2, 0x2, 0x40b, 0x40c, 0x9, 0x7, 0x2, 
    0x2, 0x40c, 0xc1, 0x3, 0x2, 0x2, 0x2, 0x40d, 0x412, 0x5, 0xc4, 0x63, 
    0x2, 0x40e, 0x40f, 0x7, 0x69, 0x2, 0x2, 0x40f, 0x411, 0x5, 0xc4, 0x63, 
    0x2, 0x410, 0x40e, 0x3, 0x2, 0x2, 0x2, 0x411, 0x414, 0x3, 0x2, 0x2, 
    0x2, 0x412, 0x410, 0x3, 0x2, 0x2, 0x2, 0x412, 0x413, 0x3, 0x2, 0x2, 
    0x2, 0x413, 0xc3, 0x3, 0x2, 0x2, 0x2, 0x414, 0x412, 0x3, 0x2, 0x2, 0x2, 
    0x415, 0x41a, 0x5, 0xc6, 0x64, 0x2, 0x416, 0x417, 0x7, 0x68, 0x2, 0x2, 
    0x417, 0x419, 0x5, 0xc6, 0x64, 0x2, 0x418, 0x416, 0x3, 0x2, 0x2, 0x2, 
    0x419, 0x41c, 0x3, 0x2, 0x2, 0x2, 0x41a, 0x418, 0x3, 0x2, 0x2, 0x2, 
    0x41a, 0x41b, 0x3, 0x2, 0x2, 0x2, 0x41b, 0xc5, 0x3, 0x2, 0x2, 0x2, 0x41c, 
    0x41a, 0x3, 0x2, 0x2, 0x2, 0x41d, 0x422, 0x5, 0xc8, 0x65, 0x2, 0x41e, 
    0x41f, 0x7, 0x67, 0x2, 0x2, 0x41f, 0x421, 0x5, 0xc8, 0x65, 0x2, 0x420, 
    0x41e, 0x3, 0x2, 0x2, 0x2, 0x421, 0x424, 0x3, 0x2, 0x2, 0x2, 0x422, 
    0x420, 0x3, 0x2, 0x2, 0x2, 0x422, 0x423, 0x3, 0x2, 0x2, 0x2, 0x423, 
    0xc7, 0x3, 0x2, 0x2, 0x2, 0x424, 0x422, 0x3, 0x2, 0x2, 0x2, 0x425, 0x42a, 
    0x5, 0xca, 0x66, 0x2, 0x426, 0x427, 0x7, 0x65, 0x2, 0x2, 0x427, 0x429, 
    0x5, 0xca, 0x66, 0x2, 0x428, 0x426, 0x3, 0x2, 0x2, 0x2, 0x429, 0x42c, 
    0x3, 0x2, 0x2, 0x2, 0x42a, 0x428, 0x3, 0x2, 0x2, 0x2, 0x42a, 0x42b, 
    0x3, 0x2, 0x2, 0x2, 0x42b, 0xc9, 0x3, 0x2, 0x2, 0x2, 0x42c, 0x42a, 0x3, 
    0x2, 0x2, 0x2, 0x42d, 0x432, 0x5, 0xcc, 0x67, 0x2, 0x42e, 0x42f, 0x7, 
    0x66, 0x2, 0x2, 0x42f, 0x431, 0x5, 0xcc, 0x67, 0x2, 0x430, 0x42e, 0x3, 
    0x2, 0x2, 0x2, 0x431, 0x434, 0x3, 0x2, 0x2, 0x2, 0x432, 0x430, 0x3, 
    0x2, 0x2, 0x2, 0x432, 0x433, 0x3, 0x2, 0x2, 0x2, 0x433, 0xcb, 0x3, 0x2, 
    0x2, 0x2, 0x434, 0x432, 0x3, 0x2, 0x2, 0x2, 0x435, 0x43b, 0x5, 0xd0, 
    0x69, 0x2, 0x436, 0x437, 0x5, 0xce, 0x68, 0x2, 0x437, 0x438, 0x5, 0xd0, 
    0x69, 0x2, 0x438, 0x43a, 0x3, 0x2, 0x2, 0x2, 0x439, 0x436, 0x3, 0x2, 
    0x2, 0x2, 0x43a, 0x43d, 0x3, 0x2, 0x2, 0x2, 0x43b, 0x439, 0x3, 0x2, 
    0x2, 0x2, 0x43b, 0x43c, 0x3, 0x2, 0x2, 0x2, 0x43c, 0xcd, 0x3, 0x2, 0x2, 
    0x2, 0x43d, 0x43b, 0x3, 0x2, 0x2, 0x2, 0x43e, 0x43f, 0x9, 0x8, 0x2, 
    0x2, 0x43f, 0xcf, 0x3, 0x2, 0x2, 0x2, 0x440, 0x446, 0x5, 0xd4, 0x6b, 
    0x2, 0x441, 0x442, 0x5, 0xd2, 0x6a, 0x2, 0x442, 0x443, 0x5, 0xd4, 0x6b, 
    0x2, 0x443, 0x445, 0x3, 0x2, 0x2, 0x2, 0x444, 0x441, 0x3, 0x2, 0x2, 
    0x2, 0x445, 0x448, 0x3, 0x2, 0x2, 0x2, 0x446, 0x444, 0x3, 0x2, 0x2, 
    0x2, 0x446, 0x447, 0x3, 0x2, 0x2, 0x2, 0x447, 0xd1, 0x3, 0x2, 0x2, 0x2, 
    0x448, 0x446, 0x3, 0x2, 0x2, 0x2, 0x449, 0x44a, 0x7, 0x45, 0x2, 0x2, 
    0x44a, 0xd3, 0x3, 0x2, 0x2, 0x2, 0x44b, 0x450, 0x5, 0xd6, 0x6c, 0x2, 
    0x44c, 0x44d, 0x7, 0x7a, 0x2, 0x2, 0x44d, 0x44f, 0x5, 0xd6, 0x6c, 0x2, 
    0x44e, 0x44c, 0x3, 0x2, 0x2, 0x2, 0x44f, 0x452, 0x3, 0x2, 0x2, 0x2, 
    0x450, 0x44e, 0x3, 0x2, 0x2, 0x2, 0x450, 0x451, 0x3, 0x2, 0x2, 0x2, 
    0x451, 0xd5, 0x3, 0x2, 0x2, 0x2, 0x452, 0x450, 0x3, 0x2, 0x2, 0x2, 0x453, 
    0x459, 0x5, 0xda, 0x6e, 0x2, 0x454, 0x455, 0x5, 0xd8, 0x6d, 0x2, 0x455, 
    0x456, 0x5, 0xda, 0x6e, 0x2, 0x456, 0x458, 0x3, 0x2, 0x2, 0x2, 0x457, 
    0x454, 0x3, 0x2, 0x2, 0x2, 0x458, 0x45b, 0x3, 0x2, 0x2, 0x2, 0x459, 
    0x457, 0x3, 0x2, 0x2, 0x2, 0x459, 0x45a, 0x3, 0x2, 0x2, 0x2, 0x45a, 
    0xd7, 0x3, 0x2, 0x2, 0x2, 0x45b, 0x459, 0x3, 0x2, 0x2, 0x2, 0x45c, 0x45d, 
    0x9, 0x9, 0x2, 0x2, 0x45d, 0xd9, 0x3, 0x2, 0x2, 0x2, 0x45e, 0x464, 0x5, 
    0xde, 0x70, 0x2, 0x45f, 0x460, 0x5, 0xdc, 0x6f, 0x2, 0x460, 0x461, 0x5, 
    0xde, 0x70, 0x2, 0x461, 0x463, 0x3, 0x2, 0x2, 0x2, 0x462, 0x45f, 0x3, 
    0x2, 0x2, 0x2, 0x463, 0x466, 0x3, 0x2, 0x2, 0x2, 0x464, 0x462, 0x3, 
    0x2, 0x2, 0x2, 0x464, 0x465, 0x3, 0x2, 0x2, 0x2, 0x465, 0xdb, 0x3, 0x2, 
    0x2, 0x2, 0x466, 0x464, 0x3, 0x2, 0x2, 0x2, 0x467, 0x468, 0x9, 0xa, 
    0x2, 0x2, 0x468, 0xdd, 0x3, 0x2, 0x2, 0x2, 0x469, 0x46b, 0x5, 0xe0, 
    0x71, 0x2, 0x46a, 0x469, 0x3, 0x2, 0x2, 0x2, 0x46b, 0x46e, 0x3, 0x2, 
    0x2, 0x2, 0x46c, 0x46a, 0x3, 0x2, 0x2, 0x2, 0x46c, 0x46d, 0x3, 0x2, 
    0x2, 0x2, 0x46d, 0x46f, 0x3, 0x2, 0x2, 0x2, 0x46e, 0x46c, 0x3, 0x2, 
    0x2, 0x2, 0x46f, 0x470, 0x5, 0xe2, 0x72, 0x2, 0x470, 0xdf, 0x3, 0x2, 
    0x2, 0x2, 0x471, 0x472, 0x9, 0xb, 0x2, 0x2, 0x472, 0xe1, 0x3, 0x2, 0x2, 
    0x2, 0x473, 0x475, 0x5, 0xe4, 0x73, 0x2, 0x474, 0x473, 0x3, 0x2, 0x2, 
    0x2, 0x475, 0x478, 0x3, 0x2, 0x2, 0x2, 0x476, 0x474, 0x3, 0x2, 0x2, 
    0x2, 0x476, 0x477, 0x3, 0x2, 0x2, 0x2, 0x477, 0x479, 0x3, 0x2, 0x2, 
    0x2, 0x478, 0x476, 0x3, 0x2, 0x2, 0x2, 0x479, 0x47a, 0x5, 0xe6, 0x74, 
    0x2, 0x47a, 0xe3, 0x3, 0x2, 0x2, 0x2, 0x47b, 0x47c, 0x9, 0xc, 0x2, 0x2, 
    0x47c, 0xe5, 0x3, 0x2, 0x2, 0x2, 0x47d, 0x481, 0x5, 0xf6, 0x7c, 0x2, 
    0x47e, 0x480, 0x5, 0xe8, 0x75, 0x2, 0x47f, 0x47e, 0x3, 0x2, 0x2, 0x2, 
    0x480, 0x483, 0x3, 0x2, 0x2, 0x2, 0x481, 0x47f, 0x3, 0x2, 0x2, 0x2, 
    0x481, 0x482, 0x3, 0x2, 0x2, 0x2, 0x482, 0xe7, 0x3, 0x2, 0x2, 0x2, 0x483, 
    0x481, 0x3, 0x2, 0x2, 0x2, 0x484, 0x48a, 0x7, 0x78, 0x2, 0x2, 0x485, 
    0x48a, 0x7, 0x79, 0x2, 0x2, 0x486, 0x48a, 0x5, 0xea, 0x76, 0x2, 0x487, 
    0x48a, 0x5, 0xec, 0x77, 0x2, 0x488, 0x48a, 0x5, 0xf0, 0x79, 0x2, 0x489, 
    0x484, 0x3, 0x2, 0x2, 0x2, 0x489, 0x485, 0x3, 0x2, 0x2, 0x2, 0x489, 
    0x486, 0x3, 0x2, 0x2, 0x2, 0x489, 0x487, 0x3, 0x2, 0x2, 0x2, 0x489, 
    0x488, 0x3, 0x2, 0x2, 0x2, 0x48a, 0xe9, 0x3, 0x2, 0x2, 0x2, 0x48b, 0x48c, 
    0x7, 0x51, 0x2, 0x2, 0x48c, 0x48d, 0x5, 0x10e, 0x88, 0x2, 0x48d, 0x48e, 
    0x7, 0x52, 0x2, 0x2, 0x48e, 0xeb, 0x3, 0x2, 0x2, 0x2, 0x48f, 0x490, 
    0x5, 0xf2, 0x7a, 0x2, 0x490, 0x491, 0x9, 0x2, 0x2, 0x2, 0x491, 0xed, 
    0x3, 0x2, 0x2, 0x2, 0x492, 0x496, 0x5, 0xb4, 0x5b, 0x2, 0x493, 0x497, 
    0x7, 0x7e, 0x2, 0x2, 0x494, 0x497, 0x7, 0x4d, 0x2, 0x2, 0x495, 0x497, 
    0x5, 0xae, 0x58, 0x2, 0x496, 0x493, 0x3, 0x2, 0x2, 0x2, 0x496, 0x494, 
    0x3, 0x2, 0x2, 0x2, 0x496, 0x495, 0x3, 0x2, 0x2, 0x2, 0x497, 0xef, 0x3, 
    0x2, 0x2, 0x2, 0x498, 0x499, 0x5, 0xf2, 0x7a, 0x2, 0x499, 0x49a, 0x7, 
    0x7e, 0x2, 0x2, 0x49a, 0x49c, 0x7, 0x4f, 0x2, 0x2, 0x49b, 0x49d, 0x5, 
    0x10e, 0x88, 0x2, 0x49c, 0x49b, 0x3, 0x2, 0x2, 0x2, 0x49c, 0x49d, 0x3, 
    0x2, 0x2, 0x2, 0x49d, 0x49e, 0x3, 0x2, 0x2, 0x2, 0x49e, 0x49f, 0x7, 
    0x50, 0x2, 0x2, 0x49f, 0xf1, 0x3, 0x2, 0x2, 0x2, 0x4a0, 0x4a1, 0x7, 
    0x55, 0x2, 0x2, 0x4a1, 0xf3, 0x3, 0x2, 0x2, 0x2, 0x4a2, 0x4a3, 0x5, 
    0x118, 0x8d, 0x2, 0x4a3, 0xf5, 0x3, 0x2, 0x2, 0x2, 0x4a4, 0x4b0, 0x5, 
    0x140, 0xa1, 0x2, 0x4a5, 0x4b0, 0x5, 0x104, 0x83, 0x2, 0x4a6, 0x4b0, 
    0x5, 0x11c, 0x8f, 0x2, 0x4a7, 0x4b0, 0x7, 0x7e, 0x2, 0x2, 0x4a8, 0x4b0, 
    0x5, 0xf8, 0x7d, 0x2, 0x4a9, 0x4b0, 0x5, 0x11e, 0x90, 0x2, 0x4aa, 0x4b0, 
    0x5, 0xfa, 0x7e, 0x2, 0x4ab, 0x4b0, 0x5, 0xfc, 0x7f, 0x2, 0x4ac, 0x4b0, 
    0x5, 0xfe, 0x80, 0x2, 0x4ad, 0x4b0, 0x5, 0x100, 0x81, 0x2, 0x4ae, 0x4b0, 
    0x5, 0x102, 0x82, 0x2, 0x4af, 0x4a4, 0x3, 0x2, 0x2, 0x2, 0x4af, 0x4a5, 
    0x3, 0x2, 0x2, 0x2, 0x4af, 0x4a6, 0x3, 0x2, 0x2, 0x2, 0x4af, 0x4a7, 
    0x3, 0x2, 0x2, 0x2, 0x4af, 0x4a8, 0x3, 0x2, 0x2, 0x2, 0x4af, 0x4a9, 
    0x3, 0x2, 0x2, 0x2, 0x4af, 0x4aa, 0x3, 0x2, 0x2, 0x2, 0x4af, 0x4ab, 
    0x3, 0x2, 0x2, 0x2, 0x4af, 0x4ac, 0x3, 0x2, 0x2, 0x2, 0x4af, 0x4ad, 
    0x3, 0x2, 0x2, 0x2, 0x4af, 0x4ae, 0x3, 0x2, 0x2, 0x2, 0x4b0, 0xf7, 0x3, 
    0x2, 0x2, 0x2, 0x4b1, 0x4b2, 0x7, 0x75, 0x2, 0x2, 0x4b2, 0x4b3, 0x7, 
    0x7e, 0x2, 0x2, 0x4b3, 0xf9, 0x3, 0x2, 0x2, 0x2, 0x4b4, 0x4b6, 0x7, 
    0x43, 0x2, 0x2, 0x4b5, 0x4b7, 0x5, 0x13e, 0xa0, 0x2, 0x4b6, 0x4b5, 0x3, 
    0x2, 0x2, 0x2, 0x4b6, 0x4b7, 0x3, 0x2, 0x2, 0x2, 0x4b7, 0xfb, 0x3, 0x2, 
    0x2, 0x2, 0x4b8, 0x4ba, 0x7, 0x42, 0x2, 0x2, 0x4b9, 0x4bb, 0x5, 0x12e, 
    0x98, 0x2, 0x4ba, 0x4b9, 0x3, 0x2, 0x2, 0x2, 0x4ba, 0x4bb, 0x3, 0x2, 
    0x2, 0x2, 0x4bb, 0xfd, 0x3, 0x2, 0x2, 0x2, 0x4bc, 0x4be, 0x7, 0x41, 
    0x2, 0x2, 0x4bd, 0x4bf, 0x5, 0x13a, 0x9e, 0x2, 0x4be, 0x4bd, 0x3, 0x2, 
    0x2, 0x2, 0x4be, 0x4bf, 0x3, 0x2, 0x2, 0x2, 0x4bf, 0xff, 0x3, 0x2, 0x2, 
    0x2, 0x4c0, 0x4c2, 0x7, 0x3f, 0x2, 0x2, 0x4c1, 0x4c3, 0x5, 0x12e, 0x98, 
    0x2, 0x4c2, 0x4c1, 0x3, 0x2, 0x2, 0x2, 0x4c2, 0x4c3, 0x3, 0x2, 0x2, 
    0x2, 0x4c3, 0x101, 0x3, 0x2, 0x2, 0x2, 0x4c4, 0x4c6, 0x7, 0x53, 0x2, 
    0x2, 0x4c5, 0x4c7, 0x5, 0x10e, 0x88, 0x2, 0x4c6, 0x4c5, 0x3, 0x2, 0x2, 
    0x2, 0x4c6, 0x4c7, 0x3, 0x2, 0x2, 0x2, 0x4c7, 0x4c8, 0x3, 0x2, 0x2, 
    0x2, 0x4c8, 0x4d1, 0x7, 0x54, 0x2, 0x2, 0x4c9, 0x4cb, 0x7, 0x53, 0x2, 
    0x2, 0x4ca, 0x4cc, 0x5, 0x10e, 0x88, 0x2, 0x4cb, 0x4ca, 0x3, 0x2, 0x2, 
    0x2, 0x4cb, 0x4cc, 0x3, 0x2, 0x2, 0x2, 0x4cc, 0x4cd, 0x3, 0x2, 0x2, 
    0x2, 0x4cd, 0x4ce, 0x7, 0x73, 0x2, 0x2, 0x4ce, 0x4cf, 0x7, 0x54, 0x2, 
    0x2, 0x4cf, 0x4d1, 0x8, 0x82, 0x1, 0x2, 0x4d0, 0x4c4, 0x3, 0x2, 0x2, 
    0x2, 0x4d0, 0x4c9, 0x3, 0x2, 0x2, 0x2, 0x4d1, 0x103, 0x3, 0x2, 0x2, 
    0x2, 0x4d2, 0x4d3, 0x7, 0x4f, 0x2, 0x2, 0x4d3, 0x4d4, 0x5, 0xae, 0x58, 
    0x2, 0x4d4, 0x4d5, 0x7, 0x50, 0x2, 0x2, 0x4d5, 0x105, 0x3, 0x2, 0x2, 
    0x2, 0x4d6, 0x4d7, 0x8, 0x84, 0x1, 0x2, 0x4d7, 0x4f5, 0x5, 0x108, 0x85, 
    0x2, 0x4d8, 0x4f5, 0x5, 0x114, 0x8b, 0x2, 0x4d9, 0x4f5, 0x5, 0xac, 0x57, 
    0x2, 0x4da, 0x4dc, 0x7, 0x43, 0x2, 0x2, 0x4db, 0x4dd, 0x5, 0x13c, 0x9f, 
    0x2, 0x4dc, 0x4db, 0x3, 0x2, 0x2, 0x2, 0x4dc, 0x4dd, 0x3, 0x2, 0x2, 
    0x2, 0x4dd, 0x4f5, 0x3, 0x2, 0x2, 0x2, 0x4de, 0x4e0, 0x7, 0x42, 0x2, 
    0x2, 0x4df, 0x4e1, 0x5, 0x128, 0x95, 0x2, 0x4e0, 0x4df, 0x3, 0x2, 0x2, 
    0x2, 0x4e0, 0x4e1, 0x3, 0x2, 0x2, 0x2, 0x4e1, 0x4f5, 0x3, 0x2, 0x2, 
    0x2, 0x4e2, 0x4e4, 0x7, 0x41, 0x2, 0x2, 0x4e3, 0x4e5, 0x5, 0x138, 0x9d, 
    0x2, 0x4e4, 0x4e3, 0x3, 0x2, 0x2, 0x2, 0x4e4, 0x4e5, 0x3, 0x2, 0x2, 
    0x2, 0x4e5, 0x4f5, 0x3, 0x2, 0x2, 0x2, 0x4e6, 0x4e8, 0x7, 0x3f, 0x2, 
    0x2, 0x4e7, 0x4e9, 0x5, 0x128, 0x95, 0x2, 0x4e8, 0x4e7, 0x3, 0x2, 0x2, 
    0x2, 0x4e8, 0x4e9, 0x3, 0x2, 0x2, 0x2, 0x4e9, 0x4f5, 0x3, 0x2, 0x2, 
    0x2, 0x4ea, 0x4ec, 0x7, 0x53, 0x2, 0x2, 0x4eb, 0x4ed, 0x5, 0x10c, 0x87, 
    0x2, 0x4ec, 0x4eb, 0x3, 0x2, 0x2, 0x2, 0x4ec, 0x4ed, 0x3, 0x2, 0x2, 
    0x2, 0x4ed, 0x4ee, 0x3, 0x2, 0x2, 0x2, 0x4ee, 0x4f5, 0x7, 0x54, 0x2, 
    0x2, 0x4ef, 0x4f5, 0x5, 0xf8, 0x7d, 0x2, 0x4f0, 0x4f1, 0x9, 0xc, 0x2, 
    0x2, 0x4f1, 0x4f5, 0x5, 0x106, 0x84, 0x12, 0x4f2, 0x4f3, 0x9, 0xb, 0x2, 
    0x2, 0x4f3, 0x4f5, 0x5, 0x106, 0x84, 0x11, 0x4f4, 0x4d6, 0x3, 0x2, 0x2, 
    0x2, 0x4f4, 0x4d8, 0x3, 0x2, 0x2, 0x2, 0x4f4, 0x4d9, 0x3, 0x2, 0x2, 
    0x2, 0x4f4, 0x4da, 0x3, 0x2, 0x2, 0x2, 0x4f4, 0x4de, 0x3, 0x2, 0x2, 
    0x2, 0x4f4, 0x4e2, 0x3, 0x2, 0x2, 0x2, 0x4f4, 0x4e6, 0x3, 0x2, 0x2, 
    0x2, 0x4f4, 0x4ea, 0x3, 0x2, 0x2, 0x2, 0x4f4, 0x4ef, 0x3, 0x2, 0x2, 
    0x2, 0x4f4, 0x4f0, 0x3, 0x2, 0x2, 0x2, 0x4f4, 0x4f2, 0x3, 0x2, 0x2, 
    0x2, 0x4f5, 0x530, 0x3, 0x2, 0x2, 0x2, 0x4f6, 0x4f7, 0xc, 0x10, 0x2, 
    0x2, 0x4f7, 0x4f8, 0x9, 0xa, 0x2, 0x2, 0x4f8, 0x52f, 0x5, 0x106, 0x84, 
    0x11, 0x4f9, 0x4fa, 0xc, 0xf, 0x2, 0x2, 0x4fa, 0x4fb, 0x9, 0x9, 0x2, 
    0x2, 0x4fb, 0x52f, 0x5, 0x106, 0x84, 0x10, 0x4fc, 0x4fd, 0xc, 0xe, 0x2, 
    0x2, 0x4fd, 0x4fe, 0x9, 0xd, 0x2, 0x2, 0x4fe, 0x52f, 0x5, 0x106, 0x84, 
    0xf, 0x4ff, 0x500, 0xc, 0xd, 0x2, 0x2, 0x500, 0x501, 0x7, 0x7a, 0x2, 
    0x2, 0x501, 0x52f, 0x5, 0x106, 0x84, 0xe, 0x502, 0x503, 0xc, 0xc, 0x2, 
    0x2, 0x503, 0x504, 0x9, 0x8, 0x2, 0x2, 0x504, 0x52f, 0x5, 0x106, 0x84, 
    0xd, 0x505, 0x506, 0xc, 0xb, 0x2, 0x2, 0x506, 0x507, 0x9, 0x7, 0x2, 
    0x2, 0x507, 0x52f, 0x5, 0x106, 0x84, 0xc, 0x508, 0x509, 0xc, 0xa, 0x2, 
    0x2, 0x509, 0x50a, 0x7, 0x67, 0x2, 0x2, 0x50a, 0x52f, 0x5, 0x106, 0x84, 
    0xb, 0x50b, 0x50c, 0xc, 0x9, 0x2, 0x2, 0x50c, 0x50d, 0x7, 0x68, 0x2, 
    0x2, 0x50d, 0x52f, 0x5, 0x106, 0x84, 0xa, 0x50e, 0x50f, 0xc, 0x8, 0x2, 
    0x2, 0x50f, 0x510, 0x7, 0x69, 0x2, 0x2, 0x510, 0x52f, 0x5, 0x106, 0x84, 
    0x9, 0x511, 0x512, 0xc, 0x7, 0x2, 0x2, 0x512, 0x513, 0x7, 0x45, 0x2, 
    0x2, 0x513, 0x52f, 0x5, 0x106, 0x84, 0x8, 0x514, 0x515, 0xc, 0x6, 0x2, 
    0x2, 0x515, 0x516, 0x9, 0x6, 0x2, 0x2, 0x516, 0x52f, 0x5, 0x106, 0x84, 
    0x7, 0x517, 0x518, 0xc, 0x5, 0x2, 0x2, 0x518, 0x519, 0x9, 0x5, 0x2, 
    0x2, 0x519, 0x52f, 0x5, 0x106, 0x84, 0x6, 0x51a, 0x51b, 0xc, 0x4, 0x2, 
    0x2, 0x51b, 0x51c, 0x9, 0x4, 0x2, 0x2, 0x51c, 0x52f, 0x5, 0x106, 0x84, 
    0x4, 0x51d, 0x51e, 0xc, 0x3, 0x2, 0x2, 0x51e, 0x51f, 0x9, 0x3, 0x2, 
    0x2, 0x51f, 0x52f, 0x5, 0x106, 0x84, 0x3, 0x520, 0x521, 0xc, 0x1d, 0x2, 
    0x2, 0x521, 0x525, 0x7, 0x55, 0x2, 0x2, 0x522, 0x526, 0x7, 0x7e, 0x2, 
    0x2, 0x523, 0x526, 0x7, 0x4d, 0x2, 0x2, 0x524, 0x526, 0x5, 0x122, 0x92, 
    0x2, 0x525, 0x522, 0x3, 0x2, 0x2, 0x2, 0x525, 0x523, 0x3, 0x2, 0x2, 
    0x2, 0x525, 0x524, 0x3, 0x2, 0x2, 0x2, 0x526, 0x52f, 0x3, 0x2, 0x2, 
    0x2, 0x527, 0x528, 0xc, 0x1c, 0x2, 0x2, 0x528, 0x529, 0x7, 0x51, 0x2, 
    0x2, 0x529, 0x52a, 0x5, 0x10c, 0x87, 0x2, 0x52a, 0x52b, 0x7, 0x52, 0x2, 
    0x2, 0x52b, 0x52f, 0x3, 0x2, 0x2, 0x2, 0x52c, 0x52d, 0xc, 0x13, 0x2, 
    0x2, 0x52d, 0x52f, 0x9, 0xe, 0x2, 0x2, 0x52e, 0x4f6, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x4f9, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x4fc, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x4ff, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x502, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x505, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x508, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x50b, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x50e, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x511, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x514, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x517, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x51a, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x51d, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x520, 0x3, 0x2, 0x2, 
    0x2, 0x52e, 0x527, 0x3, 0x2, 0x2, 0x2, 0x52e, 0x52c, 0x3, 0x2, 0x2, 
    0x2, 0x52f, 0x532, 0x3, 0x2, 0x2, 0x2, 0x530, 0x52e, 0x3, 0x2, 0x2, 
    0x2, 0x530, 0x531, 0x3, 0x2, 0x2, 0x2, 0x531, 0x107, 0x3, 0x2, 0x2, 
    0x2, 0x532, 0x530, 0x3, 0x2, 0x2, 0x2, 0x533, 0x534, 0x7, 0x4f, 0x2, 
    0x2, 0x534, 0x535, 0x5, 0x106, 0x84, 0x2, 0x535, 0x536, 0x7, 0x50, 0x2, 
    0x2, 0x536, 0x53a, 0x3, 0x2, 0x2, 0x2, 0x537, 0x53a, 0x5, 0x140, 0xa1, 
    0x2, 0x538, 0x53a, 0x7, 0x7e, 0x2, 0x2, 0x539, 0x533, 0x3, 0x2, 0x2, 
    0x2, 0x539, 0x537, 0x3, 0x2, 0x2, 0x2, 0x539, 0x538, 0x3, 0x2, 0x2, 
    0x2, 0x53a, 0x109, 0x3, 0x2, 0x2, 0x2, 0x53b, 0x53c, 0x7, 0x4f, 0x2, 
    0x2, 0x53c, 0x53d, 0x5, 0x106, 0x84, 0x2, 0x53d, 0x53e, 0x7, 0x50, 0x2, 
    0x2, 0x53e, 0x10b, 0x3, 0x2, 0x2, 0x2, 0x53f, 0x544, 0x5, 0x106, 0x84, 
    0x2, 0x540, 0x541, 0x7, 0x73, 0x2, 0x2, 0x541, 0x543, 0x5, 0x106, 0x84, 
    0x2, 0x542, 0x540, 0x3, 0x2, 0x2, 0x2, 0x543, 0x546, 0x3, 0x2, 0x2, 
    0x2, 0x544, 0x542, 0x3, 0x2, 0x2, 0x2, 0x544, 0x545, 0x3, 0x2, 0x2, 
    0x2, 0x545, 0x10d, 0x3, 0x2, 0x2, 0x2, 0x546, 0x544, 0x3, 0x2, 0x2, 
    0x2, 0x547, 0x54c, 0x5, 0xae, 0x58, 0x2, 0x548, 0x549, 0x7, 0x73, 0x2, 
    0x2, 0x549, 0x54b, 0x5, 0xae, 0x58, 0x2, 0x54a, 0x548, 0x3, 0x2, 0x2, 
    0x2, 0x54b, 0x54e, 0x3, 0x2, 0x2, 0x2, 0x54c, 0x54a, 0x3, 0x2, 0x2, 
    0x2, 0x54c, 0x54d, 0x3, 0x2, 0x2, 0x2, 0x54d, 0x10f, 0x3, 0x2, 0x2, 
    0x2, 0x54e, 0x54c, 0x3, 0x2, 0x2, 0x2, 0x54f, 0x550, 0x7, 0x7e, 0x2, 
    0x2, 0x550, 0x552, 0x7, 0x6e, 0x2, 0x2, 0x551, 0x54f, 0x3, 0x2, 0x2, 
    0x2, 0x551, 0x552, 0x3, 0x2, 0x2, 0x2, 0x552, 0x553, 0x3, 0x2, 0x2, 
    0x2, 0x553, 0x554, 0x5, 0x106, 0x84, 0x2, 0x554, 0x111, 0x3, 0x2, 0x2, 
    0x2, 0x555, 0x55a, 0x5, 0x110, 0x89, 0x2, 0x556, 0x557, 0x7, 0x73, 0x2, 
    0x2, 0x557, 0x559, 0x5, 0x110, 0x89, 0x2, 0x558, 0x556, 0x3, 0x2, 0x2, 
    0x2, 0x559, 0x55c, 0x3, 0x2, 0x2, 0x2, 0x55a, 0x558, 0x3, 0x2, 0x2, 
    0x2, 0x55a, 0x55b, 0x3, 0x2, 0x2, 0x2, 0x55b, 0x113, 0x3, 0x2, 0x2, 
    0x2, 0x55c, 0x55a, 0x3, 0x2, 0x2, 0x2, 0x55d, 0x55e, 0x7, 0x7e, 0x2, 
    0x2, 0x55e, 0x560, 0x7, 0x4f, 0x2, 0x2, 0x55f, 0x561, 0x5, 0x112, 0x8a, 
    0x2, 0x560, 0x55f, 0x3, 0x2, 0x2, 0x2, 0x560, 0x561, 0x3, 0x2, 0x2, 
    0x2, 0x561, 0x562, 0x3, 0x2, 0x2, 0x2, 0x562, 0x563, 0x7, 0x50, 0x2, 
    0x2, 0x563, 0x115, 0x3, 0x2, 0x2, 0x2, 0x564, 0x569, 0x5, 0x11a, 0x8e, 
    0x2, 0x565, 0x566, 0x7, 0x73, 0x2, 0x2, 0x566, 0x568, 0x5, 0x11a, 0x8e, 
    0x2, 0x567, 0x565, 0x3, 0x2, 0x2, 0x2, 0x568, 0x56b, 0x3, 0x2, 0x2, 
    0x2, 0x569, 0x567, 0x3, 0x2, 0x2, 0x2, 0x569, 0x56a, 0x3, 0x2, 0x2, 
    0x2, 0x56a, 0x117, 0x3, 0x2, 0x2, 0x2, 0x56b, 0x569, 0x3, 0x2, 0x2, 
    0x2, 0x56c, 0x56d, 0x7, 0x4f, 0x2, 0x2, 0x56d, 0x56e, 0x5, 0x120, 0x91, 
    0x2, 0x56e, 0x56f, 0x7, 0x50, 0x2, 0x2, 0x56f, 0x573, 0x3, 0x2, 0x2, 
    0x2, 0x570, 0x571, 0x7, 0x4f, 0x2, 0x2, 0x571, 0x573, 0x7, 0x50, 0x2, 
    0x2, 0x572, 0x56c, 0x3, 0x2, 0x2, 0x2, 0x572, 0x570, 0x3, 0x2, 0x2, 
    0x2, 0x573, 0x119, 0x3, 0x2, 0x2, 0x2, 0x574, 0x575, 0x5, 0xae, 0x58, 
    0x2, 0x575, 0x11b, 0x3, 0x2, 0x2, 0x2, 0x576, 0x577, 0x7, 0x7e, 0x2, 
    0x2, 0x577, 0x578, 0x5, 0x118, 0x8d, 0x2, 0x578, 0x11d, 0x3, 0x2, 0x2, 
    0x2, 0x579, 0x57a, 0x7, 0x7e, 0x2, 0x2, 0x57a, 0x57b, 0x7, 0x76, 0x2, 
    0x2, 0x57b, 0x57c, 0x5, 0x11c, 0x8f, 0x2, 0x57c, 0x11f, 0x3, 0x2, 0x2, 
    0x2, 0x57d, 0x582, 0x5, 0x11a, 0x8e, 0x2, 0x57e, 0x57f, 0x7, 0x73, 0x2, 
    0x2, 0x57f, 0x581, 0x5, 0x11a, 0x8e, 0x2, 0x580, 0x57e, 0x3, 0x2, 0x2, 
    0x2, 0x581, 0x584, 0x3, 0x2, 0x2, 0x2, 0x582, 0x580, 0x3, 0x2, 0x2, 
    0x2, 0x582, 0x583, 0x3, 0x2, 0x2, 0x2, 0x583, 0x121, 0x3, 0x2, 0x2, 
    0x2, 0x584, 0x582, 0x3, 0x2, 0x2, 0x2, 0x585, 0x586, 0x7, 0x7e, 0x2, 
    0x2, 0x586, 0x588, 0x7, 0x4f, 0x2, 0x2, 0x587, 0x589, 0x5, 0x10c, 0x87, 
    0x2, 0x588, 0x587, 0x3, 0x2, 0x2, 0x2, 0x588, 0x589, 0x3, 0x2, 0x2, 
    0x2, 0x589, 0x58a, 0x3, 0x2, 0x2, 0x2, 0x58a, 0x58b, 0x7, 0x50, 0x2, 
    0x2, 0x58b, 0x123, 0x3, 0x2, 0x2, 0x2, 0x58c, 0x58f, 0x7, 0x7e, 0x2, 
    0x2, 0x58d, 0x58e, 0x7, 0x6e, 0x2, 0x2, 0x58e, 0x590, 0x5, 0x106, 0x84, 
    0x2, 0x58f, 0x58d, 0x3, 0x2, 0x2, 0x2, 0x58f, 0x590, 0x3, 0x2, 0x2, 
    0x2, 0x590, 0x597, 0x3, 0x2, 0x2, 0x2, 0x591, 0x594, 0x7, 0x4d, 0x2, 
    0x2, 0x592, 0x593, 0x7, 0x6e, 0x2, 0x2, 0x593, 0x595, 0x5, 0x106, 0x84, 
    0x2, 0x594, 0x592, 0x3, 0x2, 0x2, 0x2, 0x594, 0x595, 0x3, 0x2, 0x2, 
    0x2, 0x595, 0x597, 0x3, 0x2, 0x2, 0x2, 0x596, 0x58c, 0x3, 0x2, 0x2, 
    0x2, 0x596, 0x591, 0x3, 0x2, 0x2, 0x2, 0x597, 0x125, 0x3, 0x2, 0x2, 
    0x2, 0x598, 0x59d, 0x5, 0x124, 0x93, 0x2, 0x599, 0x59a, 0x7, 0x73, 0x2, 
    0x2, 0x59a, 0x59c, 0x5, 0x124, 0x93, 0x2, 0x59b, 0x599, 0x3, 0x2, 0x2, 
    0x2, 0x59c, 0x59f, 0x3, 0x2, 0x2, 0x2, 0x59d, 0x59b, 0x3, 0x2, 0x2, 
    0x2, 0x59d, 0x59e, 0x3, 0x2, 0x2, 0x2, 0x59e, 0x127, 0x3, 0x2, 0x2, 
    0x2, 0x59f, 0x59d, 0x3, 0x2, 0x2, 0x2, 0x5a0, 0x5a2, 0x7, 0x53, 0x2, 
    0x2, 0x5a1, 0x5a3, 0x5, 0x126, 0x94, 0x2, 0x5a2, 0x5a1, 0x3, 0x2, 0x2, 
    0x2, 0x5a2, 0x5a3, 0x3, 0x2, 0x2, 0x2, 0x5a3, 0x5a4, 0x3, 0x2, 0x2, 
    0x2, 0x5a4, 0x5a5, 0x7, 0x54, 0x2, 0x2, 0x5a5, 0x129, 0x3, 0x2, 0x2, 
    0x2, 0x5a6, 0x5a9, 0x7, 0x7e, 0x2, 0x2, 0x5a7, 0x5a8, 0x7, 0x6e, 0x2, 
    0x2, 0x5a8, 0x5aa, 0x5, 0xae, 0x58, 0x2, 0x5a9, 0x5a7, 0x3, 0x2, 0x2, 
    0x2, 0x5a9, 0x5aa, 0x3, 0x2, 0x2, 0x2, 0x5aa, 0x5b1, 0x3, 0x2, 0x2, 
    0x2, 0x5ab, 0x5ae, 0x7, 0x4d, 0x2, 0x2, 0x5ac, 0x5ad, 0x7, 0x6e, 0x2, 
    0x2, 0x5ad, 0x5af, 0x5, 0xae, 0x58, 0x2, 0x5ae, 0x5ac, 0x3, 0x2, 0x2, 
    0x2, 0x5ae, 0x5af, 0x3, 0x2, 0x2, 0x2, 0x5af, 0x5b1, 0x3, 0x2, 0x2, 
    0x2, 0x5b0, 0x5a6, 0x3, 0x2, 0x2, 0x2, 0x5b0, 0x5ab, 0x3, 0x2, 0x2, 
    0x2, 0x5b1, 0x12b, 0x3, 0x2, 0x2, 0x2, 0x5b2, 0x5b7, 0x5, 0x12a, 0x96, 
    0x2, 0x5b3, 0x5b4, 0x7, 0x73, 0x2, 0x2, 0x5b4, 0x5b6, 0x5, 0x12a, 0x96, 
    0x2, 0x5b5, 0x5b3, 0x3, 0x2, 0x2, 0x2, 0x5b6, 0x5b9, 0x3, 0x2, 0x2, 
    0x2, 0x5b7, 0x5b5, 0x3, 0x2, 0x2, 0x2, 0x5b7, 0x5b8, 0x3, 0x2, 0x2, 
    0x2, 0x5b8, 0x12d, 0x3, 0x2, 0x2, 0x2, 0x5b9, 0x5b7, 0x3, 0x2, 0x2, 
    0x2, 0x5ba, 0x5bc, 0x7, 0x53, 0x2, 0x2, 0x5bb, 0x5bd, 0x5, 0x12c, 0x97, 
    0x2, 0x5bc, 0x5bb, 0x3, 0x2, 0x2, 0x2, 0x5bc, 0x5bd, 0x3, 0x2, 0x2, 
    0x2, 0x5bd, 0x5be, 0x3, 0x2, 0x2, 0x2, 0x5be, 0x5c7, 0x7, 0x54, 0x2, 
    0x2, 0x5bf, 0x5c1, 0x7, 0x53, 0x2, 0x2, 0x5c0, 0x5c2, 0x5, 0x12c, 0x97, 
    0x2, 0x5c1, 0x5c0, 0x3, 0x2, 0x2, 0x2, 0x5c1, 0x5c2, 0x3, 0x2, 0x2, 
    0x2, 0x5c2, 0x5c3, 0x3, 0x2, 0x2, 0x2, 0x5c3, 0x5c4, 0x7, 0x73, 0x2, 
    0x2, 0x5c4, 0x5c5, 0x7, 0x54, 0x2, 0x2, 0x5c5, 0x5c7, 0x8, 0x98, 0x1, 
    0x2, 0x5c6, 0x5ba, 0x3, 0x2, 0x2, 0x2, 0x5c6, 0x5bf, 0x3, 0x2, 0x2, 
    0x2, 0x5c7, 0x12f, 0x3, 0x2, 0x2, 0x2, 0x5c8, 0x5cb, 0x5, 0x106, 0x84, 
    0x2, 0x5c9, 0x5ca, 0x7, 0x56, 0x2, 0x2, 0x5ca, 0x5cc, 0x5, 0x106, 0x84, 
    0x2, 0x5cb, 0x5c9, 0x3, 0x2, 0x2, 0x2, 0x5cb, 0x5cc, 0x3, 0x2, 0x2, 
    0x2, 0x5cc, 0x131, 0x3, 0x2, 0x2, 0x2, 0x5cd, 0x5d0, 0x5, 0xae, 0x58, 
    0x2, 0x5ce, 0x5cf, 0x7, 0x56, 0x2, 0x2, 0x5cf, 0x5d1, 0x5, 0xae, 0x58, 
    0x2, 0x5d0, 0x5ce, 0x3, 0x2, 0x2, 0x2, 0x5d0, 0x5d1, 0x3, 0x2, 0x2, 
    0x2, 0x5d1, 0x133, 0x3, 0x2, 0x2, 0x2, 0x5d2, 0x5d7, 0x5, 0x130, 0x99, 
    0x2, 0x5d3, 0x5d4, 0x7, 0x73, 0x2, 0x2, 0x5d4, 0x5d6, 0x5, 0x130, 0x99, 
    0x2, 0x5d5, 0x5d3, 0x3, 0x2, 0x2, 0x2, 0x5d6, 0x5d9, 0x3, 0x2, 0x2, 
    0x2, 0x5d7, 0x5d5, 0x3, 0x2, 0x2, 0x2, 0x5d7, 0x5d8, 0x3, 0x2, 0x2, 
    0x2, 0x5d8, 0x135, 0x3, 0x2, 0x2, 0x2, 0x5d9, 0x5d7, 0x3, 0x2, 0x2, 
    0x2, 0x5da, 0x5df, 0x5, 0x132, 0x9a, 0x2, 0x5db, 0x5dc, 0x7, 0x73, 0x2, 
    0x2, 0x5dc, 0x5de, 0x5, 0x132, 0x9a, 0x2, 0x5dd, 0x5db, 0x3, 0x2, 0x2, 
    0x2, 0x5de, 0x5e1, 0x3, 0x2, 0x2, 0x2, 0x5df, 0x5dd, 0x3, 0x2, 0x2, 
    0x2, 0x5df, 0x5e0, 0x3, 0x2, 0x2, 0x2, 0x5e0, 0x137, 0x3, 0x2, 0x2, 
    0x2, 0x5e1, 0x5df, 0x3, 0x2, 0x2, 0x2, 0x5e2, 0x5e4, 0x7, 0x53, 0x2, 
    0x2, 0x5e3, 0x5e5, 0x5, 0x134, 0x9b, 0x2, 0x5e4, 0x5e3, 0x3, 0x2, 0x2, 
    0x2, 0x5e4, 0x5e5, 0x3, 0x2, 0x2, 0x2, 0x5e5, 0x5e6, 0x3, 0x2, 0x2, 
    0x2, 0x5e6, 0x5ef, 0x7, 0x54, 0x2, 0x2, 0x5e7, 0x5e9, 0x7, 0x53, 0x2, 
    0x2, 0x5e8, 0x5ea, 0x5, 0x134, 0x9b, 0x2, 0x5e9, 0x5e8, 0x3, 0x2, 0x2, 
    0x2, 0x5e9, 0x5ea, 0x3, 0x2, 0x2, 0x2, 0x5ea, 0x5eb, 0x3, 0x2, 0x2, 
    0x2, 0x5eb, 0x5ec, 0x7, 0x73, 0x2, 0x2, 0x5ec, 0x5ed, 0x7, 0x54, 0x2, 
    0x2, 0x5ed, 0x5ef, 0x8, 0x9d, 0x1, 0x2, 0x5ee, 0x5e2, 0x3, 0x2, 0x2, 
    0x2, 0x5ee, 0x5e7, 0x3, 0x2, 0x2, 0x2, 0x5ef, 0x139, 0x3, 0x2, 0x2, 
    0x2, 0x5f0, 0x5f2, 0x7, 0x53, 0x2, 0x2, 0x5f1, 0x5f3, 0x5, 0x136, 0x9c, 
    0x2, 0x5f2, 0x5f1, 0x3, 0x2, 0x2, 0x2, 0x5f2, 0x5f3, 0x3, 0x2, 0x2, 
    0x2, 0x5f3, 0x5f4, 0x3, 0x2, 0x2, 0x2, 0x5f4, 0x5fd, 0x7, 0x54, 0x2, 
    0x2, 0x5f5, 0x5f7, 0x7, 0x53, 0x2, 0x2, 0x5f6, 0x5f8, 0x5, 0x136, 0x9c, 
    0x2, 0x5f7, 0x5f6, 0x3, 0x2, 0x2, 0x2, 0x5f7, 0x5f8, 0x3, 0x2, 0x2, 
    0x2, 0x5f8, 0x5f9, 0x3, 0x2, 0x2, 0x2, 0x5f9, 0x5fa, 0x7, 0x73, 0x2, 
    0x2, 0x5fa, 0x5fb, 0x7, 0x54, 0x2, 0x2, 0x5fb, 0x5fd, 0x8, 0x9e, 0x1, 
    0x2, 0x5fc, 0x5f0, 0x3, 0x2, 0x2, 0x2, 0x5fc, 0x5f5, 0x3, 0x2, 0x2, 
    0x2, 0x5fd, 0x13b, 0x3, 0x2, 0x2, 0x2, 0x5fe, 0x600, 0x7, 0x53, 0x2, 
    0x2, 0x5ff, 0x601, 0x5, 0x10c, 0x87, 0x2, 0x600, 0x5ff, 0x3, 0x2, 0x2, 
    0x2, 0x600, 0x601, 0x3, 0x2, 0x2, 0x2, 0x601, 0x602, 0x3, 0x2, 0x2, 
    0x2, 0x602, 0x609, 0x7, 0x54, 0x2, 0x2, 0x603, 0x605, 0x7, 0x4f, 0x2, 
    0x2, 0x604, 0x606, 0x5, 0x10c, 0x87, 0x2, 0x605, 0x604, 0x3, 0x2, 0x2, 
    0x2, 0x605, 0x606, 0x3, 0x2, 0x2, 0x2, 0x606, 0x607, 0x3, 0x2, 0x2, 
    0x2, 0x607, 0x609, 0x7, 0x50, 0x2, 0x2, 0x608, 0x5fe, 0x3, 0x2, 0x2, 
    0x2, 0x608, 0x603, 0x3, 0x2, 0x2, 0x2, 0x609, 0x13d, 0x3, 0x2, 0x2, 
    0x2, 0x60a, 0x60c, 0x7, 0x53, 0x2, 0x2, 0x60b, 0x60d, 0x5, 0x10e, 0x88, 
    0x2, 0x60c, 0x60b, 0x3, 0x2, 0x2, 0x2, 0x60c, 0x60d, 0x3, 0x2, 0x2, 
    0x2, 0x60d, 0x60e, 0x3, 0x2, 0x2, 0x2, 0x60e, 0x623, 0x7, 0x54, 0x2, 
    0x2, 0x60f, 0x611, 0x7, 0x4f, 0x2, 0x2, 0x610, 0x612, 0x5, 0x10e, 0x88, 
    0x2, 0x611, 0x610, 0x3, 0x2, 0x2, 0x2, 0x611, 0x612, 0x3, 0x2, 0x2, 
    0x2, 0x612, 0x613, 0x3, 0x2, 0x2, 0x2, 0x613, 0x623, 0x7, 0x50, 0x2, 
    0x2, 0x614, 0x616, 0x7, 0x53, 0x2, 0x2, 0x615, 0x617, 0x5, 0x10e, 0x88, 
    0x2, 0x616, 0x615, 0x3, 0x2, 0x2, 0x2, 0x616, 0x617, 0x3, 0x2, 0x2, 
    0x2, 0x617, 0x618, 0x3, 0x2, 0x2, 0x2, 0x618, 0x619, 0x7, 0x73, 0x2, 
    0x2, 0x619, 0x61a, 0x7, 0x54, 0x2, 0x2, 0x61a, 0x623, 0x8, 0xa0, 0x1, 
    0x2, 0x61b, 0x61d, 0x7, 0x4f, 0x2, 0x2, 0x61c, 0x61e, 0x5, 0x10e, 0x88, 
    0x2, 0x61d, 0x61c, 0x3, 0x2, 0x2, 0x2, 0x61d, 0x61e, 0x3, 0x2, 0x2, 
    0x2, 0x61e, 0x61f, 0x3, 0x2, 0x2, 0x2, 0x61f, 0x620, 0x7, 0x73, 0x2, 
    0x2, 0x620, 0x621, 0x7, 0x50, 0x2, 0x2, 0x621, 0x623, 0x8, 0xa0, 0x1, 
    0x2, 0x622, 0x60a, 0x3, 0x2, 0x2, 0x2, 0x622, 0x60f, 0x3, 0x2, 0x2, 
    0x2, 0x622, 0x614, 0x3, 0x2, 0x2, 0x2, 0x622, 0x61b, 0x3, 0x2, 0x2, 
    0x2, 0x623, 0x13f, 0x3, 0x2, 0x2, 0x2, 0x624, 0x62a, 0x5, 0x142, 0xa2, 
    0x2, 0x625, 0x62a, 0x5, 0x144, 0xa3, 0x2, 0x626, 0x62a, 0x7, 0x4c, 0x2, 
    0x2, 0x627, 0x62a, 0x7, 0x4d, 0x2, 0x2, 0x628, 0x62a, 0x7, 0x4e, 0x2, 
    0x2, 0x629, 0x624, 0x3, 0x2, 0x2, 0x2, 0x629, 0x625, 0x3, 0x2, 0x2, 
    0x2, 0x629, 0x626, 0x3, 0x2, 0x2, 0x2, 0x629, 0x627, 0x3, 0x2, 0x2, 
    0x2, 0x629, 0x628, 0x3, 0x2, 0x2, 0x2, 0x62a, 0x141, 0x3, 0x2, 0x2, 
    0x2, 0x62b, 0x62c, 0x9, 0xf, 0x2, 0x2, 0x62c, 0x143, 0x3, 0x2, 0x2, 
    0x2, 0x62d, 0x62e, 0x9, 0x10, 0x2, 0x2, 0x62e, 0x145, 0x3, 0x2, 0x2, 
    0x2, 0xa4, 0x149, 0x151, 0x159, 0x161, 0x168, 0x16c, 0x171, 0x179, 0x183, 
    0x18b, 0x191, 0x196, 0x19d, 0x1a4, 0x1a7, 0x1b0, 0x1e0, 0x1f6, 0x1fe, 
    0x207, 0x20c, 0x213, 0x21c, 0x221, 0x22b, 0x231, 0x246, 0x24f, 0x258, 
    0x260, 0x271, 0x277, 0x27c, 0x283, 0x28a, 0x294, 0x29e, 0x2a7, 0x2b0, 
    0x2b9, 0x2be, 0x2c7, 0x2d8, 0x2de, 0x2e4, 0x2ed, 0x2f4, 0x2f8, 0x2ff, 
    0x303, 0x308, 0x30d, 0x312, 0x31a, 0x322, 0x327, 0x32b, 0x32f, 0x354, 
    0x35b, 0x363, 0x368, 0x36c, 0x370, 0x376, 0x37b, 0x383, 0x385, 0x389, 
    0x38f, 0x394, 0x39c, 0x39e, 0x3a2, 0x3ab, 0x3af, 0x3b2, 0x3b7, 0x3bb, 
    0x3c4, 0x3c8, 0x3cb, 0x3d0, 0x3dc, 0x3e7, 0x3f2, 0x3fd, 0x408, 0x412, 
    0x41a, 0x422, 0x42a, 0x432, 0x43b, 0x446, 0x450, 0x459, 0x464, 0x46c, 
    0x476, 0x481, 0x489, 0x496, 0x49c, 0x4af, 0x4b6, 0x4ba, 0x4be, 0x4c2, 
    0x4c6, 0x4cb, 0x4d0, 0x4dc, 0x4e0, 0x4e4, 0x4e8, 0x4ec, 0x4f4, 0x525, 
    0x52e, 0x530, 0x539, 0x544, 0x54c, 0x551, 0x55a, 0x560, 0x569, 0x572, 
    0x582, 0x588, 0x58f, 0x594, 0x596, 0x59d, 0x5a2, 0x5a9, 0x5ae, 0x5b0, 
    0x5b7, 0x5bc, 0x5c1, 0x5c6, 0x5cb, 0x5d0, 0x5d7, 0x5df, 0x5e4, 0x5e9, 
    0x5ee, 0x5f2, 0x5f7, 0x5fc, 0x600, 0x605, 0x608, 0x60c, 0x611, 0x616, 
    0x61d, 0x622, 0x629, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

EscriptParser::Initializer EscriptParser::_init;
