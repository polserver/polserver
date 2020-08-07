#include "CodeEmitter.h"

#include "StoredToken.h"

namespace Pol::Bscript::Compiler
{
CodeEmitter::CodeEmitter( CodeSection& code ) : code( code ) {}

unsigned CodeEmitter::append( const StoredToken& st )
{
  unsigned index = code.size();
  code.push_back( st );
  return index;
}

void CodeEmitter::update_offset( unsigned index, unsigned offset )
{
  code[index].offset = offset;
}

unsigned CodeEmitter::next_address() const
{
  return code.size();
}

}  // namespace Pol::Bscript::Compiler
