/** @file
 *
 * @par History
 */

#ifndef __MSGFILTR_H
#define __MSGFILTR_H
namespace Pol
{
namespace Core
{
class MessageTypeFilter
{
public:
  enum MessageDefault
  {
    NormallyDisabled,
    NormallyEnabled
  };
  // terminate exceptions list with -1
private:
  MessageTypeFilter( MessageDefault def, int* exceptions );

public:
  unsigned char msgtype_allowed[256];
  static void createMessageFilter();
};
}
}
#endif
