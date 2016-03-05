// History
//   2006/09/26 Shinigami: GCC 3.4.x fix - added "this" to NewCryptEngineTmpl

#ifndef CRYPTENGINE_H
#define CRYPTENGINE_H

#include "cryptbase.h"
#include "cryptkey.h"
namespace Pol {
  namespace Crypt {
	CCryptBase* create_crypt_engine( TCryptInfo& infoCrypt );
  }
}
#endif
