/** @file
 *
 * @par History
 */


#ifndef POL_TESTENV_H
#define POL_TESTENV_H
namespace Pol
{
namespace Mobile
{
class NPC;
}
namespace Items
{
class Item;
}
namespace Testing
{
extern Mobile::NPC* test_banker;
extern Mobile::NPC* test_banker2;
extern Mobile::NPC* test_banker3;
extern Items::Item* test_guard_door;
extern Mobile::NPC* test_water_elemental;
extern Mobile::NPC* test_seaserpent;
extern Items::Item* test_chest1;
extern Items::Item* test_chest2;
extern Mobile::NPC* test_orclord;

Items::Item* add_item( unsigned int objtype, unsigned short x, unsigned short y, short z );
void add_multi( unsigned int objtype, unsigned short x, unsigned short y, short z );
void add_multi( unsigned int objtype, unsigned short x, unsigned short y, short z, int flags );
Mobile::NPC* add_npc( const char* npctype, unsigned short x, unsigned short y, short z );


void create_test_environment();
void inc_failures();
void inc_successes();
void display_test_results();

void map_test();
void skilladv_test();
void walk_test();
void multiwalk_test();
void drop_test();
void los_test();
void dynprops_test();
void dummy();
void packet_test();
}
}
#endif
