#include "InstructionEmitter.h"

#include "ModuleDeclarationRegistrar.h"
#include "StoredToken.h"
#include "compiler/representation/CompiledScript.h"
#include "modules.h"

namespace Pol::Bscript::Compiler
{
InstructionEmitter::InstructionEmitter( CodeSection& code, DataSection& data,
                                        ModuleDeclarationRegistrar& module_declaration_registrar )
  : code_emitter( code ),
    data_emitter( data ),
    module_declaration_registrar( module_declaration_registrar )
{
  initialize_data();
}

void InstructionEmitter::initialize_data()
{
  std::byte nul{};
  data_emitter.store( &nul, sizeof nul );
}

void InstructionEmitter::call_modulefunc(
    const ModuleFunctionDeclaration& module_function_declaration )
{
  unsigned module_id, function_index;
  module_declaration_registrar.lookup_or_register_module_function( module_function_declaration,
                                                                   module_id, function_index );
  unsigned sympos = 0; // include_debug ? emit_data( module_function_declaration.name ) : 0;
  StoredToken token(
      module_id, TOK_FUNC,
      static_cast<BTokenType>(
          function_index ),  // function index, stored in Token.lval, saved in StoredToken.type
      sympos );
  append_token( token );
}

void InstructionEmitter::consume()
{
  emit_token( TOK_CONSUMER, TYP_UNARY_OPERATOR );
}

void InstructionEmitter::progend()
{
  emit_token( CTRL_PROGEND, TYP_CONTROL );
}

void InstructionEmitter::value( double v )
{
  unsigned offset = data_emitter.append( v );
  emit_token( TOK_DOUBLE, TYP_OPERAND, offset );
}

void InstructionEmitter::value( const std::string& v )
{
  unsigned data_offset = emit_data( v );
  emit_token( TOK_STRING, TYP_OPERAND, data_offset );
}

unsigned InstructionEmitter::emit_data( const std::string& s )
{
  return data_emitter.store( s );
}

unsigned InstructionEmitter::emit_token( BTokenId id, BTokenType type, unsigned offset )
{
  StoredToken token( Mod_Basic, id, type, offset );
  return append_token( token );
}

unsigned InstructionEmitter::append_token( StoredToken& token )
{
  return code_emitter.append( token );
}

}  // namespace Pol::Bscript::Compiler
