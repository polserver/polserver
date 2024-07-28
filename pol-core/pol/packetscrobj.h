/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef PACKETSCROBJ_H
#define PACKETSCROBJ_H


#ifndef POLOBJECT_H
#include "polobject.h"
#endif

#include <string>

#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"

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
class BPacket final : public Core::PolObjectImp
{
public:
  BPacket();
  BPacket( const BPacket& copyfrom );
  BPacket( u8 type, signed short length );
  BPacket( const unsigned char* data, unsigned short length, bool variable_len );
  ~BPacket();

  std::vector<unsigned char> buffer;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin = false ) override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual std::string getStringRep() const override;
  bool SetSize( u16 newsize );
  Bscript::BObjectImp* SetSize( u16 newsize, bool giveReturn );
  virtual size_t sizeEstimate() const override { return Clib::memsize( buffer ); }
  virtual const char* typeOf() const override { return "Packet"; }
  virtual u8 typeOfInt() const override { return OTPacket; }
  bool is_variable_length;
};
}  // namespace Core
}  // namespace Pol
#endif
