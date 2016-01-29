/** @file
 *
 * @par History
 */

#ifndef __UMANIP_H
#define __UMANIP_H
namespace Pol {
  namespace Items {
	class Item;
  }
  namespace Core {
	void subtract_amount_from_item( Items::Item* item, unsigned short amount );
  }
}
#endif
