#ifndef CLIB_TIMER_H
#define CLIB_TIMER_H

#include <chrono>
#include <string>


namespace Pol::Tools  // global ns is enough polluted
{
// automatically print into the console
struct DebugT
{
  static void print( const std::string& name, long long time );
};
// silent version
struct SilentT
{
  static void print( const std::string& name, long long time );
};


// simple Timer class
// instantiate it by giving it a name and wait for destroy
// or use start/stop methods to control it
// uses by default Silent version for calculation and stuff
// or template version for Debug print
template <class printer = SilentT>
class Timer : protected printer
{
  using Clock = std::chrono::high_resolution_clock;
  using ms = std::chrono::milliseconds;

public:
  Timer( std::string name );
  Timer();
  ~Timer();
  void start();
  void stop();
  // no one wants to know more then milliseconds..
  long long ellapsed() const;
  double ellapsed_s() const;
  void print() const;

private:
  std::string _name;
  Clock::time_point _start;
  Clock::time_point _end;
};

class HighPerfTimer
{
public:
  using Clock = std::chrono::high_resolution_clock;
  using time_mu = std::chrono::microseconds;

  HighPerfTimer();
  time_mu ellapsed() const;

private:
  Clock::time_point _start;
};
}  // namespace Pol::Tools


#endif  // CLIB_TIMER_H
