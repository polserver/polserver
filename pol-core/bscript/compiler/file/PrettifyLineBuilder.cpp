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

std::vector<PrettifyLineBuilder::LineSplit> PrettifyLineBuilder::buildLineSplits()
{
  std::vector<LineSplit> lines;
  std::string line;
  size_t firstgroup = 0;
  bool has_varcomma = false;  // only if a "var," exists split based on these, otherwise var
                              // statement would be splitted
  for ( size_t i = 0; i < _line_parts.size(); ++i )
  {
    if ( _line_parts[i].context == FmtContext::VAR_COMMA )
    {
      has_varcomma = true;
      break;
    }
  }
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
    if ( _line_parts[i].style & FmtToken::FORCED_BREAK ||
         ( has_varcomma && ( _line_parts[i].context == FmtContext::VAR_STATEMENT ||
                             _line_parts[i].context == FmtContext::VAR_COMMA ) ) )
    {
      if ( _line_parts[i].context == FmtContext::VAR_STATEMENT ||
           _line_parts[i].context == FmtContext::VAR_COMMA )
        _line_parts[i].style |= FmtToken::PREFERRED_BREAK_VAR;
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

  return lines;
}

std::vector<std::string> PrettifyLineBuilder::createBasedOnGroups(
    const std::vector<LineSplit>& lines ) const
{
  std::vector<std::string> finallines;
  std::string line;
  // store for each group the alignment
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
      finallines.emplace_back( std::move( line ) );
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
          finallines.emplace_back( std::move( line ) );
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
        finallines.emplace_back( std::move( line ) );
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
          finallines.emplace_back( std::move( line ) );
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
          finallines.emplace_back( std::move( line ) );
        line = alignmentSpacing( alignmentspace[firstgroup] );
        line += ident;
      }
    }

    lastgroup = firstgroup;
  }
  if ( !line.empty() )
  {
    stripline( line );
    finallines.emplace_back( std::move( line ) );
  }
  return finallines;
}

// helper to find the alignment of the last open parenthesis and use this as alignment for the next
// line
void PrettifyLineBuilder::parenthesisAlign( const std::vector<std::string>& finallines,
                                            size_t alignmentspace, std::string& line ) const
{
  std::vector<size_t> parenthesisalign;
  for ( const auto& finalline : finallines )
  {
    size_t i = 0;
    for ( auto c : finalline )
    {
      if ( c == '(' )
        parenthesisalign.push_back( i );
      else if ( c == ')' && !parenthesisalign.empty() )
        parenthesisalign.pop_back();
      ++i;
    }
  }
  if ( !parenthesisalign.empty() )
  {
    // if its at the end of last line start at the beginning
    if ( parenthesisalign.back() >= ( finallines.back().size() - 1 ) )
      return;
    line = alignmentSpacing( parenthesisalign.back() - alignmentspace +
                             ( compilercfg.FormatterBracketSpacing ? 2 : 1 ) ) +
           line;
  }
}

std::vector<std::string> PrettifyLineBuilder::createBasedOnPreferredBreaks(
    const std::vector<LineSplit>& lines, bool logical ) const
{
  std::vector<std::string> finallines;
  std::string line;
  // first join parts until a forced/preferred break
  size_t alignmentspace = 0;
  std::vector<std::pair<std::string, int>> parts;
  std::string tmp;
  // if breaks exists from logical "and/or" we only split based on them
  // in a long if-statement we want to break line on the logical points and not mixed with
  // commas from functions
  int breakflag = logical ? FmtToken::PREFERRED_BREAK_LOGICAL : FmtToken::PREFERRED_BREAK;
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

    if ( ( style & FmtToken::FORCED_BREAK ) || ( style & FmtToken::PREFERRED_BREAK_VAR ) ||
         ( style & breakflag ) )
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
      // TODO if next is linecomment dont split now, but split comment
      stripline( line );
      if ( !logical )  // TODO does not work
        parenthesisAlign( finallines, alignmentspace, line );
      finallines.emplace_back( std::move( line ) );
      line = alignmentSpacing( alignmentspace );
    }
    line += l;
    // linewidth reached add current line, start a new one
    if ( line.size() > compilercfg.FormatterLineWidth || style & FmtToken::FORCED_BREAK ||
         style & FmtToken::PREFERRED_BREAK_VAR )
    {
      // TODO if next is linecomment dont split now, but split comment
      stripline( line );
      if ( !logical )
        parenthesisAlign( finallines, alignmentspace, line );
      finallines.emplace_back( std::move( line ) );
      line.clear();
    }
  }
  if ( !line.empty() )
  {
    stripline( line );
    if ( !logical )
      parenthesisAlign( finallines, alignmentspace, line );
    finallines.emplace_back( std::move( line ) );
  }
  return finallines;
}

