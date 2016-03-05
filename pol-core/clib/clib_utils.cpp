/** @file
 *
 * @par History
 */


#include "clib.h"

namespace Pol
{
namespace Clib
{
OnlineStatistics::OnlineStatistics() :
  _count(0),
  _max(0),
  _mean(0),
  _m2(0)
{
}

void OnlineStatistics::update(double value)
{
  ++_count;
  double delta = value - _mean;
  _mean += delta / _count;
  _m2 += delta * (value - _mean);
  _max = value > _max ? value : _max;
}

double OnlineStatistics::variance() const
{
  if (_count < 2)
    return 0;
  return _m2 / (_count - 1);
}

double OnlineStatistics::mean() const
{
  return _mean;
}
u64 OnlineStatistics::count() const
{
  return _count;
}
double OnlineStatistics::max() const
{
  return _max;
}
}
}
