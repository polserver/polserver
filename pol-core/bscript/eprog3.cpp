/*
History
=======

Notes
=======

*/

#include "eprog.h"

#include <iomanip>

namespace Pol {
  namespace Bscript {
	void EScriptProgram::dump( std::ostream& os )
	{
	  Token token;
	  unsigned PC;
	  if ( !exported_functions.empty() )
	  {
		os << "Exported Functions:" << std::endl;
        os << "   PC  Args  Name" << std::endl;
		for ( auto &elem : exported_functions )
		{
		  os << std::setw( 5 ) << elem.PC
			<< std::setw( 6 ) << elem.nargs
			<< "  " << elem.name
            << std::endl;
		}
	  }
	  unsigned nLines = tokens.length() / sizeof( StoredToken );
	  for ( PC = 0; PC < nLines; PC++ )
	  {
		if ( _readToken( token, PC ) )
		{
		  return;
		}
		else
		{
		  if ( fileline.size() > PC && !fileline[PC].empty() )
              os << fileline[PC] << std::endl;
		  if ( program_PC == PC && !program_decl.empty() )
              os << program_decl << std::endl;
		  if ( function_decls.size() > PC && !function_decls[PC].empty() )
              os << function_decls[PC] << std::endl;
		  if ( sourcelines.size() > PC && !sourcelines[PC].empty() )
              os << sourcelines[PC] << std::endl;
          os << PC << ": " << token << std::endl;
		  if ( token.id == INS_CASEJMP )
		  {
			dump_casejmp( os, token );
		  }
		}
	  }
	}

	void EScriptProgram::dump_casejmp( std::ostream& os, const Token& token )
	{
	  const unsigned char* dataptr = token.dataptr;
	  for ( ;; )
	  {
		unsigned short offset = *(const unsigned short*)dataptr;
		dataptr += 2;
		unsigned char type = *dataptr;
		dataptr += 1;
		if ( type == CASE_TYPE_LONG )
		{
		  unsigned int lval = *(const unsigned int*)dataptr;
		  dataptr += 4;
          os << "\t" << lval << ": @" << offset << std::endl;
		}
		else if ( type == CASE_TYPE_DEFAULT )
		{
            os << "\tdefault: @" << offset << std::endl;
		  break;
		}
		else
		{ // type is the length of the string, otherwise
            os << "\t\"" << std::string((const char*)dataptr, type) << "\": @" << offset << std::endl;
		  dataptr += type;
		}
	  }
	}
  }
}
