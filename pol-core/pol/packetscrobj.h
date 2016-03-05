/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef PACKETSCROBJ_H
#define PACKETSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../clib/rawtypes.h"

namespace Pol
{
namespace Core
{
class BPacket : public Bscript::BObjectImp
{
public:
  BPacket();
  BPacket( const BPacket& copyfrom );
  BPacket( u8 type, signed short length );
  BPacket( const unsigned char* data, unsigned short length, bool variable_len );
  ~BPacket();

  std::vector<unsigned char> buffer;
  virtual Bscript::BObjectRef get_member( const char* membername ) POL_OVERRIDE;
  virtual Bscript::BObjectRef get_member_id( const int id ) POL_OVERRIDE; //id test
  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* copy( ) const POL_OVERRIDE;
  virtual std::string getStringRep() const POL_OVERRIDE;
  bool SetSize( u16 newsize );
  Bscript::BObjectImp* SetSize( u16 newsize, bool giveReturn );
  virtual size_t sizeEstimate( ) const POL_OVERRIDE
  {
    return sizeof( *this ) + 3 * sizeof(void*) * buffer.capacity( ) * sizeof( unsigned char );
  }
  virtual const char* typeOf() const POL_OVERRIDE
  {
    return "Packet";
  }
  virtual u8 typeOfInt() const POL_OVERRIDE
  {
    return OTPacket;
  }

  bool is_variable_length;
};
}
}
#endif
