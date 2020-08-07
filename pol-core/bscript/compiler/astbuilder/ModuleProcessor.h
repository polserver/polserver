#ifndef POLSERVER_MODULEPROCESSOR_H
#define POLSERVER_MODULEPROCESSOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <memory>
#include <string>

#include "DeclarationBuilder.h"

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFile;
class SourceFileIdentifier;
class BuilderWorkspace;

class ModuleProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  ModuleProcessor( const SourceFileIdentifier&, BuilderWorkspace&, std::string modulename );

  void process_module( long long* us_counted, SourceFile& sf );

  antlrcpp::Any visitModuleDeclarationStatement(
      EscriptGrammar::EscriptParser::ModuleDeclarationStatementContext* ) override;
  antlrcpp::Any visitUnambiguousModuleDeclarationStatement(
      EscriptGrammar::EscriptParser::UnambiguousModuleDeclarationStatementContext* ) override;

private:
  Profile& profile;
  Report& report;
  const SourceFileIdentifier& source_file_identifier;
  BuilderWorkspace& workspace;

  DeclarationBuilder tree_builder;
  const std::string modulename;
};

}  // namespace Pol::Bscript::Compiler
#endif  // POLSERVER_MODULEPROCESSOR_H
