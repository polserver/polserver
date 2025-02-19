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
        _packableline_allowed = false;
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
      _lines.push_back( indentSpacing() + _comments.front().text );
      if ( _packableline_allowed &&
           _packablelinestart ==
               _lines.size() - 1 )  // potential packed line not yet build, so safe to add a comment
        ++_packablelinestart;
      else if ( _comments.front().style & FmtToken::FORCED_BREAK )
        _packableline_allowed = false;

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
      _packableline_allowed = false;
      auto info = _comments.front();
      info.group = i ? _line_parts[i - 1].group : _currentgroup;
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
      // no disable of packableline since its at the end
      addPart( _comments.front() );
      _comments.erase( _comments.begin() );
    }
  }
}

std::vector<FmtToken> PrettifyLineBuilder::buildLineSplits()
{
  std::vector<FmtToken> lines;
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
  FmtToken part;
  for ( size_t i = 0; i < _line_parts.size(); ++i )
  {
#ifdef DEBUG_FORMAT_BREAK
    INFO_PRINTLN( "\"{}\" {} {}", _line_parts[i].text, _line_parts[i].group, _line_parts[i].style );
#endif
    if ( part.text.empty() )
    {
      part.firstgroup = _line_parts[i].group;
      part.pos = _line_parts[i].pos;
      part.scope = _line_parts[i].scope;
      part.context = _line_parts[i].context;
    }
    part.text += _line_parts[i].text;
    if ( part.context == FmtContext::NONE )
      part.context = _line_parts[i].context;
    // add space if set, but not if the following part is attached
    if ( _line_parts[i].style & FmtToken::SPACE )
    {
      if ( i + 1 < _line_parts.size() )
      {
        if ( !( _line_parts[i + 1].style & FmtToken::ATTACHED ) )
          part.text += ' ';
      }
      else
        part.text += ' ';
      // if something is left and not attached
      if ( i + 1 < _line_parts.size() )
      {
        if ( !( _line_parts[i + 1].style & FmtToken::ATTACHED ) )
        {
          // if its already quite long split it
          if ( part.text.size() > compilercfg.FormatterLineWidth * 0.5 )
          {
            part.style = _line_parts[i].style;
            part.group = _line_parts[i].group;
            part.pos_end = _line_parts[i].pos_end;
            lines.emplace_back( std::move( part ) );
            part = FmtToken{};
            continue;
          }
        }
      }
    }
    // if a forced break does not appear at the end disallow merging the lines
    if ( _line_parts[i].style & FmtToken::FORCED_BREAK && i < _line_parts.size() - 1 )
      _packableline_allowed = false;
    if ( _line_parts[i].style & FmtToken::FORCED_BREAK ||
         ( has_varcomma && ( _line_parts[i].context == FmtContext::VAR_STATEMENT ||
                             _line_parts[i].context == FmtContext::VAR_COMMA ) ) )
    {
      // need to break directly after "var" to align multiple variables
      if ( _line_parts[i].context == FmtContext::VAR_STATEMENT ||
           _line_parts[i].context == FmtContext::VAR_COMMA )
        _line_parts[i].style |= FmtToken::PREFERRED_BREAK_VAR;
      part.style = _line_parts[i].style;
      part.group = _line_parts[i].group;
      part.pos_end = _line_parts[i].pos_end;
      lines.emplace_back( std::move( part ) );
      part = FmtToken{};
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
      part.style = _line_parts[i].style;
      part.group = _line_parts[i].group;
      part.pos_end = _line_parts[i].pos_end;
      lines.emplace_back( std::move( part ) );
      part = FmtToken{};
    }
  }
  // add remainings
  if ( !part.text.empty() )
  {
    part.style = _line_parts.back().style;
    part.group = _line_parts.back().group;
    part.pos_end = _line_parts.back().pos_end;
    lines.emplace_back( std::move( part ) );
  }

  return lines;
}

