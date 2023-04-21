#ifndef POLSERVER_SOURCEFILE_H
#define POLSERVER_SOURCEFILE_H

#include <memory>
#include <mutex>
#include <string>

#include "bscript/compiler/file/ConformingCharStream.h"
#include "bscript/compiler/file/ErrorListener.h"
#include <EscriptGrammar/EscriptLexer.h>
#include <EscriptGrammar/EscriptParser.h>

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceLocation;

class SourceFile
{
public:
  SourceFile( const std::string& pathname, const std::string& contents, Profile& );
  ~SourceFile();

  static bool enforced_case_sensitivity_mismatch( const SourceLocation& referencing_location,
                                                  const std::string& pathname, Report& report );
  static std::shared_ptr<SourceFile> load( const SourceFileIdentifier&, Profile&, Report& );

  void propagate_errors_to( Report&, const SourceFileIdentifier& );

  EscriptGrammar::EscriptParser::CompilationUnitContext* get_compilation_unit(
      Report&, const SourceFileIdentifier& );
  EscriptGrammar::EscriptParser::ModuleUnitContext* get_module_unit( Report&,
                                                                     const SourceFileIdentifier& );
  EscriptGrammar::EscriptParser::EvaluateUnitContext* get_evaluate_unit( Report& );

  const std::string pathname;

private:
  antlr4::ANTLRInputStream input;
  ConformingCharStream conformer;
  EscriptGrammar::EscriptLexer lexer;
  antlr4::CommonTokenStream tokens;
  EscriptGrammar::EscriptParser parser;
  ErrorListener error_listener;

  std::mutex mutex;

  std::atomic<EscriptGrammar::EscriptParser::CompilationUnitContext*> compilation_unit;
  std::atomic<EscriptGrammar::EscriptParser::ModuleUnitContext*> module_unit;
  std::atomic<EscriptGrammar::EscriptParser::EvaluateUnitContext*> evaluate_unit;

  std::atomic<unsigned> access_count;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_SOURCEFILE_H
