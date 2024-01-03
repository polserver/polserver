#include "timer.h"

#include "logfacility.h"

namespace Pol
{
namespace Tools
{
void DebugT::print( const std::string& name, long long time )
{
  INFO_PRINT2(
      "\n----------------------------------\n"
      "{}: {} ms"
      "\n----------------------------------",
      name, time );
}
void SilentT::print( const std::string&, long long ) {}

template <class printer>
Timer<printer>::Timer( std::string name ) : _name( std::move( name ) )
{
  start();
}
template <class printer>
Timer<printer>::Timer() : _name( "" )
{
  start();
}
template <class printer>
Timer<printer>::~Timer()
{
  if ( _start == _end )
    stop();
  printer::print( _name, ellapsed() );
}
template <class printer>
void Timer<printer>::start()
{
  _start = _end = Clock::now();
}
template <class printer>
void Timer<printer>::stop()
{
  _end = Clock::now();
}

template <class printer>
long long Timer<printer>::ellapsed() const
{
  Clock::time_point _now = Clock::now();
  if ( _start != _end )  // already stopped
    _now = _end;
  return std::chrono::duration_cast<ms>( _now - _start ).count();
}
template <class printer>
double Timer<printer>::ellapsed_s() const
{
  return ellapsed() * static_cast<double>( ms::period::num ) / ms::period::den;
}
template <class printer>
void Timer<printer>::print() const
{
  DebugT::print( _name, ellapsed() );
}


HighPerfTimer::HighPerfTimer() : _start( Clock::now() ) {}

HighPerfTimer::time_mu HighPerfTimer::ellapsed() const
{
  Clock::time_point _now = Clock::now();
  return std::chrono::duration_cast<time_mu>( _now - _start );
}

// forward declarce both versions
template class Timer<DebugT>;
template class Timer<SilentT>;
}  // namespace Tools
}  // namespace Pol
