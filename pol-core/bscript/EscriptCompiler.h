//
// Created by Eric Swanson on 4/27/20.
//

#ifndef POLSERVER_ESCRIPTCOMPILER_H
#define POLSERVER_ESCRIPTCOMPILER_H

#include <string>
#include <vector>

namespace Pol
{
namespace Bscript
{

class EscriptCompiler
{
public:
  int compileFile( const std::string& fname );

private:
  std::vector<std::string> referenced_pathnames;

};

} // namespace Bscript
} // namespace Pol

#endif  // POLSERVER_ESCRIPTCOMPILER_H
