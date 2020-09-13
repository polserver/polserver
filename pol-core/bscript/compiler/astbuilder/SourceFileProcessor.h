#ifndef POLSERVER_SOURCEFILEPROCESSOR_H
#define POLSERVER_SOURCEFILEPROCESSOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <boost/optional.hpp>
#include <memory>
#include <string>

#include "compiler/astbuilder/ProgramBuilder.h"
#include "compiler/model/UserFunctionInclusion.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class SourceFile;

class SourceFileProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  SourceFileProcessor( const SourceFileIdentifier&, BuilderWorkspace&, bool is_src,
                       UserFunctionInclusion );

public:
  void use_module( const std::string& name, SourceLocation& including_location,
                   long long* micros_counted = nullptr );
  void process_source( SourceFile& );
  void process_include( SourceFile&, long long* micros_counted );

  void handle_include_declaration( EscriptGrammar::EscriptParser::IncludeDeclarationContext*,
                                   long long* micros_counted );
  boost::optional<std::string> locate_include_file( const SourceLocation& source_location,
                                                    const std::string& include_name );

  void handle_use_declaration( EscriptGrammar::EscriptParser::UseDeclarationContext*,
                               long long* micros_counted );

  antlrcpp::Any visitIncludeDeclaration(
      EscriptGrammar::EscriptParser::IncludeDeclarationContext* ) override;
  antlrcpp::Any visitFunctionDeclaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* ) override;
  antlrcpp::Any visitProgramDeclaration(
      EscriptGrammar::EscriptParser::ProgramDeclarationContext* ) override;
  antlrcpp::Any visitStatement( EscriptGrammar::EscriptParser::StatementContext* ) override;
  antlrcpp::Any visitUseDeclaration(
      EscriptGrammar::EscriptParser::UseDeclarationContext* ) override;

  SourceLocation location_for( antlr4::ParserRuleContext& ) const;

protected:
  Profile& profile;
  Report& report;
  const SourceFileIdentifier& source_file_identifier;
  BuilderWorkspace& workspace;

  ProgramBuilder tree_builder;

private:
  const bool is_src;
  const UserFunctionInclusion user_function_inclusion;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILEPROCESSOR_H
