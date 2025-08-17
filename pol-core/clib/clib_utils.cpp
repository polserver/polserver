/** @file
 *
 * @par History
 */


#include "clib.h"
#include "rawtypes.h"
#include <cmath>

namespace Pol::Clib
{
void OnlineStatistics::update( double value )
{
  _total += value;
  ++_count;
  double delta = value - _mean;
  _mean += delta / _count;
  _m2 += delta * ( value - _mean );
  _max = value > _max ? value : _max;
}

double OnlineStatistics::variance() const
{
  if ( _count < 2 )
    return 0;
  return _m2 / ( _count - 1 );
}

double OnlineStatistics::standard_deviation() const
{
  return std::sqrt( variance() );
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
double OnlineStatistics::total() const
{
  return _total;
}
}  // namespace Pol::Clib

fmt::format_context::iterator fmt::formatter<Pol::Clib::OnlineStatistics>::format(
    const Pol::Clib::OnlineStatistics& s, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format(
      fmt::format( "mean: {:.2f} max: {:.2f} standard_deviation: {:.2f} count: {} total: {:.2f}",
                   s.mean(), s.max(), s.standard_deviation(), s.count(), s.total() ),
      ctx );
}
