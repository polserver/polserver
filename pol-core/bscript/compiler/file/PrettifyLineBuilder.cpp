#include "PrettifyLineBuilder.h"

#include "bscript/compilercfg.h"
#include "clib/filecont.h"
#include "clib/logfacility.h"
#include <EscriptGrammar/EscriptLexer.h>

#include <algorithm>
#include <iostream>
#include <utility>

// #define DEBUG_FORMAT_BREAK

namespace Pol::Bscript::Compiler
{
using namespace EscriptGrammar;

const std::vector<std::string>& PrettifyLineBuilder::formattedLines() const
{
  return _lines;
}

void PrettifyLineBuilder::setRawLines( std::vector<std::string> rawlines )
{
  _rawlines = std::move( rawlines );
}

void PrettifyLineBuilder::setComments( std::vector<FmtToken> comments )
{
  _comments = std::move( comments );
}

void PrettifyLineBuilder::setSkipLines( std::vector<Range> skiplines )
{
  _skiplines = std::move( skiplines );
}

void PrettifyLineBuilder::addPart( FmtToken part )
{
  for ( const auto& skip : _skiplines )
    if ( skip.contains( part.pos ) )
      return;
  _line_parts.emplace_back( std::move( part ) );
}

bool PrettifyLineBuilder::finalize()
{
  mergeEOFNonTokens();
  return _line_parts.empty() && _comments.empty() && _skiplines.empty();
}

const std::vector<FmtToken>& PrettifyLineBuilder::currentTokens() const
{
  return _line_parts;
}

void PrettifyLineBuilder::mergeRawContent( size_t nextlineno )
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

void PrettifyLineBuilder::mergeCommentsBefore( size_t nextlineno )
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
  mergeRawContent( _line_parts.front().pos.line_number );
  mergeCommentsBefore( _line_parts.front().pos.line_number );
  // add comments inbetween
  for ( size_t i = 0; i < _line_parts.size(); )
  {
    if ( _comments.empty() )
      break;
    if ( _line_parts[i].pos.token_index > _comments.front().pos.token_index )
    {
      auto info = _comments.front();
      info.group = _currentgroup;
      _line_parts.insert( _line_parts.begin() + i, std::move( info ) );
      _comments.erase( _comments.begin() );
    }
    else
      ++i;
  }
  // add comments at the end of the current line
  if ( !_comments.empty() )
  {
    if ( _comments.front().pos.line_number == _line_parts.back().pos.line_number &&
         _comments.front().pos.token_index <= _line_parts.back().pos.token_index + 1 )
    {
      addPart( _comments.front() );
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
  // <splitted string, groupid, firstgroup, style>
  std::vector<std::tuple<std::string, size_t, size_t, int>> lines;
  {
    std::string line;
    size_t firstgroup = 0;
    for ( size_t i = 0; i < _line_parts.size(); ++i )
    {
      if ( line.empty() )
        firstgroup = _line_parts[i].group;
      line += _line_parts[i].text;
      // add space if set, but not if the following part is attached
      if ( _line_parts[i].style & FmtToken::SPACE )
      {
        if ( i + 1 < _line_parts.size() )
        {
          if ( !( _line_parts[i + 1].style & FmtToken::ATTACHED ) )
            line += ' ';
        }
        else
          line += ' ';
      }
      if ( _line_parts[i].style & FmtToken::FORCED_BREAK )
      {
        lines.emplace_back( std::make_tuple( std::move( line ), _line_parts[i].group, firstgroup,
                                             _line_parts[i].style ) );
        line.clear();
      }
      // start a new line if breakpoint
      else if ( _line_parts[i].style & FmtToken::BREAKPOINT )
      {
        // if the next part is attached, dont break now and instead later
        // eg dont split blubb[1]()
        bool skip = false;
        if ( i + 1 < _line_parts.size() )
        {
          // next one is attached
          if ( _line_parts[i + 1].style & FmtToken::ATTACHED )
          {
            // search for space or breakpoint
            for ( size_t j = i + 1; j < _line_parts.size(); ++j )
            {
              if ( _line_parts[j].style & FmtToken::ATTACHED )
                skip = true;
              if ( _line_parts[j].style & FmtToken::SPACE ||
                   _line_parts[j].style & FmtToken::BREAKPOINT )
              {
                _line_parts[j].style |= FmtToken::BREAKPOINT;
                skip = true;
                break;
              }
            }
          }
        }
        if ( skip )
          continue;
        lines.emplace_back( std::make_tuple( std::move( line ), _line_parts[i].group, firstgroup,
                                             _line_parts[i].style ) );
        line.clear();
      }
    }
    // add remainings
    if ( !line.empty() )
    {
      lines.emplace_back( std::make_tuple( std::move( line ), _line_parts.back().group,
                                           _line_parts.back().group, _line_parts.back().style ) );
      line.clear();
    }
  }
#ifdef DEBUG_FORMAT_BREAK
  INFO_PRINTLN( "BREAK " );
  for ( auto& [l, group, firstgroup, style] : lines )
    INFO_PRINTLN( "\"{}\" {}-{} ->{}", l, group, firstgroup, style );
#endif
  // add newline from original sourcecode
  addEmptyLines( _line_parts.front().pos.line_number );

  // sum up linelength, are groups inside or preferred breaks
  bool groups = false;
  bool has_preferred = false;
  bool has_preferred_logical = false;
  size_t linelength = 0;
  for ( auto& [l, group, firstgroup, style] : lines )
  {
    if ( group != 0 )
      groups = true;
    if ( style & FmtToken::PREFERRED_BREAK )
      has_preferred = true;
    if ( style & FmtToken::PREFERRED_BREAK_LOGICAL )
      has_preferred_logical = true;
    linelength += l.size();
  }

  std::string line;
  // split based on groups
  if ( groups && linelength > compilercfg.FormatterLineWidth )
  {
    // store for each group thr alignment
    std::map<size_t, size_t> alignmentspace = {};
    size_t lastgroup = 0xffffFFFF;
    bool newline = false;
    auto ident = identSpacing();
    line = ident;
    bool groupdiffered = false;
    for ( auto& [l, group, firstgroup, style] : lines )
    {
      if ( style & FmtToken::FORCED_BREAK )
      {
        line += l;
        stripline( line );
        _lines.emplace_back( std::move( line ) );
        line = ident;
        continue;
      }
      if ( lastgroup == 0xffffFFFF )
      {
        line += l;  // first part
        alignmentspace[firstgroup] = line.size() - ident.size();
        alignmentspace[group] = line.size() - ident.size();
        newline = true;
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "first {}", line );
#endif
      }
      else if ( lastgroup < firstgroup )  // new group
      {
        groupdiffered = true;
        bool newgroup = false;
        if ( !alignmentspace.count( firstgroup ) )
        {
          newgroup = true;
          alignmentspace[firstgroup] = line.size() - ident.size();
        }
        // if its not a new group and not a fresh line start a new one
        // nested structs eg should start at the same line
        if ( !newgroup && !newline )
        {
          stripline( line );
          if ( !line.empty() )
            _lines.emplace_back( std::move( line ) );
          line = alignmentSpacing( alignmentspace[firstgroup] );
          line += ident;
          line += l;
          newline = true;
#ifdef DEBUG_FORMAT_BREAK
          INFO_PRINTLN( "!new {} - {}", line, alignmentspace );
#endif
        }
        else
        {
          line += l;
#ifdef DEBUG_FORMAT_BREAK
          INFO_PRINTLN( "new {} - {}", line, alignmentspace );
#endif
        }
        if ( !alignmentspace.count( group ) )
          alignmentspace[group] = line.size() - ident.size();
      }
      else if ( lastgroup > firstgroup )  // descending
      {
        newline = true;
        stripline( line );
        if ( !line.empty() )
          _lines.emplace_back( std::move( line ) );
        line = alignmentSpacing( alignmentspace[firstgroup] );
        line += ident;
        line += l;
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "desc {} - {}", line, alignmentspace );
#endif
      }
      else  // same group
      {
        // only if we already had a different group split the line
        if ( groupdiffered )
        {
          newline = false;
          stripline( line );
          if ( !line.empty() )
            _lines.emplace_back( std::move( line ) );
          line = alignmentSpacing( alignmentspace[firstgroup] );
          line += ident;
        }
        line += l;
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "same {} - {}", line, alignmentspace );
#endif
        if ( line.size() > compilercfg.FormatterLineWidth )
        {
          stripline( line );
          if ( !line.empty() )
            _lines.emplace_back( std::move( line ) );
          line = alignmentSpacing( alignmentspace[firstgroup] );
          line += ident;
        }
      }

      lastgroup = firstgroup;
    }
  }
  else  // split based on parts
  {
    // with preferred breaks
    if ( has_preferred || has_preferred_logical )
    {
      // first join parts until a forced/preferred break
      size_t alignmentspace = 0;
      std::vector<std::pair<std::string, int>> parts;
      std::string tmp;
      // if breaks exists from logical "and/or" we only split based on them
      // in a long if-statement we want to break line on the logical points and not mixed with
      // commas from functions
      int breakflag =
          has_preferred_logical ? FmtToken::PREFERRED_BREAK_LOGICAL : FmtToken::PREFERRED_BREAK;
      for ( auto& [l, group, firstgroup, style] : lines )
      {
        if ( !alignmentspace )
        {
          auto ident = identSpacing();
          alignmentspace = l.size() + ident.size();
          line += ident;
          parts.push_back( { l, FmtToken::NONE } );
          continue;
        }
        tmp += l;
        if ( ( style & FmtToken::FORCED_BREAK ) || ( style & breakflag ) )
        {
          parts.push_back( { std::move( tmp ), style } );
          tmp.clear();
        }
      }
      if ( !tmp.empty() )
        parts.push_back( { tmp, FmtToken::NONE } );
      // now build the actual line(s)
      bool alignpart = false;
      for ( auto& [l, style] : parts )
      {
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "'{}'{} -{} FILTERED", l, l.size(), style );
#endif
        if ( line.empty() && alignmentspace && alignpart )
          line = alignmentSpacing( alignmentspace );
        alignpart = true;  // otherwise first part would get spacing
        // with a margin of 50% start a new line before
        if ( ( line.size() + ( l.size() / 2 ) ) > compilercfg.FormatterLineWidth )
        {
          stripline( line );
          _lines.emplace_back( std::move( line ) );
          line = alignmentSpacing( alignmentspace );
        }
        line += l;
        // linewidth reached add current line, start a new one
        if ( line.size() > compilercfg.FormatterLineWidth || style & FmtToken::FORCED_BREAK )
        {
          stripline( line );
          _lines.emplace_back( std::move( line ) );
          line.clear();
        }
      }
    }
    else  // simple splitting
    {
      // build final lines
      size_t alignmentspace = 0;
      for ( auto& [l, group, firstgroup, style] : lines )
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
        if ( line.size() > compilercfg.FormatterLineWidth || style & FmtToken::FORCED_BREAK )
        {
          stripline( line );
          _lines.emplace_back( std::move( line ) );
          line.clear();
        }
      }
    }
  }
  if ( !line.empty() )
  {
    stripline( line );
    _lines.emplace_back( std::move( line ) );
  }
  _last_line = _line_parts.back().pos.line_number;
  _line_parts.clear();
}


