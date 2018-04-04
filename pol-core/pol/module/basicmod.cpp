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
#include <picojson.h>
#include <string>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/dict.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../bscript/impunicode.h"
#include "../../clib/stlutil.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

BasicExecutorModule::BasicExecutorModule( Executor& exec )
    : Bscript::TmplExecutorModule<BasicExecutorModule>( "Basic", exec )
{
}

Bscript::BObjectImp* BasicExecutorModule::len()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTArray ) )
  {
    Bscript::ObjArray* arr = static_cast<Bscript::ObjArray*>( imp );
    return new BLong( static_cast<int>( arr->ref_arr.size() ) );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTString )
    || imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    BaseString* bs = static_cast<BaseString*>( imp );
    return new BLong( static_cast<int>( bs->length() ) );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTError ) )
  {
    BError* err = static_cast<BError*>( imp );
    return new BLong( static_cast<int>( err->mapcount() ) );
  }
  else
  {
    return new BLong( 0 );
  }
}

Bscript::BObjectImp* BasicExecutorModule::find()
{
  exec.makeString( 0 );
  String* str = static_cast<String*>( exec.getParamImp( 0 ) );
  const char* s = exec.paramAsString( 1 );
  int d = static_cast<int>( exec.paramAsLong( 2 ) );

  int posn = str->find( d ? ( d - 1 ) : 0, s ) + 1;

  return new BLong( posn );
}

Bscript::BObjectImp* BasicExecutorModule::mf_substr()
{
  exec.makeString( 0 );
  String* str = static_cast<String*>( exec.getParamImp( 0 ) );
  int start = static_cast<int>( exec.paramAsLong( 1 ) );
  int length = static_cast<int>( exec.paramAsLong( 2 ) );

  return str->substr( start-1, length );
}

Bscript::BObjectImp* BasicExecutorModule::mf_Trim()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );

  BaseString::TrimTypes type;
  switch ( exec.paramAsLong( 1 ) ) {
  case 1:
    type = BaseString::TrimTypes::TRIM_LEFT;
    break;
  case 2:
    type = BaseString::TrimTypes::TRIM_RIGHT;
    break;
  default:
    type = BaseString::TrimTypes::TRIM_BOTH;
  }
  const char* cset = exec.paramAsString( 2 );

  if ( imp->isa( Bscript::BObjectImp::OTString ) )
  {
    String* string = static_cast<String*>( imp );
    String* newStr = new String(*string);
    newStr->trim(std::string(cset), type);
    return newStr;
  }
  else if ( imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    //TODO: implement it on Unicode
    return new BError( "Not implemented. Please report this error." );
  }
  else
  {
    return new BError( "Param 1 must be a string or unicode." );
  }
}

/*
eScript Function Useage

StrReplace(str, to_replace, replace_with);
*/
Bscript::BObjectImp* BasicExecutorModule::mf_StrReplace()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );

  if ( imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    //TODO: implement unicode
    return new BError( "Not implemented yet. Please report this error." );
  }

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

  string->replace( *to_replace, *replace_with );

  return string.release();
}

// SubStrReplace(str, replace_with, start, length:=0);
Bscript::BObjectImp* BasicExecutorModule::mf_SubStrReplace()
{
  //TODO: implement Unicode
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

  string->replace( *replace_with, static_cast<unsigned>( index ), static_cast<unsigned>( len ) );

  return string.release();
}

// OMG I HATED THIS REQUEST. Ugly code, but necessary for all the checks
// just in case someone's code is bugged :(
Bscript::BObjectImp* BasicExecutorModule::mf_Compare()
{
  std::string str1 = exec.paramAsString( 0 );
  std::string str2 = exec.paramAsString( 1 );
  int pos1_index = static_cast<int>( exec.paramAsLong( 2 ) );
  int pos1_len = static_cast<int>( exec.paramAsLong( 3 ) );
  int pos2_index = static_cast<int>( exec.paramAsLong( 4 ) );
  int pos2_len = static_cast<int>( exec.paramAsLong( 5 ) );

  if ( pos1_index != 0 )
  {
    if ( pos1_index < 0 )
      return new BError( "Index must not be negative for param 1" );
    if ( static_cast<unsigned>( pos1_index - 1 ) > str1.length() )
      return new BError( "Index out of range for param 1" );
  }
  if ( pos2_index != 0 )
  {
    if ( pos2_index < 0 )
      return new BError( "Index must not be negative for param 2" );
    if ( static_cast<unsigned>( pos2_index - 1 ) > str2.length() )
      return new BError( "Index out of range for param 2" );
  }


  if ( pos1_len < 0 )
    return new BError( "Length must not be negative for param 1" );
  if ( static_cast<unsigned>( pos1_len ) > ( str1.length() - pos1_index ) )
    return new BError( "Length out of range for param 1" );
  if ( pos2_len < 0 )
    return new BError( "Length must not be negative for param 2" );
  if ( static_cast<unsigned>( pos2_len ) > ( str2.length() - pos2_index ) )
    return new BError( "Length out of range for param 2" );


  if ( pos1_index == 0 )
  {
    unsigned int result = str1.compare( str2 );
    if ( result != 0 )
      return new BLong( 0 );
    else
      return new BLong( 1 );
  }
  else if ( pos1_index > 0 && pos2_index == 0 )
  {
    unsigned int result = str1.compare( pos1_index - 1, pos1_len, str2 );
    if ( result != 0 )
      return new BLong( 0 );
    else
      return new BLong( 1 );
  }
  else
  {
    unsigned int result = str1.compare( pos1_index - 1, pos1_len, str2, pos2_index - 1, pos2_len );
    if ( result != 0 )
      return new BLong( 0 );
    else
      return new BLong( 1 );
  }
}

