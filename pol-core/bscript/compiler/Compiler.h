#ifndef POLSERVER_COMPILER_H
#define POLSERVER_COMPILER_H

#include "../facility/Compiler.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
struct LegacyFunctionOrder;
class Profile;
class Report;

class Compiler : public Pol::Bscript::Facility::Compiler
{
public:
  explicit Compiler( Profile& );
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
  void display_outcome( const std::string& filename, Report& );

  Profile& profile;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_COMPILER_H
