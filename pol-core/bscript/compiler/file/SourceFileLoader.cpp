#include "SourceFile.h"

#include <cstring>

#include "clib/filecont.h"
#include "clib/fileutil.h"
#include "clib/strutil.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "compilercfg.h"

using EscriptGrammar::EscriptLexer;
using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
std::string SourceFileLoader::get_contents( const std::string& pathname ) const
{
  Clib::FileContents fc( pathname.c_str(), true );
  return std::string( fc.contents() );
}

}  // namespace Pol::Bscript::Compiler
