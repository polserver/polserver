/*
History
=======
2007/12/09 Shinigami: removed ( is.peek() != EOF ) check from String::unpackWithLen()
                      will not work with Strings in Arrays, Dicts, etc.
2008/02/08 Turley:    String::unpackWithLen() will accept zero length Strings
2009/09/12 MuadDib:   Disabled 4244 in this file due to it being on a string iter. Makes no sense.
2015/20/12 Bodom:     moved most of the methods/props to BaseString

Notes
=======

*/

#include "impstr.h"

#include "berror.h"
#include "bobject.h"
#include "objmethods.h"
#include "executor.h"
#include "../clib/stlutil.h"

#include <cstdlib>
#include <cstring>

#ifdef __GNUG__
#	include <streambuf>
#endif

namespace Pol {
  namespace Bscript {

    /**
     * Creates a String from a char* pointer of a fixed length
     *
     * @param s: the pointer to start reading chars from
     * @param len: Number of chars to read
     */
    String::String( const char *s, int len )
    {
      value_.append( s, len );
    }

	String *String::StrStr( int begin, int len )
	{
	  return new String( value_.substr( begin - 1, len ) );
	}

	String *String::ETrim( const char* CRSet, int type )
	{
	  std::string tmp = value_;

	  if ( type == 1 ) // This is for Leading Only.
	  {
		// Find the first character position after excluding leading blank spaces 
		size_t startpos = tmp.find_first_not_of( CRSet );
		if ( std::string::npos != startpos )
		  tmp = tmp.substr( startpos );
		else
		  tmp = "";
		return new String( tmp );
	  }
	  else if ( type == 2 ) // This is for Trailing Only.
	  {
		// Find the first character position from reverse 
		size_t endpos = tmp.find_last_not_of( CRSet );
        if (std::string::npos != endpos)
		  tmp = tmp.substr( 0, endpos + 1 );
		else
		  tmp = "";
		return new String( tmp );
	  }
	  else if ( type == 3 )
	  {
		// Find the first character position after excluding leading blank spaces  
		size_t startpos = tmp.find_first_not_of( CRSet );
		// Find the first character position from reverse af  
		size_t endpos = tmp.find_last_not_of( CRSet );

		// if all spaces or empty return on empty string  
        if ((std::string::npos == startpos) || (std::string::npos == endpos))
		  tmp = "";
		else
		  tmp = tmp.substr( startpos, endpos - startpos + 1 );
		return new String( tmp );
	  }
	  else
		return new String( tmp );
	}

	void String::EStrReplace( String* str1, String* str2 )
	{
        std::string::size_type valpos = 0;
        while (std::string::npos != (valpos = value_.find(str1->value_, valpos)))
	  {
		value_.replace( valpos, str1->length(), str2->value_ );
        valpos += str2->length();
	  }
	}

	void String::ESubStrReplace( String* replace_with, unsigned int index, unsigned int len )
	{
	  value_.replace( index - 1, len, replace_with->value_ );
	}

    std::string String::pack() const
	{
	  return "s" + value_;
	}

    void String::packonto(std::ostream& os) const
	{
	  os << "S" << value_.size() << ":" << value_;
	}
    void String::packonto(std::ostream& os, const std::string& value)
	{
	  os << "S" << value.size() << ":" << value;
	}

	BObjectImp* String::unpack( const char* pstr )
	{
	  return new String( pstr );
	}

    BObjectImp* String::unpack(std::istream& is)
	{
      std::string tmp;
	  getline( is, tmp );

	  return new String( tmp );
	}

    BObjectImp* String::unpackWithLen(std::istream& is)
	{
	  unsigned len;
	  char colon;
	  if ( !( is >> len >> colon ) )
	  {
		return new BError( "Unable to unpack string length." );
	  }
	  if ( (int)len < 0 )
	  {
		return new BError( "Unable to unpack string length. Invalid length!" );
	  }
	  if ( colon != ':' )
	  {
		return new BError( "Unable to unpack string length. Bad format. Colon not found!" );
	  }

      is.unsetf(std::ios::skipws);
      std::string tmp;
	  tmp.reserve( len );
	  while ( len-- )
	  {
		char ch = '\0';
		if ( !( is >> ch ) || ch == '\0' )
		{
		  return new BError( "Unable to unpack string length. String length excessive." );
		}
		tmp += ch;
	  }

      is.setf(std::ios::skipws);
	  return new String( tmp );
	}

	size_t String::sizeEstimate() const
	{
	  return sizeof(String)+value_.capacity();
	}

	void String::toUpper( void )
	{
	  for( char &c : value_ )
	  {
		c = static_cast<char>(toupper( c ));
	  }
	}

	void String::toLower( void )
	{
	  for( char &c : value_ )
	  {
		c = static_cast<char>(tolower( c ));
	  }
	}


	// -- format related stuff --

    bool s_parse_int(int &i, std::string const &s)
	{
	  if ( s.empty() )
		return false;

	  char* end;
	  i = strtol( s.c_str(), &end, 10 );

	  if ( !*end )
	  {
		return true;
	  }
	  else
	  {
		return false;
	  }
	}

