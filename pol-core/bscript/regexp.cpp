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
  if ( d < 0 || static_cast<size_t>( d ) >= value_->length() )
    return new BLong( 0 );

  try
  {
    return std::visit(
        [&]( auto&& re )
        {
          using ReT = std::decay_t<decltype( re )>;

          if constexpr ( std::is_same_v<ReT, boost::regex> )
          {
            boost::smatch match;
            if ( boost::regex_search( value_->value().cbegin() + d, value_->value().cend(), match,
                                      re, match_flags_ ) )
            {
              return new BLong( Clib::clamp_convert<int>( match.position() + d + 1 ) );
            }
          }
          else
          {
            auto input = Clib::to_wstring( value_->value() );
            boost::wsmatch match;
            if ( boost::regex_search( input.cbegin() + d, input.cend(), match, re, match_flags_ ) )
            {
              return new BLong( Clib::clamp_convert<int>( match.position() + d + 1 ) );
            }
          }

          return new BLong( 0 );
        },
        regex_ );
  }
  catch ( ... )
  {
    return new BError( "Error during regular expression operation" );
  }
}

BObjectImp* BRegExp::match( const String* value_ ) const
{
  auto add_match = []<typename MatchT>( const MatchT& pieces_match )
  {
    std::unique_ptr<BStruct> result( new BStruct );
    std::unique_ptr<ObjArray> groups( new ObjArray );
    result->addMember( "matched", new String( pieces_match.str( 0 ) ) );
    for ( size_t i = 1; i < pieces_match.size(); ++i )
    {
      std::unique_ptr<BStruct> group( new BStruct );
      group->addMember( "matched", new String( pieces_match.str( i ) ) );
      group->addMember( "offset",
                        new BLong( Clib::clamp_convert<int>( pieces_match.position( i ) + 1 ) ) );
      groups->addElement( group.release() );
    }
    result->addMember( "groups", groups.release() );
    result->addMember( "offset",
                       new BLong( Clib::clamp_convert<int>( pieces_match.position() + 1 ) ) );

    return result.release();
  };

  try
  {
    // Non-global regex: Return first struct{ matched, groups } or uninit
    if ( match_flags_ & boost::regex_constants::format_first_only )
    {
      return std::visit(
          [&]( auto&& re ) -> BObjectImp*
          {
            using ReT = std::decay_t<decltype( re )>;

            if constexpr ( std::is_same_v<ReT, boost::regex> )
            {
              boost::smatch pieces_match;
              if ( boost::regex_search( value_->value().cbegin(), value_->value().cend(),
                                        pieces_match, re ) )
              {
                return add_match( pieces_match );
              }
              else
              {
                return UninitObject::create();
              }
            }
            else
            {
              boost::wsmatch pieces_match;
              auto input = Clib::to_wstring( value_->value() );
              if ( boost::regex_search( input.cbegin(), input.cend(), pieces_match, re ) )
              {
                return add_match( pieces_match );
              }
              else
              {
                return UninitObject::create();
              }
            }
          },
          regex_ );
    }

    // Global regex: Return array of all struct{ matched, groups }
    return std::visit(
        [&]( auto&& re ) -> BObjectImp*
        {
          using ReT = std::decay_t<decltype( re )>;

          if constexpr ( std::is_same_v<ReT, boost::regex> )
          {
            boost::sregex_iterator current_match( value_->value().cbegin(), value_->value().cend(),
                                                  re, match_flags_ );
            boost::sregex_iterator last_match;

            std::unique_ptr<ObjArray> all_matches( new ObjArray );

            while ( current_match != last_match )
            {
              all_matches->addElement( add_match( *current_match ) );
              ++current_match;
            }

            return all_matches.release();
          }
          else
          {
            auto input = Clib::to_wstring( value_->value() );
            boost::wsregex_iterator current_match( input.cbegin(), input.cend(), re, match_flags_ );
            boost::wsregex_iterator last_match;

            std::unique_ptr<ObjArray> all_matches( new ObjArray );

            while ( current_match != last_match )
            {
              all_matches->addElement( add_match( *current_match ) );
              ++current_match;
            }

            return all_matches.release();
          }
        },
        regex_ );
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
        [&]( auto&& re ) -> BObjectImp*
        {
          using ReT = std::decay_t<decltype( re )>;

          if constexpr ( std::is_same_v<ReT, boost::regex> )
          {
            return new String(
                boost::regex_replace( value_->value(), re, replacement->value(), match_flags_ ) );
          }
          else
          {
            auto input = Clib::to_wstring( value_->value() );
            return new String( boost::regex_replace(
                input, re, Clib::to_wstring( replacement->value() ), match_flags_ ) );
          }
        },
        regex_ );
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
    auto make_args =
        []<typename MatchT, typename StringT>( const MatchT& match, const StringT& input )
    {
      BObjectRefVec args;

      // Matched string
      args.push_back( BObjectRef( new String( match.str() ) ) );

      // Add group captures as an array
      std::unique_ptr<ObjArray> groups( new ObjArray );
      for ( size_t i = 1; i < match.size(); ++i )
      {
        std::unique_ptr<BStruct> group( new BStruct );
        group->addMember( "matched", new String( match.str( i ) ) );
        group->addMember( "offset",
                          new BLong( Clib::clamp_convert<int>( match.position( i ) + 1 ) ) );
        groups->addElement( group.release() );
      }
      args.push_back( BObjectRef( groups.release() ) );

      // Add offset and original string as arguments
      args.push_back( BObjectRef( new BLong( Clib::clamp_convert<int>( match.position() + 1 ) ) ) );
      args.push_back( BObjectRef( new String( input ) ) );

      return args;
    };

    return std::visit(
        [&]( auto&& re ) -> BObjectImp*
        {
          using ReT = std::decay_t<decltype( re )>;

          if constexpr ( std::is_same_v<ReT, boost::regex> )
          {
            auto input = std::make_shared<std::string>( str->value() );

            boost::sregex_iterator it( input->cbegin(), input->cend(), re, match_flags_ );
            boost::sregex_iterator end;

            if ( it == end )
            {
              // No matches found, return the original string
              return str->copy();
            }

            std::string replaced_result;
            std::size_t lastPos = 0;
            boost::smatch match = *it;

            BObjectRefVec args = make_args( match, *input );

            // Append the text before the first match
            replaced_result.append( *input, lastPos, match.position() - lastPos );
            lastPos = match.position() + match.length();

            auto callback = [make_args = std::move( make_args ),
                             regex = BObjectRef( this ),  // keep regex alive
                             input = std::move( input ), it = std::move( it ),
                             end = std::move( end ), lastPos = std::move( lastPos ),
                             match = std::move( match ),
                             replaced_result = std::move( replaced_result )](
                                Executor& ex, BContinuation* continuation,
                                BObjectRef result ) mutable -> BObjectImp*
            {
              // Append the replacement from the callback
              replaced_result.append( result->impptr()->getStringRep() );

              ++it;

              // If no more matches, or not global, return the result
              if ( it == end || ( regex->impptr<BRegExp>()->match_flags_ &
                                  boost::regex_constants::format_first_only ) )
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

            return ex.makeContinuation( BObjectRef( new BObject( replacement_callback ) ), callback,
                                        std::move( args ) );
          }
          else
          {
            auto input = std::make_shared<std::wstring>( Clib::to_wstring( str->value() ) );

            boost::wsregex_iterator it( input->cbegin(), input->cend(), re, match_flags_ );
            boost::wsregex_iterator end;

            if ( it == end )
            {
              // No matches found, return the original string
              return str->copy();
            }

            std::wstring replaced_result;
            std::size_t lastPos = 0;
            boost::wsmatch match = *it;

            BObjectRefVec args = make_args( match, *input );

            // Append the text before the first match
            replaced_result.append( *input, lastPos, match.position() - lastPos );
            lastPos = match.position() + match.length();

            auto callback = [make_args = std::move( make_args ),
                             regex = BObjectRef( this ),  // keep regex alive
                             input = std::move( input ), it = std::move( it ),
                             end = std::move( end ), lastPos = std::move( lastPos ),
                             match = std::move( match ),
                             replaced_result = std::move( replaced_result )](
                                Executor& ex, BContinuation* continuation,
                                BObjectRef result ) mutable -> BObjectImp*
            {
              // Append the replacement from the callback
              replaced_result.append( Clib::to_wstring( result->impptr()->getStringRep() ) );

              ++it;

              // If no more matches, or not global, return the result
              if ( it == end || ( regex->impptr<BRegExp>()->match_flags_ &
                                  boost::regex_constants::format_first_only ) )
              {
                // Append the tail after the last match
                replaced_result.append( *input, lastPos, std::string::npos );

                // Use Tainted::YES because the callback might have returned some funky strings
                return new String( replaced_result, String::Tainted::YES );
              }

              // Process the next match
              match = *it;
              replaced_result.append( *input, lastPos, match.position() - lastPos );
              lastPos = match.position() + match.length();

              BObjectRefVec args = make_args( match, *input );

              return ex.withContinuation( continuation, std::move( args ) );
            };

            return ex.makeContinuation( BObjectRef( new BObject( replacement_callback ) ), callback,
                                        std::move( args ) );
          }
        },
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
