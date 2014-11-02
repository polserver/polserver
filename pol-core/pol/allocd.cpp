/*
History
=======
2006/10/06 Shinigami: Win needs malloc.h for _HEAPINFO

Notes
=======

*/

#include "allocd.h"

#ifdef _WIN32
#	include <malloc.h>
#endif

#include <cstddef>
#include <cstdio>
#include "../clib/logfacility.h"

namespace Pol {
  namespace Core {
	size_t last_blocks_used = 0;
	size_t last_bytes_used = 0;
	size_t last_blocks_free = 0;
	size_t last_bytes_free = 0;

	void PrintAllocationData()
	{
#ifdef _WIN32
	  _HEAPINFO hinfo;
	  int heapstatus;
	  hinfo._pentry = NULL;
	  size_t blocks_used = 0, bytes_used = 0, blocks_free = 0, bytes_free = 0;
	  while ( ( heapstatus = _heapwalk( &hinfo ) ) == _HEAPOK )
	  {
		if ( hinfo._useflag == _USEDENTRY )
		{
		  ++blocks_used;
		  bytes_used += hinfo._size;
		}
		else
		{
		  ++blocks_free;
		  bytes_free += hinfo._size;
		}
	  }
      fmt::Writer _tmp;
      _tmp.Format( "Heap:  Used {} blocks, {} bytes, Free {} blocks, {} bytes\n" )
        << blocks_used << bytes_used << blocks_free << bytes_free;
      _tmp.Format( "Delta: Used {} blocks, {} bytes, Free {} blocks, {} bytes\n" )
        << ( blocks_used - last_blocks_used )
        << ( bytes_used - last_bytes_used )
        << ( blocks_free - last_blocks_free )
        << ( bytes_free - last_bytes_free );
	  last_blocks_used = blocks_used;
	  last_bytes_used = bytes_used;
	  last_blocks_free = blocks_free;
	  last_bytes_free = bytes_free;

	  switch ( heapstatus )
	  {
        case _HEAPEMPTY:      _tmp << "OK - empty heap\n";      break;
        case _HEAPEND:      _tmp << "OK - end of heap\n";      break;
        case _HEAPBADPTR:      _tmp << "ERROR - bad pointer to heap\n";      break;
        case _HEAPBADBEGIN:     _tmp << "ERROR - bad start of heap\n";      break;
        case _HEAPBADNODE:      _tmp << "ERROR - bad node in heap\n";      break;
	  }
      INFO_PRINT << _tmp.c_str();
#endif
	}
  }
}
