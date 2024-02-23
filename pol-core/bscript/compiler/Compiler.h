#ifndef POLSERVER_COMPILER_H
#define POLSERVER_COMPILER_H

#include <memory>
#include <string>

#include "bscript/compiler/model/UserFunctionInclusion.h"

namespace Pol::Bscript::Compiler
{
class CompiledScript;
class CompilerWorkspace;
class SourceFileCache;
class Profile;
class Report;

class Compiler
{
public:
  Compiler( SourceFileCache& em_cache, SourceFileCache& inc_cache, Profile& );
  ~Compiler();
  Compiler( const Compiler& ) = delete;
  Compiler& operator=( const Compiler& ) = delete;

  bool compile_file( const std::string& filename );
  bool write_ecl( const std::string& pathname );
  void write_listing( const std::string& pathname );
  void write_dbg( const std::string& pathname, bool include_debug_text );
  void write_included_filenames( const std::string& pathname );
  void set_include_compile_mode();

  void compile_file_steps( const std::string& pathname, Report& );
  bool format_file( const std::string& filename, bool is_module, bool inplace );

private:
  std::unique_ptr<CompilerWorkspace> build_workspace( const std::string&, Report& );
  void register_constants( CompilerWorkspace&, Report& );
  void optimize( CompilerWorkspace&, Report& );
  void disambiguate( CompilerWorkspace&, Report& );
  void analyze( CompilerWorkspace&, Report& );
  std::unique_ptr<CompiledScript> generate( std::unique_ptr<CompilerWorkspace> );

  void display_outcome( const std::string& filename, Report& );

  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;
  Profile& profile;
  std::unique_ptr<CompiledScript> output;
  UserFunctionInclusion user_function_inclusion = UserFunctionInclusion::ReferencedOnly;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILER_H
