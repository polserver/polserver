#ifndef POLSERVER_DATAEMITTER_H
#define POLSERVER_DATAEMITTER_H

#include <string>
#include <vector>

#include "compiler/representation/CompiledScript.h"

namespace Pol::Bscript::Compiler
{

class DataEmitter
{
public:
  explicit DataEmitter( DataSection& );

  unsigned append( double );
  unsigned append( int );
  unsigned store( const std::string& );

  unsigned append( const std::byte* data, unsigned len );
  unsigned find( const std::byte* data, unsigned len );
  unsigned store( const std::byte* data, unsigned len ); // find or append

private:
  DataSection& data_section;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_DATAEMITTER_H
