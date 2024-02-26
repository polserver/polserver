#pragma once

#include <string>

namespace Pol::Bscript::Compiler
{
class Profile;
struct Range;
class Report;
class SourceFileLoader;

class PrettifyBuilder
{
public:
  PrettifyBuilder( Profile&, Report& );

  std::string build( SourceFileLoader& source_loader, const std::string& pathname, bool is_module,
                     std::optional<Range> format_range );

  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler
