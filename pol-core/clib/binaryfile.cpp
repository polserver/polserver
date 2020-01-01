/** @file
 *
 * @par History
 * - 2009/08/25 Shinigami: STLport-5.2.1 fix: Error message in BinaryFile::Seek stripped little bit.
 * - 2009/09/01 MuadDib:   STLPort-5.x fix: BinaryFile::Seek requires casting to long in decint
 * under Win32
 */


#include "binaryfile.h"

#include <stdexcept>

#include "passert.h"
#include "strutil.h"

namespace Pol
{
namespace Clib
{
BinaryFile::BinaryFil e() {}

BinaryFile::BinaryFile( const std::string& filename, std::ios::openmode mode ) : _filename( "" )
{
  Open( filename, mode );
}

BinaryFile::~BinaryFile() {}

void BinaryFile::Open( const std::string& filename, std::ios::openmode mode )
{
  passert( !_file.is_open() );

  _filename = filename;

  _file.open( _filename.c_str(), mode | std::ios::binary );
  if ( !_file.is_open() )
    throw std::runtime_error( "BinaryFile::Open('" + _filename + ", " + hexint( mode ) +
                              ") failed." );
}

void BinaryFile::Close()
{
  passert( _file.is_open() );

  _file.close();
  _filename = "";
}

void BinaryFile::Seek( std::fstream::pos_type abs_offset )
{
  passert( _file.is_open() );

  if ( !_file.seekg( abs_offset, std::ios::beg ) )
    throw std::runtime_error( "BinaryFile::Seek('" + _filename + "') failed." );
}

void BinaryFile::ReadBuffer( void* buffer, std::streamsize length )
{
  if ( !_file.read( reinterpret_cast<char*>( buffer ), length ) )
    throw std::runtime_error( "BinaryFile::Read('" + _filename + "') failed to read " +
                              tostring( static_cast<int>( length ) ) + " bytes." );
}

std::fstream::pos_type BinaryFile::FileSize()
{
  passert( _file.is_open() );

  std::fstream::pos_type save_pos = _file.tellg();
  if ( save_pos == std::fstream::pos_type( -1 ) )
    throw std::runtime_error( "BinaryFile::FileSize('" + _filename +
                              "' failed to determine current position." );

  if ( !_file.seekg( 0, std::ios::end ) )
    throw std::runtime_error( "BinaryFile::FileSize('" + _filename +
                              "') failed to seek to end of file." );
  std::fstream::pos_type size = _file.tellg();
  if ( size == std::fstream::pos_type( -1 ) )
    throw std::runtime_error( "BinaryFile::FileSize('" + _filename +
                              "') could not determine file size." );

  if ( !_file.seekg( save_pos ) )
    throw std::runtime_error( "BinaryFile::FileSize('" + _filename +
                              "') failed to seek to original position." );

  return size;
}

size_t BinaryFile::GetElementCount( size_t elemsize )
{
  std::fstream::pos_type filesize = FileSize();
  if ( ( filesize % elemsize ) != 0 )
  {
    throw std::runtime_error( _filename +
                              " does not contain an integral number of elements of size " +
                              tostring( elemsize ) );
  }
  return static_cast<unsigned int>( filesize / elemsize );
}

size_t BinaryFile::sizeEstimate() const
{
  size_t size = sizeof( *this ) + _filename.capacity();
  return size;
}
}
}
