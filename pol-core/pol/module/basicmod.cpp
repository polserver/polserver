/** @file
 *
 * @par History
 * - 2005/11/24 Shinigami: added itoa for Linux because it is not ANSI C/C++
 * - 2005/11/29 Shinigami: mf_SplitWords will now accept each type of to-split-value as same as in
 * the past
 * - 2006/10/07 Shinigami: FreeBSD fix - changed __linux__ to __unix__
 * - 2006/12/29 Shinigami: mf_SplitWords will not hang server on queue of delimiter
 */


#include "basicmod.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <picojson/picojson.h>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/dict.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../bscript/regexp.h"
#include "../../clib/stlutil.h"

#include <module_defs/basic.h>

namespace Pol
{
namespace Module
{
using namespace Bscript;

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


static inline bool is_base64( unsigned char c )
{
  return ( isalnum( c ) || ( c == '+' ) || ( c == '/' ) );
}

BasicExecutorModule::BasicExecutorModule( Executor& exec )
    : Bscript::TmplExecutorModule<BasicExecutorModule, Bscript::ExecutorModule>( exec )
{
}

Bscript::BObjectImp* BasicExecutorModule::mf_Len()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( auto* a = impptrIf<Bscript::ObjArray>( imp ) )
    return new BLong( static_cast<int>( a->ref_arr.size() ) );
  else if ( auto* s = impptrIf<Bscript::String>( imp ) )
    return new BLong( static_cast<int>( s->length() ) );
  else if ( auto* e = impptrIf<Bscript::BError>( imp ) )
    return new BLong( static_cast<int>( e->mapcount() ) );
  return new BLong( 0 );
}

Bscript::BObjectImp* BasicExecutorModule::mf_Find()
{
  exec.makeString( 0 );
  String* str = static_cast<String*>( exec.getParamImp( 0 ) );
  const char* s = exec.paramAsString( 1 );
  int d = static_cast<int>( exec.paramAsLong( 2 ) );

  int posn = str->find( d ? ( d - 1 ) : 0, s ) + 1;

  return new BLong( posn );
}

Bscript::BObjectImp* BasicExecutorModule::mf_SubStr()
{
  exec.makeString( 0 );
  String* str = static_cast<String*>( exec.getParamImp( 0 ) );
  int start = static_cast<int>( exec.paramAsLong( 1 ) );
  int length = static_cast<int>( exec.paramAsLong( 2 ) );

  return str->StrStr( start, length );
}

Bscript::BObjectImp* BasicExecutorModule::mf_Trim()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  auto* str = impptrIf<Bscript::String>( imp );
  if ( !str )
    return new BError( "Param 1 must be a string." );
  int type = static_cast<int>( exec.paramAsLong( 1 ) );
  const char* cset = exec.paramAsString( 2 );
  if ( type > 3 )
    type = 3;
  if ( type < 1 )
    type = 1;

  return str->ETrim( cset, type );
}

/*
eScript Function Useage

StrReplace(str, to_replace, replace_with);
*/
Bscript::BObjectImp* BasicExecutorModule::mf_StrReplace()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  std::unique_ptr<String> string( new String( imp->getStringRep().c_str() ) );
  String* to_replace = static_cast<String*>( exec.getParamImp( 1, Bscript::BObjectImp::OTString ) );
  if ( !to_replace )
    return new BError( "Invalid parameter type" );
  String* replace_with =
      static_cast<String*>( exec.getParamImp( 2, Bscript::BObjectImp::OTString ) );
  if ( !replace_with )
    return new BError( "Invalid parameter type" );

  if ( string->length() < 1 )
    return new BError( "Cannot use empty string for string haystack." );
  if ( to_replace->length() < 1 )
    return new BError( "Cannot use empty string for string needle." );

  string->EStrReplace( to_replace, replace_with );

  return string.release();
}

