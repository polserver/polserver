/** @file
 *
 * @par History
 */


#ifndef __TOKEN_H
#define __TOKEN_H

#include "tokens.h"

#include <fmt/format.h>

namespace Pol::Bscript
{
struct DebugToken
{
  unsigned sourceFile;
  unsigned offset;
  unsigned strOffset;
};

class Token
{
public:
  BTokenId id;
  BTokenType type;
  double dval;
  union
  {
    int precedence;
    int sourceFile;
  };
  int dbg_filenum;
  int dbg_linenum;
  union
  {
    int lval;
    const unsigned char* dataptr;
  };
  bool deprecated;
  unsigned char module;

  static unsigned int instances();
  static void show_instances();

protected:
  std::string token;

public:
  const char* tokval() const { return token.c_str(); }
  Token();

  void nulStr();
  void setStr( std::string str );

  std::string tostring() const;
};

}  // namespace Pol::Bscript
template <>
struct fmt::formatter<Pol::Bscript::Token> : fmt::formatter<std::string>
{
  fmt::format_context::iterator format( const Pol::Bscript::Token& t,
                                        fmt::format_context& ctx ) const;
};
#endif
