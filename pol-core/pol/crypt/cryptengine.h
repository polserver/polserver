// History
//   2006/09/26 Shinigami: GCC 3.4.x fix - added "this" to NewCryptEngineTmpl

#ifndef CRYPTENGINE_H
#define CRYPTENGINE_H

#include "cryptkey.h"

namespace Pol::Crypt
{
class CCryptBase;
CCryptBase* create_crypt_engine( TCryptInfo& infoCrypt );
}  // namespace Pol::Crypt

#endif
