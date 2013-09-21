#include <chrono>
#include <ratio>
#include <iosfwd>
#include <string>

namespace Tools // global ns is enough polluted 
{
	template<typename U>
	class Timer;

	// automatically print into the console
	struct DebugT
	{
		template<typename U>
		friend class Timer;

	protected:
		static void print(std::string name, double time)
		{
			cout << endl << "----------------------------------" << endl;
			cout << name << ": " << time << " s" << endl;
			cout <<  "----------------------------------" << endl;
		}
	};
	// silent version
	struct SilentT
	{
	protected:
		static void print(std::string name, double time)
		{
		}
	};

	
	// simple Timer class
	// instantiate it by giving it a name and wait for destroy
	// or use start/stop methods to control it
	// use default template version for Debug print
	// or Silent version for calculation and stuff
	template <class printer = DebugT>
	class Timer: protected printer
	{
		typedef std::chrono::high_resolution_clock Clock;
	public:
		Timer(std::string name) : _name(name)
		{
			start();
		};
		~Timer()
		{
			if (_start == _end)
				stop();
			printer::print(_name,ellapsed());
		};
		void start()
		{
			_start = _end = Clock::now();
		};
		void stop()
		{
			_end = Clock::now();		
		};
		// no one wants to know more then milliseconds..
		double ellapsed()
		{
			if (_start == _end)
				stop();
			auto msratio = std::chrono::milliseconds::period::ratio();
			auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start).count();
			return (diff * static_cast<double>(msratio.num) / msratio.den);
		};
		void print()
		{
			DebugT::print(_name,ellapsed());
		};
	private:
		std::string _name;
		Clock::time_point _start;
		Clock::time_point _end;
	};
}
