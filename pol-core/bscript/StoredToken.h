#ifndef POLSERVER_STOREDTOKEN_H
#define POLSERVER_STOREDTOKEN_H

#ifndef __TOKENS_H
#include "tokens.h"
#endif


namespace Pol::Bscript
{

#ifdef _MSC_VER
#pragma pack( push, 1 )
#endif

// bitfields
class StoredToken
{
public:
  unsigned char type;
  unsigned char id;
  unsigned short offset;
  unsigned char module;

  explicit StoredToken( unsigned char aModule = 0, int aID = CTRL_STATEMENTBEGIN,
                        BTokenType aType = TYP_CONTROL,
                        unsigned aOffset = 0  //,
                                              // unsigned aNargs = 0
  );
} POSTSTRUCT_PACK;
#ifdef _MSC_VER
#pragma pack( pop )
#endif

}  // namespace Pol::Bscript

#endif  // POLSERVER_STOREDTOKEN_H
