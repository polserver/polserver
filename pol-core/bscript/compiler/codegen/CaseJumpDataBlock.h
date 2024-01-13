#ifndef POLSERVER_CASEJUMPDATABLOCK_H
#define POLSERVER_CASEJUMPDATABLOCK_H

#include <cstdint>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class CaseJumpDataBlock
{
public:
  void on_boolean_value_jump_to( bool value, uint16_t address );
  void on_integer_value_jump_to( int32_t value, uint16_t address );
  void on_string_value_jump_to( const std::string& value, uint16_t address );
  void on_uninitialized_value_jump_to( uint16_t address );
  void on_default_jump_to( uint16_t address );

  [[nodiscard]] const std::vector<std::byte>& get_data() const;

private:
  std::vector<std::byte> data;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CASEJUMPDATABLOCK_H
