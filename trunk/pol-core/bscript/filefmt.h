/*
History
=======
2005/12/08 Shinigami: New Version added to force Recompiling...
2006/09/16 Shinigami: New Version... Internal handling of UO.EM Function execution changed.
2007/11/07 Shinigami: New Version... Removed some tokens like Left+Mid
2009/08/27 Turley:    New Version Muad removed members/methods

Notes
=======

*/

#ifndef __BSCRIPT_FILEFMT_H
#define __BSCRIPT_FILEFMT_H

#include "../clib/rawtypes.h"

#ifdef _MSC_VER
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

#define BSCRIPT_FILE_MAGIC0 'C'
#define BSCRIPT_FILE_MAGIC1 'E'
#define ESCRIPT_FILE_VER_0000_obs   0x0000
#define ESCRIPT_FILE_VER_0001_obs   0x0001
#define ESCRIPT_FILE_VER_0002_obs   0x0002
#define ESCRIPT_FILE_VER_0003       0x0003
#define ESCRIPT_FILE_VER_0004       0x0004
#define ESCRIPT_FILE_VER_0005       0x0005
#define ESCRIPT_FILE_VER_0006		0x0006
#define ESCRIPT_FILE_VER_0007		0x0007
#define ESCRIPT_FILE_VER_0008		0x0008
#define ESCRIPT_FILE_VER_0009		0x0009
#define ESCRIPT_FILE_VER_000A		0x000A
#define ESCRIPT_FILE_VER_000B		0x000B

/*
	NOTE: Update ESCRIPT_FILE_VER_CURRENT when you make a
	new escript file version, to force recompile of scripts
	and report this to users when an older compiled version
	is attempted to be executed - TJ
*/
#define ESCRIPT_FILE_VER_CURRENT	(ESCRIPT_FILE_VER_000B)

struct BSCRIPT_FILE_HDR
{
	char magic2[2];
	unsigned short version;
    unsigned short globals;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_FILE_HDR), 6 );

struct BSCRIPT_SECTION_HDR
{
	unsigned short type;
	unsigned long length;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_SECTION_HDR), 6 );

enum BSCRIPT_SECTION {
    BSCRIPT_SECTION_MODULE  =   1,
    BSCRIPT_SECTION_CODE	=   2,
    BSCRIPT_SECTION_SYMBOLS =   3,
    BSCRIPT_SECTION_PROGDEF =   4,
    BSCRIPT_SECTION_GLOBALVARNAMES  = 5,
    BSCRIPT_SECTION_EXPORTED_FUNCTIONS = 6
};


struct BSCRIPT_MODULE_HDR
{
	char modulename[ 14 ];
	unsigned long nfuncs;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_MODULE_HDR), 18 );

struct BSCRIPT_MODULE_FUNCTION
{
	char funcname[ 33 ];
	unsigned char nargs;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_MODULE_FUNCTION), 34 );

struct BSCRIPT_PROGDEF_HDR
{
    unsigned expectedArgs;
    unsigned char rfu[12];
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_PROGDEF_HDR), 16 );

struct BSCRIPT_GLOBALVARNAMES_HDR
{
    unsigned nGlobalVars;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_GLOBALVARNAMES_HDR), 4 );
struct BSCRIPT_GLOBALVARNAME_HDR
{
    unsigned namelen;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_GLOBALVARNAME_HDR), 4 );

struct BSCRIPT_DBG_INSTRUCTION
{
    unsigned filenum;
    unsigned linenum;
    unsigned blocknum;
    unsigned statementbegin;
    unsigned rfu1;
    unsigned rfu2;
} POSTSTRUCT_PACK;
asserteql( sizeof(BSCRIPT_DBG_INSTRUCTION), 24 );

struct BSCRIPT_EXPORTED_FUNCTION
{
    char funcname[33];
    unsigned nargs;
    unsigned PC;
};
asserteql( sizeof(BSCRIPT_EXPORTED_FUNCTION),41 );

#ifdef _MSC_VER
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif
