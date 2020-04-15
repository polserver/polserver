/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "binaryfilescrobj.h"

#include <fstream>
#include <stddef.h>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/clib_endian.h"
#include "../clib/rawtypes.h"

namespace Pol
{
namespace Core
{
BBinaryfile::BBinaryfile()
    : Bscript::BObjectImp( OTBinaryFile ),
      _filename( "" ),
      openmode( std::ios::in ),
      bigendian( true )
{
}

BBinaryfile::BBinaryfile( std::string filename, unsigned short mode, bool _bigendian )
    : Bscript::BObjectImp( OTBinaryFile ),
      _filename( std::move( filename ) ),
      bigendian( _bigendian )
{
  using std::ios;

  // FIXME: ms::stl has different flag values then stlport :(
  openmode = static_cast<ios::openmode>( 0x0 );
  if ( mode & 0x01 )
    openmode |= ios::in;
  if ( mode & 0x02 )
    openmode |= ios::out;
  if ( mode & 0x04 )
    openmode |= ios::ate;
  if ( mode & 0x08 )
    openmode |= ios::app;
  if ( mode & 0x10 )
    openmode |= ios::trunc;

  if ( !file.Open( _filename, openmode ) )
    return;
}

BBinaryfile::~BBinaryfile()
{
  file.Close();
}

size_t BBinaryfile::sizeEstimate() const
{
  return sizeof( *this ) + _filename.capacity();
}

Bscript::BObjectRef BBinaryfile::get_member_id( const int /*id*/ )  // id test
{
  return Bscript::BObjectRef( new Bscript::BLong( 0 ) );
}
Bscript::BObjectRef BBinaryfile::get_member( const char* membername )
{
  Bscript::ObjMember* objmember = Bscript::getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  else
    return Bscript::BObjectRef( Bscript::UninitObject::create() );
}

Bscript::BObjectImp* BBinaryfile::call_method( const char* methodname, Bscript::Executor& ex )
{
  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_method_id( objmethod->id, ex );
  else
    return nullptr;
}

Bscript::BObjectImp* BBinaryfile::call_method_id( const int id, Bscript::Executor& ex,
                                                  bool /*forcebuiltin*/ )
{
  using namespace Bscript;
  switch ( id )
  {
  case MTH_CLOSE:
    file.Close();
    return new BLong( 1 );
  case MTH_SIZE:
    return new BLong( static_cast<int>( file.FileSize( ex ) ) );
  case MTH_SEEK:
  {
    using std::ios;

    if ( ex.numParams() != 2 )
      return new BError( "Seek requires 2 parameter." );
    int value, type;
    if ( ( !ex.getParam( 0, value ) ) || ( !ex.getParam( 1, type ) ) )
      return new BError( "Invalid parameter" );
    // FIXME: ms::stl has different flag values then stlport :(
    ios::seekdir seekdir;
    if ( type & 0x01 )
      seekdir = ios::cur;
    else if ( type & 0x02 )
      seekdir = ios::end;
    else
      seekdir = ios::beg;
    if ( !file.Seek( value, seekdir ) )
      return new BLong( 0 );
    return new BLong( 1 );
  }
  case MTH_TELL:
    return new BLong( static_cast<int>( file.Tell() ) );
  case MTH_PEEK:
    return new BLong( file.Peek() );
  case MTH_FLUSH:
    file.Flush();
    return new BLong( 1 );

  case MTH_GETINT32:
  {
    u32 _u32;
    if ( !file.Read( _u32 ) )
      return new BError( "Failed to read" );
    if ( bigendian )
      _u32 = cfBEu32( _u32 );
    return new BLong( _u32 );
  }
  case MTH_GETSINT32:
  {
    s32 _s32;
    if ( !file.Read( _s32 ) )
      return new BError( "Failed to read" );
    if ( bigendian )
      _s32 = cfBEu32( _s32 );
    return new BLong( _s32 );
  }
  case MTH_GETINT16:
  {
    u16 _u16;
    if ( !file.Read( _u16 ) )
      return new BError( "Failed to read" );
    if ( bigendian )
      _u16 = cfBEu16( _u16 );
    return new BLong( _u16 );
  }
  case MTH_GETSINT16:
  {
    s16 _s16;
    if ( !file.Read( _s16 ) )
      return new BError( "Failed to read" );
    if ( bigendian )
      _s16 = cfBEu16( _s16 );
    return new BLong( _s16 );
  }
  case MTH_GETINT8:
  {
    u8 _u8;
    if ( !file.Read( _u8 ) )
      return new BError( "Failed to read" );
    return new BLong( _u8 );
  }
  case MTH_GETSINT8:
  {
    s8 _s8;
    if ( !file.Read( _s8 ) )
      return new BError( "Failed to read" );
    return new BLong( _s8 );
  }
  case MTH_GETSTRING:
  {
    if ( ex.numParams() != 1 )
      return new BError( "GetString requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    if ( value < 1 )
      return new BError( "Len is negative or 0." );
    std::vector<unsigned char> _char;
    _char.resize( value );
    if ( !file.Read( &_char[0], value ) )
      return new BError( "Failed to read" );
    int len = 0;
    const char* _str = reinterpret_cast<const char*>( &_char[0] );
    // Returns maximum of len characters or up to the first null-byte
    while ( len < value && *( _str + len ) )
      len++;
    return new String( _str, len, String::Tainted::YES );
  }

  case MTH_SETINT32:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetInt32 requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    u32 _u32 = static_cast<u32>( value );
    if ( bigendian )
      _u32 = cfBEu32( _u32 );
    if ( !file.Write( _u32 ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }
  case MTH_SETSINT32:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetSInt32 requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    s32 _s32 = static_cast<s32>( value );
    if ( bigendian )
      _s32 = cfBEu32( _s32 );
    if ( !file.Write( _s32 ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }
  case MTH_SETINT16:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetInt16 requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    u16 _u16 = static_cast<u16>( value );
    if ( bigendian )
      _u16 = cfBEu16( _u16 );
    if ( !file.Write( _u16 ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }
  case MTH_SETSINT16:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetSInt16 requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    s16 _s16 = static_cast<s16>( value );
    if ( bigendian )
      _s16 = cfBEu16( _s16 );
    if ( !file.Write( _s16 ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }
  case MTH_SETINT8:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetInt8 requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    u8 _u8 = static_cast<u8>( value );
    if ( !file.Write( _u8 ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }
  case MTH_SETSINT8:
  {
    if ( ex.numParams() != 1 )
      return new BError( "SetSInt8 requires 1 parameter." );
    int value;
    if ( !ex.getParam( 0, value ) )
      return new BError( "Invalid parameter" );
    s8 _s8 = static_cast<s8>( value );
    if ( !file.Write( _s8 ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }
  case MTH_SETSTRING:
  {
    if ( ex.numParams() != 2 )
      return new BError( "SetString requires 2 parameters." );
    int value;
    const String* text;
    if ( ( !ex.getStringParam( 0, text ) ) || ( !ex.getParam( 1, value ) ) )
      return new BError( "Invalid parameter" );
    u32 len = static_cast<u32>( text->value().length() );
    if ( value == 1 )
      len++;
    if ( !file.WriteString( text->value().c_str(), len ) )
      return new BError( "Failed to write" );
    return new BLong( 1 );
  }

  default:
    return nullptr;
  }
}

Bscript::BObjectImp* BBinaryfile::copy() const
{
  return new BBinaryfile( _filename, static_cast<unsigned short>( openmode ), bigendian );
}

std::string BBinaryfile::getStringRep() const
{
  return "BinaryFile";
}

bool BBinaryfile::isTrue() const
{
  if ( _filename == "" )
    return false;
  return file.IsOpen();
}

bool BBinaryfile::operator==( const Bscript::BObjectImp& objimp ) const
{
  if ( objimp.isa( Bscript::BObjectImp::OTBinaryFile ) )
  {
    if ( ( (BBinaryfile&)objimp )._filename == _filename )
      return true;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  return false;
}

BinFile::BinFile( const std::string& filename, std::ios::openmode mode )
{
  Open( filename, mode );
}

BinFile::~BinFile()
{
  Close();
}

bool BinFile::Open( const std::string& filename, std::ios::openmode mode )
{
  if ( _file.is_open() )
    return true;

  _file.open( filename.c_str(), mode | std::ios::binary );
  if ( !_file.is_open() )
    return false;
  return true;
}

bool BinFile::IsOpen()
{
  return _file.is_open();
}

void BinFile::Close()
{
  if ( !_file.is_open() )
    return;

  _file.close();
}

bool BinFile::Seek( std::fstream::pos_type abs_offset, std::ios::seekdir origin )
{
  if ( !_file.is_open() )
    return false;

  if ( !_file.seekg( abs_offset, origin ) )
    return false;
  return true;
}

bool BinFile::ReadBuffer( void* buffer, std::streamsize length )
{
  if ( !_file.read( reinterpret_cast<char*>( buffer ), length ) )
    return false;
  return true;
}

bool BinFile::WriteBuffer( void* buffer, std::streamsize length )
{
  if ( !_file.write( reinterpret_cast<char*>( buffer ), length ) )
    return false;
  return true;
}

bool BinFile::WriteString( const char* chr, unsigned len )
{
  if ( !_file.write( chr, len ) )
    return false;
  return true;
}

std::fstream::pos_type BinFile::FileSize( Bscript::Executor& exec )
{
  if ( !_file.is_open() )
    return std::fstream::pos_type( 0 );

  std::fstream::pos_type save_pos = _file.tellg();
  if ( save_pos == std::fstream::pos_type( -1 ) )
  {
    exec.setFunctionResult(
        new Bscript::BError( "FileSize failed to determine current position." ) );
    return std::fstream::pos_type( 0 );
  }
  if ( !_file.seekg( 0, std::ios::end ) )
  {
    exec.setFunctionResult( new Bscript::BError( "FileSize failed to seek to end of file." ) );
    return std::fstream::pos_type( 0 );
  }
  std::fstream::pos_type size = _file.tellg();
  if ( size == std::fstream::pos_type( -1 ) )
  {
    exec.setFunctionResult( new Bscript::BError( "FileSize could not determine file size." ) );
    return std::fstream::pos_type( 0 );
  }
  if ( !_file.seekg( save_pos ) )
  {
    exec.setFunctionResult(
        new Bscript::BError( "FileSize failed to seek to original position." ) );
    return std::fstream::pos_type( 0 );
  }
  return size;
}

std::fstream::pos_type BinFile::Tell()
{
  if ( !_file.is_open() )
    return -1;
  return _file.tellg();
}

int BinFile::Peek()
{
  if ( !_file.is_open() )
    return -1;
  return _file.peek();
}

void BinFile::Flush()
{
  if ( _file.is_open() )
    _file.flush();
}
}  // namespace Core
}  // namespace Pol
