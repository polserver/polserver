#include "PrettifyLineBuilder.h"

#include "bscript/compilercfg.h"
#include "clib/filecont.h"
#include "clib/logfacility.h"

#include <algorithm>
#include <iostream>
#include <utility>

// #define DEBUG_FORMAT_BREAK

namespace Pol::Bscript::Compiler
{

const std::vector<std::string>& PrettifyLineBuilder::formattedLines() const
{
  return _lines;
}

void PrettifyLineBuilder::setRawLines( std::vector<std::string> rawlines )
{
  _rawlines = std::move( rawlines );
}

void PrettifyLineBuilder::setComments( std::vector<CommentInfo> comments )
{
  _comments = std::move( comments );
}

void PrettifyLineBuilder::setSkipLines( std::vector<Range> skiplines )
{
  _skiplines = std::move( skiplines );
}

void PrettifyLineBuilder::addPart( TokenPart part )
{
  for ( const auto& skip : _skiplines )
    if ( skip.contains( part.pos ) )
      return;
  _line_parts.emplace_back( std::move( part ) );
}

bool PrettifyLineBuilder::finalize()
{
  mergeEOFComments();
  return _line_parts.empty();
}

const std::vector<TokenPart>& PrettifyLineBuilder::currentTokens() const
{
  return _line_parts;
}

void PrettifyLineBuilder::mergeRawContent( int nextlineno )
{
  for ( auto itr = _skiplines.begin(); itr != _skiplines.end(); )
  {
    if ( itr->start.line_number < nextlineno )
    {
      mergeCommentsBefore( itr->start.line_number );
      addEmptyLines( itr->start.line_number );
      for ( size_t i = itr->start.line_number - 1; i < itr->end.line_number && i < _rawlines.size();
            ++i )
      {
        _lines.emplace_back( _rawlines[i] );
        _last_line = i + 1;
      }
      itr = _skiplines.erase( itr );
      continue;
    }
    ++itr;
  }
}

void PrettifyLineBuilder::mergeCommentsBefore( int nextlineno )
{
  // add comments before current line
  while ( !_comments.empty() )
  {
    if ( _comments.front().pos.line_number < nextlineno )
    {
      addEmptyLines( _comments.front().pos.line_number );
      _lines.push_back( identSpacing() + _comments.front().text );
      _last_line = _comments.front().pos_end.line_number;
      _comments.erase( _comments.begin() );
    }
    else
      break;
  }
}

void PrettifyLineBuilder::mergeComments()
{
  if ( _line_parts.empty() )
    return;
  mergeRawContent( _line_parts.front().lineno );
  mergeCommentsBefore( _line_parts.front().lineno );
  // add comments inbetween
  for ( size_t i = 0; i < _line_parts.size(); )
  {
    if ( _comments.empty() )
      break;
    if ( _line_parts[i].tokenid > _comments.front().pos.token_index )
    {
      TokenPart p{ std::move( _comments.front().text ), _comments.front().pos, TokenPart::SPACE,
                   _currentgroup };
      if ( _comments.front().linecomment )
        p.style |= TokenPart::FORCED_BREAK;
      _line_parts.insert( _line_parts.begin() + i, p );
      _comments.erase( _comments.begin() );
    }
    else
      ++i;
  }
  // add comments at the end of the current line
  if ( !_comments.empty() )
  {
    if ( _comments.front().pos.line_number == _line_parts.back().lineno &&
         _comments.front().pos.token_index <= _line_parts.back().tokenid + 1 )
    {
      addPart(
          { std::move( _comments.front().text ), _comments.front().pos, TokenPart::SPACE, 0 } );
      _comments.erase( _comments.begin() );
    }
  }
}

void PrettifyLineBuilder::buildLine( size_t current_ident )
{
  _currident = current_ident;
  if ( _line_parts.empty() )
    return;
  auto stripline = []( std::string& line )
  {
    if ( !line.empty() )
    {
      auto lastchar = line.find_last_not_of( ' ' );
      line.erase( line.begin() + lastchar + 1, line.end() );
    }
  };
  mergeComments();

  // fill lines with final strings splitted at breakpoints
  // <splitted string, forcenewline, groupid>
  std::vector<std::tuple<std::string, bool, size_t>> lines;
  std::string line;
  for ( size_t i = 0; i < _line_parts.size(); ++i )
  {
    line += _line_parts[i].text;
    // add space if set, but not if the following part is attached
    if ( _line_parts[i].style & TokenPart::SPACE )
    {
      if ( i + 1 < _line_parts.size() )
      {
        if ( !( _line_parts[i + 1].style & TokenPart::ATTACHED ) )
          line += ' ';
      }
      else
        line += ' ';
    }
    if ( _line_parts[i].style & TokenPart::FORCED_BREAK )
    {
      lines.emplace_back( std::make_tuple( std::move( line ), true, _line_parts[i].group ) );
      line.clear();
    }
    // start a new line if breakpoint
    else if ( _line_parts[i].style & TokenPart::BREAKPOINT )
    {
      // if the next part is attached, dont break now and instead later
      // eg dont split blubb[1]()
      bool skip = false;
      if ( i + 1 < _line_parts.size() )
      {
        // next one is attached
        if ( _line_parts[i + 1].style & TokenPart::ATTACHED )
        {
          // search for space or breakpoint
          for ( size_t j = i + 1; j < _line_parts.size(); ++j )
          {
            if ( _line_parts[j].style & TokenPart::ATTACHED )
              skip = true;
            else if ( _line_parts[j].style & TokenPart::SPACE ||
                      _line_parts[j].style & TokenPart::BREAKPOINT )
            {
              _line_parts[j].style |= TokenPart::BREAKPOINT;
              skip = true;
              break;
            }
          }
        }
      }
      if ( skip )
        continue;
      lines.emplace_back( std::make_tuple( std::move( line ), false, _line_parts[i].group ) );
      line.clear();
    }
  }
  // add remainings
  if ( !line.empty() )
  {
    lines.emplace_back( std::make_tuple( std::move( line ), false, _line_parts.back().group ) );
    line.clear();
  }
#ifdef DEBUG_FORMAT_BREAK
  INFO_PRINTLN( "BREAK " );
  for ( auto& [l, forced, group] : lines )
    INFO_PRINTLN( "\"{}\" {}", l, group );
#endif
  // add newline from original sourcecode
  addEmptyLines( _line_parts.front().lineno );

  // sum up linelength, are groups inside
  bool groups = false;
  size_t linelength = 0;
  for ( auto& [l, forced, group] : lines )
  {
    if ( group != 0 )
      groups = true;
    linelength += l.size();
  }

  // split based on groups
  if ( groups && linelength > compilercfg.FormatterLineWidth )
  {
    std::vector<size_t> alignmentspace = {};
    size_t lastgroup = 0xffffFFFF;
    for ( auto& [l, forced, group] : lines )
    {
      if ( forced )
      {
        line += l;
        stripline( line );
        _lines.emplace_back( std::move( line ) );
        continue;
      }
      if ( lastgroup == 0xffffFFFF )
      {
        line += l;  // first part
        alignmentspace.push_back( line.size() );
      }
      else if ( lastgroup < group )  // new group
      {
        // first store current size as alignment
        alignmentspace.push_back( line.size() );
        line += l;
      }
      else if ( lastgroup > group )  // descending
      {
        stripline( line );
        if ( !line.empty() )
          _lines.emplace_back( std::move( line ) );
        // use last group alignment
        line = alignmentSpacing( alignmentspace.back() );
        // remove last group alignment
        alignmentspace.pop_back();
        line += l;
      }
      else  // same group
      {
        stripline( line );
        if ( !line.empty() )
          _lines.emplace_back( std::move( line ) );
        // use current group alignment
        line = alignmentSpacing( alignmentspace.back() );
        line += l;
      }

      lastgroup = group;
    }
  }
  else  // split based on parts
  {
    // build final lines
    size_t alignmentspace = 0;
    for ( auto& [l, forced, group] : lines )
    {
      // following lines need to be aligned
      if ( line.empty() && alignmentspace )
        line = alignmentSpacing( alignmentspace );
      // first breakpoint defines the alignment and add initial ident level
      if ( !alignmentspace )
      {
        auto ident = identSpacing();
        alignmentspace = l.size() + ident.size();
        line += ident;
      }
      line += l;
      // linewidth reached add current line, start a new one
      if ( line.size() > compilercfg.FormatterLineWidth || forced )
      {
        stripline( line );
        _lines.emplace_back( std::move( line ) );
        line.clear();
      }
    }
  }
  if ( !line.empty() )
  {
    stripline( line );
    _lines.emplace_back( std::move( line ) );
  }
  _last_line = _line_parts.back().lineno;
  _line_parts.clear();
}


void PrettifyLineBuilder::addEmptyLines( int line_number )
{
  if ( compilercfg.FormatterMergeEmptyLines )
  {
    if ( line_number > _last_line + 1 )
      _lines.emplace_back( "" );
  }
  else
  {
    while ( line_number > _last_line + 1 )
    {
      _lines.emplace_back( "" );
      ++_last_line;
    }
  }
}

int PrettifyLineBuilder::closingParenthesisStyle( size_t begin_size )
{
  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == begin_size )
  {
    if ( !compilercfg.FormatterEmptyParenthesisSpacing )
      style |= TokenPart::ATTACHED;
    else if ( !_line_parts.empty() )
      _line_parts.back().style |= TokenPart::SPACE;
  }
  else if ( !compilercfg.FormatterParenthesisSpacing )
    style |= TokenPart::ATTACHED;
  return style;
}