// SubStrReplace(str, replace_with, start, length:=0);
Bscript::BObjectImp* BasicExecutorModule::mf_SubStrReplace()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  std::unique_ptr<String> string( new String( imp->getStringRep().c_str() ) );
  String* replace_with =
      static_cast<String*>( exec.getParamImp( 1, Bscript::BObjectImp::OTString ) );
  if ( !replace_with )
    return new BError( "Invalid parameter type" );
  int index = static_cast<int>( exec.paramAsLong( 2 ) );
  int len = static_cast<int>( exec.paramAsLong( 3 ) );

  if ( index < 0 )
    return new BError( "Index must not be negative" );
  if ( static_cast<unsigned>( index - 1 ) > string->length() )
    return new BError( "Index out of range" );

  // We set it to 1 because of doing -1 later to stay with eScript handling.
  if ( !index )
    index = 1;

  if ( static_cast<unsigned>( len ) > ( string->length() - index ) )
    return new BError( "Length out of range" );
  if ( len < 0 )
    return new BError( "Length must not be negative" );

  if ( !len )
    len = static_cast<int>( replace_with->length() - index );

  string->ESubStrReplace( replace_with, static_cast<unsigned>( index ),
                          static_cast<unsigned>( len ) );

  return string.release();
}

// OMG I HATED THIS REQUEST. Ugly code, but necessary for all the checks
// just in case someone's code is bugged :(
Bscript::BObjectImp* BasicExecutorModule::mf_Compare()
{
  String str1( exec.paramAsString( 0 ) );
  String str2( exec.paramAsString( 1 ) );
  int pos1_index = static_cast<int>( exec.paramAsLong( 2 ) );
  int pos1_len = static_cast<int>( exec.paramAsLong( 3 ) );
  int pos2_index = static_cast<int>( exec.paramAsLong( 4 ) );
  int pos2_len = static_cast<int>( exec.paramAsLong( 5 ) );

  size_t str1length( str1.length() );
  size_t str2length( str2.length() );
  if ( pos1_index != 0 )
  {
    if ( pos1_index < 0 )
      return new BError( "Index must not be negative for param 1" );
    if ( static_cast<unsigned>( pos1_index - 1 ) > str1length )
      return new BError( "Index out of range for param 1" );
  }
  if ( pos2_index != 0 )
  {
    if ( pos2_index < 0 )
      return new BError( "Index must not be negative for param 2" );
    if ( static_cast<unsigned>( pos2_index - 1 ) > str2length )
      return new BError( "Index out of range for param 2" );
  }


  if ( pos1_len < 0 )
    return new BError( "Length must not be negative for param 1" );
  if ( static_cast<unsigned>( pos1_len ) > ( str1length - pos1_index ) )
    return new BError( "Length out of range for param 1" );
  if ( pos2_len < 0 )
    return new BError( "Length must not be negative for param 2" );
  if ( static_cast<unsigned>( pos2_len ) > ( str2length - pos2_index ) )
    return new BError( "Length out of range for param 2" );


  if ( pos1_index == 0 )
  {
    if ( !str1.compare( str2 ) )
      return new BLong( 0 );
    else
      return new BLong( 1 );
  }
  else if ( pos1_index > 0 && pos2_index == 0 )
  {
    if ( !str1.compare( pos1_index - 1, pos1_len, str2 ) )
      return new BLong( 0 );
    else
      return new BLong( 1 );
  }
  else
  {
    if ( !str1.compare( pos1_index - 1, pos1_len, str2, pos2_index - 1, pos2_len ) )
      return new BLong( 0 );
    else
      return new BLong( 1 );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_Lower()
{
  String* string = new String( exec.paramAsString( 0 ) );
  string->toLower();
  return string;
}

Bscript::BObjectImp* BasicExecutorModule::mf_Upper()
{
  String* string = new String( exec.paramAsString( 0 ) );
  string->toUpper();
  return string;
}

Bscript::BObjectImp* BasicExecutorModule::mf_CInt()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTLong ) )
    return imp->copy();
  else if ( auto* s = impptrIf<String>( imp ) )
    return new BLong( strtoul( s->data(), nullptr, 0 ) );
  else if ( auto* d = impptrIf<Double>( imp ) )
    return new BLong( static_cast<int>( d->value() ) );
  return new BLong( 0 );
}

