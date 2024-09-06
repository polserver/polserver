#include "DebugStoreSerializer.h"

#include <fmt/format.h>
#include <fstream>

#include "bscript/compiler/representation/ClassDescriptor.h"
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
    auto varlast = static_cast<unsigned int>( varfirst + block.local_variable_names.size() - 1 );
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
      *text_ofs << fmt::format(
                       " {}: address={}, parameters={}, captures={}, variadic={}, class_index={}",
                       index++, function_reference.address(), function_reference.parameter_count(),
                       function_reference.capture_count(), function_reference.is_variadic(),
                       function_reference.class_index() )
                << std::endl;
    }
  }

  if ( text_ofs && !compiled_script.class_descriptors.empty() )
  {
    // Used if the offset is out-of-bounds (should never really happen)
    const char* unknown_name = "<unknown name>";
    int index = 0;
    *text_ofs << "Class descriptors:\n";
    for ( const auto& class_descriptor : compiled_script.class_descriptors )
    {
      if ( class_descriptor.name_offset >= compiled_script.data.size() )
      {
        *text_ofs << "Invalid class descriptor name offset: " << class_descriptor.name_offset
                  << std::endl;
        continue;
      }
      const char* class_name = class_descriptor.name_offset < compiled_script.data.size()
                                   ? reinterpret_cast<const char*>( compiled_script.data.data() +
                                                                    class_descriptor.name_offset )
                                   : unknown_name;

      // Handle class
      *text_ofs << fmt::format( " {}: name={}, constructors={}, methods={}", index++, class_name,
                                class_descriptor.constructors.size(),
                                class_descriptor.methods.size() )
                << std::endl;

      // Handle constructors
      if ( !class_descriptor.constructors.empty() )
      {
        *text_ofs << "    - Constructor chain (type tag offset):";
        for ( const auto& constructor : class_descriptor.constructors )
        {
          *text_ofs << fmt::format( "\n      - {} ({})",
                                    reinterpret_cast<const char*>( compiled_script.data.data() +
                                                                   constructor.type_tag_offset ),
                                    constructor.type_tag_offset );
        }
        *text_ofs << std::endl;
      }

      // Handle methods
      if ( !class_descriptor.methods.empty() )
      {
        *text_ofs << "    - Methods:";

        for ( const auto& method_descriptor : class_descriptor.methods )
        {
          const char* method_name =
              method_descriptor.name_offset < compiled_script.data.size()
                  ? reinterpret_cast<const char*>( compiled_script.data.data() +
                                                   method_descriptor.name_offset )
                  : unknown_name;

          *text_ofs << fmt::format( "\n      - {}: PC={} FuncRef={}", method_name,
                                    method_descriptor.address,
                                    method_descriptor.function_reference_index );
        }
        *text_ofs << std::endl;
      }
    }
  }
}

}  // namespace Pol::Bscript::Compiler
