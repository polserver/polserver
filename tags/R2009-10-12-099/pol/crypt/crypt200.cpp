#define CCrypt CCrypt200
#include "clib/stl_inc.h"
#include "crypt200/crypt.cpp"

#include "cryptengine.h"

CryptEngine* create_crypt_2_0_0_engine()
{
    return new CryptEngineTmpl<CCrypt200>;
}