	// remove leading/trailing spaces
    void s_trim(std::string &s)
	{
	  std::stringstream trimmer;
	  trimmer << s;
	  s.clear();
	  trimmer >> s;
	}

	void int_to_binstr( int& value, std::stringstream &s )
	{
	  int i;
	  for ( i = 31; i > 0; i-- )
	  {
		if ( value & ( 1 << i ) )
		  break;
	  }
	  for ( ; i >= 0; i-- )
	  {
		if ( value & ( 1 << i ) )
		  s << "1";
		else
		  s << "0";
	  }
	}

    // suplementory function to format
    bool try_to_format( std::stringstream& to_stream, BObjectImp* what, std::string& frmt )
    {
      if ( frmt.empty() )
      {
        to_stream << what->getStringRep();
        return false;
      }

      if ( frmt.find( 'b' ) != std::string::npos )
      {
        if ( !what->isa( BObjectImp::OTLong ) )
        {
          to_stream << "<needs Int>";
          return false;
        }
        BLong* plong = static_cast<BLong*>( what );
        int n = plong->value();
        if ( frmt.find( '#' ) != std::string::npos )
          to_stream << ( ( n < 0 ) ? "-" : "" ) << "0b";
        int_to_binstr( n, to_stream );
      }
      else if ( frmt.find( 'x' ) != std::string::npos )
      {
        if ( !what->isa( BObjectImp::OTLong ) )
        {
          to_stream << "<needs Int>";
          return false;
        }
        BLong* plong = static_cast<BLong*>( what );
        int n = plong->value();
        if ( frmt.find( '#' ) != std::string::npos )
          to_stream << "0x";
        to_stream << std::hex << n;
      }
      else if ( frmt.find( 'o' ) != std::string::npos )
      {
        if ( !what->isa( BObjectImp::OTLong ) )
        {
          to_stream << "<needs Int>";
          return false;
        }
        BLong* plong = static_cast<BLong*>( what );
        int n = plong->value();
        if ( frmt.find( '#' ) != std::string::npos )
          to_stream << "0o";
        to_stream << std::oct << n;
      }
      else if ( frmt.find( 'd' ) != std::string::npos )
      {
        int n;
        if ( what->isa( BObjectImp::OTLong ) )
        {
          BLong* plong = static_cast<BLong*>( what );
          n = plong->value();
        }
        else if ( what->isa( BObjectImp::OTDouble ) )
        {
          Double* pdbl = static_cast<Double*>( what );
          n = (int)pdbl->value();
        }
        else
        {
          to_stream << "<needs Int, Double>";
          return false;
        }
        to_stream << std::dec << n;
      }
      else
      {
        to_stream << "<bad format: " << frmt << ">";
        return false;
      }
      return true;
    }
    // --