Bscript::BObjectImp* BasicExecutorModule::mf_CDbl()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( auto* l = impptrIf<BLong>( imp ) )
    return new Double( l->value() );
  else if ( auto* s = impptrIf<String>( imp ) )
    return new Double( strtod( s->data(), nullptr ) );
  else if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
    return imp->copy();
  return new Double( 0 );
}

Bscript::BObjectImp* BasicExecutorModule::mf_CStr()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  return new String( imp->getStringRep() );
}

Bscript::BObjectImp* BasicExecutorModule::mf_CAsc()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( auto* str = impptrIf<String>( imp ) )
  {
    std::unique_ptr<String> substr( str->StrStr( 1, 1 ) );
    const auto& utf16 = substr->toUTF16();
    if ( utf16.empty() )
      return new BLong( 0 );
    else if ( utf16.size() > 1 )
      return new BError( "Cannot be represented by a single number" );
    return new BLong( utf16[0] );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_CAscZ()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  String tmp( imp->getStringRep() );
  int nullterm = static_cast<int>( exec.paramAsLong( 1 ) );
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
  const auto& utf16 = tmp.toUTF16();
  for ( const auto& code : utf16 )
  {
    arr->addElement( new BLong( code ) );
  }
  if ( nullterm )
    arr->addElement( new BLong( 0 ) );

  return arr.release();
}

