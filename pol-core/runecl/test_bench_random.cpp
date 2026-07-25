
#include "clib/clib_endian.h"
#include <benchmark/benchmark.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <sstream>
static void BM_makro( benchmark::State& state )
{
  int i = 0;
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    int j = i++;
    os << j;
  }
}
BENCHMARK( BM_makro );
static void BM_boost( benchmark::State& state )
{
  int i = 0;
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    int j = i++;
    os << fmt::format( "{}", j );
  }
}
BENCHMARK( BM_boost );
static void BM_boosttostring( benchmark::State& state )
{
  int i = 0;
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    int j = i++;
    os << fmt::to_string( j );
  }
}
BENCHMARK( BM_boosttostring );
static void BM_boosts( benchmark::State& state )
{
  using namespace fmt::literals;
  int i = 0;
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    int j = i++;
    os << fmt::format( "{}"_cf, j );
  }
}
BENCHMARK( BM_boosts );
static void BM_str( benchmark::State& state )
{
  std::string s{ "123" };
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    os << "S" << s.size() << ":" << s;
  }
}
BENCHMARK( BM_str );
static void BM_str1( benchmark::State& state )
{
  std::string s{ "123" };
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    os << fmt::format( "S{}:{}", s.size(), s );
  }
}
BENCHMARK( BM_str1 );
static void BM_str2( benchmark::State& state )
{
  using namespace fmt::literals;
  std::string s{ "123" };
  std::ostringstream os{};
  while ( state.KeepRunning() )
  {
    os << fmt::format( "S{}:{}"_cf, s.size(), s );
  }
}
BENCHMARK( BM_str2 );