int PrettifyLineBuilder::closingBracketStyle( size_t begin_size )
{
  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == begin_size )
  {
    if ( !compilercfg.FormatterEmptyBracketSpacing )
      style |= TokenPart::ATTACHED;
    else if ( !_line_parts.empty() )
      _line_parts.back().style |= TokenPart::SPACE;
  }
  else if ( !compilercfg.FormatterBracketSpacing )
    style |= TokenPart::ATTACHED;
  return style;
}

int PrettifyLineBuilder::openingParenthesisStyle()
{
  return TokenPart::ATTACHED | TokenPart::BREAKPOINT |
         ( compilercfg.FormatterParenthesisSpacing ? TokenPart::SPACE : TokenPart::NONE );
}

int PrettifyLineBuilder::openingBracketStyle()
{
  return TokenPart::ATTACHED | TokenPart::BREAKPOINT |
         ( compilercfg.FormatterBracketSpacing ? TokenPart::SPACE : TokenPart::NONE );
}

int PrettifyLineBuilder::delimiterStyle()
{
  return TokenPart::ATTACHED | TokenPart::BREAKPOINT |
         ( compilercfg.FormatterDelimiterSpacing ? TokenPart::SPACE : TokenPart::NONE );
}

int PrettifyLineBuilder::terminatorStyle()
{
  return TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT;
}

