#include "CompiledScriptSerializer.h"

#include <cstring>
#include <fstream>

#include "StoredToken.h"
#include "bscript/compiler/representation/CompiledScript.h"
#include "bscript/compiler/representation/ExportedFunction.h"
#include "bscript/compiler/representation/FunctionReferenceDescriptor.h"
#include "bscript/compiler/representation/ModuleDescriptor.h"
#include "bscript/compiler/representation/ModuleFunctionDescriptor.h"
#include "clib/clib.h"
#include "clib/logfacility.h"
#include "filefmt.h"

namespace Pol::Bscript::Compiler
{
CompiledScriptSerializer::CompiledScriptSerializer( const CompiledScript& compiled_script )
    : compiled_script( compiled_script )
{
}

void CompiledScriptSerializer::write( const std::string& pathname ) const
{
  std::ofstream ofs( pathname, std::ofstream::binary );

  BSCRIPT_FILE_HDR hdr{};
  hdr.magic2[0] = BSCRIPT_FILE_MAGIC0;
  hdr.magic2[1] = BSCRIPT_FILE_MAGIC1;
  hdr.version = ESCRIPT_FILE_VER_CURRENT;  // auto-set to latest version (see filefmt.h)
  hdr.globals = static_cast<unsigned short>( compiled_script.global_variable_names.size() );
  ofs.write( reinterpret_cast<const char*>( &hdr ), sizeof hdr );

  BSCRIPT_SECTION_HDR sechdr{};

  if ( compiled_script.program_info )
  {
    BSCRIPT_PROGDEF_HDR progdef_hdr{};
    memset( &progdef_hdr, 0, sizeof progdef_hdr );
    sechdr.type = BSCRIPT_SECTION_PROGDEF;
    sechdr.length = sizeof progdef_hdr;
    ofs.write( reinterpret_cast<const char*>( &sechdr ), sizeof sechdr );
    progdef_hdr.expectedArgs = compiled_script.program_info->parameter_count;
    ofs.write( reinterpret_cast<const char*>( &progdef_hdr ), sizeof progdef_hdr );
  }

  for ( auto& module : compiled_script.module_descriptors )
  {
    sechdr.type = BSCRIPT_SECTION_MODULE;
    sechdr.length = 0;
    ofs.write( reinterpret_cast<const char*>( &sechdr ), sizeof sechdr );

    BSCRIPT_MODULE_HDR modhdr{};
    memset( &modhdr, 0, sizeof modhdr );
    Clib::stracpy( modhdr.modulename, module.name.c_str(), sizeof modhdr.modulename );
    modhdr.nfuncs = static_cast<unsigned int>( module.functions.size() );
    ofs.write( reinterpret_cast<const char*>( &modhdr ), sizeof modhdr );
    for ( auto& module_func : module.functions )
    {
      BSCRIPT_MODULE_FUNCTION func{};
      memset( &func, 0, sizeof func );

      Clib::stracpy( func.funcname, module_func.name.c_str(), sizeof func.funcname );
      func.nargs = static_cast<unsigned char>( module_func.parameter_count );

      ofs.write( reinterpret_cast<const char*>( &func ), sizeof func );
    }
  }

  auto code_len = static_cast<u32>( compiled_script.code.size() * sizeof( StoredToken ) );
  sechdr.type = BSCRIPT_SECTION_CODE;
  sechdr.length = sizeof code_len + code_len;
  ofs.write( reinterpret_cast<const char*>( &sechdr ), sizeof sechdr );
  ofs.write( reinterpret_cast<const char*>( &code_len ), sizeof code_len );
  ofs.write( reinterpret_cast<const char*>( compiled_script.code.data() ), code_len );

  const std::vector<std::byte>& data = compiled_script.data;
  auto data_len = static_cast<u32>( data.size() );
  sechdr.type = BSCRIPT_SECTION_SYMBOLS;
  sechdr.length = sizeof data_len + data_len;
  ofs.write( reinterpret_cast<const char*>( &sechdr ), sizeof sechdr );
  ofs.write( reinterpret_cast<const char*>( &data_len ), sizeof data_len );
  ofs.write( reinterpret_cast<const char*>( data.data() ), data_len );

  if ( !compiled_script.exported_functions.empty() )
  {
    BSCRIPT_EXPORTED_FUNCTION bef{};
    sechdr.type = BSCRIPT_SECTION_EXPORTED_FUNCTIONS;
    sechdr.length =
        static_cast<unsigned int>( compiled_script.exported_functions.size() * sizeof bef );
    ofs.write( reinterpret_cast<const char*>( &sechdr ), sizeof sechdr );
    for ( auto& elem : compiled_script.exported_functions )
    {
      Clib::stracpy( bef.funcname, elem.name.c_str(), sizeof bef.funcname );
      bef.nargs = elem.parameter_count;
      bef.PC = elem.entrypoint_program_counter;
      ofs.write( reinterpret_cast<const char*>( &bef ), sizeof bef );
    }
  }

  if ( !compiled_script.function_references.empty() )
  {
    BSCRIPT_FUNCTION_REFERENCE bfr{};
    sechdr.type = BSCRIPT_SECTION_FUNCTION_REFERENCES;
    sechdr.length =
        static_cast<unsigned int>( compiled_script.function_references.size() * sizeof bfr );
    ofs.write( reinterpret_cast<const char*>( &sechdr ), sizeof sechdr );
    for ( auto& elem : compiled_script.function_references )
    {
      bfr.parameter_count = elem.parameter_count();
      bfr.capture_count = elem.capture_count();
      bfr.is_variadic = elem.is_variadic();
      ofs.write( reinterpret_cast<const char*>( &bfr ), sizeof bfr );
    }
  }
}

}  // namespace Pol::Bscript::Compiler
