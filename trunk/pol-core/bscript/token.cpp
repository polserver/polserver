/*
History
=======
2006/10/06 Shinigami: malloc.h -> stdlib.h

Notes
=======

*/

#include "clib/stl_inc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "clib/esignal.h"

#include "modules.h"
#include "tokens.h"
#include "token.h"


unsigned long Token::_instances = 0;

#if STORE_INSTANCELIST
set<Token*> Token::_instancelist;
#endif

unsigned long Token::instances()
{
    return _instances;
}
void Token::show_instances()
{
#if STORE_INSTANCELIST
    for( Instances::iterator itr = _instancelist.begin(), end = _instancelist.end(); itr != end; ++itr )
    {
        Token* tkn = (*itr);
        cout << tkn << ": " << (*tkn) << endl;
    }
#endif
}

void Token::register_instance()
{
    ++_instances;
#if STORE_INSTANCELIST
    _instancelist.insert( this );
#endif
}

void Token::unregister_instance()
{
#if defined(_DEBUG) && defined(_DBG_TRYING_TO_FIND_WIN32_SHUTDOWN_ASSERTION)
    if (exit_signalled) {
        cout << "TOK/unreginst: " << (token?token:"<unknown>") << endl;
        printOn(cout); cout << "----" << endl;
    }
#endif
    --_instances;
#if STORE_INSTANCELIST
    _instancelist.erase( this );
#endif
}

Token::Token() :
    id(TOK_TERM),
    type(TYP_TERMINATOR),
    module( Mod_Basic ),
    precedence(-1),
    deprecated(false),
    ownsStr(0),
    dval(0.0),
    lval(0),
    userfunc(NULL),
    dbg_filenum(0),
    dbg_linenum(0),
    token(NULL)
{
    register_instance();
}

Token::Token(const Token& tok) :
    id( tok.id ),
    type( tok.type ),
    module( tok.module ),
    precedence( tok.precedence ),
    deprecated( tok.deprecated ),
    ownsStr(0),
    dval(tok.dval),
    lval(tok.lval),
    userfunc(tok.userfunc),
    dbg_filenum(tok.dbg_filenum),
    dbg_linenum(tok.dbg_linenum),
    token(NULL)
{
    register_instance();
    if (tok.token) 
    {
        if (!tok.ownsStr)
            setStr(tok.token);
        else
            copyStr(tok.token);
    }
}

Token& Token::operator=( const Token& tok )
{
    module = tok.module;
    id = tok.id;
    type = tok.type;
    precedence = tok.precedence;
    deprecated = tok.deprecated;

    nulStr();
    ownsStr = 0;
    if (tok.token) 
    {
        if (!tok.ownsStr)
            setStr(tok.token);
        else
            copyStr(tok.token);
    }
    dval = tok.dval;
    lval = tok.lval;

    dbg_filenum = tok.dbg_filenum;
    dbg_linenum = tok.dbg_linenum;

    userfunc = tok.userfunc;
    
    return *this;
}

Token::Token( ModuleID i_module,
              BTokenId i_id,
              BTokenType i_type ) :
    id(i_id),
    type(i_type),
    module(static_cast<unsigned char>(i_module)),
    precedence(-1),
    deprecated(false),
    ownsStr(0),
    dval(0.0),
    lval(0),
    userfunc(NULL),
    dbg_filenum(0),
    dbg_linenum(0),
    token(NULL)
{
    register_instance();
}

Token::Token( BTokenId i_id,
              BTokenType i_type ) :
    id(i_id),
    type(i_type),
    module(Mod_Basic),
    precedence(-1),
    deprecated(false),
    ownsStr(0),
    dval(0.0),
    lval(0),
    userfunc(NULL),
    dbg_filenum(0),
    dbg_linenum(0),
    token(NULL)
{
    register_instance();
}

Token::Token( ModuleID i_module,
              BTokenId i_id,
              BTokenType i_type,
              UserFunction *i_userfunc) :
    id(i_id),
    type(i_type),
    module(static_cast<unsigned char>(i_module)),
    precedence(-1),
    deprecated(false),
    ownsStr(0),
    dval(0.0),
    lval(0),
    userfunc( i_userfunc ),
    dbg_filenum(0),
    dbg_linenum(0),
    token(NULL)
{
    register_instance();
}

void Token::nulStr()
{
    if (token && ownsStr) 
    {
        char *tmp = (char *) token;
        delete[] tmp;
    }
    token = NULL;
}

void Token::setStr(const char *s)
{
    nulStr();
    ownsStr = 0;
    token =s;
}

void Token::copyStr(const char *s)
{
    nulStr();
    ownsStr = 1;
    int len = strlen(s);
    char *tmp = new char[len+1];
    if (tmp) 
    {
        memcpy(tmp, s, len+1);
        token = tmp;
    }
    else
    {
        token = NULL;
    }
}

void Token::copyStr(const char *s, int len)
{
    nulStr();
    ownsStr =1;
    char *tmp = new char[len+1];
    if (tmp)
    {
        memcpy(tmp, s, len);
        tmp[len] = '\0';
        token = tmp;
    }
    else
    {
        token = NULL;
    }
}


Token::~Token()
{
    nulStr();
    unregister_instance();
}   
