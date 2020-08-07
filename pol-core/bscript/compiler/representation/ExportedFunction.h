#ifndef POLSERVER_EXPORTEDFUNCTION_H
#define POLSERVER_EXPORTEDFUNCTION_H

#include <string>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class ExportedFunction
{
public:
  ExportedFunction( std::string, unsigned parameter_count, unsigned entrypoint_program_counter );

  const std::string name;
  const unsigned parameter_count;
  const unsigned entrypoint_program_counter;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_EXPORTEDFUNCTION_H