	BObjectImp* String::call_method( const char* methodname, Executor& ex )
	{
	  ObjMethod* objmethod = getKnownObjMethod( methodname );
	  if ( objmethod != NULL )
		return this->call_method_id( objmethod->id, ex );
	  else
		return NULL;
	}
    BObjectImp* String::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
    {
      switch ( id )
      {
        case MTH_LENGTH:
          if ( ex.numParams() == 0 )
            return new BLong( static_cast<int>( value_.length() ) );
          else
            return new BError( "string.length() doesn't take parameters." );
          break;
        case MTH_FIND:
        {
          if ( ex.numParams() > 2 )
            return new BError( "string.find(Search, [Start]) takes only two parameters" );
          if ( ex.numParams() < 1 )
            return new BError( "string.find(Search, [Start]) takes at least one parameter" );
          const char* s = ex.paramAsString( 0 );
          int d = 0;
          if ( ex.numParams() == 2 )
            d = ex.paramAsLong( 1 );
          int posn = find( d ? ( d - 1 ) : 0, s ) + 1;
          return new BLong( posn );
        }
        case MTH_UPPER:
        {
          if ( ex.numParams() == 0 )
          {
            toUpper();
            return this;
          }
          else
            return new BError( "string.upper() doesn't take parameters." );
        }

        case MTH_LOWER:
        {
          if ( ex.numParams() == 0 )
          {
            toLower();
            return this;
          }
          else
            return new BError( "string.lower() doesn't take parameters." );
        }
        case MTH_FORMAT:
        {
          if ( ex.numParams() > 0 )
          {
            // string s = this->getStringRep(); // string itself
            std::stringstream result;

            size_t tag_start_pos;  // the position of tag's start "{"
            size_t tag_stop_pos;   // the position of tag's end "}"
            size_t tag_dot_pos;

            int tag_param_idx;

            size_t str_pos = 0;         // current string position
            unsigned int next_param_idx = 0;  // next index of .format() parameter

            char w_spaces[] = "\t ";

           	// Tells whether last found tag was an integer
            bool last_tag_was_int = true;

            while ( ( tag_start_pos = value_.find( "{", str_pos ) ) != std::string::npos )
            {
              if ( ( tag_stop_pos = value_.find( "}", tag_start_pos ) ) != std::string::npos )
              {
                result << value_.substr( str_pos, tag_start_pos - str_pos );
                str_pos = tag_stop_pos + 1;

                std::string tag_body =
                    value_.substr( tag_start_pos + 1, ( tag_stop_pos - tag_start_pos ) - 1 );

                tag_start_pos = tag_body.find_first_not_of( w_spaces );
                tag_stop_pos = tag_body.find_last_not_of( w_spaces );

                // cout << "' tag_body1: '" << tag_body << "'";

                // trim the tag of whitespaces (slightly faster code ~25%)
                if ( tag_start_pos != std::string::npos && tag_stop_pos != std::string::npos )
                  tag_body = tag_body.substr( tag_start_pos, ( tag_stop_pos - tag_start_pos ) + 1 );
                else if ( tag_start_pos != std::string::npos )
                  tag_body = tag_body.substr( tag_start_pos );
                else if ( tag_stop_pos != std::string::npos )
                  tag_body = tag_body.substr( 0, tag_stop_pos + 1 );

                // s_trim( tag_body ); // trim the tag of whitespaces

                // cout << "' tag_body2: '" << tag_body << "'";

                std::string frmt;
                size_t formatter_pos = tag_body.find( ':' );

                if ( formatter_pos != std::string::npos )
                {
                  frmt = tag_body.substr( formatter_pos + 1, std::string::npos );  //
                  tag_body = tag_body.substr( 0, formatter_pos );  // remove property from the tag
                }

                std::string prop_name;
                // parsing {1.this_part}
                tag_dot_pos = tag_body.find( ".", 0 );

                // '.' is found within the tag, there is a property name
                if ( tag_dot_pos != std::string::npos )
                {
                  last_tag_was_int = true;
                  prop_name = tag_body.substr( tag_dot_pos + 1, std::string::npos );  //
                  tag_body = tag_body.substr( 0, tag_dot_pos );  // remove property from the tag

                  // if s_tag_body is numeric then use it as an index
                  if ( s_parse_int( tag_param_idx, tag_body ) )
                  {
                    tag_param_idx -= 1;  // sinse POL indexes are 1-based
                  }
                  else
                  {
                    result << "<idx required before: '" << prop_name << "'>";
                    continue;
                  }
                }
                else
                {
                  if ( tag_body == "" )
                  {
                    // empty body just takes next integer idx
                    last_tag_was_int = true;
                    tag_param_idx = next_param_idx++;
                  }
                  else if ( s_parse_int( tag_param_idx, tag_body ) )
                  {
                    last_tag_was_int = true;
                    tag_param_idx -= 1;  // sinse POL indexes are 1-based
                  }
                  else
                  {
                    // string body takes next idx in line if this is
                    // the first string body occurrence,
                    // will reuse last idx if this is 2nd or more in a row
                    last_tag_was_int = false;
                    prop_name = tag_body;
                    tag_param_idx = last_tag_was_int ? next_param_idx++ : next_param_idx;
                  }
                }

                // -- end of property parsing

                // cout << "prop_name: '" << prop_name << "' tag_body: '" << tag_body << "'";

                if ( ex.numParams() <= tag_param_idx )
                {
                  result << "<invalid index: #" << ( tag_param_idx + 1 ) << ">";
                  continue;
                }

                BObjectImp* imp = ex.getParamImp( tag_param_idx );

                if ( !prop_name.empty() )
                {  // accesing object
                  BObjectRef obj_member = imp->get_member( prop_name.c_str() );
                  BObjectImp* member_imp = obj_member->impptr();
                  try_to_format( result, member_imp, frmt );
                }
                else
                {
                  try_to_format( result, imp, frmt );
                }
              }
              else
              {
                break;
              }
            }

            if ( str_pos < value_.length() )
            {
              result << value_.substr( str_pos, std::string::npos );
            }

            return new String( result.str() );
          }
          else
          {
            return new BError( "string.format() requires a parameter." );
          }
        }
        case MTH_JOIN:
        {
          BObject* cont;
          if ( ex.numParams() == 1 && ( cont = ex.getParamObj( 0 ) ) != NULL )
          {
            if ( !( cont->isa( OTArray ) ) )
              return new BError( "string.join expects an array" );
            ObjArray* container = static_cast<ObjArray*>( cont->impptr() );
            // no empty check here on purpose
            OSTRINGSTREAM joined;
            bool first = true;
            for ( const BObjectRef& ref : container->ref_arr )
            {
              if ( ref.get() )
              {
                BObject* bo = ref.get();

                if ( bo == NULL )
                  continue;
                if ( !first )
                  joined << value_;
                else
                  first = false;
                joined << bo->impptr()->getStringRep();
              }
            }
            return new String( OSTRINGSTREAM_STR( joined ) );
          }
          else
            return new BError( "string.join(array) requires a parameter." );
        }
        default:
          return NULL;
      }
    }
  }
}
