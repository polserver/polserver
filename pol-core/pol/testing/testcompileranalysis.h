/** @file
 *
 * @par History
 */

#ifndef POL_TESTCOMPILERANALYSIS_H
#define POL_TESTCOMPILERANALYSIS_H

#include <map>
#include <memory>
#include <string>

#include "bscript/compiler/Compiler.h"
#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compilercfg.h"

// Runs Compiler::analyze() the way vscode-escript does, on sources held in memory.

namespace Pol::Testing::CompilerAnalysis
{
// No file exists under this prefix. The test sources live only in MemoryLoader.
inline const std::string memdir = "inmemory/";

// Serves the test sources from memory, the way the extension serves an unsaved buffer. Anything
// else, like basic.em, comes from disk. A `use` or `include` checks the disk before it loads, so a
// file that is used or included has to be written out.
class MemoryLoader : public Bscript::Compiler::SourceFileLoader
{
public:
  std::string get_contents( const std::string& pathname ) const override
  {
    auto itr = sources.find( pathname );
    if ( itr != sources.end() )
      return itr->second;
    return SourceFileLoader::get_contents( pathname );
  }
  std::map<std::string, std::string> sources;
};

// One analysis, holding everything the workspace's diagnostics and AST point into.
struct Analysis
{
  Analysis() : report( reporter ), em_cache( loader, profile ), inc_cache( loader, profile ) {}

  Bscript::Compiler::DiagnosticReporter reporter;
  Bscript::Compiler::Report report;
  Bscript::Compiler::Profile profile;
  MemoryLoader loader;
  Bscript::Compiler::SourceFileCache em_cache;
  Bscript::Compiler::SourceFileCache inc_cache;
  std::unique_ptr<Bscript::Compiler::CompilerWorkspace> workspace;

  std::string add_source( const std::string& name, const std::string& body )
  {
    const std::string path = memdir + name;
    loader.sources[path] = body;
    return path;
  }

  void run( const std::string& path, bool is_module, bool continue_on_error,
            bool include_all_functions )
  {
    Bscript::Compiler::Compiler compiler( loader, em_cache, inc_cache, profile );
    if ( include_all_functions )
      compiler.set_include_compile_mode();
    workspace = compiler.analyze( path, report, is_module, continue_on_error );
  }

  bool has_message( const std::string& text ) const
  {
    for ( const auto& diagnostic : reporter.diagnostics )
      if ( diagnostic.message.find( text ) != std::string::npos )
        return true;
    return false;
  }
};

inline std::unique_ptr<Analysis> analyze_src( const std::string& name, const std::string& body,
                                              bool continue_on_error = true,
                                              bool include_all_functions = true )
{
  auto analysis = std::make_unique<Analysis>();
  analysis->run( analysis->add_source( name, body ), false /* is_module */, continue_on_error,
                 include_all_functions );
  return analysis;
}

// analyze() resolves modules through the global compiler config. The test shard's ecompile.cfg
// points ModuleDirectory at the shipped .em files. The previous config comes back on scope exit.
class ScopedCompilerConfig
{
public:
  ScopedCompilerConfig() : saved( Bscript::compilercfg )
  {
    Bscript::compilercfg.Read( "ecompile.cfg" );
  }
  ~ScopedCompilerConfig() { Bscript::compilercfg = saved; }
  ScopedCompilerConfig( const ScopedCompilerConfig& ) = delete;
  ScopedCompilerConfig& operator=( const ScopedCompilerConfig& ) = delete;

private:
  const Bscript::CompilerConfig saved;
};
}  // namespace Pol::Testing::CompilerAnalysis

#endif
