#include "DebugStoreSerializer.h"

#include <fmt/format.h>
#include <fstream>

#include "bscript/compiler/representation/CompiledScript.h"
#include "bscript/compiler/representation/DebugBlock.h"
#include "bscript/compiler/representation/FunctionReferenceDescriptor.h"
#include "filefmt.h"

namespace Pol::Bscript::Compiler
{
DebugStoreSerializer::DebugStoreSerializer( CompiledScript& compiled_script )
  : compiled_script( compiled_script ), debug_store( compiled_script.debug )
{
}

void DebugStoreSerializer::write( std::ofstream& ofs, std::ofstream* text_ofs )
{
  // just a version number
  uint32_t version = 3;
  ofs.write( reinterpret_cast<char*>( &version ), sizeof version );

  uint32_t count;

  count = static_cast<uint32_t>( debug_store.filenames.size() );
  ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
  unsigned filenum = 0;
  for ( auto& filename : debug_store.filenames )
  {
    if ( text_ofs )
      *text_ofs << "File " << filenum++ << ": " << filename << std::endl;
    count = static_cast<unsigned int>( filename.size() + 1 );
    ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
    ofs.write( filename.c_str(), count );
  }
  count = static_cast<uint32_t>( compiled_script.global_variable_names.size() );
  ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
  unsigned globalnum = 0;
  for ( auto& global_name : compiled_script.global_variable_names )
  {
    if ( text_ofs )
      *text_ofs << "Global " << globalnum++ << ": " << global_name << std::endl;
    count = static_cast<unsigned int>( global_name.size() + 1 );
    ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
    ofs.write( global_name.c_str(), count );
  }
  count = static_cast<uint32_t>( debug_store.instructions.size() );
  ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
  unsigned instruction_index = 0;
  for ( auto& instruction_info : debug_store.instructions )
  {
    if ( text_ofs )
      *text_ofs << "Ins " << instruction_index++ << ": File " << instruction_info.file_index
                << ", Line " << instruction_info.line_number << ", Block "
                << instruction_info.block_index << " "
                << ( instruction_info.statement_begin ? "StatementBegin" : "" ) << std::endl;

    BSCRIPT_DBG_INSTRUCTION ins{};
    ins.filenum = instruction_info.file_index;
    ins.linenum = instruction_info.line_number;
    ins.blocknum = instruction_info.block_index;
    ins.statementbegin = instruction_info.statement_begin ? 1 : 0;
    ins.rfu1 = 0;
    ins.rfu2 = 0;
    ofs.write( reinterpret_cast<char*>( &ins ), sizeof ins );
  }

  count = static_cast<uint32_t>( debug_store.blocks.size() );
  ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
  unsigned blocknum = 0;
  for ( auto& block : debug_store.blocks )
  {
    if ( text_ofs )
    {
      *text_ofs << "Block " << blocknum++ << ":" << std::endl;
      *text_ofs << "  Parent block: " << block.parent_block_index << std::endl;
    }

    uint32_t tmp;
    tmp = block.parent_block_index;
    ofs.write( reinterpret_cast<char*>( &tmp ), sizeof tmp );
    tmp = block.base_index;
    ofs.write( reinterpret_cast<char*>( &tmp ), sizeof tmp );

    count = static_cast<uint32_t>( block.local_variable_names.size() );
    ofs.write( reinterpret_cast<char*>( &count ), sizeof count );

    unsigned int varfirst = block.base_index;
    auto varlast =
        static_cast<unsigned int>( varfirst + block.local_variable_names.size() - 1 );
    if ( varlast >= varfirst )
    {
      if ( text_ofs )
        *text_ofs << "  Local variables " << varfirst << "-" << varlast << ": " << std::endl;
      unsigned j = 0;
      for ( auto& varname : block.local_variable_names )
      {
        if ( text_ofs )
          *text_ofs << "      " << varfirst + j++ << ": " << varname << std::endl;

        count = static_cast<unsigned int>( varname.size() + 1 );
        ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
        ofs.write( varname.c_str(), count );
      }
    }
  }
  count = static_cast<uint32_t>( debug_store.user_functions.size() );
  ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
  unsigned funcnum = 0;
  for ( auto& user_function : debug_store.user_functions )
  {
    if ( text_ofs )
    {
      *text_ofs << "Function " << funcnum++ << ": " << user_function.name << std::endl;
      *text_ofs << "  FirstPC=" << user_function.first_instruction
                << ", lastPC=" << user_function.last_instruction << std::endl;
    }
    count = static_cast<uint32_t>( user_function.name.size() + 1 );
    ofs.write( reinterpret_cast<char*>( &count ), sizeof count );
    ofs.write( user_function.name.c_str(), count );
    uint32_t tmp;
    tmp = user_function.first_instruction;
    ofs.write( reinterpret_cast<char*>( &tmp ), sizeof tmp );
    tmp = user_function.last_instruction;
    ofs.write( reinterpret_cast<char*>( &tmp ), sizeof tmp );
  }
  if ( text_ofs && !compiled_script.function_references.empty() )
  {
    int index = 0;
    *text_ofs << "Function references:\n";
    for ( const auto& function_reference : compiled_script.function_references )
    {
      *text_ofs << fmt::format( " {}: parameters={}, captures={}, variadic={}", index++,
                                function_reference.parameter_count(),
                                function_reference.capture_count(),
                                function_reference.is_variadic() )
                << std::endl;
    }
  }
}

}  // namespace Pol::Bscript::Compiler
