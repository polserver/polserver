/** @file
 *
 * @par History
 */

#include <map>
#include <memory>
#include <string>

#include "bscript/compiler/Compiler.h"
#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileLoader.h"
#include "bscript/compiler/model/CompilerWorkspace.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compilercfg.h"
#include "pol/testing/testenv.h"

// Compiler::analyze() is what vscode-escript calls for diagnostics, hover, completion and the
// rest; nothing in ecompile or pol reaches it. These cases pin the behaviour that only analysis
// mode has: it keeps going after errors that stop ecompile, and it builds the scope tree.

namespace Pol::Testing
{
using namespace Bscript::Compiler;

namespace
{
// No file exists under this prefix. The test sources live only in MemoryLoader.
const std::string memdir = "inmemory/";

// Serves the test sources from memory, the way the extension serves an unsaved buffer. Anything
// else, like basic.em, comes from disk. A `use` or `include` checks the disk before it loads, so a
// file that is used or included has to be written out.
class MemoryLoader : public SourceFileLoader
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

  DiagnosticReporter reporter;
  Report report;
  Profile profile;
  MemoryLoader loader;
  SourceFileCache em_cache;
  SourceFileCache inc_cache;
  std::unique_ptr<CompilerWorkspace> workspace;

  std::string add_source( const std::string& name, const std::string& body )
  {
    const std::string path = memdir + name;
    loader.sources[path] = body;
    return path;
  }

