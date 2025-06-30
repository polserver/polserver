/** @file
 *
 * @par History
 */


#ifndef __GETITEM_H
#define __GETITEM_H
#include "base/position.h"
namespace Pol
{
namespace Network
{
class Client;
}
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Core
{
struct PKTIN_07;

enum class GOTTEN_ITEM_TYPE : u8
{
  GOTTEN_ITEM_ON_GROUND,
  GOTTEN_ITEM_EQUIPPED,
  GOTTEN_ITEM_IN_CONTAINER
};
class GottenItem
{
public:
  GottenItem() = default;

  Items::Item* item() { return _item; };
  void undo( Mobile::Character* chr );
  static void handle( Network::Client* client, PKTIN_07* msg );

  bool operator==( const GottenItem& o ) const { return _item == o._item; }

private:
  GottenItem( Items::Item* item, const Core::Pos4d& pos );
  Items::Item* _item = nullptr;
  Core::Pos3d _pos = Core::Pos3d( 0, 0, 0 );
  // Use string realm instead of Pos4d, as realm could be deleted when handling
  // the undo
  std::string _realm;
  u32 _owner_serial = 0;
  u8 _slot_index = 0;
  GOTTEN_ITEM_TYPE _source = GOTTEN_ITEM_TYPE::GOTTEN_ITEM_ON_GROUND;
};

}  // namespace Core
}  // namespace Pol
#endif
