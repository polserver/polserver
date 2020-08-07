#ifndef POLSERVER_SOURCEFILEPROCESSOR_H
#define POLSERVER_SOURCEFILEPROCESSOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <boost/optional.hpp>
#include <memory>
#include <string>

#include "ProgramBuilder.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class SourceFile;

class SourceFileProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  SourceFileProcessor( const SourceFileIdentifier&, BuilderWorkspace&, bool is_src );

public:
  void use_module( const std::string& name, SourceLocation& including_location,
                   long long* micros_counted = nullptr );
  void process_source( SourceFile& );
  void process_include( long long* us_counted, SourceFile& );

  void handle_include_declaration( EscriptGrammar::EscriptParser::IncludeDeclarationContext*,
                                   long long* us_spent );
  boost::optional<std::string> locate_include_file( const SourceLocation& source_location,
                                                    const std::string& include_name );

  long long handle_use_declaration( EscriptGrammar::EscriptParser::UseDeclarationContext* );

  antlrcpp::Any visitStatement( EscriptGrammar::EscriptParser::StatementContext* ) override;
  antlrcpp::Any visitUnambiguousStatement(
      EscriptGrammar::EscriptParser::UnambiguousStatementContext* ) override;
  antlrcpp::Any visitProgramDeclaration(
      EscriptGrammar::EscriptParser::ProgramDeclarationContext* ) override;
  antlrcpp::Any visitUnambiguousProgramDeclaration(
      EscriptGrammar::EscriptParser::UnambiguousProgramDeclarationContext* ) override;
  antlrcpp::Any visitIncludeDeclaration(
      EscriptGrammar::EscriptParser::IncludeDeclarationContext* ) override;
  antlrcpp::Any visitUseDeclaration(
      EscriptGrammar::EscriptParser::UseDeclarationContext* ) override;
  antlrcpp::Any visitFunctionDeclaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* ) override;
  antlrcpp::Any visitUnambiguousFunctionDeclaration(
      EscriptGrammar::EscriptParser::UnambiguousFunctionDeclarationContext* ) override;

  SourceLocation location_for( antlr4::ParserRuleContext& ) const;

protected:
  Profile& profile;
  Report& report;
  const SourceFileIdentifier& source_file_identifier;
  BuilderWorkspace& workspace;

  ProgramBuilder tree_builder;

private:
  const bool is_src;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILEPROCESSOR_H
