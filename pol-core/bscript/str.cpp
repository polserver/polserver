#include "str.h"

#include "bdouble.h"
#include "blong.h"
#include "bobject.h"

namespace Pol::Bscript
{
void int_to_binstr( int& value, std::stringstream& s )
{
  int i;
  for ( i = 31; i > 0; i-- )
  {
    if ( value & ( 1 << i ) )
      break;
  }
  for ( ; i >= 0; i-- )
  {
    if ( value & ( 1 << i ) )
      s << "1";
    else
      s << "0";
  }
}

bool try_to_format( std::stringstream& to_stream, BObjectImp* what, std::string& frmt )
{
  if ( frmt.empty() )
  {
    to_stream << what->getStringRep();
    return false;
  }

  if ( frmt.find( 'b' ) != std::string::npos )
  {
    if ( auto* plong = impptrIf<BLong>( what ) )
    {
      int n = plong->value();
      if ( frmt.find( '#' ) != std::string::npos )
        to_stream << ( ( n < 0 ) ? "-" : "" ) << "0b";
      int_to_binstr( n, to_stream );
    }
    else
    {
      to_stream << "<needs Int>";
      return false;
    }
  }
  else if ( frmt.find( 'x' ) != std::string::npos )
  {
    if ( auto* plong = impptrIf<BLong>( what ) )
    {
      int n = plong->value();
      if ( frmt.find( '#' ) != std::string::npos )
        to_stream << "0x";
      to_stream << std::hex << n << std::dec;
    }
    else
    {
      to_stream << "<needs Int>";
      return false;
    }
  }
  else if ( frmt.find( 'o' ) != std::string::npos )
  {
    if ( auto* plong = impptrIf<BLong>( what ) )
    {
      int n = plong->value();
      if ( frmt.find( '#' ) != std::string::npos )
        to_stream << "0o";
      to_stream << std::oct << n << std::dec;
    }
    else
    {
      to_stream << "<needs Int>";
      return false;
    }
  }
  else if ( frmt.find( 'd' ) != std::string::npos )
  {
    int n;
    if ( auto* plong = impptrIf<BLong>( what ) )
      n = plong->value();
    else if ( auto* pdbl = impptrIf<Double>( what ) )
      n = (int)pdbl->value();
    else
    {
      to_stream << "<needs Int, Double>";
      return false;
    }
    to_stream << std::dec << n;
  }
  else
  {
    to_stream << "<bad format: " << frmt << ">";
    return false;
  }
  return true;
}

std::string get_formatted( BObjectImp* what, std::string& frmt )
{
  std::stringstream result;
  try_to_format( result, what, frmt );
  return result.str();
}
}  // namespace Pol::Bscript
