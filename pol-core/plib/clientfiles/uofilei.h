/** @file
 *
 * @par History
 */


#ifndef UOFILEI_H
#define UOFILEI_H

#include "../uconst.h"
#include "../udatfile.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <vector>


namespace Pol::Plib
{
struct USTRUCT_VERSION;
struct USTRUCT_MULTI_ELEMENT;

FILE* open_uo_file( const std::string& filename_part, size_t* out_file_size = nullptr );
bool open_uopmulti_file( std::map<unsigned int, std::vector<USTRUCT_MULTI_ELEMENT>>& multi_map );

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
}  // namespace Pol::Plib

#endif