bool PrettifyLineBuilder::binPack( const FmtToken& part, std::string line, size_t index,
                                   size_t upto, const std::vector<FmtToken>& lines,
                                   bool only_single_line, std::vector<std::string>* finallines,
                                   std::map<size_t, size_t>* alignmentspace, size_t* skipindex,
                                   const std::map<size_t, size_t>& initial_alignmentspace,
                                   std::string& newpart ) const
{
  auto packTaktic =
      []( size_t index, size_t end_group, size_t max_len, const std::vector<FmtToken>& lines )
  {
    // fill up lines with given max_len
    std::string currline;
    std::vector<std::string> lineparts;
    for ( size_t j = index; j < end_group; ++j )
    {
      const auto& lpart = lines[j].text;
      if ( lines[j].style & FmtToken::FORCED_BREAK )
      {
        currline += lpart;
        lineparts.push_back( std::move( currline ) );
        currline.clear();
        continue;
      }
      // new part still fits, add to line
      if ( currline.size() + lpart.size() <= max_len )
      {
        currline += lpart;
        continue;
      }
      // we are already at end, start a new line
      if ( currline.size() >= max_len )
      {
        lineparts.push_back( std::move( currline ) );
        currline = lpart;
        continue;
      }
      // we were smaller, add it anyway and start a new line
      // the line will be bigger then the available room
      // but I think looks better
      currline += lpart;
      lineparts.push_back( std::move( currline ) );
      currline.clear();
    }
    if ( !currline.empty() )
      lineparts.push_back( currline );
    return lineparts;
  };
  auto indent = indentSpacing();
  size_t room = compilercfg.FormatterLineWidth - line.size();
  size_t end_group = 0;
  bool prefferesbreak = false;
  // where does the group end?
  for ( size_t j = index + 1; j <= upto; ++j )
  {
    if ( lines[j].style & FmtToken::PREFERRED_BREAK_VAR )
    {
      prefferesbreak = true;
    }
    if ( lines[j].group == part.group || lines[j].firstgroup == part.group )
    {
      continue;
    }
    end_group = j;
    break;
  }
  if ( end_group == 0 )
    end_group = upto + 1;
  *skipindex = end_group;  // independent of the result we will skip till end of group
  std::string total;
  bool total_valid{ true };
  for ( size_t j = index; j < end_group; ++j )
  {
    total += lines[j].text;
    if ( lines[j].style & FmtToken::FORCED_BREAK )
    {
      total_valid = false;
      break;
    }
  }
  // it fits directly into the line
  if ( total_valid && total.size() + line.size() < compilercfg.FormatterLineWidth )
  {
    if ( !alignmentspace->count( part.group ) )
      ( *alignmentspace )[part.group] = line.size() - indent.size();
    line += total;
    newpart = total;
    stripline( line );
    finallines->emplace_back( std::move( line ) );
    line.clear();
    return true;
  }
  // if its part of an active groupformatting only allow a single line
  if ( only_single_line )
    return false;
  // TODO: if only a few chars room are left start a newline before
  //  option 1: try to equally split the line into two, for structs and dicts half the available
  //  room
  size_t lenOption2 = std::min( room, total.size() / 2 );
  if ( ( part.scope & FmtToken::Scope::STRUCT ) == FmtToken::Scope::STRUCT ||
       ( part.scope & FmtToken::Scope::DICT ) == FmtToken::Scope::DICT )
  {
    lenOption2 = room / 2;
  }

  auto lineparts = packTaktic( index, end_group, lenOption2, lines );

  // option1 failed, for arrays its allowed to use 3 lines
  // no array/dict/struct scope means any length is allowed
  if ( lineparts.size() != 2 && part.scope != FmtToken::Scope::NONE &&
       part.scope != FmtToken::Scope::FUNCTION && part.scope != FmtToken::Scope::VAR )
  {
    if ( ( part.scope & FmtToken::Scope::STRUCT ) == FmtToken::Scope::STRUCT ||
         ( part.scope & FmtToken::Scope::DICT ) == FmtToken::Scope::DICT )
    {
      return false;
    }
    size_t lenOption3 = std::min( room, total.size() / 3 );
    lineparts = packTaktic( index, end_group, lenOption3, lines );
    if ( lineparts.size() != 3 )
    {
      return false;
    }
  }
  // dont keep spacing over var,
  if ( prefferesbreak )
    *alignmentspace = initial_alignmentspace;
  // we found a solution
  if ( !alignmentspace->count( part.group ) )
    ( *alignmentspace )[part.group] = line.size() - indent.size();
  newpart = "";
  for ( const auto& lpart : lineparts )
  {
    if ( line.empty() )
    {
      line = alignmentSpacing( ( *alignmentspace )[part.firstgroup] );
      line += indent;
    }
    line += lpart;
    newpart += lpart;
    stripline( line );
    finallines->emplace_back( std::move( line ) );
    line.clear();
  }
  return true;
}

