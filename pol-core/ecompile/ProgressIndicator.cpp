#include "ProgressIndicator.h"

#include "clib/clib.h"
#include "clib/logfacility.h"

namespace Pol::ECompile
{
namespace
{
// Wide enough to be informative, narrow enough not to wrap an 80-column console: a wrapped
// line leaves a second row that the next carriage return cannot reach.
constexpr size_t LINE_WIDTH = 78;
constexpr long long REDRAW_INTERVAL_MICROS = 100'000;

size_t digits( size_t value )
{
  size_t n = 1;
  while ( value >= 10 )
  {
    value /= 10;
    ++n;
  }
  return n;
}
}  // namespace

ProgressIndicator::ProgressIndicator( bool enabled, size_t total )
    : _enabled( enabled && Clib::stdout_is_tty() ),
      _total( total ),
      _count_width( digits( total ) )  // fixed, so the counter column does not jitter
{
}

ProgressIndicator::~ProgressIndicator()
{
  clear();
}

void ProgressIndicator::advance( const std::string& path )
{
  if ( !_enabled )
    return;

  size_t done = ++_done;
  auto now = _timer.ellapsed().count();
  auto last = _last_draw.load( std::memory_order_relaxed );
  if ( now - last < REDRAW_INTERVAL_MICROS )
    return;
  // The exchange is the whole mutual exclusion: only one thread per interval wins it and draws.
  if ( !_last_draw.compare_exchange_strong( last, now, std::memory_order_relaxed ) )
    return;

  draw( done, path );
}

void ProgressIndicator::draw( size_t done, const std::string& path )
{
  size_t pct = _total ? done * 100 / _total : 100;
  auto line = fmt::format( "Compiling [{:>{}}/{}] {:3}%  ", done, _count_width, _total, pct );

  size_t room = LINE_WIDTH > line.size() ? LINE_WIDTH - line.size() : 0;
  if ( path.size() > room )
  {
    // Keep the tail: the package and file name identify the script, the leading path does not.
    line += room > 3 ? "..." + path.substr( path.size() - ( room - 3 ) ) : std::string();
  }
  else
    line += path;

  if ( line.size() < LINE_WIDTH )
    line.append( LINE_WIDTH - line.size(), ' ' );

  _drawn.store( true, std::memory_order_relaxed );
  // The trailing return parks the cursor at column 0, so a diagnostic printed next overwrites
  // this row from the left and leaves only the padding spaces behind.
  INFO_PRINT( "\r{}\r", line );
}

void ProgressIndicator::clear()
{
  if ( !_enabled || !_drawn.exchange( false ) )
    return;
  INFO_PRINT( "\r{}\r", std::string( LINE_WIDTH, ' ' ) );
}

}  // namespace Pol::ECompile
