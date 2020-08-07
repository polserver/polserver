#ifndef POLSERVER_COMPILER_H
#define POLSERVER_COMPILER_H

#include "../facility/Compiler.h"

#include <memory>

namespace Pol::Bscript::Compiler
{
class CompiledScript;
class CompilerWorkspace;
struct LegacyFunctionOrder;
class SourceFileCache;
class Profile;
class Report;

class Compiler : public Pol::Bscript::Facility::Compiler
{
public:
  Compiler( SourceFileCache& em_cache, SourceFileCache& inc_cache, Profile& );
  ~Compiler() override;
  Compiler( const Compiler& ) = delete;
  Compiler& operator=( const Compiler& ) = delete;

  bool compile_file( const std::string& filename ) override;
  bool write_ecl( const std::string& pathname ) override;
  void write_listing( const std::string& pathname ) override;
  void write_dbg( const std::string& pathname, bool include_debug_text ) override;
  void write_included_filenames( const std::string& pathname ) override;

  bool compile_file( const std::string& filename, const LegacyFunctionOrder* );
  void compile_file_steps( const std::string& pathname, const LegacyFunctionOrder*, Report& );

private:
  std::unique_ptr<CompilerWorkspace> build_workspace( const std::string&,
                                                      const LegacyFunctionOrder*,
                                                      Report& );
  void register_constants( CompilerWorkspace& );
  void optimize( CompilerWorkspace&, Report& );
  void disambiguate( CompilerWorkspace&, Report& );
  void analyze( CompilerWorkspace&, Report& );
  std::unique_ptr<CompiledScript> generate( std::unique_ptr<CompilerWorkspace>,
                                            const LegacyFunctionOrder* );

  void display_outcome( const std::string& filename, Report& );

  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;
  Profile& profile;
  std::unique_ptr<CompiledScript> output;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILER_H
