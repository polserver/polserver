#include <chrono>
#include <ratio>
#include <iosfwd>
#include <string>

#include "logfacility.h"
namespace Pol {
  namespace Tools // global ns is enough polluted 
  {
	// automatically print into the console
	struct DebugT
	{
	protected:
	  static void print( const std::string &name, long long time )
	  {
        INFO_PRINT << "\n----------------------------------\n"
          << name << ": " << time << " ms\n"
          << "----------------------------------\n";
	  }
	};
	// silent version
	struct SilentT
	{
	protected:
	  static void print( const std::string &name, long long time )
	  {}
	};


	// simple Timer class
	// instantiate it by giving it a name and wait for destroy
	// or use start/stop methods to control it
	// uses by default Silent version for calculation and stuff
	// or template version for Debug print
	template <class printer = SilentT>
	class Timer : protected printer
	{
	  typedef std::chrono::high_resolution_clock Clock;
	  typedef std::chrono::milliseconds ms;
	public:
	  Timer( std::string name ) : _name( name )
	  {
		start();
	  }
	  Timer() : _name( "" )
	  {
		start();
	  }
	  ~Timer()
	  {
		if( _start == _end )
		  stop();
		printer::print( _name, ellapsed() );
	  }
	  void start()
	  {
		_start = _end = Clock::now();
	  }
	  void stop()
	  {
		_end = Clock::now();
	  }
	  // no one wants to know more then milliseconds..
	  long long ellapsed() const
	  {
		Clock::time_point _now = Clock::now();
		if( _start != _end ) // already stopped
		  _now = _end;
		return std::chrono::duration_cast<ms>( _now - _start ).count();
	  }
	  double ellapsed_s() const
	  {
		return ellapsed() * static_cast<double>( ms::period::num ) / ms::period::den;
	  }
	  void print() const
	  {
		DebugT::print( _name, ellapsed() );
	  }
	private:
	  std::string _name;
	  Clock::time_point _start;
	  Clock::time_point _end;
	};
  }
}