std::vector<std::string> PrettifyLineBuilder::createBasedOnGroups(
    std::vector<FmtToken>& lines ) const
{
  // TODO align closing brackets better
  std::vector<std::string> finallines;
  std::string line;
  // store for each group the alignment
  std::map<size_t, size_t> alignmentspace = {};
  std::map<size_t, size_t> initial_alignmentspace = {};
  size_t lastgroup = 0xffffFFFF;
  bool newline = false;
  auto indent = indentSpacing();
  line = indent;
  bool groupdiffered = false;
  size_t i = 0, skipuntil = 0, tried_binpack_until = 0;
  for ( auto& part : lines )
  {
    if ( skipuntil > i )  // already handled during binpack
    {
      ++i;
      continue;
    }
    // it was a binpack before
    if ( line.empty() && !finallines.empty() )
    {
      // add a following comment to the line if it fits
      if ( part.context == FmtContext::LINE_COMMENT || part.context == FmtContext::COMMENT )
      {
        if ( part.text.size() + finallines.back().size() < compilercfg.FormatterLineWidth )
        {
          // if the last one was forced (comment) let the default behaviour later do it
          if ( !( lines[i - 1].style & FmtToken::FORCED_BREAK ) )
          {
            if ( lines[i - 1].style & FmtToken::SPACE )
              finallines.back() += ' ';  // needed since already stripped
            finallines.back() += part.text;
            stripline( finallines.back() );
            line = indent;
            ++i;
            continue;
          }
        }
      }
      // last element still fits
      if ( i + 1 == lines.size() &&
           ( part.text.size() + finallines.back().size() <= compilercfg.FormatterLineWidth ) )
      {
        if ( !( lines[i - 1].style & FmtToken::FORCED_BREAK ) )
        {
          if ( lines[i - 1].style & FmtToken::SPACE )
            finallines.back() += ' ';  // needed since already stripped
          finallines.back() += part.text;
          stripline( finallines.back() );
          ++i;
          continue;
        }
      }
    }
    if ( line.empty() && !alignmentspace.empty() )
    {
      if ( alignmentspace.find( part.firstgroup ) == alignmentspace.end() )
        alignmentspace[part.firstgroup] = alignmentspace[part.firstgroup - 1];
      line = alignmentSpacing( alignmentspace[part.firstgroup] );
      line += indent;
    }
    if ( part.style & FmtToken::FORCED_BREAK )
    {
      auto nonspace = line.find_first_not_of( " \t" );
      line += part.text;
      // do not indent comments if they start at the beginning
      if ( part.context == FmtContext::LINE_COMMENT || part.context == FmtContext::COMMENT )
      {
        if ( part.pos.character_column < 4 &&
             nonspace ==
                 std::string::npos )  // TODO startpos does not include original space indent
        {
          auto nonspace_part = part.text.find_first_not_of( " \t" );
          // is the comment really at the beginning
          if ( nonspace_part != std::string::npos && part.text[nonspace_part] == '/' )
            line = part.text;
        }
      }
      stripline( line );
      finallines.emplace_back( std::move( line ) );
      line.clear();
      line = alignmentSpacing( alignmentspace[part.firstgroup] );
      line += indent;
      ++i;
      // dont keep spacing over var,
      if ( part.style & FmtToken::PREFERRED_BREAK_VAR )
        alignmentspace = initial_alignmentspace;
      continue;
    }
    if ( lastgroup == 0xffffFFFF )
    {
      line += part.text;  // first part
      alignmentspace[part.firstgroup] = line.size() - indent.size();
      alignmentspace[part.group] = line.size() - indent.size();
      initial_alignmentspace = alignmentspace;
      newline = true;
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "first {} {} a{}", line, part.firstgroup, alignmentspace );
#endif
      ++i;
      lastgroup = part.firstgroup;
      // dont keep spacing over var,
      if ( part.style & FmtToken::PREFERRED_BREAK_VAR )
        alignmentspace = initial_alignmentspace;
      continue;
    }
    if ( tried_binpack_until <= i )
    {
      size_t upto = lines.size() - 1;
      // check for nested groups, binpack is not allowed for these eg array of struct
      for ( size_t j = i + 1; j < lines.size(); ++j )
      {
        if ( lines[j].group > part.group )
        {
          upto = 0;
          break;
        }
      }
      bool invarpack = false;
      if ( upto == 0 && lines[i ? i - 1 : 0].style & FmtToken::PREFERRED_BREAK_VAR )
      {
        // multiple variables in a single statement, find matching groups and pack these
        for ( size_t j = i + 1; j < lines.size(); ++j )
        {
          // we are not inside of some group and eg array starts
          if ( lastgroup == 0 && part.group == 1 )
          {
            if ( lines[j].group > 2 )  // entries are 2
              break;
          }
          else if ( lines[j].group > part.group )
          {
            break;
          }
          if ( lines[j].group != part.group )
            if ( lines[j].style & FmtToken::PREFERRED_BREAK_VAR )
              break;  // stop at var, when another group starts
          if ( lines[j].style & FmtToken::PREFERRED_BREAK_VAR )
            upto = j;
        }
        if ( upto > 0 )
        {
          invarpack = true;  // retry with newline
        }
      }
      if ( upto > 0 && i != upto )
      {
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "trybinpack {}->{} {}->{}", i, upto, lines[i].text, lines[upto].text );
#endif
        size_t skip;
        std::string newpart;
        if ( binPack( part, line, i, upto, lines, tried_binpack_until > 0, &finallines,
                      &alignmentspace, &skip, initial_alignmentspace, newpart ) )
        {
#ifdef DEBUG_FORMAT_BREAK
          INFO_PRINTLN( "binpack {}->{} {}->{}", i, upto, lines[i].text, lines[skip - 1].text );
#endif
          skipuntil = skip;
          line.clear();
          ++i;
          // dont keep spacing over var,
          if ( part.style & FmtToken::PREFERRED_BREAK_VAR )
            alignmentspace = initial_alignmentspace;
          continue;
        }
        else if ( invarpack && !line.empty() )
        {
          // try it again when starting a new line
          std::string oldline = line;
          stripline( line );
          if ( !line.empty() )
            finallines.emplace_back( std::move( line ) );
          line.clear();
          line = alignmentSpacing( alignmentspace[part.firstgroup] );
          line += indent;
          std::string tmp;
          if ( binPack( part, line, i, upto, lines, tried_binpack_until > 0, &finallines,
                        &alignmentspace, &skip, initial_alignmentspace, tmp ) )
          {
#ifdef DEBUG_FORMAT_BREAK
            INFO_PRINTLN( "binpack2 {}->{} {}->{}", i, upto, lines[i].text, lines[skip - 1].text );
#endif
            skipuntil = skip;
            line.clear();
            ++i;
            // dont keep spacing over var,
            if ( part.style & FmtToken::PREFERRED_BREAK_VAR )
              alignmentspace = initial_alignmentspace;
            continue;
          }
          else
          {
            line = oldline;
            finallines.pop_back();
          }
        }
        tried_binpack_until = skip;
      }
    }
    if ( lines[i].context == FmtContext::PREFERRED_BREAK_START )
    {
      // try to pack function calls, by modifying the current part
      size_t upto = lines.size() - 1;
      for ( size_t j = i + 1; j < lines.size(); ++j )
      {
        if ( lines[j].context == FmtContext::PREFERRED_BREAK_END )
        {
          upto = j;
          break;
        }
      }
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "trybinpackfunc {}->{} {}->{}", i, upto, lines[i].text, lines[upto].text );
#endif
      size_t skip;
      std::string oldline = line;
      std::string newpart;
      if ( binPack( part, line, i, upto, lines, true, &finallines, &alignmentspace, &skip,
                    initial_alignmentspace, newpart ) )
      {
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "binpackfunc {}->{} {}->{}", i, upto, lines[i].text, lines[skip - 1].text );
#endif
        skipuntil = skip;
        // dont keep spacing over var,
        if ( part.style & FmtToken::PREFERRED_BREAK_VAR )
          alignmentspace = initial_alignmentspace;
        // use the packed text as current part
        part.text = newpart;
        finallines.pop_back();
        line = oldline;
        part.group = lines[skip].group;
      }
    }
    if ( lastgroup < part.firstgroup )  // new group
    {
      groupdiffered = true;
      bool newgroup = false;
      if ( !alignmentspace.count( part.firstgroup ) )
      {
        newgroup = true;
        alignmentspace[part.firstgroup] = line.size() - indent.size();
      }
      // if its not a new group and not a fresh line start a new one
      // nested structs eg should start at the same line
      if ( !newgroup && !newline )
      {
        stripline( line );
        if ( !line.empty() )
          finallines.emplace_back( std::move( line ) );
        line = alignmentSpacing( alignmentspace[part.firstgroup] );
        line += indent;
        line += part.text;
        newline = true;
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "!new {} - {}", line, alignmentspace );
#endif
      }
      else
      {
        line += part.text;
#ifdef DEBUG_FORMAT_BREAK
        INFO_PRINTLN( "new {} - {}", line, alignmentspace );
#endif
      }
      if ( !alignmentspace.count( part.group ) )
        alignmentspace[part.group] = line.size() - indent.size();
    }
    else if ( lastgroup > part.firstgroup )  // descending
    {
      newline = true;
      stripline( line );
      if ( !line.empty() )
        finallines.emplace_back( std::move( line ) );
      line = alignmentSpacing( alignmentspace[part.firstgroup] );
      line += indent;
      line += part.text;
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "desc {} - {}", line, alignmentspace );
#endif
    }
    else  // same group
    {
      // only if we already had a different group split the line
      bool closing = false;
      if ( !part.text.empty() )
        closing = part.text[0] == ')';  // shouldnt force a new line
      if ( groupdiffered && !closing )
      {
        newline = false;
        stripline( line );
        if ( !line.empty() )
          finallines.emplace_back( std::move( line ) );
        line = alignmentSpacing( alignmentspace[part.firstgroup] );
        line += indent;
      }
      line += part.text;
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "same {} - {} {} {}", line, alignmentspace, groupdiffered, part.text );
#endif
      if ( line.size() > compilercfg.FormatterLineWidth )
      {
        stripline( line );
        if ( !line.empty() )
          finallines.emplace_back( std::move( line ) );
        line = alignmentSpacing( alignmentspace[part.firstgroup] );
        line += indent;
        // dont keep spacing over var,
        if ( part.style & FmtToken::PREFERRED_BREAK_VAR )
          alignmentspace = initial_alignmentspace;
      }
    }

    lastgroup = part.firstgroup;
    ++i;
  }
  if ( !line.empty() )
  {
#ifdef DEBUG_FORMAT_BREAK
    INFO_PRINTLN( "remaining {}", line );
#endif
    // look for something like ); and add it to the last line
    if ( auto nonspace = line.find_first_not_of( " \t" );
         nonspace != std::string::npos && line.size() - nonspace < 4 )
    {
      if ( lines.size() >= 2 )
      {
        if ( !( lines[lines.size() - 2].style & FmtToken::FORCED_BREAK ) )
        {
          if ( lines[lines.size() - 2].style & FmtToken::SPACE )
            finallines.back() += ' ';  // needed since already stripped
          finallines.back() += line.erase( 0, nonspace );
          stripline( finallines.back() );
          return finallines;
        }
      }
    }
    stripline( line );
    finallines.emplace_back( std::move( line ) );
  }
  return finallines;
}

