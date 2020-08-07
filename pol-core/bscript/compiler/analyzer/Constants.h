#ifndef POLSERVER_CONSTANTS_H
#define POLSERVER_CONSTANTS_H

#include <map>
#include <memory>
#include <string>

namespace Pol::Bscript::Compiler
{
class ConstDeclaration;
class Report;

class Constants
{
public:
  explicit Constants( Report& );

  ConstDeclaration* find( const std::string& name );
  void create( ConstDeclaration& );

private:
  std::map<std::string, ConstDeclaration*> constants;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_CONSTANTS_H
