#ifndef POLSERVER_FACILITY_COMPILER_H
#define POLSERVER_FACILITY_COMPILER_H

#include <string>

namespace Pol
{
namespace Bscript
{
namespace Facility
{
class Compiler
{
public:
  virtual ~Compiler() = default;
  virtual bool compile_file( const std::string& filename ) = 0;
  virtual bool write_ecl( const std::string& pathname ) = 0;
  virtual void write_listing( const std::string& pathname ) = 0;
  virtual void write_dbg( const std::string& pathname, bool include_debug_text ) = 0;
  virtual void write_included_filenames( const std::string& pathname ) = 0;
  virtual void set_include_compile_mode() = 0;
};

}  // namespace Facility
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_COMPILERINTERFACE_H
