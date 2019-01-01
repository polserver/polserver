/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef BINARYSCROBJ_H
#define BINARYSCROBJ_H

#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include <fstream>
#include <string>

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Core
{
class BinFile final
{
public:
  BinFile() = default;
  BinFile( const std::string& filename, std::ios::openmode mode );
  BinFile(const BinFile& ) = delete;
  BinFile operator=( const BinFile& ) = delete;
  ~BinFile();

  bool Open( const std::string& filename, std::ios::openmode mode );
  void Close();

  template <class T>
  bool Read( T& val )
  {
    return ReadBuffer( &val, sizeof( T ) );
  }

  template <class T>
  bool Read( T* arr, size_t count )
  {
    return ReadBuffer( arr, sizeof( T ) * count );
  }

  template <class T>
  bool Write( T& val )
  {
    return WriteBuffer( &val, sizeof( T ) );
  }

  bool WriteString( const char* chr, unsigned len );
  bool Seek( std::fstream::pos_type abs_offset, std::ios::seekdir origin );
  std::fstream::pos_type FileSize( Bscript::Executor& exec );
  std::fstream::pos_type Tell();
  int Peek();
  void Flush();
  bool IsOpen();


private:
  bool ReadBuffer( void* buffer, std::streamsize length );
  bool WriteBuffer( void* buffer, std::streamsize length );

  std::fstream _file;
};

class BBinaryfile final : public Bscript::BObjectImp
{
public:
  BBinaryfile();
  BBinaryfile( std::string filename, unsigned short mode, bool _bigendian );
  ~BBinaryfile();
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex,
                                               bool forcebuiltin = false ) override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;
  virtual const char* typeOf() const override { return "BinaryFile"; }
  virtual u8 typeOfInt() const override { return OTBinaryFile; }
  virtual bool isTrue() const override;
  virtual bool operator==( const Bscript::BObjectImp& objimp ) const override;

private:
  mutable BinFile file;
  std::string _filename;
  std::ios::openmode openmode;
  bool bigendian;
};
}
}

#endif