void PrettifyLineBuilder::addEmptyLines( size_t line_number )
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
  auto style = FmtToken::SPACE | FmtToken::BREAKPOINT;
  if ( _line_parts.size() == begin_size )
  {
    if ( !compilercfg.FormatterEmptyParenthesisSpacing )
      style |= FmtToken::ATTACHED;
    else if ( !_line_parts.empty() )
      _line_parts.back().style |= FmtToken::SPACE;
  }
  else if ( !compilercfg.FormatterParenthesisSpacing )
    style |= FmtToken::ATTACHED;
  return style;
}

int PrettifyLineBuilder::closingBracketStyle( size_t begin_size )
{
  auto style = FmtToken::SPACE | FmtToken::BREAKPOINT;
  if ( _line_parts.size() == begin_size )
  {
    if ( !compilercfg.FormatterEmptyBracketSpacing )
      style |= FmtToken::ATTACHED;
    else if ( !_line_parts.empty() )
      _line_parts.back().style |= FmtToken::SPACE;
  }
  else if ( !compilercfg.FormatterBracketSpacing )
    style |= FmtToken::ATTACHED;
  return style;
}

int PrettifyLineBuilder::openingParenthesisStyle()
{
  return FmtToken::ATTACHED | FmtToken::BREAKPOINT |
         ( compilercfg.FormatterParenthesisSpacing ? FmtToken::SPACE : FmtToken::NONE );
}

int PrettifyLineBuilder::openingBracketStyle()
{
  return FmtToken::ATTACHED | FmtToken::BREAKPOINT |
         ( compilercfg.FormatterBracketSpacing ? FmtToken::SPACE : FmtToken::NONE );
}

int PrettifyLineBuilder::delimiterStyle()
{
  return FmtToken::ATTACHED | FmtToken::BREAKPOINT |
         ( compilercfg.FormatterDelimiterSpacing ? FmtToken::SPACE : FmtToken::NONE );
}

int PrettifyLineBuilder::terminatorStyle()
{
  return FmtToken::SPACE | FmtToken::ATTACHED | FmtToken::BREAKPOINT;
}

int PrettifyLineBuilder::assignmentStyle()
{
  if ( !compilercfg.FormatterAssignmentSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
}

int PrettifyLineBuilder::comparisonStyle()
{
  if ( !compilercfg.FormatterComparisonSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
}

int PrettifyLineBuilder::operatorStyle()
{
  if ( !compilercfg.FormatterOperatorSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
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

void PrettifyLineBuilder::mergeEOFNonTokens()
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
  // purge raw content
  mergeRawContent( std::numeric_limits<size_t>::max() );
  // if the original file ends with a newline, make sure to also end
  if ( !_lines.empty() && !_lines.back().empty() && !_rawlines.empty() && _rawlines.back().empty() )
    _lines.push_back( "" );
}

}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::FmtToken>::format(
    const Pol::Bscript::Compiler::FmtToken& t, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format(
      fmt::format( "{} ({}:{}:{})", t.text, t.style, t.pos.line_number, t.pos.token_index ), ctx );
}