Bscript::BObjectImp* BasicExecutorModule::mf_CChr()
{
  int val;
  if ( getParam( 0, val ) )
  {
    return new String( String::fromUTF16( static_cast<u16>( val & 0xffff ) ) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_CChrZ()
{
  std::string res;
  Bscript::ObjArray* arr =
      static_cast<Bscript::ObjArray*>( exec.getParamImp( 0, Bscript::BObjectImp::OTArray ) );
  int break_first_null = static_cast<int>( exec.paramAsLong( 1 ) );
  if ( !arr )
    return new BError( "Invalid parameter type" );
  return String::fromUCArray( arr, break_first_null != 0 );
}

Bscript::BObjectImp* BasicExecutorModule::mf_Hex()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( auto* l = impptrIf<BLong>( imp ) )
  {
    char s[20];
    snprintf( s, Clib::arsize( s ), "0x%X", static_cast<unsigned int>( l->value() ) );
    return new String( s );
  }
  else if ( auto* d = impptrIf<Double>( imp ) )
  {
    char s[20];
    snprintf( s, Clib::arsize( s ), "0x%X", static_cast<unsigned int>( d->value() ) );
    return new String( s );
  }
  else if ( auto* simp = impptrIf<String>( imp ) )
  {
    char s[20];
    snprintf( s, Clib::arsize( s ), "0x%X",
              static_cast<unsigned int>( strtoul( simp->data(), nullptr, 0 ) ) );
    return new String( s );
  }
  return new BError( "Hex() expects an Integer, Real, or String" );
}

#ifdef __unix__
char* itoa( int value, char* result, int base )
{
  // check that the base is valid
  if ( base < 2 || base > 16 )
  {
    *result = 0;
    return result;
  }

  char* out = result;
  int quotient = value;

  do
  {
    *out = "0123456789abcdef"[std::abs( quotient % base )];
    ++out;
    quotient /= base;
  } while ( quotient );

  // Only apply negative sign for base 10
  if ( value < 0 && base == 10 )
    *out++ = '-';

  std::reverse( result, out );
  *out = 0;
  return result;
}
#endif

Bscript::BObjectImp* BasicExecutorModule::mf_Bin()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( auto* v = impptrIf<BLong>( imp ) )
  {
    int number = v->value();
#ifdef __APPLE__
    std::string bits = std::bitset<2>( number ).to_string();
    return new String( bits );
#else
    char buffer[sizeof( int ) * 8 + 1];
    return new String( itoa( number, buffer, 2 ) );
#endif
  }
  return new BError( "Bin() expects an Integer" );
}

Bscript::BObjectImp* BasicExecutorModule::mf_SplitWords()
{
  Bscript::BObjectImp* bimp_split = exec.getParamImp( 0 );
  std::string source = bimp_split->getStringRep();

  const String* bimp_delimiter;
  if ( !exec.getStringParam( 1, bimp_delimiter ) )
  {
    return new BError( "Invalid parameter type." );
  }
  std::string delimiter = bimp_delimiter->getStringRep();

  // max_split parameter
  int max_split = -1;
  int count = 0;

  if ( exec.hasParams( 3 ) )
  {
    max_split = static_cast<int>( exec.paramAsLong( 2 ) );
  }

  std::unique_ptr<Bscript::ObjArray> objarr( new Bscript::ObjArray );

  // Support for how it previously worked.
  // Kept to support spaces and tabs as the same.
  if ( delimiter == " " )
  {
    ISTRINGSTREAM is( source );
    std::string tmp;
    std::streamoff tellg = -1;
    bool splitted = false;

    while ( is >> tmp )
    {
      tellg = is.tellg();
      if ( count == max_split && tellg != -1 )
      {  // added max_split parameter
        splitted = true;
        break;
      }
      objarr->addElement( new String( tmp ) );
      tmp = "";
      count += 1;
    }

    // Merges the remaining of the string
    if ( splitted )
    {
      std::string remaining_string;
      remaining_string = source.substr( tellg - tmp.length(), source.length() );
      objarr->addElement( new String( remaining_string ) );
    }

    return objarr.release();
  }

  // New delimiter support.
  std::string new_string = source;
  std::string::size_type found;
  do
  {
    found = new_string.find( delimiter, 0 );
    if ( found == std::string::npos )
      break;
    else if ( count == max_split )
    {  // added max_split parameter
      break;
    }

    std::string add_string = new_string.substr( 0, found );

    // Shinigami: will not hang server on queue of delimiter
    // if ( add_string.empty() )
    //  continue;

    objarr->addElement( new String( add_string ) );
    std::string tmp_string = new_string.substr( found + delimiter.length(), new_string.length() );
    new_string = tmp_string;
    count += 1;
  } while ( found != std::string::npos );

  // Catch leftovers here.
  if ( !new_string.empty() )
    objarr->addElement( new String( new_string ) );

  return objarr.release();
}

Bscript::BObjectImp* BasicExecutorModule::mf_Pack()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  return new String( imp->pack() );
}

Bscript::BObjectImp* BasicExecutorModule::mf_Unpack()
{
  const String* str;

  if ( exec.getStringParam( 0, str ) )
  {
    return Bscript::BObjectImp::unpack( str->data() );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}
Bscript::BObjectImp* BasicExecutorModule::mf_TypeOf()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  return new String( imp->typeOf() );
}
Bscript::BObjectImp* BasicExecutorModule::mf_SizeOf()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  return new BLong( static_cast<int>( imp->sizeEstimate() ) );
}
Bscript::BObjectImp* BasicExecutorModule::mf_TypeOfInt()
{
  BObjectImp* imp = exec.getParamImp( 0 );
  return new BLong( imp->typeOfInt() );
}

