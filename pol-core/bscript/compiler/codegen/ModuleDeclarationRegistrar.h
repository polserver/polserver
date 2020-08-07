#ifndef POLSERVER_MODULEDECLARATIONREGISTRAR_H
#define POLSERVER_MODULEDECLARATIONREGISTRAR_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class ModuleDescriptor;
class ModuleFunctionDeclaration;
class ModuleFunctionDescriptor;

class ModuleDeclarationRegistrar
{
public:
  ModuleDeclarationRegistrar();

  void register_module( const ModuleFunctionDeclaration& node );
  void register_modulefunc( const ModuleFunctionDeclaration& );

  std::vector<ModuleDescriptor> take_module_descriptors();

  void lookup_or_register_module_function( const ModuleFunctionDeclaration& node,
                                           unsigned& module_index, unsigned& function_index );

private:
  struct EmittedModule
  {
    explicit EmittedModule(unsigned module_index);
    ~EmittedModule();

    const unsigned module_index;
    std::map<std::string, unsigned> function_name_to_index;
    std::vector<ModuleFunctionDescriptor> function_declarations;
  };
  std::map<std::string, std::unique_ptr<EmittedModule>> registered_modules;
  std::vector<std::string> module_names_in_order;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_MODULEDECLARATIONREGISTRAR_H
