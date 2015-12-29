/*
History
=======
2015/12/29 Bodom: created this file

Notes
=======

*/


# include "tokens.h"


namespace Pol {
  namespace Clib {
    /**
     * Template specialization for outputting the token type
     */
    template <>
    std::string tostring( const Bscript::BTokenType& v )
    {
      return tostring(static_cast<int>(v));
    }

    /**
     * Template specialization for outputting the token ID
     */
    template <>
    std::string tostring( const Bscript::BTokenId& v )
    {
      return tostring(static_cast<int>(v));
    }
  }
}
