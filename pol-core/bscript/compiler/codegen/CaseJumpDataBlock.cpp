#include "CaseJumpDataBlock.h"

#include "tokens.h"

namespace Pol::Bscript::Compiler
{
void CaseJumpDataBlock::on_integer_value_jump_to( int32_t value, uint16_t address )
{
  auto* p = reinterpret_cast<unsigned char*>( &address );
  data.push_back( p[0] );
  data.push_back( p[1] );
  data.push_back( CASE_TYPE_LONG );
  p = reinterpret_cast<unsigned char*>( &value );
  data.push_back( p[0] );
  data.push_back( p[1] );
  data.push_back( p[2] );
  data.push_back( p[3] );
}

void CaseJumpDataBlock::on_string_value_jump_to( const std::string& value, uint16_t address )
{
  if ( value.size() >= 254 )
  {
    throw std::runtime_error( "String expressions in CASE statements must be <= 253 characters." );
  }
  auto* p = reinterpret_cast<unsigned char*>( &address );
  data.push_back( p[0] );
  data.push_back( p[1] );
  data.push_back( static_cast<unsigned char>( value.size() ) );

  data.insert( data.end(), value.begin(), value.end() );
}

void CaseJumpDataBlock::on_default_jump_to( uint16_t address )
{
  auto* p = reinterpret_cast<unsigned char*>( &address );
  data.push_back( p[0] );
  data.push_back( p[1] );
  data.push_back( CASE_TYPE_DEFAULT );
}

const std::vector<unsigned char>& CaseJumpDataBlock::get_data() const
{
  return data;
}

}  // namespace Pol::Bscript::Compiler