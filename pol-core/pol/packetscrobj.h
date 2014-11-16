/*
History
=======
2009/12/21 Turley:    ._method() call fix


Notes
=======

*/

#ifndef PACKETSCROBJ_H
#define PACKETSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../clib/rawtypes.h"

namespace Pol {
  namespace Core {
	class BPacket : public Bscript::BObjectImp
	{
	public:
	  BPacket();
	  BPacket( const BPacket& copyfrom );
	  BPacket( u8 type, signed short length );
	  BPacket( const unsigned char* data, unsigned short length, bool variable_len );
	  ~BPacket();

	  std::vector<unsigned char> buffer;
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); //id test
	  virtual Bscript::BObjectImp* call_method( const char* methodname, Bscript::Executor& ex );
	  virtual Bscript::BObjectImp* call_method_id( const int id, Bscript::Executor& ex, bool forcebuiltin = false );
	  virtual Bscript::BObjectImp* copy( ) const;
	  virtual std::string getStringRep() const;
	  bool SetSize( u16 newsize );
	  Bscript::BObjectImp* SetSize( u16 newsize, bool giveReturn );
      virtual size_t sizeEstimate( ) const { return sizeof( *this ) + 3 * sizeof(void*) * buffer.capacity( ) * sizeof( unsigned char ); }
	  virtual const char* typeOf() const { return "Packet"; }
	  virtual int typeOfInt() const { return OTPacket; }

	  bool is_variable_length;
	};
  }
}
#endif
