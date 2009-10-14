#define CCrypt CCryptIgnition

#include "ignition/crypt.cpp"
#include "cryptengine.h"

CryptEngine* create_crypt_ignition_engine()
{
    return new CryptEngineTmpl<CCryptIgnition>;
}
