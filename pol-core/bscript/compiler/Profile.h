#ifndef POLSERVER_PROFILE_H
#define POLSERVER_PROFILE_H

#include <atomic>

namespace Pol::Bscript::Compiler
{
class Profile
{
public:
  std::atomic<long long> build_workspace_micros;
  std::atomic<long long> register_const_declarations_micros;
  std::atomic<long long> optimize_micros;
  std::atomic<long long> analyze_micros;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROFILE_H
