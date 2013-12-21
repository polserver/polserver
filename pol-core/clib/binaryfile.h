/*
History
=======

Notes
=======

*/

#ifndef CLIB_BINARYFILE_H
#define CLIB_BINARYFILE_H

namespace Pol {
  namespace Clib {


	class BinaryFile
	{
	public:
	  BinaryFile();
	  BinaryFile( const std::string& filename, ios::openmode mode );
	  ~BinaryFile();

	  void Open( const std::string& filename, ios::openmode mode );
	  void Close();

	  template<class T>
	  void Read( T& val )
	  {
		ReadBuffer( &val, sizeof( T ) );
	  }

	  template<class T>
	  void Read( T* arr, size_t count )
	  {
		ReadBuffer( arr, sizeof(T)* count );
	  }

	  template<class T>
	  void ReadVector( vector<T>& vec )
	  {
		size_t count = GetElementCount( sizeof( T ) );
		vec.resize( count );
		if( count > 0 )
		  Read( &vec[0], count );
	  }

	  void Seek( fstream::pos_type abs_offset );
	  fstream::pos_type FileSize();

	private:
	  void ReadBuffer( void* buffer, streamsize length );
	  size_t GetElementCount( size_t elemsize );

	  fstream _file;
	  std::string _filename;
	};

  }
}
#endif