// helper to find the alignment of the last open parenthesis and use this as alignment for the
// next line
bool PrettifyLineBuilder::parenthesisAlign( const std::vector<std::string>& finallines,
                                            std::string& line ) const
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
      if ( c == '\t' )
        i += compilercfg.FormatterTabWidth;
      else
        ++i;
    }
  }
  if ( parenthesisalign.empty() )
    return false;

  // if its at the end of last line start at the beginning
  if ( parenthesisalign.back() >= ( finallines.back().size() - 1 ) )
    return true;
  size_t alignment = line.find_first_not_of( " \t" );
  if ( alignment == std::string::npos )
    alignment = line.size();
  if ( compilercfg.FormatterUseTabs )
    alignment *= compilercfg.FormatterTabWidth;
  if ( parenthesisalign.back() + ( compilercfg.FormatterBracketSpacing ? 2 : 1 ) > alignment )
  {
    auto newspace = alignmentSpacing( parenthesisalign.back() +
                                      ( compilercfg.FormatterBracketSpacing ? 2 : 1 ) );
    auto space = line.find_first_not_of( " \t" );
    line.erase( 0, space );
    line = newspace + line;
  }
  // if its a operator +-*/<> align the actual "data" so subtract 2
  auto space = line.find_first_not_of( " \t" );
  if ( space != std::string::npos && space > 1 && space + 1 < line.size() &&
       line[space + 1] == ' ' &&
       ( line[space] == '+' || line[space] == '-' || line[space] == '*' || line[space] == '/' ||
         line[space] == '<' || line[space] == '>' ) )
  {
    // TODO tabs...
    if ( !compilercfg.FormatterUseTabs )
      line.erase( 0, 2 );
  }
  return true;
}

