#include "StoredTokenDecoder.h"

#include "StoredToken.h"

namespace Pol::Bscript::Compiler
{
StoredTokenDecoder::StoredTokenDecoder( const std::vector<ModuleDescriptor>& module_descriptors,
                                        const std::vector<std::byte>& data )
  : module_descriptors( module_descriptors ), data( data )
{
}

void StoredTokenDecoder::decode_to( const StoredToken& tkn, fmt::Writer& w )
{
  switch ( tkn.id )
  {

  default:
    w << "id=0x" << fmt::hex( tkn.id ) << " type=" << tkn.type << " offset=" << tkn.offset
      << " module=" << tkn.module;
  }
}

}  // namespace Pol::Bscript::Compiler
