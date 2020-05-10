//
// Created by Eric Swanson on 4/27/20.
//

#include "EscriptCompiler.h"

#include <iostream>

#include "antlr4-runtime.h"

#include "../clib/fileutil.h"
#include "../clib/logfacility.h"

#include <EscriptGrammar/EscriptLexer.h>
#include <EscriptGrammar/EscriptParser.h>

using antlr4::ANTLRInputStream;
using antlr4::CommonTokenStream;
using EscriptGrammar::EscriptLexer;
using EscriptGrammar::EscriptParser;

namespace Pol
{
namespace Bscript
{

int EscriptCompiler::compileFile( const std::string& filename )
{
  auto pathname = Clib::FullPath( filename.c_str() );
  std::ifstream stream( pathname );

  ANTLRInputStream input( stream );
  EscriptLexer lexer( &input );
  CommonTokenStream tokens( &lexer );
  EscriptParser parser( &tokens );

  auto compilationUnit = parser.compilationUnit();

  INFO_PRINT << "compilation unit has " << compilationUnit->topLevelDeclaration().size()
             << " top-level declarations.\n";

  return 0;
}

}  // namespace Bscript
}  // namespace Pol
