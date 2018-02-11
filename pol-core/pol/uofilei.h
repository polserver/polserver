/** @file
 *
 * @par History
 */


#ifndef UOFILEI_H
#define UOFILEI_H

#include <stdio.h>
#include "uconst.h"
#include "udatfile.h"
namespace Pol
{
namespace Core
{
extern FILE* mapfile;
extern FILE* sidxfile;
extern FILE* statfile;
extern FILE* verfile;
extern FILE* tilefile;
extern FILE* stadifl_file;
extern FILE* stadifi_file;
extern FILE* stadif_file;
extern FILE* mapdifl_file;
extern FILE* mapdif_file;

struct USTRUCT_VERSION;

FILE* open_uo_file( const std::string& filename_part );

bool check_verdata( unsigned int file, unsigned int block, const USTRUCT_VERSION*& vrec );

void standheight( MOVEMODE movemode, StaticList& statics, unsigned short x, unsigned short y,
                  short oldz, bool* result, short* newz );

#define VERFILE_MAP0_MUL 0x00
#define VERFILE_STAIDX0_MUL 0x01
#define VERFILE_STATICS0_MUL 0x02
#define VERFILE_ARTIDX_MUL 0x03
#define VERFILE_ART_MUL 0x04
#define VERFILE_ANIM_IDX 0x05
#define VERFILE_ANIM_MUL 0x06
#define VERFILE_SOUNDIDX_MUL 0x07
#define VERFILE_SOUND_MUL 0x08
#define VERFILE_TEXIDX_MUL 0x09
#define VERFILE_TEXMAPS_MUL 0x0A
#define VERFILE_GUMPIDX_MUL 0x0B
#define VERFILE_GUMPART_MUL 0x0C
#define VERFILE_MULTI_IDX 0x0D
#define VERFILE_MULTI_MUL 0x0E
#define VERFILE_SKILLS_IDX 0x0F
#define VERFILE_SKILLS_MUL 0x10
#define VERFILE_TILEDATA_MUL 0x1E
#define VERFILE_ANIMDATA_MUL 0x1F

void read_static_diffs();
void read_map_difs();
}
}
#endif