picojson::value recurseE2J( BObjectImp* value )
{
  if ( auto* s = impptrIf<String>( value ) )
  {
    return picojson::value( s->getStringRep() );
  }
  else if ( auto* l = impptrIf<BLong>( value ) )
  {
    return picojson::value( static_cast<double>( l->value() ) );
  }
  else if ( auto* d = impptrIf<Double>( value ) )
  {
    return picojson::value( d->value() );
  }
  else if ( auto* b = impptrIf<BBoolean>( value ) )
  {
    return picojson::value( b->value() );
  }
  else if ( auto* a = impptrIf<ObjArray>( value ) )
  {
    picojson::array jsonArr;

    for ( const auto& elem : a->ref_arr )
    {
      BObject* bo = elem.get();
      if ( bo == nullptr )
        continue;
      BObjectImp* imp = bo->impptr();
      jsonArr.push_back( recurseE2J( imp ) );
    }
    return picojson::value( jsonArr );
  }
  else if ( auto* bstruct = impptrIf<BStruct>( value ) )
  {
    picojson::object jsonObj;
    for ( const auto& content : bstruct->contents() )
    {
      BObjectImp* imp = content.second->impptr();
      jsonObj.insert( std::pair<std::string, picojson::value>( content.first, recurseE2J( imp ) ) );
    }
    return picojson::value( jsonObj );
  }
  else if ( auto* dict = impptrIf<BDictionary>( value ) )
  {
    picojson::object jsonObj;
    for ( const auto& content : dict->contents() )
    {
      BObjectImp* imp = content.second->impptr();
      jsonObj.insert( std::pair<std::string, picojson::value>( content.first->getStringRep(),
                                                               recurseE2J( imp ) ) );
    }
    return picojson::value( jsonObj );
  }
  return picojson::value();
}
Bscript::BObjectImp* BasicExecutorModule::mf_PackJSON()
{
  BObjectImp* imp = exec.getParamImp( 0 );
  auto prettify = exec.getParamImp( 1 )->isTrue();

  return new String( recurseE2J( imp ).serialize( prettify ) );
}

Bscript::BObjectImp* recurseJ2E( const picojson::value& v )
{
  if ( v.is<std::string>() )
  {
    return new String( v.get<std::string>() );
  }
  else if ( v.is<double>() )
  {
    // Possible improvement: separate into BLong and Double
    return new Double( v.get<double>() );
  }
  else if ( v.is<bool>() )
  {
    return new BBoolean( v.get<bool>() );
  }
  else if ( v.is<picojson::array>() )
  {
    std::unique_ptr<ObjArray> objarr( new ObjArray );
    const picojson::array& arr = v.get<picojson::array>();
    for ( const auto& elem : arr )
    {
      objarr->addElement( recurseJ2E( elem ) );
    }
    return objarr.release();
  }
  else if ( v.is<picojson::object>() )
  {
    std::unique_ptr<BStruct> objstruct( new BStruct );
    for ( const auto& content : v.get<picojson::object>() )
    {
      objstruct->addMember( content.first.c_str(), recurseJ2E( content.second ) );
    }
    return objstruct.release();
  }
  else
    return UninitObject::create();
}

