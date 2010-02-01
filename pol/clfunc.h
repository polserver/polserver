// History

#ifndef __CLFUNC_H
#define __CLFUNC_H

// this is for NULLs found..
#include <stddef.h>

#include "../clib/rawtypes.h"

#include "ufunc.h"
#include "uconst.h"

/*
void send_sysmessage_cl(Client *client,                // System message (lower left corner)
//						Character *chr_from,
//						ObjArray* oText,
						unsigned int cliloc_num,
						unsigned short font = DEFAULT_TEXT_FONT,
						unsigned short color = DEFAULT_TEXT_COLOR);
*/
void send_sysmessage_cl(Client *client,                // System message (lower left corner)
//						Character *chr_from,
						unsigned int cliloc_num,
						const u16 *arguments = 0,
						unsigned short font = DEFAULT_TEXT_FONT,
						unsigned short color = DEFAULT_TEXT_COLOR);

void say_above_cl(UObject *obj,
						unsigned int cliloc_num,
						const u16 *arguments = 0,
						unsigned short font = DEFAULT_TEXT_FONT,
						unsigned short color = DEFAULT_TEXT_COLOR);

void private_say_above_cl(Character *chr,
					    const UObject* obj,
						unsigned int cliloc_num,
						const u16 *arguments = 0,
						unsigned short font = DEFAULT_TEXT_FONT,
						unsigned short color = DEFAULT_TEXT_COLOR);

void send_sysmessage_cl_affix(Client *client,
							  unsigned int cliloc_num,
							  const char* affix,
							  bool prepend = false,
							  const u16 *arguments = 0,
							  unsigned short font = DEFAULT_TEXT_COLOR,
							  unsigned short color = DEFAULT_TEXT_COLOR );
void say_above_cl_affix(UObject *obj,
						unsigned int cliloc_num,
						const char* affix,
						bool prepend = false,
						const u16 *arguments = 0,
						unsigned short font = DEFAULT_TEXT_FONT,
						unsigned short color = DEFAULT_TEXT_COLOR);

void private_say_above_cl_affix(Character *chr,
					    const UObject* obj,
						unsigned int cliloc_num,
						const char* affix,
						bool prepend = false,
						const u16 *arguments = 0,
						unsigned short font = DEFAULT_TEXT_FONT,
						unsigned short color = DEFAULT_TEXT_COLOR);

unsigned char* build_sysmessage_cl(unsigned* msglen,unsigned int cliloc_num, const u16 *arguments=0,
						unsigned short font= DEFAULT_TEXT_FONT, unsigned short color = DEFAULT_TEXT_COLOR);
unsigned char* build_sysmessage_cl_affix(unsigned* msglen,unsigned int cliloc_num, const char* affix,
							  bool prepend, const u16 *arguments=0, unsigned short font= DEFAULT_TEXT_FONT,
							  unsigned short color = DEFAULT_TEXT_COLOR);
#endif
