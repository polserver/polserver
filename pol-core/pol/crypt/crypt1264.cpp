#define CCrypt CCrypt1264

#include "crypt1264/crypt.cpp"

#include "cryptengine.h"

CryptEngine* create_crypt_1_26_4_engine()
{
    return new CryptEngineTmpl<CCrypt1264>;
}