Bscript::BObjectImp* BasicExecutorModule::mf_UnpackJSON()
{
  const String* str;

  if ( exec.getStringParam( 0, str ) )
  {
    picojson::value v;
    std::string err = picojson::parse( v, str->data() );
    if ( !err.empty() )
    {
      return new BError( err );
    }
    return recurseJ2E( v );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_Boolean()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( auto* l = impptrIf<BLong>( imp ) )
    return new BBoolean( l->value() != 0 );
  else if ( auto* b = impptrIf<BBoolean>( imp ) )
    return new BBoolean( *b );
  return new BError( "Boolean() expects an Integer or Boolean" );
}

/*
Base64 encode/decode snippit by Rene Nyffenegger rene.nyffenegger@adp-gmbh.ch
Copyright (C) 2004-2008 Rene Nyffenegger
*/
Bscript::BObjectImp* BasicExecutorModule::mf_EncodeBase64()
{
  std::string encoded_string = exec.paramAsString( 0 );

  if ( encoded_string.empty() )
    return new BError( "Invalid parameter type" );

  unsigned int in_len = static_cast<unsigned int>( encoded_string.length() );

  unsigned char const* bytes_to_encode =
      reinterpret_cast<const unsigned char*>( encoded_string.c_str() );
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while ( in_len-- )
  {
    char_array_3[i++] = *( bytes_to_encode++ );
    if ( i == 3 )
    {
      char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
      char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
      char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );
      char_array_4[3] = char_array_3[2] & 0x3f;

      for ( i = 0; ( i < 4 ); i++ )
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if ( i )
  {
    for ( j = i; j < 3; j++ )
      char_array_3[j] = '\0';

    char_array_4[0] = ( char_array_3[0] & 0xfc ) >> 2;
    char_array_4[1] = ( ( char_array_3[0] & 0x03 ) << 4 ) + ( ( char_array_3[1] & 0xf0 ) >> 4 );
    char_array_4[2] = ( ( char_array_3[1] & 0x0f ) << 2 ) + ( ( char_array_3[2] & 0xc0 ) >> 6 );
    char_array_4[3] = char_array_3[2] & 0x3f;

    for ( j = 0; ( j < i + 1 ); j++ )
      ret += base64_chars[char_array_4[j]];

    while ( ( i++ < 3 ) )
      ret += '=';
  }

  return new String( ret );
}

Bscript::BObjectImp* BasicExecutorModule::mf_DecodeBase64()
{
  std::string encoded_string = exec.paramAsString( 0 );

  if ( encoded_string.empty() )
    return new BError( "Invalid parameter type" );

  int in_len = static_cast<int>( encoded_string.size() );
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while ( in_len-- && ( encoded_string[in_] != '=' ) && is_base64( encoded_string[in_] ) )
  {
    char_array_4[i++] = encoded_string[in_];
    in_++;
    if ( i == 4 )
    {
      for ( i = 0; i < 4; i++ )
        char_array_4[i] = static_cast<u8>( base64_chars.find( char_array_4[i] ) );

      char_array_3[0] = ( char_array_4[0] << 2 ) + ( ( char_array_4[1] & 0x30 ) >> 4 );
      char_array_3[1] = ( ( char_array_4[1] & 0xf ) << 4 ) + ( ( char_array_4[2] & 0x3c ) >> 2 );
      char_array_3[2] = ( ( char_array_4[2] & 0x3 ) << 6 ) + char_array_4[3];

      for ( i = 0; ( i < 3 ); i++ )
        ret += char_array_3[i];
      i = 0;
    }
  }

  if ( i )
  {
    for ( j = i; j < 4; j++ )
      char_array_4[j] = 0;

    for ( j = 0; j < 4; j++ )
      char_array_4[j] = static_cast<u8>( base64_chars.find( char_array_4[j] ) );

    char_array_3[0] = ( char_array_4[0] << 2 ) + ( ( char_array_4[1] & 0x30 ) >> 4 );
    char_array_3[1] = ( ( char_array_4[1] & 0xf ) << 4 ) + ( ( char_array_4[2] & 0x3c ) >> 2 );
    char_array_3[2] = ( ( char_array_4[2] & 0x3 ) << 6 ) + char_array_4[3];

    for ( j = 0; ( j < i - 1 ); j++ )
      ret += char_array_3[j];
  }

  return new String( ret );
}

Bscript::BObjectImp* BasicExecutorModule::mf_RegExp()
{
  const String* expr;
  const String* flags;
  if ( !getStringParam( 0, expr ) || !getStringParam( 1, flags ) )
    return new BError( "Invalid parameter type" );

  boost::regex_constants::syntax_option_type flag = boost::regex_constants::ECMAScript;
  bool global = false;
  bool multiline = false;

  for ( const auto& ch : flags->value() )
  {
    switch ( ch )
    {
    case 'i':
      flag |= boost::regex_constants::icase;
      break;
    case 'm':
      // flag |= std::regex_constants::multiline;
      multiline = true;
      break;
    case 'g':
      global = true;
      break;
    default:
      return new BError( "Invalid flag character" );
    }
  }

  try
  {
    return new BRegExp( expr->getStringRep(), flag, global, multiline );
  }
  catch ( ... )
  {
    return new BError( "Invalid regular expression" );
  }
}

}  // namespace Module
}  // namespace Pol
