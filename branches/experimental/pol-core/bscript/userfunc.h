/*
History
=======

Notes
=======

*/

#ifndef BSCRIPT_USERFUNC_H
#define BSCRIPT_USERFUNC_H

#include "options.h"

#include <string>
#include <vector>
#include <set>

#include "compctx.h"
#include "token.h"

class UserParam
{
public:
    string name;
	int have_default;
	Token dflt_value;
    bool pass_by_reference;

    UserParam() : name(""),have_default(false),pass_by_reference(false) {}
};

class UserFunction
{
public:
    UserFunction();
    ~UserFunction();

    string name;
    typedef vector<UserParam> Parameters;
    Parameters parameters;
	unsigned position;
    typedef vector<unsigned> Addresses;
    Addresses forward_callers;

    CompilerContext ctx;
    char* function_body; // FIXME: this is leaked
    bool exported;
    bool emitted;
    string declaration;

public:
    static unsigned int instances();
    static void show_instances();
protected:
    static unsigned int _instances;
    static set<UserFunction*> _instancelist;
    void register_instance();
    void unregister_instance();
};

#endif