std::vector<std::string> PrettifyLineBuilder::createBasedOnPreferredBreaks(
    const std::vector<FmtToken>& lines, bool logical ) const
{
  std::vector<std::string> finallines;
  std::string line;
  // first join parts until a forced/preferred break
  size_t alignmentspace = 0;
  size_t assignpos = std::string::npos;
  std::vector<std::tuple<std::string, int, FmtContext>> parts;
  std::string tmp;
  // if breaks exists from logical "and/or" we only split based on them
  // in a long if-statement we want to break line on the logical points and not mixed with
  // commas from functions
  int breakflag = logical ? FmtToken::PREFERRED_BREAK_LOGICAL : FmtToken::PREFERRED_BREAK;
  std::vector<size_t> parenthesisalign;
  FmtContext tmpcontext = FmtContext::NONE;
  for ( const auto& part : lines )
  {
    if ( !alignmentspace )
    {
      auto indent = indentSpacing();
      alignmentspace = part.text.size() + indent.size();
      line += indent;
      parts.push_back( { part.text, FmtToken::NONE, part.context } );
      assignpos = part.text.find( ":=" );
      if ( assignpos != std::string::npos )
        assignpos += indent.size() + 2;
      continue;
    }
    // if it gets to long we still have to split the line
    if ( tmp.size() + part.text.size() > compilercfg.FormatterLineWidth )
    {
      parts.push_back( { std::move( tmp ), FmtToken::NONE, tmpcontext } );
      tmp.clear();
      tmpcontext = part.context;
    }
    size_t i = 0;
    for ( auto c : tmp )
    {
      if ( c == '(' )
        parenthesisalign.push_back( i );
      else if ( c == ')' && !parenthesisalign.empty() )
        parenthesisalign.pop_back();
      ++i;
    }
    // something closed split it
    if ( parenthesisalign.empty() )
    {
      parts.push_back( { std::move( tmp ), FmtToken::NONE, tmpcontext } );
      tmp.clear();
      tmpcontext = part.context;
    }
    tmp += part.text;
    if ( tmpcontext == FmtContext::NONE )
      tmpcontext = part.context;
    else if ( tmpcontext == FmtContext::PREFERRED_BREAK_START &&
              part.context == FmtContext::PREFERRED_BREAK_END )
      tmpcontext = FmtContext::NONE;  // start and stop so clear

    if ( ( part.style & FmtToken::FORCED_BREAK ) ||
         ( part.style & FmtToken::PREFERRED_BREAK_VAR ) || ( part.style & breakflag ) )
    {
      parts.push_back( { std::move( tmp ), part.style, tmpcontext } );
      tmp.clear();
      if ( part.context != FmtContext::PREFERRED_BREAK_END )
        tmpcontext = part.context;
      else
        tmpcontext = FmtContext::NONE;
    }
  }
  if ( !tmp.empty() )
    parts.push_back( { tmp, FmtToken::NONE, tmpcontext } );
  // now build the actual line(s)
  bool alignpart = false;
  size_t parti = 0;
  for ( auto& [l, style, context] : parts )
  {
#ifdef DEBUG_FORMAT_BREAK
    INFO_PRINTLN( "'{}'{} -{} {} FILTERED", l, l.size(), style, (int)context );
#endif
    if ( line.empty() && alignmentspace && alignpart )
      line = alignmentSpacing( alignmentspace );

    if ( !logical )  // align line directly to have the correct linelen
      parenthesisAlign( finallines, line );

    alignpart = true;  // otherwise first part would get spacing
    bool newcontext_longer = false;
    // if we start a new parenthesis group check if it fit completely
    auto currlinespace = line.find_first_not_of( " \t" );
    if ( context == FmtContext::PREFERRED_BREAK_START && currlinespace != std::string::npos )
    {
      size_t prefferedsize = l.size();
      for ( size_t j = parti + 1; j < parts.size(); ++j )
      {
        prefferedsize += std::get<0>( parts[j] ).size();
        if ( std::get<2>( parts[j] ) == FmtContext::PREFERRED_BREAK_END )
        {
          // does not fit into current line, start a new one
          if ( prefferedsize + line.size() > compilercfg.FormatterLineWidth )
            newcontext_longer = true;
          break;
        }
      }
    }
    // with a margin of 75% start a new line before
    if ( ( ( line.size() + ( l.size() * 0.75 ) ) > compilercfg.FormatterLineWidth ) ||
         newcontext_longer )
    {
      // TODO if next is linecomment dont split now, but split comment
      std::string origline = line;  // parenthesisAlign modifies
      auto space = line.find_first_not_of( " \t" );
      if ( space != std::string::npos )  // line contained only alignment
      {
        if ( !logical )
        {
          if ( !parenthesisAlign( finallines, line ) )
          {
            if ( assignpos != std::string::npos )
            {
              space = line.find_first_not_of( " \t" );
              if ( space > assignpos )  // TODO operator shift of 2
                line.erase( 0, space - assignpos );
            }
          }
        }
        // line shrinks due to align, check again
        if ( newcontext_longer ||
             ( line.size() + ( l.size() * 0.75 ) ) > compilercfg.FormatterLineWidth )
        {
          bool skip = false;
          if ( parti == parts.size() - 1 )  // last
          {
            if ( l.size() < 4 ||
                 line.size() + l.size() <= compilercfg.FormatterLineWidth )  // small
              skip = true;
            else if ( !l.empty() && l[0] == '/' )  // comment
              skip = true;
          }
          else if ( auto c = line.find_last_not_of( " \t" );
                    c != std::string::npos &&
                    ( line[c] == '(' || ( finallines.empty() &&
                                          line.size() < compilercfg.FormatterLineWidth * 0.3 ) ) )
          {
            // a bracket just started, dont directly start a newline
            // or no completed line (prevents eg empty "var" line)
            skip = true;
          }
          if ( !skip )
          {
            stripline( line );
            finallines.emplace_back( std::move( line ) );
            line = alignmentSpacing( alignmentspace );
          }
          else
            line = origline;
        }
        else
          line = origline;
      }
    }
    line += l;
    // linewidth reached add current line, start a new one
    if ( line.size() > compilercfg.FormatterLineWidth || style & FmtToken::FORCED_BREAK ||
         style & FmtToken::PREFERRED_BREAK_VAR )
    {
      // TODO if next is linecomment dont split now, but split comment
      std::string origline = line;  // parenthesisAlign modifies
      if ( !logical )
      {
        if ( !parenthesisAlign( finallines, line ) )
        {
          if ( assignpos != std::string::npos )
          {
            auto space = line.find_first_not_of( " \t" );
            if ( space > assignpos )  // TODO operator shift of 2
              line.erase( 0, space - assignpos );
          }
        }
      }
      bool startnew = true;
      if ( style & FmtToken::FORCED_BREAK || style & FmtToken::PREFERRED_BREAK_VAR )
        startnew = true;
      else if ( line.size() < compilercfg.FormatterLineWidth )
        startnew = false;
      else if ( parti + 1 == parts.size() - 1 && std::get<0>( parts[parti + 1] ).size() < 4 )
        startnew = false;  // next is last and small so add it
      else if ( parti + 1 == parts.size() - 1 && !std::get<0>( parts[parti + 1] ).empty() &&
                std::get<0>( parts[parti + 1] )[0] == '/' )
        startnew = false;  // next is last and comment so add it
      if ( startnew )
      {
        stripline( line );
        finallines.emplace_back( std::move( line ) );
        line.clear();
      }
      else
        line = origline;
    }
    ++parti;
  }
  if ( !line.empty() )
  {
    stripline( line );
    if ( !logical )
      parenthesisAlign( finallines, line );
    finallines.emplace_back( std::move( line ) );
  }
  return finallines;
}

