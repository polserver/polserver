#ifndef POLSERVER_BUILDERWORKSPACE_H
#define POLSERVER_BUILDERWORKSPACE_H

namespace Pol
{
namespace Bscript
{
namespace Compiler
{
class CompilerWorkspace;
class Profile;
class Report;

class BuilderWorkspace
{
public:
  BuilderWorkspace( CompilerWorkspace&, Profile& profile, Report& report );
  ~BuilderWorkspace();

  CompilerWorkspace& compiler_workspace;

  Profile& profile;
  Report& report;
};

}  // namespace Compiler
}  // namespace Bscript
}  // namespace Pol

#endif  // POLSERVER_BUILDERWORKSPACE_H
