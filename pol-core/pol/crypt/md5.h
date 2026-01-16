//////////////////////////////////////////////////////////////////////
//
// crypt/md5.h
//
//////////////////////////////////////////////////////////////////////

#ifndef __MD5_H__
#define __MD5_H__
namespace Pol
{
namespace Crypt
{
// Struct for MD5

using md5_state = struct tagmd5_state
{
  unsigned int count[2];
  unsigned int abcd[4];
  unsigned char buf[64];
};

class MD5Crypt
{
  // Constructor / Destructor
public:
  MD5Crypt();
  ~MD5Crypt();

  // Member Functions

public:
  void Init( unsigned char* Data, unsigned int Size );
  void Encrypt( unsigned char* in, unsigned char* out, int len );

protected:
  static void process( md5_state* pms, const unsigned char* data );
  static void start( md5_state* pms );
  static void append( md5_state* pms, const unsigned char* data, int nbytes );
  static void finish( md5_state* pms, unsigned char digest[16] );

  unsigned int TableIdx;
  unsigned char Digest[16];
};
}  // namespace Crypt
}  // namespace Pol
#endif  //__MD5_H__
