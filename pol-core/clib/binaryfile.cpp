/*
History
=======
2009/08/25 Shinigami: STLport-5.2.1 fix: Error message in BinaryFile::Seek stripped little bit.
2009/09/01 MuadDib:   STLPort-5.x fix: BinaryFile::Seek requires casting to long in decint under Win32

Notes
=======

*/

#include "stl_inc.h"

#include "binaryfile.h"
#include "passert.h"
#include "strutil.h"
namespace Pol {
  namespace Clib {
	BinaryFile::BinaryFile()
	{}

	BinaryFile::BinaryFile( const string& filename, ios::openmode mode ) :
	  _filename( "" )
	{
	  Open( filename, mode );
	}

	BinaryFile::~BinaryFile()
	{}

	void BinaryFile::Open( const string& filename, ios::openmode mode )
	{
	  passert( !_file.is_open() );

	  _filename = filename;

	  _file.open( _filename.c_str(), mode | ios::binary );
	  if( !_file.is_open() )
		throw runtime_error( "BinaryFile::Open('" + _filename + ", " + hexint( mode ) + ") failed." );
	}

	void BinaryFile::Close()
	{
	  passert( _file.is_open() );

	  _file.close();
	  _filename = "";
	}

	void BinaryFile::Seek( fstream::pos_type abs_offset )
	{
	  passert( _file.is_open() );

	  if( !_file.seekg( abs_offset, ios::beg ) )
		throw runtime_error( "BinaryFile::Seek('" + _filename + "') failed." );
	}

	void BinaryFile::ReadBuffer( void* buffer, streamsize length )
	{
	  if( !_file.read( reinterpret_cast<char*>( buffer ), length ) )
		throw runtime_error( "BinaryFile::Read('" + _filename + "') failed to read " + decint( static_cast<int>( length ) ) + " bytes." );
	}

	fstream::pos_type BinaryFile::FileSize()
	{
	  passert( _file.is_open() );

	  fstream::pos_type save_pos = _file.tellg();
	  if( save_pos == fstream::pos_type( -1 ) )
		throw runtime_error( "BinaryFile::FileSize('" + _filename + "' failed to determine current position." );

	  if( !_file.seekg( 0, ios::end ) )
		throw runtime_error( "BinaryFile::FileSize('" + _filename + "') failed to seek to end of file." );
	  fstream::pos_type size = _file.tellg();
	  if( size == fstream::pos_type( -1 ) )
		throw runtime_error( "BinaryFile::FileSize('" + _filename + "') could not determine file size." );

	  if( !_file.seekg( save_pos ) )
		throw runtime_error( "BinaryFile::FileSize('" + _filename + "') failed to seek to original position." );

	  return size;
	}

	size_t BinaryFile::GetElementCount( size_t elemsize )
	{
	  fstream::pos_type filesize = FileSize();
	  if( ( filesize % elemsize ) != 0 )
	  {
		throw runtime_error( _filename + " does not contain an integral number of elements of size " + decint( elemsize ) );
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