std::vector<std::string> PrettifyLineBuilder::createSimple(
    const std::vector<LineSplit>& lines ) const
{
  std::vector<std::string> finallines;
  std::string line;
  size_t alignmentspace = 0;
  size_t i = 0;
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
    bool forcebreakVarToLong{ false };
    // if its a var definition, check if the next var would fit in the line,
    // otherwise start directly a new line
    if ( style & FmtToken::PREFERRED_BREAK_VAR && i > 0 )
    {
      std::string next_var;
      for ( size_t j = i + 1; j < lines.size(); ++j )
      {
        next_var += std::get<0>( lines[j] );
        if ( std::get<3>( lines[j] ) & FmtToken::PREFERRED_BREAK_VAR )
          break;
      }
      if ( ( line.size() + next_var.size() + alignmentSpacing( alignmentspace ).size() ) >
           compilercfg.FormatterLineWidth )
        forcebreakVarToLong = true;
    }
    // linewidth reached add current line, start a new one
    if ( line.size() > compilercfg.FormatterLineWidth || style & FmtToken::FORCED_BREAK ||
         forcebreakVarToLong )
    {
      // TODO if next is linecomment dont split now, but split comment
      stripline( line );
      parenthesisAlign( finallines, alignmentspace, line );
      finallines.emplace_back( std::move( line ) );
      line.clear();
    }
    ++i;
  }
  if ( !line.empty() )
  {
    stripline( line );
    parenthesisAlign( finallines, alignmentspace, line );
    finallines.emplace_back( std::move( line ) );
  }
  return finallines;
}

void PrettifyLineBuilder::stripline( std::string& line ) const
{
  if ( line.empty() )
    return;
  auto lastchar = line.find_last_not_of( ' ' );
  line.erase( line.begin() + lastchar + 1, line.end() );
}

void PrettifyLineBuilder::buildLine( size_t current_ident )
{
  _currident = current_ident;
  if ( _line_parts.empty() )
    return;
  mergeComments();

  // fill lines with final strings splitted at breakpoints
  // <splitted string, groupid, firstgroup, style>
  auto lines = buildLineSplits();
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

  // split based on groups
  if ( groups && linelength > compilercfg.FormatterLineWidth )
  {
#ifdef DEBUG_FORMAT_BREAK
    INFO_PRINTLN( "split groupbased" );
#endif
    auto finallines = createBasedOnGroups( lines );
    _lines.insert( _lines.end(), std::make_move_iterator( finallines.begin() ),
                   std::make_move_iterator( finallines.end() ) );
  }
  else  // split based on parts
  {
    // with preferred breaks
    if ( has_preferred || has_preferred_logical )
    {
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "split preferred" );
#endif
      auto finallines = createBasedOnPreferredBreaks( lines, has_preferred_logical );
      _lines.insert( _lines.end(), std::make_move_iterator( finallines.begin() ),
                     std::make_move_iterator( finallines.end() ) );
    }
    else  // simple splitting
    {
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "split simple" );
#endif
      auto finallines = createSimple( lines );
      _lines.insert( _lines.end(), std::make_move_iterator( finallines.begin() ),
                     std::make_move_iterator( finallines.end() ) );
    }
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

int PrettifyLineBuilder::openingParenthesisStyle() const
{
  return FmtToken::ATTACHED | FmtToken::BREAKPOINT |
         ( compilercfg.FormatterParenthesisSpacing ? FmtToken::SPACE : FmtToken::NONE );
}

int PrettifyLineBuilder::openingBracketStyle() const
{
  return FmtToken::ATTACHED | FmtToken::BREAKPOINT |
         ( compilercfg.FormatterBracketSpacing ? FmtToken::SPACE : FmtToken::NONE );
}

int PrettifyLineBuilder::delimiterStyle() const
{
  return FmtToken::ATTACHED | FmtToken::BREAKPOINT |
         ( compilercfg.FormatterDelimiterSpacing ? FmtToken::SPACE : FmtToken::NONE );
}

int PrettifyLineBuilder::terminatorStyle() const
{
  return FmtToken::SPACE | FmtToken::ATTACHED | FmtToken::BREAKPOINT;
}

int PrettifyLineBuilder::assignmentStyle() const
{
  if ( !compilercfg.FormatterAssignmentSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
}

int PrettifyLineBuilder::comparisonStyle() const
{
  if ( !compilercfg.FormatterComparisonSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
}

int PrettifyLineBuilder::operatorStyle() const
{
  if ( !compilercfg.FormatterOperatorSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
}

std::string PrettifyLineBuilder::identSpacing() const
{
  if ( !compilercfg.FormatterUseTabs )
    return std::string( _currident * compilercfg.FormatterIdentLevel, ' ' );
  size_t total = _currident * compilercfg.FormatterIdentLevel;
  size_t tabs = total / compilercfg.FormatterTabWidth;
  size_t remaining = total % compilercfg.FormatterTabWidth;
  return std::string( tabs, '\t' ) + std::string( remaining, ' ' );
}

std::string PrettifyLineBuilder::alignmentSpacing( size_t count ) const
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
