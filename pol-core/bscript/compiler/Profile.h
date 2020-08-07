#ifndef POLSERVER_PROFILE_H
#define POLSERVER_PROFILE_H

#include <atomic>

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class Profile
{
public:
  std::atomic<long long> build_workspace_micros;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_PROFILE_H
