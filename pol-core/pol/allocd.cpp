/** @file
 *
 * @par History
 * - 2006/10/06 Shinigami: Win needs malloc.h for _HEAPINFO
 */


#include "allocd.h"

#include <cstddef>
#ifdef _WIN32
#include "../clib/logfacility.h"

#include <malloc.h>
#endif


namespace Pol::Core
{
size_t last_blocks_used = 0;
size_t last_bytes_used = 0;
size_t last_blocks_free = 0;
size_t last_bytes_free = 0;

void PrintAllocationData()
{
#ifdef _WIN32
  _HEAPINFO hinfo;
  int heapstatus;
  hinfo._pentry = nullptr;
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
  std::string tmp;
  tmp += fmt::format( "Heap:  Used {} blocks, {} bytes, Free {} blocks, {} bytes\n", blocks_used,
                      bytes_used, blocks_free, bytes_free );
  tmp += fmt::format( "Delta: Used {} blocks, {} bytes, Free {} blocks, {} bytes\n",
                      blocks_used - last_blocks_used, bytes_used - last_bytes_used,
                      blocks_free - last_blocks_free, bytes_free - last_bytes_free );
  last_blocks_used = blocks_used;
  last_bytes_used = bytes_used;
  last_blocks_free = blocks_free;
  last_bytes_free = bytes_free;

  switch ( heapstatus )
  {
  case _HEAPEMPTY:
    tmp += "OK - empty heap\n";
    break;
  case _HEAPEND:
    tmp += "OK - end of heap\n";
    break;
  case _HEAPBADPTR:
    tmp += "ERROR - bad pointer to heap\n";
    break;
  case _HEAPBADBEGIN:
    tmp += "ERROR - bad start of heap\n";
    break;
  case _HEAPBADNODE:
    tmp += "ERROR - bad node in heap\n";
    break;
  }
  INFO_PRINTLN( tmp );
#endif
}
}  // namespace Pol::Core
