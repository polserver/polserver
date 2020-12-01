#ifndef POLSERVER_CODEEMITTER_H
#define POLSERVER_CODEEMITTER_H

#include "compiler/representation/CompiledScript.h"

namespace Pol::Bscript
{
class StoredToken;
}
namespace Pol::Bscript::Compiler
{
class CodeEmitter
{
public:
  explicit CodeEmitter( CodeSection& code );

  CodeEmitter( const CodeEmitter& ) = delete;
  CodeEmitter& operator=( const CodeEmitter& ) = delete;

  unsigned append( const StoredToken& );
  void update_offset( unsigned index, unsigned offset );

  [[nodiscard]] unsigned next_address() const;

private:
  CodeSection& code;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CODEEMITTER_H
