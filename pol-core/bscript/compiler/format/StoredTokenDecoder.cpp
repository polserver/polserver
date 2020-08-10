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
  case TOK_DOUBLE:
    w << double_at( tkn.offset ) << " (float)"
      << " offset=0x" << fmt::hex( tkn.offset );
    //<< " file=0x" << fmt::hex( tkn.offset + data_offset );
    break;

  case TOK_CONSUMER:
    w << "# (consume)";
    break;

  case CTRL_PROGEND:
    w << "progend";
    break;

  default:
    w << "id=0x" << fmt::hex( tkn.id ) << " type=" << tkn.type << " offset=" << tkn.offset
      << " module=" << tkn.module;
  }
}

double StoredTokenDecoder::double_at( unsigned offset ) const
{
  if ( offset > data.size() - sizeof( double ) )
    throw std::runtime_error( "data overflow reading double at offset " + std::to_string( offset ) +
                              ".  Data size is " + std::to_string( data.size() ) );

  return *reinterpret_cast<const double*>( &data[offset] );
}

}  // namespace Pol::Bscript::Compiler
