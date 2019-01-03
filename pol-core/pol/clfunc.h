// History

#ifndef __CLFUNC_H
#define __CLFUNC_H


// this is for nullptrs found..
#include <cstddef>

#include "../clib/rawtypes.h"
#include "../plib/uconst.h"
#include "network/packets.h"

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
                                                   // Character *chr_from,
                         unsigned int cliloc_num, const std::string& arguments = "",
                         unsigned short font = Plib::DEFAULT_TEXT_FONT,
                         unsigned short color = Plib::DEFAULT_TEXT_COLOR );

void say_above_cl( UObject* obj, unsigned int cliloc_num, const std::string& arguments = "",
                   unsigned short font = Plib::DEFAULT_TEXT_FONT,
                   unsigned short color = Plib::DEFAULT_TEXT_COLOR );

void private_say_above_cl( Mobile::Character* chr, const UObject* obj, unsigned int cliloc_num,
                           const std::string& arguments = "",
                           unsigned short font = Plib::DEFAULT_TEXT_FONT,
                           unsigned short color = Plib::DEFAULT_TEXT_COLOR );

void send_sysmessage_cl_affix( Network::Client* client, unsigned int cliloc_num,
                               const std::string& affix, bool prepend = false,
                               const std::string& arguments = "",
                               unsigned short font = Plib::DEFAULT_TEXT_COLOR,
                               unsigned short color = Plib::DEFAULT_TEXT_COLOR );
void say_above_cl_affix( UObject* obj, unsigned int cliloc_num, const std::string& affix,
                         bool prepend = false, const std::string& arguments = "",
                         unsigned short font = Plib::DEFAULT_TEXT_FONT,
                         unsigned short color = Plib::DEFAULT_TEXT_COLOR );

void private_say_above_cl_affix( Mobile::Character* chr, const UObject* obj,
                                 unsigned int cliloc_num, const std::string& affix,
                                 bool prepend = false, const std::string& arguments = "",
                                 unsigned short font = Plib::DEFAULT_TEXT_FONT,
                                 unsigned short color = Plib::DEFAULT_TEXT_COLOR );

void build_sysmessage_cl( Network::PktOut_C1* msg, unsigned int cliloc_num,
                          const std::string& arguments = "",
                          unsigned short font = Plib::DEFAULT_TEXT_FONT,
                          unsigned short color = Plib::DEFAULT_TEXT_COLOR );
void build_sysmessage_cl_affix( Network::PktOut_CC* msg, unsigned int cliloc_num,
                                const std::string& affix, bool prepend,
                                const std::string& arguments = "",
                                unsigned short font = Plib::DEFAULT_TEXT_FONT,
                                unsigned short color = Plib::DEFAULT_TEXT_COLOR );
}  // namespace Core
}  // namespace Pol
#endif
