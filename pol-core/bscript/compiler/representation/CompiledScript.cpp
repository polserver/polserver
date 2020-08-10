#include "CompiledScript.h"

#include <utility>

#include "compiler/file/SourceFileIdentifier.h"
#include "ExportedFunction.h"
#include "ModuleDescriptor.h"
#include "ModuleFunctionDescriptor.h"
#include "StoredToken.h"

namespace Pol::Bscript::Compiler
{
CompiledScript::CompiledScript( CodeSection code,
                                DataSection data,
                                ExportedFunctions exported_functions,
                                std::vector<std::string> global_variable_names,
                                ModuleDescriptors module_descriptors,
                                std::unique_ptr<ProgramInfo> program_info,
                                SourceFileIdentifiers source_file_identifiers)
  : code( std::move( code ) ),
    data( std::move( data ) ),
    exported_functions( std::move( exported_functions ) ),
    global_variable_names( std::move( global_variable_names ) ),
    module_descriptors( std::move( module_descriptors ) ),
    program_info( std::move( program_info ) ),
    source_file_identifiers( std::move( source_file_identifiers ) )
{
}

CompiledScript::~CompiledScript() = default;

}  // namespace Pol::Bscript::Compiler
