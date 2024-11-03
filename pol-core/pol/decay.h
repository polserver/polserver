/** @file
 *
 * @par History
 */

#pragma once
#include <atomic>

#include "base/range.h"
#include "gameclck.h"
#include "reftypes.h"
#include "uobject.h"

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/key_extractors.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace Pol::Testing
{
void decay_test();
}
namespace Pol::Realms
{
class Realm;
}

namespace Pol::Core
{
class Decay
{
public:
  Decay() = default;
  static void decay_thread( void* arg );
  void after_realms_size_changed();
  void on_delete_realm( Realms::Realm* realm );

  static constexpr int SKIP_FURTHER_CHECKS = 2;

private:
  void threadloop();
  void step();
  bool should_switch_realm() const;
  void switch_realm();
  void decay_worldzone();
  void calculate_sleeptime();

  std::atomic<unsigned> sleeptime = 0;
  size_t realm_index = ~0lu;
  Range2d area;
  Range2dItr area_itr;

  friend void Pol::Testing::decay_test();
};

class WorldDecay
{
public:
  WorldDecay();
  void initialize();

  void addObject( Items::Item* item, gameclock_t decaytime );
  void removeObject( Items::Item* item );
  gameclock_t getDecayTime( const Items::Item* item ) const;
  void decayTask();

private:
  struct DecayItem
  {
    DecayItem( gameclock_t decaytime, ItemRef itemref );
    gameclock_t time;
    ItemRef obj;
  };
  struct IndexByTime
  {
  };
  struct IndexByObject
  {
  };
  struct SerialFromDecayItem
  {
    typedef u32 result_type;
    result_type operator()( const DecayItem& i ) const;
  };
  // multiindex container unique by serial_ext and ordered by decaytime
  using DecayContainer = boost::multi_index_container<
      DecayItem, boost::multi_index::indexed_by<
                     boost::multi_index::hashed_unique<boost::multi_index::tag<IndexByObject>,
                                                       SerialFromDecayItem>,
                     boost::multi_index::ordered_non_unique<
                         boost::multi_index::tag<IndexByTime>,
                         boost::multi_index::member<DecayItem, gameclock_t, &DecayItem::time>>>>;

  DecayContainer decay_cont;
};
}  // namespace Pol::Core
