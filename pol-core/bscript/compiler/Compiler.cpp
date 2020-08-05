#include "Compiler.h"

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
Compiler::Compiler() = default;

Compiler::~Compiler() = default;

bool Compiler::compile_file( const std::string& /*filename*/ )
{
  return false;
}

bool Compiler::write_ecl( const std::string& /*pathname*/ )
{
  return false;
}

void Compiler::write_listing( const std::string& /*pathname*/ )
{
}

void Compiler::write_dbg( const std::string& /*pathname*/, bool /*include_debug_text*/ )
{
}

void Compiler::write_included_filenames( const std::string& /*pathname*/ )
{
}

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol
