/** @file
 *
 * @par History
 */


#ifndef MDELTA_H
#define MDELTA_H
namespace Pol {
  namespace Core {
	struct MoveDelta
	{
	  short xmove;
	  short ymove;
	};

	extern MoveDelta move_delta[8];
  }
}
#endif
