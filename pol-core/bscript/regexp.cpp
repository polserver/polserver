#include "regexp.h"
#include <stdexcept>

#include "bscript/berror.h"
#include "bscript/impstr.h"
#include "clib/strutil.h"
#include "exectype.h"
#include "executor.h"
#include "executor.inl.h"

namespace Pol::Bscript
{
// Traits to unify narrow vs wide regex handling
template <typename RegexT>
struct regex_traits;

template <>
struct regex_traits<boost::regex>
{
  using string_type = std::string;
  using match_type = boost::smatch;
  using iterator_type = boost::sregex_iterator;

  static string_type convert( const std::string& s ) { return s; }
};

template <>
struct regex_traits<boost::wregex>
{
  using string_type = std::wstring;
  using match_type = boost::wsmatch;
  using iterator_type = boost::wsregex_iterator;

  static string_type convert( const std::string& s ) { return Clib::to_wstring( s ); }
};

// Generic helpers
template <typename RegexT>
BObjectImp* do_find( const RegexT& re, const String* value, int d, boost::match_flag_type flags )
{
  using traits = regex_traits<RegexT>;
  using match_type = typename traits::match_type;
  using string_type = typename traits::string_type;

  string_type input = traits::convert( value->value() );
  match_type match;

  if ( d >= 0 && static_cast<size_t>( d ) < input.size() &&
       boost::regex_search( input.cbegin() + d, input.cend(), match, re, flags ) )
  {
    return new BLong( Clib::clamp_convert<int>( match.position() + d + 1 ) );
  }

  return new BLong( 0 );
}

template <typename RegexT>
BObjectImp* do_match( const RegexT& re, const String* value, boost::match_flag_type flags )
{
  using traits = regex_traits<RegexT>;
  using match_type = typename traits::match_type;
  using string_type = typename traits::string_type;
  using iterator_type = typename traits::iterator_type;

  auto add_match = []<typename MatchT>( const MatchT& pieces_match )
  {
    std::unique_ptr<BStruct> result( new BStruct );
    std::unique_ptr<ObjArray> groups( new ObjArray );
    result->addMember( "matched", new String( pieces_match.str( 0 ) ) );
    for ( size_t i = 1; i < pieces_match.size(); ++i )
    {
      if ( pieces_match[i].matched )
      {
        std::unique_ptr<BStruct> group( new BStruct );

        group->addMember( "matched", new String( pieces_match.str( i ) ) );
        group->addMember( "offset",
                          new BLong( Clib::clamp_convert<int>( pieces_match.position( i ) + 1 ) ) );

        groups->addElement( group.release() );
      }
      else
      {
        groups->addElement( UninitObject::create() );
      }
    }
    result->addMember( "groups", groups.release() );
    result->addMember( "offset",
                       new BLong( Clib::clamp_convert<int>( pieces_match.position() + 1 ) ) );

    return result.release();
  };

  string_type input = traits::convert( value->value() );

  // Non-global regex: Return first struct{ matched, groups, offset } or uninit
  if ( flags & boost::regex_constants::format_first_only )
  {
    match_type match;
    if ( boost::regex_search( input.cbegin(), input.cend(), match, re ) )
    {
      return add_match( match );
    }

    return UninitObject::create();
  }
  // Global regex: Return array of all struct{ matched, groups, offset }
  else
  {
    iterator_type current_match( input.cbegin(), input.cend(), re, flags );
    iterator_type last_match;
    std::unique_ptr<ObjArray> all_matches( new ObjArray );

    while ( current_match != last_match )
    {
      all_matches->addElement( add_match( *current_match ) );
      ++current_match;
    }

    return all_matches.release();
  }
}

template <typename RegexT>
BObjectImp* do_replace( const RegexT& re, const String* value, const String* replacement,
                        boost::match_flag_type flags )
{
  using traits = regex_traits<RegexT>;
  auto input = traits::convert( value->value() );
  auto repl = traits::convert( replacement->value() );

  auto result = boost::regex_replace( input, re, repl, flags );
  return new String( result );
}

template <typename RegexT>
BObjectImp* do_replace( const RegexT& re, Executor& ex, BRegExp* bregexp, const String* value,
                        BFunctionRef* replacement_callback, boost::match_flag_type flags )
{
  using traits = regex_traits<RegexT>;
  using match_type = typename traits::match_type;
  using string_type = typename traits::string_type;
  using iterator_type = typename traits::iterator_type;

  auto make_args = []( const auto& match, const auto& input )
  {
    BObjectRefVec args;

    // Matched string
    args.push_back( BObjectRef( new String( match.str() ) ) );

    // Add group captures as an array
    std::unique_ptr<ObjArray> groups( new ObjArray );
    for ( size_t i = 1; i < match.size(); ++i )
    {
      if ( match[i].matched )
      {
        std::unique_ptr<BStruct> group( new BStruct );

        group->addMember( "matched", new String( match.str( i ) ) );
        group->addMember( "offset",
                          new BLong( Clib::clamp_convert<int>( match.position( i ) + 1 ) ) );

        groups->addElement( group.release() );
      }
      else
      {
        groups->addElement( UninitObject::create() );
      }
    }
    args.push_back( BObjectRef( groups.release() ) );

    // Add offset and original string as arguments
    args.push_back( BObjectRef( new BLong( Clib::clamp_convert<int>( match.position() + 1 ) ) ) );
    args.push_back( BObjectRef( new String( input ) ) );

    return args;
  };

  auto input = std::make_unique<string_type>( traits::convert( value->value() ) );
  iterator_type it( input->cbegin(), input->cend(), re, flags );
  iterator_type end;

  if ( it == end )
  {
    // No matches found, return the original string
    return value->copy();
  }

  string_type replaced_result;
  std::size_t lastPos = 0;
  match_type match = *it;

  BObjectRefVec args = make_args( match, *input );

  // Append the text before the first match
  replaced_result.append( *input, lastPos, match.position() - lastPos );
  lastPos = match.position() + match.length();

  auto callback = [make_args = std::move( make_args ),
                   regex = BObjectRef( bregexp ),  // keep regex alive
                   flags, input = std::move( input ), it = std::move( it ), end = std::move( end ),
                   lastPos = std::move( lastPos ), match = std::move( match ),
                   replaced_result = std::move( replaced_result )](
                      Executor& ex, BContinuation* continuation,
                      BObjectRef result ) mutable -> BObjectImp*
  {
    // Append the replacement from the callback
    replaced_result.append( traits::convert( result->impptr()->getStringRep() ) );

    ++it;

    // If no more matches, or not global, return the result
    if ( it == end || ( flags & boost::regex_constants::format_first_only ) )
    {
      // Append the tail after the last match
      replaced_result.append( *input, lastPos, std::string::npos );
      return new String( replaced_result );
    }

    // Process the next match
    match = *it;
    replaced_result.append( *input, lastPos, match.position() - lastPos );
    lastPos = match.position() + match.length();

    BObjectRefVec args = make_args( match, *input );

    return ex.withContinuation( continuation, std::move( args ) );
  };

  return ex.makeContinuation( BObjectRef( new BObject( replacement_callback ) ),
                              std::move( callback ), std::move( args ) );
}

template <typename RegexT>
BObjectImp* do_split( const RegexT& re, const String* value, size_t limit,
                      boost::match_flag_type flags )
{
  using traits = regex_traits<RegexT>;
  using string_type = typename traits::string_type;
  using iterator_type = typename traits::iterator_type;

  std::unique_ptr<ObjArray> result( new ObjArray );

  auto input = traits::convert( value->value() );
  auto start = input.cbegin();
  auto end = input.cend();

  iterator_type it( start, end, re, flags );
  iterator_type end_it;

  std::size_t last_pos = 0;

  for ( ; it != end_it && result->ref_arr.size() < limit - 1; ++it )
  {
    auto m = *it;

    // text before the match
    result->addElement( new String( string_type( start + last_pos, m[0].first ) ) );

    // captured groups (if any)
    for ( std::size_t i = 1; i < m.size(); ++i )
    {
      if ( result->ref_arr.size() >= limit )
        break;

      if ( m[i].matched )
      {
        result->addElement( new String( m[i].str() ) );
      }
      else
      {
        result->addElement( UninitObject::create() );
      }
    }

    last_pos = m[0].second - start;
  }

  // remaining tail
  if ( result->ref_arr.size() < limit )
  {
    result->addElement( new String( string_type( start + last_pos, end ) ) );
  }

  return result.release();
}

BRegExp::BRegExp( RegexT regex, boost::match_flag_type match_flags )
    : BObjectImp( OTRegExp ), regex_( std::move( regex ) ), match_flags_( match_flags )
{
}

BObjectImp* BRegExp::create( const std::string& pattern, const std::string& flags )
{
  boost::match_flag_type match_flags =
      boost::regex_constants::match_single_line | boost::regex_constants::format_first_only;

  boost::regex_constants::syntax_option_type flag = boost::regex_constants::ECMAScript;
  bool unicode = false;

  for ( const auto& ch : flags )
  {
    switch ( ch )
    {
    case 'u':
      unicode = true;
      break;
    case 'i':
      flag |= boost::regex_constants::icase;
      break;
    case 'm':
      match_flags &= ~boost::regex_constants::match_single_line;
      break;
    case 'g':
      match_flags &= ~boost::regex_constants::format_first_only;
      break;
    default:
      return new BError( "Invalid flag character" );
    }
  }

  try
  {
    if ( unicode )
    {
      auto pattern_w = Clib::to_wstring( pattern );
      return new BRegExp( boost::wregex( pattern_w, flag ), match_flags );
    }

    return new BRegExp( boost::regex( pattern, flag ), match_flags );
  }
  catch ( ... )
  {
    return new BError( "Invalid regular expression" );
  }
}

BRegExp::BRegExp( const BRegExp& i )
    : BObjectImp( OTRegExp ), regex_( i.regex_ ), match_flags_( i.match_flags_ )
{
}

BObjectImp* BRegExp::find( const String* value_, int d ) const
{
  try
  {
    return std::visit( [&]( auto&& re ) { return do_find( re, value_, d, match_flags_ ); },
                       regex_ );
  }
  catch ( ... )
  {
    return new BError( "Error during regular expression operation" );
  }
}

BObjectImp* BRegExp::match( const String* value_ ) const
{
  try
  {
    return std::visit( [&]( auto&& re ) { return do_match( re, value_, match_flags_ ); }, regex_ );
  }
  catch ( ... )
  {
    return new BError( "Error during regular expression operation" );
  }
}

BObjectImp* BRegExp::replace( const String* value_, const String* replacement ) const
{
  try
  {
    return std::visit(
        [&]( auto&& re ) { return do_replace( re, value_, replacement, match_flags_ ); }, regex_ );
  }
  catch ( ... )
  {
    return new BError( "Error during regular expression operation" );
  }
}

BObjectImp* BRegExp::replace( Executor& ex, const String* str, BFunctionRef* replacement_callback )
{
  try
  {
    return std::visit(
        [&]( auto&& re )
        { return do_replace( re, ex, this, str, replacement_callback, match_flags_ ); },
        regex_ );
  }
  catch ( ... )
  {
    return new BError( "Error during regular expression operation" );
  }
}

BObjectImp* BRegExp::split( const String* str, size_t limit ) const
{
  try
  {
    return std::visit( [&]( auto&& re ) { return do_split( re, str, limit, match_flags_ ); },
                       regex_ );
  }
  catch ( ... )
  {
    return new BError( "Error during regular expression operation" );
  }
}

BObjectImp* BRegExp::copy() const
{
  return new BRegExp( *this );
}

const char* BRegExp::typeOf() const
{
  return "RegExp";
}

u8 BRegExp::typeOfInt() const
{
  return OTRegExp;
}

std::string BRegExp::getStringRep() const
{
  return "<RegExp>";
}

size_t BRegExp::sizeEstimate() const
{
  return sizeof( BRegExp );
}

bool BRegExp::operator<( const BObjectImp& ) const
{
  return false;
}

bool BRegExp::isTrue() const
{
  return true;
}
}  // namespace Pol::Bscript
