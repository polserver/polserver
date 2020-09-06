#ifndef POLSERVER_COMPILEDSCRIPTSERIALIZER_H
#define POLSERVER_COMPILEDSCRIPTSERIALIZER_H

#include <string>

namespace Pol::Bscript::Compiler
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

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILEDSCRIPTSERIALIZER_H
