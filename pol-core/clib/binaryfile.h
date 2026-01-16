/** @file
 *
 * @par History
 */


#ifndef CLIB_BINARYFILE_H
#define CLIB_BINARYFILE_H

#include <fstream>
#include <string>
#include <vector>


namespace Pol::Clib
{
class BinaryFile
{
public:
  BinaryFile();
  BinaryFile( const std::string& filename, std::ios::openmode mode );
  ~BinaryFile();

  void Open( const std::string& filename, std::ios::openmode mode );
  void Close();

  template <class T>
  void Read( T& val )
  {
    ReadBuffer( &val, sizeof( T ) );
  }

  template <class T>
  void Read( T* arr, size_t count )
  {
    ReadBuffer( arr, sizeof( T ) * count );
  }

  template <class T>
  void ReadVector( std::vector<T>& vec )
  {
    size_t count = GetElementCount( sizeof( T ) );
    vec.resize( count );
    if ( count > 0 )
      Read( &vec[0], count );
  }

  void Seek( std::fstream::pos_type abs_offset );
  std::fstream::pos_type FileSize();
  size_t sizeEstimate() const;

private:
  void ReadBuffer( void* buffer, std::streamsize length );
  size_t GetElementCount( size_t elemsize );

  std::fstream _file;
  std::string _filename;
};
}  // namespace Pol::Clib

#endif
