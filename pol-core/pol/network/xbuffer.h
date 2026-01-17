/** @file
 *
 * @par History
 */


#ifndef __XBUFFER_H
#define __XBUFFER_H

// Note on XmitBuffer: generally, 'lenleft' will start out as the number of data bytes,
// and 'nsent' will start as 0.  As more data is sent, nsent will move to the original lenleft,
// while lenleft will move to 0.
// Also note that malloc/free are used to allocate this structure, since
// data[] will be followed by (lenleft-1) more bytes, to house the data.

namespace Pol::Core
{
struct XmitBuffer
{
  XmitBuffer* next;
  unsigned short nsent;    // how many bytes sent already
  unsigned short lenleft;  // how many bytes left to send
  unsigned char data[1];
};
}  // namespace Pol::Core

#endif