std::vector<std::string> PrettifyLineBuilder::createSimple(
    const std::vector<FmtToken>& lines ) const
{
  std::vector<std::string> finallines;
  std::string line;
  size_t alignmentspace = 0;
  size_t i = 0;
  for ( const auto& part : lines )
  {
    // following lines need to be aligned
    if ( line.empty() && alignmentspace )
      line = alignmentSpacing( alignmentspace );
    // first breakpoint defines the alignment and add initial indent level
    if ( !alignmentspace )
    {
      auto indent = indentSpacing();
      alignmentspace = part.text.size() + indent.size();
      line += indent;
    }
    line += part.text;
    bool forcebreakVarToLong{ false };
    // if its a var definition, check if the next var would fit in the line,
    // otherwise start directly a new line
    if ( part.style & FmtToken::PREFERRED_BREAK_VAR && i > 0 )
    {
      std::string next_var;
      for ( size_t j = i + 1; j < lines.size(); ++j )
      {
        next_var += lines[j].text;
        if ( lines[j].style & FmtToken::PREFERRED_BREAK_VAR )
          break;
      }
      if ( ( line.size() + next_var.size() + alignmentSpacing( alignmentspace ).size() ) >
           compilercfg.FormatterLineWidth )
        forcebreakVarToLong = true;
    }
    // linewidth reached add current line, start a new one
    if ( line.size() > compilercfg.FormatterLineWidth || part.style & FmtToken::FORCED_BREAK ||
         forcebreakVarToLong )
    {
      // TODO if next is linecomment dont split now, but split comment
      // be extra generous if we only have a few chars left
      if ( ( part.style & FmtToken::FORCED_BREAK ) == 0 )
      {
        if ( i + 1 == lines.size() - 1 && lines[i + 1].text.size() < 4 )
        {
          ++i;
          continue;
        }
      }
      stripline( line );
      parenthesisAlign( finallines, line );
      finallines.emplace_back( std::move( line ) );
      line.clear();
    }
    ++i;
  }
  if ( !line.empty() )
  {
    stripline( line );
    parenthesisAlign( finallines, line );
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

void PrettifyLineBuilder::buildLine( size_t current_indent )
{
  _currindent = current_indent;
  if ( _line_parts.empty() )
  {
    packLines();
    return;
  }
  mergeComments();

  // fill lines with final strings splitted at breakpoints
  auto lines = buildLineSplits();
#ifdef DEBUG_FORMAT_BREAK
  INFO_PRINTLN( "BREAK" );
  for ( const auto& part : lines )
    INFO_PRINTLN( "\"{}\" {}-{} ->{} :{}-{}", part.text, part.group, part.firstgroup, part.style,
                  (int)part.scope, (int)part.context );
#endif
  // add newline from original sourcecode
  addEmptyLines( _line_parts.front().pos.line_number );

  // sum up linelength, are groups inside or preferred breaks
  bool groups = false;
  bool has_preferred = false;
  bool has_preferred_logical = false;
  size_t linelength = 0;
  for ( const auto& part : lines )
  {
    if ( part.group != 0 )
      groups = true;
    if ( part.style & FmtToken::PREFERRED_BREAK )
      has_preferred = true;
    if ( part.style & FmtToken::PREFERRED_BREAK_LOGICAL )
      has_preferred_logical = true;
    linelength += part.text.size();
  }

  std::vector<std::string> finallines;
  // split based on groups
  if ( groups && linelength > compilercfg.FormatterLineWidth )
  {
#ifdef DEBUG_FORMAT_BREAK
    INFO_PRINTLN( "split groupbased" );
#endif
    finallines = createBasedOnGroups( lines );
  }
  else  // split based on parts
  {
    // with preferred breaks
    if ( has_preferred || has_preferred_logical )
    {
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "split preferred" );
#endif
      finallines = createBasedOnPreferredBreaks( lines, has_preferred_logical );
    }
    else  // simple splitting
    {
#ifdef DEBUG_FORMAT_BREAK
      INFO_PRINTLN( "split simple" );
#endif
      finallines = createSimple( lines );
    }
  }
  alignComments( finallines );
  _lines.insert( _lines.end(), std::make_move_iterator( finallines.begin() ),
                 std::make_move_iterator( finallines.end() ) );
  _last_line = _line_parts.back().pos.line_number;
  _line_parts.clear();

  packLines();
}

void PrettifyLineBuilder::packLines()
{
  if ( !_packableline_allowed || !_packablelineend )
    return;
  _packableline_allowed = false;
  _packablelineend = false;

  // 3 lines max
  // for case its "label: blubb; break;
  // for funcrefs its @(){ blubb; };
  if ( _lines.size() - _packablelinestart >= 4 )
    return;
  std::string packedline;
  for ( size_t i = _packablelinestart; i < _lines.size(); ++i )
  {
    auto l = _lines[i];
    if ( l.find( "//" ) != std::string::npos )
    {
      // if the line comment is not at the end we cannot pack
      if ( i < _lines.size() - 1 )
        return;
    }
    stripline( l );
    if ( packedline.empty() )
      packedline += l;
    else
    {
      auto begin = _lines[i].find_first_not_of( " \t" );
      if ( begin > 0 && begin != std::string::npos )
        l.erase( 0, begin );
      packedline += " " + l;
    }
  }
  if ( packedline.empty() )
    return;

  if ( packedline.size() <= compilercfg.FormatterLineWidth )
  {
    _lines.resize( _packablelinestart );
    _lines.push_back( std::move( packedline ) );
  }
}

void PrettifyLineBuilder::alignComments( std::vector<std::string>& finallines )
{
  if ( !compilercfg.FormatterAlignTrailingComments )
    return;
  std::vector<size_t> commentstart;
  // collect comment columns
  for ( size_t i = 0; i < finallines.size(); ++i )
  {
    auto linecomment = finallines[i].find( "//" );
    if ( linecomment == std::string::npos )
      commentstart.push_back( 0 );
    else
    {
      // if the line only contains a comment dont align it
      auto other = finallines[i].find_first_not_of( " \t" );
      if ( other == linecomment )
        commentstart.push_back( 0 );
      else
        commentstart.push_back( linecomment );
    }
  }
  if ( commentstart.empty() )
    return;

  // add spaces at comment start foreach comment other then max
  auto max = *std::max_element( commentstart.begin(), commentstart.end() );
  if ( max == 0 )
    return;
  for ( size_t i = 0; i < finallines.size(); ++i )
  {
    if ( !commentstart[i] || max == commentstart[i] )
      continue;
    finallines[i].insert( commentstart[i], max - commentstart[i], ' ' );
  }
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

int PrettifyLineBuilder::closingParenthesisStyle( bool args )
{
  auto style = FmtToken::SPACE | FmtToken::BREAKPOINT;
  if ( !args )
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

int PrettifyLineBuilder::openingBracketStyle( bool typeinit, bool force_unattached )
{
  if ( ( typeinit && !compilercfg.FormatterBracketAttachToType ) || force_unattached )
    return FmtToken::BREAKPOINT |
           ( compilercfg.FormatterBracketSpacing ? FmtToken::SPACE : FmtToken::NONE );
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

int PrettifyLineBuilder::ellipsisStyle() const
{
  if ( !compilercfg.FormatterEllipsisSpacing )
    return FmtToken::ATTACHED | FmtToken::SPACE;
  return FmtToken::SPACE;
}

int PrettifyLineBuilder::operatorStyle() const
{
  if ( !compilercfg.FormatterOperatorSpacing )
    return FmtToken::ATTACHED;
  return FmtToken::SPACE;
}

std::string PrettifyLineBuilder::indentSpacing() const
{
  if ( !compilercfg.FormatterUseTabs )
    return std::string( _currindent * compilercfg.FormatterIndentLevel, ' ' );
  size_t total = _currindent * compilercfg.FormatterIndentLevel;
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
    _lines.push_back( indentSpacing() + _comments.front().text );
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

void PrettifyLineBuilder::markPackableLineStart()
{
  _packablelinestart = _lines.size();
  _packableline_allowed = true;
  _packablelineend = false;
}
void PrettifyLineBuilder::markPackableLineEnd()
{
  _packablelineend = true;
}

void PrettifyLineBuilder::markLastTokensAsSwitchLabel()
{
  if ( _line_parts.size() < 2 )
    return;
  _packable_switch_labels.push_back( _line_parts[_line_parts.size() - 2].text +
                                     _line_parts[_line_parts.size() - 1].text );
}
void PrettifyLineBuilder::alignSingleLineSwitchStatements( size_t start )
{
  if ( !compilercfg.FormatterAlignConsecutiveShortCaseStatements ||
       !compilercfg.FormatterAllowShortCaseLabelsOnASingleLine )
    return;
  size_t l_index = 0;
  std::vector<std::pair<size_t, size_t>> statementstart;
  std::optional<std::pair<size_t, size_t>> default_start;
  // collect label starts based on stored labels
  // default: is collected extra
  for ( size_t i = start; i < _lines.size(); ++i )
  {
    if ( _packable_switch_labels.size() <= l_index )
      break;
    const auto& label = _packable_switch_labels[l_index];
    auto labelend = _lines[i].find( label );
    if ( labelend == std::string::npos )
      continue;
    // there should be only whitespace before (could be in a comment)
    bool empty{ true };
    for ( size_t w = 0; w < labelend; ++w )
    {
      if ( _lines[i][w] != ' ' && _lines[i][w] != '\t' )
      {
        empty = false;
        break;
      }
    }
    if ( !empty )
      continue;

    ++l_index;  // ~valid match, so next one
    // nothing following no need to align or consider as max
    auto next = _lines[i].find_first_not_of( " \t", labelend + label.size() );
    if ( next == std::string::npos )
      continue;

    if ( label == "default:" )
      default_start = std::make_pair( i, labelend + label.size() );
    else
      statementstart.push_back( { i, labelend + label.size() } );
  }
  _packable_switch_labels.clear();
  if ( statementstart.empty() )
    return;

  auto max = std::max_element( statementstart.begin(), statementstart.end(),
                               []( auto& a, auto& b ) { return a.second < b.second; } )
                 ->second;
  if ( max == 0 )
    return;
  for ( auto& [line, pos] : statementstart )
  {
    if ( pos == max )
      continue;
    _lines[line].insert( pos, max - pos, ' ' );
  }
  // we have a default, but only align it if its not the "biggest"
  if ( default_start )
  {
    if ( max > default_start->second )
      _lines[default_start->first].insert( default_start->second, max - default_start->second,
                                           ' ' );
  }
}

}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::FmtToken>::format(
    const Pol::Bscript::Compiler::FmtToken& t, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format(
      fmt::format( "{} ({}:{}:{})", t.text, t.style, t.pos.line_number, t.pos.token_index ), ctx );
}
