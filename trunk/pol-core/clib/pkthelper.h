/*
History
=======

Notes
=======

*/

#ifndef __CLIB_PKTHELPER_H
#define __CLIB_PKTHELPER_H

#include "rawtypes.h"
namespace Pol {
  namespace Clib {
	void WritetoCharBuffer( char *buffer, u32 x, unsigned int *offset );
	void WritetoCharBuffer( char *buffer, s32 x, unsigned int *offset );

	void WritetoCharBuffer( char *buffer, u16 x, unsigned int *offset );
	void WritetoCharBuffer( char *buffer, s16 x, unsigned int *offset );

	void WritetoCharBuffer( char *buffer, u8 x, unsigned int *offset );
	void WritetoCharBuffer( char *buffer, s8 x, unsigned int *offset );

	void WritetoCharBufferFlipped( char *buffer, u32 x, unsigned int *offset );
	void WritetoCharBufferFlipped( char *buffer, s32 x, unsigned int *offset );

	void WritetoCharBufferFlipped( char *buffer, u16 x, unsigned int *offset );
	void WritetoCharBufferFlipped( char *buffer, s16 x, unsigned int *offset );
  }
}
#endif
