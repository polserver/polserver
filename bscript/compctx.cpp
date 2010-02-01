/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "compctx.h"

CompilerContext::CompilerContext() :
    s(NULL),
    line(1),
    filename(""),
    s_begin(NULL),
    dbg_filenum(0)
{
}

CompilerContext::CompilerContext( const string& filename, int dbg_filenum, const char *s ) :
    s(s),
    line(1),
    filename(filename),
    s_begin(s),
    dbg_filenum(dbg_filenum)
{
}

CompilerContext::CompilerContext( const CompilerContext& ctx ) :
    s(ctx.s),
    line(ctx.line),
    filename( ctx.filename ),
    s_begin(ctx.s_begin),
    dbg_filenum(ctx.dbg_filenum)
{
}

CompilerContext& CompilerContext::operator =( const CompilerContext& rhs )
{
    filename = rhs.filename;
    s = rhs.s;
    line = rhs.line;
    s_begin = rhs.s_begin;
    dbg_filenum = rhs.dbg_filenum;

    return *this;
}
   
void CompilerContext::skipws()
{
    while ( isspace( s[0] ))
    {
        if (s[0] == '\n')
            ++line;
        s++;
    }
}

int eatToCommentEnd( CompilerContext& ctx );
int eatToEndOfLine( CompilerContext& ctx );

int CompilerContext::skipcomments()
{
    CompilerContext tctx(*this);
    for (;;)
	{
		while (tctx.s[0] && isspace(tctx.s[0])) 
        {
            // FIXME: if (tctx.s[0] == '\t')
            // FIXME:     contains_tabs = true;
            if (tctx.s[0] == '\n')
                ++tctx.line;
            tctx.s++; // wow neat, already take care of newlines
        }

		if (!tctx.s[0]) 
        {
	        (*this) = tctx;
			return 1;
		}

		// look for comments
		if (strncmp( tctx.s, "/*",2 )==0)		// got a start of comment
		{
			int res;
			tctx.s += 2;
			res = eatToCommentEnd( tctx );
			if (res) return res;
		}
		else if (strncmp( tctx.s, "//",2 )==0) // comment, one line only
		{
			int res;
			res = eatToEndOfLine( tctx );
			if (res) return res;
		}
		else
        {
			break;
        }
	}

    (*this) = tctx;

    return 0;
}

void CompilerContext::printOn( ostream& os ) const
{
    os << "File: " << filename << ", Line " << line << endl;
}

void CompilerContext::printOnShort( ostream& os ) const
{
    os << filename << ", Line " << line << endl;
}

int eatToEndOfLine( CompilerContext& ctx )
{
	const char *t = ctx.s;
	
	while ( *t && (*t != '\r') && (*t != '\n') )
		t++;

	ctx.s = t;
	return 0;
}

int eatToCommentEnd( CompilerContext& ctx )
{
    CompilerContext tmp(ctx);

    while (tmp.s[0])
	{
		if (strncmp( tmp.s, "*/", 2 )==0)
		{
			tmp.s += 2;
			ctx = tmp;
			return 0;
		}
		else if (strncmp( tmp.s, "/*", 2 ) == 0) // nested comment
		{
            tmp.s += 2;
			int res = eatToCommentEnd( tmp );
			if (res) return res;
		}
		else if (strncmp( tmp.s, "//", 2 ) == 0) // nested eol-comment
		{
			int res = eatToEndOfLine( tmp );
			if (res) return res;
		}
        if (tmp.s[0] == '\n')
            ++tmp.line;
		tmp.s++;
	}
	return -1;
}