Bscript::BObjectImp* BasicExecutorModule::lower()
{
  String* string = new String( exec.paramAsString( 0 ) );
  string->toLower();
  return string;
}

Bscript::BObjectImp* BasicExecutorModule::upper()
{
  String* string = new String( exec.paramAsString( 0 ) );
  string->toUpper();
  return string;
}

Bscript::BObjectImp* BasicExecutorModule::mf_CInt()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTLong ) )
  {
    return imp->copy();
  }
  else if ( imp->isa( Bscript::BObjectImp::OTString )
    || imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    BaseString* str = static_cast<BaseString*>( imp );
    return new BLong( str->intval() );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
  {
    Double* dbl = static_cast<Double*>( imp );
    return new BLong( static_cast<int>( dbl->value() ) );
  }
  else
  {
    return new BLong( 0 );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_CDbl()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTLong ) )
  {
    BLong* lng = static_cast<BLong*>( imp );
    return new Double( lng->value() );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTString )
    || imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    BaseString* str = static_cast<BaseString*>( imp );
    return new Double( str->dblval() );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
  {
    return imp->copy();
  }
  else
  {
    return new Double( 0 );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_CStr()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  return new String( imp->getStringRep() );
}

Bscript::BObjectImp* BasicExecutorModule::mf_CAsc()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTString ) )
  {
    String* str = static_cast<String*>( imp );
    return new BLong( static_cast<unsigned char>( str->value()[0] ) );
  }
  if ( imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    Unicode* str = static_cast<Unicode*>( imp );
    return new BLong( static_cast<char32_t>(str->value()[0]) );
  }
  else
  {
    return new BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* BasicExecutorModule::mf_CAscZ()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  std::string tmp = imp->getStringRep();
  int nullterm = static_cast<int>( exec.paramAsLong( 1 ) );
  std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
  for ( size_t i = 0; i < tmp.size(); ++i )
  {
    arr->addElement( new BLong( static_cast<unsigned char>( tmp[i] ) ) );
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
    char s[2];
    s[0] = static_cast<char>( val );
    s[1] = '\0';
    return new String( s );
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
  for ( Bscript::ObjArray::const_iterator itr = arr->ref_arr.begin(), itrend = arr->ref_arr.end();
        itr != itrend; ++itr )
  {
    BObject* bo = ( itr->get() );
    if ( bo == NULL )
      continue;
    Bscript::BObjectImp* imp = bo->impptr();
    if ( imp )
    {
      if ( imp->isa( Bscript::BObjectImp::OTLong ) )
      {
        BLong* blong = static_cast<BLong*>( imp );
        if ( break_first_null && blong->value() == 0 )
          break;
        char s[2];
        s[0] = static_cast<char>( blong->value() );
        s[1] = '\0';
        res += s;
      }
    }
  }
  return new String( res );
}

Bscript::BObjectImp* BasicExecutorModule::mf_Hex()
{
  Bscript::BObjectImp* imp = exec.getParamImp( 0 );
  if ( imp->isa( Bscript::BObjectImp::OTLong ) )
  {
    BLong* plong = static_cast<BLong*>( imp );
    char s[20];
    sprintf( s, "0x%X", static_cast<unsigned int>( plong->value() ) );
    return new String( s );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
  {
    Double* pdbl = static_cast<Double*>( imp );
    char s[20];
    sprintf( s, "0x%X", static_cast<unsigned int>( pdbl->value() ) );
    return new String( s );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTString )
    || imp->isa( Bscript::BObjectImp::OTUnicode ) )
  {
    BaseString* str = static_cast<BaseString*>( imp );
    char s[20];
    sprintf( s, "0x%X", str->intval() );
    return new String( s );
  }
  else
  {
    return new BError( "Hex() expects an Integer, Real, String or Unicode" );
  }
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
  if ( imp->isa( Bscript::BObjectImp::OTLong ) )
  {
    BLong* plong = static_cast<BLong*>( imp );
    int number = plong->value();
    char buffer[sizeof( int ) * 8 + 1];
    return new String( itoa( number, buffer, 2 ) );
  }
  else
  {
    return new BError( "Bin() expects an Integer" );
  }
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
    return Bscript::BObjectImp::unpack( str->value().c_str() );
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

picojson::value recurseE2J( BObjectImp* v )
{
  if ( v->isa( BObjectImp::OTString ) )
  {
    return picojson::value( v->getStringRep() );
  }
  else if ( v->isa( BObjectImp::OTLong ) )
  {
    int intVal = static_cast<BLong*>( v )->value();
    return picojson::value( static_cast<double>( intVal ) );
  }
  else if ( v->isa( BObjectImp::OTDouble ) )
  {
    return picojson::value( static_cast<Double*>( v )->value() );
  }
  else if ( v->isa( BObjectImp::OTBoolean ) )
  {
    return picojson::value( static_cast<BBoolean*>( v )->value() );
  }
  else if ( v->isa( BObjectImp::OTArray ) )
  {
    ObjArray* arr = static_cast<ObjArray*>( v );
    picojson::array jsonArr;

    for ( const auto& elem : arr->ref_arr )
    {
      BObject* bo = elem.get();
      if ( bo == nullptr )
        continue;
      BObjectImp* imp = bo->impptr();
      jsonArr.push_back( recurseE2J( imp ) );
    }
    return picojson::value( jsonArr );
  }
  else if ( v->isa( BObjectImp::OTStruct ) )
  {
    BStruct* bstruct = static_cast<BStruct*>( v );
    picojson::object jsonObj;
    for ( const auto& content : bstruct->contents() )
    {
      BObjectImp* imp = content.second->impptr();
      jsonObj.insert( std::pair<std::string, picojson::value>( content.first, recurseE2J( imp ) ) );
    }
    return picojson::value( jsonObj );
  }
  else if ( v->isa( BObjectImp::OTDictionary ) )
  {
    BDictionary* cpropdict = static_cast<Bscript::BDictionary*>( v );
    picojson::object jsonObj;
    for ( const auto& content : cpropdict->contents() )
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
  return new String( recurseE2J( imp ).serialize() );
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
    std::string err = picojson::parse( v, str->value() );
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
  if ( imp->isa( Bscript::BObjectImp::OTLong ) )
  {
    BLong* plong = static_cast<BLong*>( imp );
    return new BBoolean( plong->value() != 0 );
  }
  else if ( imp->isa( Bscript::BObjectImp::OTBoolean ) )
  {
    return new BBoolean( *static_cast<BBoolean*>( imp ) );
  }
  else
  {
    return new BError( "Boolean() expects an Integer or Boolean" );
  }
}
}  // namespace Module

namespace Bscript
{
using namespace Module;
template <>
TmplExecutorModule<BasicExecutorModule>::FunctionTable
    TmplExecutorModule<BasicExecutorModule>::function_table = {
        {"find", &BasicExecutorModule::find},
        {"len", &BasicExecutorModule::len},
        {"upper", &BasicExecutorModule::upper},
        {"lower", &BasicExecutorModule::lower},
        {"Substr", &BasicExecutorModule::mf_substr},
        {"Trim", &BasicExecutorModule::mf_Trim},
        {"StrReplace", &BasicExecutorModule::mf_StrReplace},
        {"SubStrReplace", &BasicExecutorModule::mf_SubStrReplace},
        {"Compare", &BasicExecutorModule::mf_Compare},
        {"CInt", &BasicExecutorModule::mf_CInt},
        {"CStr", &BasicExecutorModule::mf_CStr},
        {"CDbl", &BasicExecutorModule::mf_CDbl},
        {"CAsc", &BasicExecutorModule::mf_CAsc},
        {"CChr", &BasicExecutorModule::mf_CChr},
        {"CAscZ", &BasicExecutorModule::mf_CAscZ},
        {"CChrZ", &BasicExecutorModule::mf_CChrZ},
        {"Bin", &BasicExecutorModule::mf_Bin},
        {"Hex", &BasicExecutorModule::mf_Hex},
        {"SplitWords", &BasicExecutorModule::mf_SplitWords},
        {"Pack", &BasicExecutorModule::mf_Pack},
        {"Unpack", &BasicExecutorModule::mf_Unpack},
        {"TypeOf", &BasicExecutorModule::mf_TypeOf},
        {"SizeOf", &BasicExecutorModule::mf_SizeOf},
        {"TypeOfInt", &BasicExecutorModule::mf_TypeOfInt},
        {"Boolean", &BasicExecutorModule::mf_Boolean},
        {"PackJSON", &BasicExecutorModule::mf_PackJSON},
        {"UnpackJSON", &BasicExecutorModule::mf_UnpackJSON}};
}
}
