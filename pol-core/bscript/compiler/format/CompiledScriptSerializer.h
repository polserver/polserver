#ifndef POLSERVER_COMPILEDSCRIPTSERIALIZER_H
#define POLSERVER_COMPILEDSCRIPTSERIALIZER_H

#include <string>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class CompiledScript;

class CompiledScriptSerializer
{
public:
  explicit CompiledScriptSerializer( const CompiledScript& );

  void write( const std::string& pathname ) const;

private:
  const CompiledScript& compiled_script;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_COMPILEDSCRIPTSERIALIZER_H
