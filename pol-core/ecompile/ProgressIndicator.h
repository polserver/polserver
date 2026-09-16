#ifndef E_COMPILE_PROGRESSINDICATOR_H
#define E_COMPILE_PROGRESSINDICATOR_H

#include "clib/timer.h"

#include <atomic>
#include <cstddef>
#include <string>

namespace Pol::ECompile
{
/// A single console line that is redrawn in place as scripts are compiled.
///
/// Every draw goes through the log facility, whose one worker thread is the only thread that
/// touches the console, so no locking is needed to keep two draws from interleaving.
class ProgressIndicator
{
public:
  /// Draws nothing unless enabled and stdout is a terminal - redirected output would otherwise
  /// collect one padded line per redraw.
  ProgressIndicator( bool enabled, size_t total );
  ~ProgressIndicator();
  ProgressIndicator( const ProgressIndicator& ) = delete;
  ProgressIndicator& operator=( const ProgressIndicator& ) = delete;

  /// Counts one more file and redraws if the rate limit allows. Called from worker threads.
  void advance( const std::string& path );

  /// Blanks the line so whatever prints next starts on a clean row.
  void clear();

private:
  void draw( size_t done, const std::string& path );

  const bool _enabled;
  const size_t _total;
  const size_t _count_width;
  std::atomic<size_t> _done{ 0 };
  std::atomic<long long> _last_draw{ 0 };
  std::atomic<bool> _drawn{ false };
  Tools::HighPerfTimer _timer;
};

}  // namespace Pol::ECompile

#endif  // E_COMPILE_PROGRESSINDICATOR_H
