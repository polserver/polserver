// History

#ifndef __CLFUNC_H
#define __CLFUNC_H


// this is for NULLs found..
#include <cstddef>

#include "../clib/rawtypes.h"
#include "network/packets.h"
#include "uconst.h"

namespace Pol
{
namespace Network
{
class Client;
}
namespace Mobile
{
class Character;
}
namespace Core
{
class UObject;

void send_sysmessage_cl( Network::Client* client,  // System message (lower left corner)
                                                   //						Character *chr_from,
                         unsigned int cliloc_num, const u16* arguments = 0,
                         unsigned short font = DEFAULT_TEXT_FONT,
                         unsigned short color = DEFAULT_TEXT_COLOR );

void say_above_cl( UObject* obj, unsigned int cliloc_num, const u16* arguments = 0,
                   unsigned short font = DEFAULT_TEXT_FONT,
                   unsigned short color = DEFAULT_TEXT_COLOR );

void private_say_above_cl( Mobile::Character* chr, const UObject* obj, unsigned int cliloc_num,
                           const u16* arguments = 0, unsigned short font = DEFAULT_TEXT_FONT,
                           unsigned short color = DEFAULT_TEXT_COLOR );

void send_sysmessage_cl_affix( Network::Client* client, unsigned int cliloc_num, const char* affix,
                               bool prepend = false, const u16* arguments = 0,
                               unsigned short font = DEFAULT_TEXT_COLOR,
                               unsigned short color = DEFAULT_TEXT_COLOR );
void say_above_cl_affix( UObject* obj, unsigned int cliloc_num, const char* affix,
                         bool prepend = false, const u16* arguments = 0,
                         unsigned short font = DEFAULT_TEXT_FONT,
                         unsigned short color = DEFAULT_TEXT_COLOR );

void private_say_above_cl_affix( Mobile::Character* chr, const UObject* obj,
                                 unsigned int cliloc_num, const char* affix, bool prepend = false,
                                 const u16* arguments = 0, unsigned short font = DEFAULT_TEXT_FONT,
                                 unsigned short color = DEFAULT_TEXT_COLOR );

void build_sysmessage_cl( Network::PktOut_C1* msg, unsigned int cliloc_num,
                          const u16* arguments = 0, unsigned short font = DEFAULT_TEXT_FONT,
                          unsigned short color = DEFAULT_TEXT_COLOR );
void build_sysmessage_cl_affix( Network::PktOut_CC* msg, unsigned int cliloc_num, const char* affix,
                                bool prepend, const u16* arguments = 0,
                                unsigned short font = DEFAULT_TEXT_FONT,
                                unsigned short color = DEFAULT_TEXT_COLOR );
}
}
#endif
