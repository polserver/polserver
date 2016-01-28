/** @file
 *
 * @par History
 */


#ifndef PLIB_MAPBLOB_H
#define PLIB_MAPBLOB_H

#define SOLIDX_X_SIZE 16
#define SOLIDX_X_SHIFT 4
#define SOLIDX_X_CELLMASK 0xF

#define SOLIDX_Y_SIZE 16
#define SOLIDX_Y_SHIFT 4
#define SOLIDX_Y_CELLMASK 0xF

namespace Pol {
  namespace Plib {
	struct SOLIDX1_ELEM
	{
	  unsigned int offset;
	};
	struct SOLIDX2_ELEM
	{
	  unsigned int baseindex;
	  unsigned short addindex[SOLIDX_X_SIZE][SOLIDX_Y_SIZE];
	};
	struct SOLIDS_ELEM
	{
	  signed char z;
	  unsigned char height;
	  unsigned char flags;
	};
	struct SOLIDS_DATA
	{
	  unsigned short nelems;
	  SOLIDS_ELEM elem[1];
	};

	const unsigned SOLIDX2_FILLER_SIZE = 4;
  }
}
#endif