int PrettifyLineBuilder::assignmentStyle()
{
  if ( !compilercfg.FormatterAssignmentSpacing )
    return TokenPart::ATTACHED;
  return TokenPart::SPACE;
}

int PrettifyLineBuilder::comparisonStyle()
{
  if ( !compilercfg.FormatterComparisonSpacing )
    return TokenPart::ATTACHED;
  return TokenPart::SPACE;
}

int PrettifyLineBuilder::operatorStyle()
{
  if ( !compilercfg.FormatterOperatorSpacing )
    return TokenPart::ATTACHED;
  return TokenPart::SPACE;
}

std::string PrettifyLineBuilder::identSpacing()
{
  if ( !compilercfg.FormatterUseTabs )
    return std::string( _currident * compilercfg.FormatterIdentLevel, ' ' );
  size_t total = _currident * compilercfg.FormatterIdentLevel;
  size_t tabs = total / compilercfg.FormatterTabWidth;
  size_t remaining = total % compilercfg.FormatterTabWidth;
  return std::string( tabs, '\t' ) + std::string( remaining, ' ' );
}

std::string PrettifyLineBuilder::alignmentSpacing( size_t count )
{
  if ( !count )
    return {};
  if ( !compilercfg.FormatterUseTabs )
    return std::string( count, ' ' );
  size_t tabs = count / compilercfg.FormatterTabWidth;
  size_t remaining = count % compilercfg.FormatterTabWidth;
  return std::string( tabs, '\t' ) + std::string( remaining, ' ' );
}

void PrettifyLineBuilder::mergeEOFComments()
{
  mergeRawContent( _last_line );
  while ( !_comments.empty() )
  {
    addEmptyLines( _comments.front().pos.line_number );
    _lines.push_back( identSpacing() + _comments.front().text );
    _last_line = _comments.front().pos_end.line_number;
    mergeRawContent( _last_line );
    _comments.erase( _comments.begin() );
  }
}

}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::TokenPart>::format(
    const Pol::Bscript::Compiler::TokenPart& t, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format(
      fmt::format( "{} ({}:{}:{})", t.text, t.style, t.lineno, t.tokenid ), ctx );
}
