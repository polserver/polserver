#include "CodeEmitter.h"

#include "StoredToken.h"

namespace Pol::Bscript::Compiler
{
CodeEmitter::CodeEmitter( CodeSection& code ) : code( code ) {}

unsigned CodeEmitter::append( const StoredToken& st )
{
  auto index = next_address();
  code.push_back( st );
  return index;
}

void CodeEmitter::update_offset( unsigned index, unsigned offset )
{
  code[index].offset = offset;
}

unsigned CodeEmitter::next_address() const
{
  size_t index = code.size();
  if ( index > std::numeric_limits<unsigned>::max() ) {
    throw std::runtime_error( "Instruction count overflow" );
  }
  return static_cast<unsigned>( index );
}

}  // namespace Pol::Bscript::Compiler
