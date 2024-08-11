#include "ModuleDeclarationRegistrar.h"

#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/representation/ModuleDescriptor.h"
#include "bscript/compiler/representation/ModuleFunctionDescriptor.h"

namespace Pol::Bscript::Compiler
{
ModuleDeclarationRegistrar::ModuleDeclarationRegistrar() = default;

void ModuleDeclarationRegistrar::register_modulefunc(
    const ModuleFunctionDeclaration& module_function_declaration )
{
  unsigned module_id, function_index;
  lookup_or_register_module_function( module_function_declaration, module_id, function_index );
}

void ModuleDeclarationRegistrar::register_module( const ModuleFunctionDeclaration& node )
{
  auto& module_name = node.scope;
  auto module_itr = registered_modules.find( module_name );
  if ( module_itr == registered_modules.end() )
  {
    auto module_index = static_cast<unsigned>( registered_modules.size() );

    registered_modules[module_name] = std::make_unique<EmittedModule>( module_index );
    module_names_in_order.push_back( module_name );
  }
}

void ModuleDeclarationRegistrar::lookup_or_register_module_function(
    const ModuleFunctionDeclaration& node, unsigned& module_index, unsigned& function_index )
{
  auto& module_name = node.scope;
  auto& function_name = node.name;
  auto argument_count = node.parameter_count();

  auto module_itr = registered_modules.find( module_name );
  if ( module_itr != registered_modules.end() )
  {
    EmittedModule& em = *( ( *module_itr ).second );
    module_index = em.module_index;

    auto function_itr = em.function_name_to_index.find( function_name );
    if ( function_itr != em.function_name_to_index.end() )
    {
      function_index = ( *function_itr ).second;
    }
    else
    {
      function_index = static_cast<unsigned>( em.function_name_to_index.size() );
      em.function_name_to_index[function_name] = function_index;
      em.function_declarations.emplace_back( function_name, argument_count );
    }
  }
  else
  {
    module_index = static_cast<unsigned>( registered_modules.size() );
    function_index = 0;

    auto mod = std::make_unique<EmittedModule>( module_index );
    mod->function_name_to_index.emplace( function_name, function_index );
    mod->function_declarations.emplace_back( function_name, argument_count );
    registered_modules[module_name] = std::move( mod );
    module_names_in_order.push_back( module_name );
  }
}

std::vector<ModuleDescriptor> ModuleDeclarationRegistrar::take_module_descriptors()
{
  std::vector<ModuleDescriptor> module_descriptors;
  for ( auto& module_name : module_names_in_order )
  {
    EmittedModule& emitted_module = *registered_modules[module_name];
    module_descriptors.emplace_back( module_name,
                                     std::move( emitted_module.function_declarations ) );
  }
  return module_descriptors;
}

ModuleDeclarationRegistrar::EmittedModule::EmittedModule( unsigned module_index )
    : module_index( module_index )
{
}
ModuleDeclarationRegistrar::EmittedModule::~EmittedModule() = default;

}  // namespace Pol::Bscript::Compiler
