/** @file
 *
 * @par History
 */

#pragma once

#include "base/range.h"

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

  unsigned sleeptime = 0;
  size_t realm_index = ~0lu;
  Range2d area;
  Range2dItr area_itr;

  friend void Pol::Testing::decay_test();
};
}  // namespace Pol::Core