  void run( const std::string& path, bool is_module, bool continue_on_error,
            bool include_all_functions )
  {
    Compiler compiler( loader, em_cache, inc_cache, profile );
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

std::unique_ptr<Analysis> analyze_src( const std::string& name, const std::string& body,
                                       bool continue_on_error = true,
                                       bool include_all_functions = true )
{
  auto analysis = std::make_unique<Analysis>();
  analysis->run( analysis->add_source( name, body ), false /* is_module */, continue_on_error,
                 include_all_functions );
  return analysis;
}

void test_clean_source()
{
  auto a = analyze_src( "clean.src",
                        "program p()\n"
                        "  var x := 1;\n"
                        "  print( x );\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "clean source builds a workspace" );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, 0u,
            "clean source has no diagnostics" );
  UnitTest( [&]() { return a->workspace && a->workspace->scope_tree != nullptr; }, true,
            "analysis builds the scope tree" );
  UnitTest( [&]() { return a->workspace && !a->workspace->tokens.empty(); }, true,
            "analysis fills the semantic tokens" );
}

void test_module()
{
  auto a = std::make_unique<Analysis>();
  a->run( a->add_source( "testmod.em",
                         "Foo( a, b := 0 );\n"
                         "Bar();\n"
                         "const BAZ := 1;\n" ),
          true /* is_module */, true /* continue_on_error */, false /* include_all_functions */ );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "module builds a workspace" );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, 0u, "module has no diagnostics" );
  UnitTest( [&]() { return a->workspace ? a->workspace->module_function_declarations.size() : 0u; },
            2u, "module declares its two functions" );
}

void test_module_unreadable()
{
  auto a = std::make_unique<Analysis>();
  a->run( memdir + "nosuchmodule.em", true /* is_module */, true /* continue_on_error */,
          false /* include_all_functions */ );
  UnitTest( [&]() { return a->workspace == nullptr; }, true, "missing module file builds nothing" );
  UnitTest( [&]() { return a->has_message( "Unable to load" ); }, true,
            "missing module file is reported" );
}

// A parse error leaves the referenced user functions unbuilt unless analysis is asked to go on.
void test_continue_on_error()
{
  const std::string body =
      "program p()\n"
      "  var x := ;\n"
      "  helper();\n"
      "endprogram\n"
      "function helper()\n"
      "  return 1;\n"
      "endfunction\n";
  auto stop = analyze_src( "stop.src", body, false /* continue_on_error */,
                           false /* include_all_functions */ );
  auto go_on = analyze_src( "goon.src", body, true /* continue_on_error */,
                            false /* include_all_functions */ );
  UnitTest( [&]() { return stop->report.error_count() > 0; }, true, "parse error is reported" );
  UnitTest( [&]() { return stop->workspace ? stop->workspace->user_functions.size() : 99u; }, 0u,
            "without continue_on_error no user function is built" );
  UnitTest( [&]() { return go_on->workspace ? go_on->workspace->user_functions.size() : 99u; }, 1u,
            "with continue_on_error the user function is built" );
}

// ecompile stops at these (escript tests errors012/errors013); analysis reports them and goes on
// to report the rest of the file.
void test_missing_module_continues()
{
  auto a = analyze_src( "missmod.src",
                        "use nosuchmodule;\n"
                        "program p()\n"
                        "  print( NoSuchFunction() );\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "missing module: workspace built" );
  UnitTest( [&]() { return a->has_message( "Unable to use module 'nosuchmodule'." ); }, true,
            "missing module is reported" );
  UnitTest( [&]() { return a->has_message( "NoSuchFunction" ); }, true,
            "missing module: the rest of the file is still analyzed" );
}

void test_missing_include_continues()
{
  auto a = analyze_src( "missinc.src",
                        "include \":nosuchpkg:nosuchinclude\";\n"
                        "program p()\n"
                        "  print( NO_SUCH_CONSTANT );\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->workspace != nullptr; }, true, "missing include: workspace built" );
  UnitTest( [&]() { return a->has_message( "Unable to read include file" ); }, true,
            "missing include is reported" );
  UnitTest( [&]() { return a->has_message( "NO_SUCH_CONSTANT" ); }, true,
            "missing include: the rest of the file is still analyzed" );
}

void test_duplicate_method_keeps_first()
{
  auto a = analyze_src( "dupmethod.src",
                        "class C()\n"
                        "  function C( this )\n"
                        "  endfunction\n"
                        "  function m( this )\n"
                        "    return 1;\n"
                        "  endfunction\n"
                        "  function m( this )\n"
                        "    return 2;\n"
                        "  endfunction\n"
                        "endclass\n"
                        "program p()\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->report.error_count() > 0; }, true, "duplicate method is an error" );
  // Only the link keeps the first declaration. The resolver keeps the last parse tree under a
  // name, so the UserFunction behind the link is built from the second one.
  UnitTest(
      [&]() -> size_t
      {
        if ( !a->workspace || !a->workspace->scope_tree )
          return 0;
        auto cd = a->workspace->scope_tree->find_class( "C" );
        if ( !cd || !cd->methods.contains( "m" ) )
          return 0;
        return cd->methods.at( "m" )->source_location.range.start.line_number;
      },
      size_t{ 4 }, "duplicate method's link points at the first declaration" );
}

void test_case_group_without_label()
{
  auto a = analyze_src( "caselabel.src",
                        "program p()\n"
                        "  var elem;\n"
                        "  case ( elem ) 5: x\n"
                        "endprogram\n" );
  UnitTest( [&]() { return a->report.error_count() > 0; }, true,
            "half-typed case is analyzed without a crash" );
}

void test_report_clear()
{
  auto a = analyze_src( "clear.src", "program p(\n" );
  UnitTest( [&]() { return a->report.error_count() > 0; }, true, "broken source has errors" );
  a->report.clear();
  UnitTest( [&]() { return a->report.error_count(); }, 0u, "Report::clear() resets the count" );
  UnitTest( [&]() { return a->reporter.diagnostics.size(); }, size_t{ 0 },
            "Report::clear() empties the DiagnosticReporter" );
}
}  // namespace

void compiler_analysis_test()
{
  // analyze() resolves modules through the global compiler config; the test shard's ecompile.cfg
  // points ModuleDirectory at the shipped .em files.
  const auto saved_cfg = Bscript::compilercfg;
  Bscript::compilercfg.Read( "ecompile.cfg" );

  test_clean_source();
  test_module();
  test_module_unreadable();
  test_continue_on_error();
  test_missing_module_continues();
  test_missing_include_continues();
  test_duplicate_method_keeps_first();
  test_case_group_without_label();
  test_report_clear();

  Bscript::compilercfg = saved_cfg;
}
}  // namespace Pol::Testing
