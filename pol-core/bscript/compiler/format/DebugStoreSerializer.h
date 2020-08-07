#ifndef POLSERVER_DEBUGSTORESERIALIZER_H
#define POLSERVER_DEBUGSTORESERIALIZER_H

#include <iosfwd>
#include <string>

namespace Pol::Bscript::Compiler
{
class CompiledScript;
class DebugStore;

class DebugStoreSerializer
{
public:
  explicit DebugStoreSerializer( CompiledScript& );

  void write( std::ofstream& ofs, std::ofstream* text_ofs );

private:
  const CompiledScript& compiled_script;
  const DebugStore& debug_store;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DEBUGSTORESERIALIZER_H
