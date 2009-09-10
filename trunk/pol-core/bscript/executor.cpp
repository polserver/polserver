/*
History
=======
2005/09/16 Shinigami: added scripts_thread_script* to support better debugging
2006/01/27 Shinigami: added missing TOK_BS* to Executor::GetInstrFunc
2006/06/10 Shinigami: getParamImp/2 - better Error Message added
2006/10/07 Shinigami: FreeBSD fix - changed __linux__ to __unix__
2007/07/07 Shinigami: added code to analyze memoryleaks in initForFnCall() (needs defined MEMORYLEAK)
2009/07/19 MuadDib: Executor::ins_member() Removed, due to no longer used since case optimization code added.
2009/09/05 Turley: Added struct .? and .- as shortcut for .exists() and .erase()

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/clib.h"
#include "clib/logfile.h"
#include "clib/mlog.h"
#include "clib/passert.h"
#include "clib/stlutil.h"
#include "clib/strutil.h"

#include "berror.h"
#include "config.h"
#include "execmodl.h"

#include "bstruct.h"
#include "dict.h"
#include "escriptv.h"
#include "impstr.h"
#include "modules.h"
#include "tokens.h"
#include "symcont.h"
#include "eprog.h"
#include "operator.h"
#include "token.h"
#include "executor.h"

std::set< Executor* > executor_instances;
void display_executor_instances()
{
    for( std::set<Executor*>::iterator itr = executor_instances.begin(); itr != executor_instances.end(); ++itr )
    {
        Executor* ex =(*itr);
		// Fix for crashes due to orphaned script instances.
		if ( !ex->empty_scriptname() )
			cout << ex->scriptname() << endl;
    }
}

extern long executor_count;
Executor::Executor( ostream& cerr ) :
        done(0),
        error_(false),
        halt_(false),
        run_ok_(false),
        debug_level(NONE),
        PC(0),
        Locals2( new BObjectRefVec ),
        nLines(0),
        current_module_function(NULL),
        prog_ok_( false ),
        viewmode_(false),
        debugging_(false),
        debug_state_(DEBUG_STATE_NONE),
        breakpoints_(),
        bp_skip_(~0),
        func_result_(NULL)
{
    ++executor_count;
    executor_instances.insert( this );
        
    if (!UninitObject::SharedInstance)
    {
        UninitObject::SharedInstance = new UninitObject;
        UninitObject::SharedInstanceOwner.set( UninitObject::SharedInstance );
    }

}

Executor::~Executor()
{
    --executor_count;
    executor_instances.erase( this );

    delete Locals2;
    Locals2 = NULL;

    while (!upperLocals2.empty())
    {
        delete upperLocals2.top();
        upperLocals2.pop();
    }

    execmodules.clear();
    delete_all( availmodules );
}

unsigned long Executor::sizeEstimate() const
{
    unsigned long size = 0;
    for( BObjectRefVec::const_iterator itr = Globals2.begin(); itr != Globals2.end(); ++itr )
    {
        size += (*itr).sizeEstimate();
    }
    for( BObjectRefVec::const_iterator itr = Locals2->begin(); itr != Locals2->end(); ++itr )
    {
        size += (*itr).sizeEstimate();
    }
    return size;
}

#include "filefmt.h"

bool Executor::AttachFunctionalityModules()
{
	for( unsigned midx = 0; midx < prog_->modules.size(); midx++ )
	{
		FunctionalityModule* fm = prog_->modules[ midx ];
 
            // if no function in the module is actually called, don't go searching for it.
        if (fm->functions.size() == 0)
        {
            execmodules.push_back( NULL );
            continue;
        }
		
        ExecutorModule* em = findModule( fm->modulename );
        execmodules.push_back( em );
		if (em == NULL)
		{
			cerr << "WARNING: " << scriptname() << ": Unable to find module " << fm->modulename << endl;
            return false;
		}

        if (!fm->have_indexes)
        {
			/*
				FIXE: Possible optimization: store these function indexes in the
				EScriptProgram object, since those are cached.  Then, we only
				have to find the module index.
			*/
            for( unsigned fidx = 0; fidx < fm->functions.size(); fidx++ )
		    {
			    ModuleFunction *func = fm->functions[ fidx ];
			    // FIXME: should check number of params, blah.
			    if (!func->name.empty())
                {
                    func->funcidx = em->functionIndex( func->name.c_str() );
			        if (func->funcidx == -1)
                    {
                        cerr << "Unable to find " << fm->modulename << "::" << func->name << endl;
                        return false;
                    }
                }
		    }
            fm->have_indexes = true;
        }
	}
    return true;
}

int Executor::getParams(unsigned howMany)
{
    if (howMany)
	{
        fparams.resize( howMany );
		for(int i = howMany - 1; i >= 0; --i) 
		{
                if (ValueStack.empty()) 
                {
                    Log( "Fatal error: Value Stack Empty! (%s,PC=%d)\n", prog_->name.c_str(), PC );
                    cerr << "Fatal error: Value Stack Empty!" << endl;
                    cerr << prog_->name << ", PC=" << PC << endl;
                    seterror(true);
                    return -1;
                }
			fparams[i] = ValueStack.top();
            ValueStack.pop();
        }
	}
    return 0;
}

int Executor::cleanParams()
{
    fparams.clear();
    return 0;
}

int Executor::makeString(unsigned param)
{
    BObject *obj = getParam(param);
    if (!obj) return -1;
    if (obj->isa( BObjectImp::OTString )) return 0;

    fparams[param].set( new BObject(new String(obj->impref())) );

    return 0;
}

const char *Executor::paramAsString(unsigned param)
{
    makeString(param);
    BObjectImp *objimp = fparams[param]->impptr();

    String *str = (String *) objimp;
    return str ? str->data() : "";
}

int Executor::makeDouble(unsigned param)
{
    BObject *obj = getParam(param);
    if (!obj) return -1;
    if (obj->isa( BObjectImp::OTDouble )) return 0;

    fparams[param].set( new BObject(new Double( static_cast<Double&>(obj->impref()) ) ) );
    
    return 0;
}

double Executor::paramAsDouble(unsigned param)
{
    makeDouble(param);
    BObjectImp *objimp = getParam(param)->impptr();

    Double *dbl = (Double *) objimp;
    return dbl ? dbl->value() : 0.0;
}

long Executor::paramAsLong( unsigned param )
{
    BObjectImp *objimp = getParam(param)->impptr();
    if (objimp->isa( BObjectImp::OTLong ))
    {
        return ((BLong *)objimp)->value();
    }
    else if (objimp->isa( BObjectImp::OTDouble ))
    {
        return static_cast<long>(((Double *)objimp)->value());
    }
    else
    {
        return 0;
    }
}
BObject *Executor::getParam(unsigned param)
{
    passert( param < fparams.size() );

    return fparams[param].get();
}

BObjectImp *Executor::getParamImp(unsigned param)
{
    passert( param < fparams.size() );

    return fparams[param].get()->impptr();
}

BObject* Executor::getParamObj( unsigned param )
{
    if (fparams.size() > param)
    {
        return fparams[param].get();
    }
    else
    {
        return NULL;
    }
}

BObjectImp* Executor::getParamImp(unsigned param, BObjectImp::BObjectType type)
{
	passert_r( param < fparams.size(), "Script Error in '" + scriptname() + ": Less Parameter than expected. "+
		"You should use *.em-files shipped with this Core and recompile ALL of your Scripts _now_! RTFM" );

    BObjectImp* imp = fparams[param].get()->impptr();
    
    passert( imp != NULL );

    if (imp->isa( type ))
    {
        return imp;
    }
    else
    {
        if (mlog.is_open())
        {
            mlog << "Script Error in '" << scriptname() << "' PC=" << PC << ": " << endl;
            if (current_module_function)
                mlog << "\tCall to function " << current_module_function->name << ":" << endl;
            else
                mlog << "\tCall to an object method." << endl;
            mlog << "\tParameter " << param << ": Expected datatype " << BObjectImp::typestr(type)
                 << ", got datatype " << BObjectImp::typestr(imp->type()) << endl;
        }
           
        return NULL;
    }
}

BObjectImp* Executor::getParamImp2(unsigned param, BObjectImp::BObjectType type)
{
	passert_r( param < fparams.size(), "Script Error in '" + scriptname() + ": Less Parameter than expected. "+
		"You should use *.em-files shipped with this Core and recompile ALL of your Scripts _now_! RTFM" );

    BObjectImp* imp = fparams[param].get()->impptr();
    
    passert( imp != NULL );

    if (imp->isa( type ))
    {
        return imp;
    }
    else
    {
        string report = "Invalid parameter type.  Expected param "
                                + decint(param)
                                + " as "
                                + BObjectImp::typestr(type)
                                + ", got "
                                + BObjectImp::typestr(imp->type());
        func_result_ = new BError( report );
        return NULL;
    }
}


const String* Executor::getStringParam( unsigned param )
{
    return explicit_cast<String*,BObjectImp*>(getParamImp( param, BObjectImp::OTString ));
}

const BLong* Executor::getLongParam( unsigned param )
{
    return explicit_cast<BLong*,BObjectImp*>(getParamImp( param, BObjectImp::OTLong ));
}

bool Executor::getStringParam( unsigned param, const String*& pstr )
{
    pstr = getStringParam( param );
    return (pstr != NULL);
}

bool Executor::getParam( unsigned param, long& value )
{
    BLong* plong = explicit_cast<BLong*,BObjectImp*>(getParamImp( param, BObjectImp::OTLong ));
    if (plong == NULL)
        return false;

    value = plong->value();
    return true;
}

void Executor::setFunctionResult( BObjectImp* imp )
{
    func_result_ = imp;
}

bool Executor::getParam( unsigned param, long& value, long maxval )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        value = plong->value();
        if (value >= 0 && value <= maxval)
        {
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(value) 
                + " out of expected range of [0.."
                + decint(maxval) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Executor::getParam( unsigned param, long& value, long minval, long maxval )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        value = plong->value();
        if (value >= minval && value <= maxval)
        {
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(value) 
                + " out of expected range of [" + decint(minval) +".."
                + decint(maxval) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Executor::getRealParam( unsigned param, double& value )
{
    BObjectImp* imp = getParamImp( param );
    if (imp->isa( BObjectImp::OTDouble ))
    {
        value = static_cast<Double*>(imp)->value();
        return true;
    }
    else if (imp->isa( BObjectImp::OTLong ))
    {
        value = static_cast<BLong*>(imp)->value();
        return true;
    }
    else
    {
        if (mlog.is_open())
            mlog << "Script Error in '" << scriptname() << "' PC=" << PC << ": " << endl
                 << "\tCall to function " << current_module_function->name << ":" << endl
                 << "\tParameter " << param << ": Expected Integer or Real"
                 << ", got datatype " << BObjectImp::typestr(imp->type()) << endl;
           
        return false;
    }
}

bool Executor::getObjArrayParam( unsigned param, ObjArray*& pobjarr )
{
    pobjarr = explicit_cast<ObjArray*,BObjectImp*>(getParamImp( param, BObjectImp::OTArray ));
    return (pobjarr != NULL);
}

void *Executor::getApplicPtrParam(unsigned param, const BApplicObjType* pointer_type)
{   
    BApplicPtr* ap = EXPLICIT_CAST(BApplicPtr*, BObjectImp*)(getParamImp(param,BObjectImp::OTApplicPtr));
    if (ap == NULL)
        return NULL;

    if ( ap->pointer_type() == pointer_type )
    {
        return ap->ptr();
    }
    else
    {
        if (mlog.is_open())
            mlog << "Script Error in '" << scriptname() << "' PC=" << PC << ": " << endl
                 << "\tCall to function " << current_module_function->name << ":" << endl
                 << "\tParameter " << param << ": Expected datatype " << pointer_type
                 << ", got datatype " << ap->pointer_type() << endl;

        return NULL;
    }
}

BApplicObjBase* Executor::getApplicObjParam( unsigned param, const BApplicObjType* object_type )
{
	BApplicObjBase* aob = EXPLICIT_CAST(BApplicObjBase*, BObjectImp*)(getParamImp(param,BObjectImp::OTApplicObj));
    if (aob == NULL)
        return NULL;

	if (aob->object_type() == object_type)
	{
		return aob;
	}
	else
	{
        if (mlog.is_open())
            mlog << "Script Error in '" << scriptname() << "' PC=" << PC << ": " << endl
                 << "\tCall to function " << current_module_function->name << ":" << endl
                 << "\tParameter " << param << ": Expected datatype " << object_type
                 << ", got datatype " << aob->object_type() << endl;

		return NULL;
	}
}

bool Executor::getParam( unsigned param, unsigned short& value, unsigned short maxval )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= 0 && longvalue <= maxval)
        {
            value = static_cast<unsigned short>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [0.."
                + decint(maxval) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }

}

bool Executor::getParam( unsigned param, 
                               unsigned short& value, 
                               unsigned short minval, 
                               unsigned short maxval )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= minval && longvalue <= maxval)
        {
            value = static_cast<unsigned short>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [" + decint(minval) +".."
                + decint(maxval) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }

}
bool Executor::getParam( unsigned param, unsigned short& value )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= 0 && longvalue <= USHRT_MAX)
        {
            value = static_cast<unsigned short>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [0.."
                + decint(USHRT_MAX) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}
bool Executor::getParam( unsigned param, unsigned& value )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= 0 && longvalue <= (long)INT_MAX)
        {
            value = static_cast<unsigned>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [0.."
                + decint(INT_MAX) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Executor::getParam( unsigned param, short& value )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= (long)SHRT_MIN && longvalue <= (long)SHRT_MAX)
        {
            value = static_cast<short>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [" + decint(SHRT_MIN) +".."
                + decint(SHRT_MAX) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Executor::getParam( unsigned param, short& value, short maxval )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= (long)SHRT_MIN && longvalue <= maxval)
        {
            value = static_cast<short>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [" + decint(SHRT_MIN) +".."
                + decint(maxval) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool Executor::getParam( unsigned param, 
                               short& value, 
                               short minval, 
                               short maxval )
{
    BObjectImp* imp = getParamImp2( param, BObjectImp::OTLong );
    if (imp)
    {
        BLong* plong = explicit_cast<BLong*,BObjectImp*>(imp);

        long longvalue = plong->value();
        if (longvalue >= minval && longvalue <= maxval)
        {
            value = static_cast<short>(longvalue);
            return true;
        }
        else
        {
            string report = "Parameter " + decint(param) + " value " + decint(longvalue) + " out of expected range of [" + decint(minval) +".."
                + decint(maxval) + "]";
            func_result_ = new BError( report );
            return false;
        }
    }
    else
    {
        return false;
    }
}


BObjectRef& Executor::LocalVar( unsigned long varnum )
{
    passert( Locals2 );
    passert( varnum < Locals2->size() );

    return (*Locals2)[varnum];
}

BObjectRef& Executor::GlobalVar( unsigned long varnum )
{
    passert( varnum < Globals2.size() );
    return Globals2[varnum];
}

int Executor::getToken(Token& token, unsigned position)
{
	if (position >= nLines)
		return -1;
	token = prog_->instr[ position ].token;
	return 0;
}


bool Executor::setProgram( EScriptProgram* i_prog )
{
    prog_.set( i_prog );
    prog_ok_ = false;
    seterror( true );
  	if (!viewmode_)
    {
        if (!AttachFunctionalityModules())
            return false;
    }

    nLines = prog_->instr.size();

    Globals2.clear();
    for( unsigned i = 0; i < prog_->nglobals; ++i )
    {
	    Globals2.push_back( BObjectRef() );
        Globals2.back().set( new BObject( UninitObject::create() ) );
    }

    prog_ok_ = true;
    seterror( false );
    ++prog_->invocations;
    return true;
}

BObject *Executor::makeObj(const Token& token)
{
   switch(token.id) {
       case TOK_IDENT: return new BObject( new BError( "Please recompile this script!" ) );
       case TOK_LOCALVAR: return LocalVar( token.lval ).get();
       case TOK_GLOBALVAR: return GlobalVar( token.lval ).get();
       case TOK_STRING: return new BObject(new String(token.tokval()));
       case TOK_LONG:  return new BObject(new BLong(token.lval));
       case TOK_DOUBLE: return new BObject(new Double(token.dval));
       case TOK_ERROR: return new BObject( new BError("unknown") );
       default:
         passert(0);
         break;
   }
   return NULL;
}

BObjectRef Executor::getObjRef(void)
{
    if (ValueStack.empty()) 
    {
        Log( "Fatal error: Value Stack Empty! (%s,PC=%d)\n", prog_->name.c_str(), PC );
        cerr << "Fatal error: Value Stack Empty!" << endl;
        cerr << prog_->name << ", PC=" << PC << endl;
        seterror(true);
        return BObjectRef( UninitObject::create() ); 
    }
    
	BObjectRef ref = ValueStack.top();
	ValueStack.pop();
	return ref;
}

void Executor::check_containers(void)
{
#ifdef INC_TMALLOC
#ifndef NDEBUG
    /* Values */
    RArray<Object>& vals = (RArray<Object>& ) Values;

        for(int i=0;i<vals.count();i++) 
        {
            if (!tm_isalloced(vals[i])) 
            {
                printerror("Values node not alloced!");
                passert(0);
            }
        }

    /* Globals */
        for(i=0;i<Globals.count();i++) {
            if (!tm_isalloced(Globals[i])) {
                printerror("Global Node not alloced!");
                passert(0);
            }
        }

#endif
#endif
		/* Locals */
}

#include <stdlib.h>

void Executor::execFunc(const Token& token)
{
	FunctionalityModule* fm = prog_->modules[ token.module ];
	ModuleFunction* modfunc = fm->functions[ token.lval ];
    current_module_function = modfunc;
    if (modfunc->funcidx == -1)
    {
        if (mlog.is_open())
            mlog << "Error in script '" << prog_->name << "':" << endl
                 << "\tModule Function " << modfunc->name << " was not found." << endl;

        throw runtime_error( "No implementation for function found." ); 
    }

    ExecutorModule *em = execmodules[ token.module ];
	
    func_result_ = NULL;
    
    BObjectImp* resimp = em->execFunc( modfunc->funcidx );
    
    if (func_result_)
    {
        if (resimp)
        {
            BObject obj(resimp);
        }
    	ValueStack.push( BObjectRef( new BObject( func_result_ ) ) );
        func_result_ = NULL;
    }
    else if (resimp)
    {
	    ValueStack.push( BObjectRef( new BObject( resimp ) ) );
    }
    else
    {
        ValueStack.push( BObjectRef( new BObject( UninitObject::create() ) ) );
    }

    current_module_function = NULL;
	return;

}
				
// RSV_LOCAL
void Executor::ins_makeLocal( const Instruction& ins )
{
    passert( Locals2 != NULL );

    Locals2->push_back( BObjectRef() );
    Locals2->back().set( new BObject( UninitObject::create() ) );

	ValueStack.push( BObjectRef( Locals2->back().get() ) );
}

// RSV_DECLARE_ARRAY
void Executor::ins_declareArray( const Instruction& ins )
{
	BObjectRef objref = getObjRef();
	
    if (!objref->isa( BObjectImp::OTUninit ))
	{
		// FIXME: weak error message
		cerr << "variable is already initialized.." << endl;
		seterror( true );
		return;
	}
    ObjArray *arr = new ObjArray;

    objref->setimp( arr );

    ValueStack.push( BObjectRef( objref ) );
}

void Executor::popParam( const Token& token )
{
    BObjectRef objref = getObjRef();

    Locals2->push_back( BObjectRef( ) );
	Locals2->back().set( new BObject( objref->impptr()->copy() ) );
}

void Executor::popParamByRef( const Token& token )
{
    BObjectRef objref = getObjRef();
   
    Locals2->push_back( BObjectRef( objref ) );
}

void Executor::getArg( const Token& token )
{
    if (ValueStack.empty())
    {
        Locals2->push_back( BObjectRef() );
        Locals2->back().set( new BObject( UninitObject::create() ) );

    }
    else
    {
        BObjectRef objref = getObjRef();
        Locals2->push_back( BObjectRef() );
	    Locals2->back().set( new BObject( objref->impptr()->copy() ) );
    }
}


BObjectRef Executor::addmember( BObject& left, const BObject& right )
{
    if (!right.isa( BObjectImp::OTString ))
	{
		return BObjectRef(left.clone());
	}

	const String& varname = static_cast<const String&>(right.impref());

    return left.impref().operDotPlus( varname.data() );
}

BObjectRef Executor::removemember( BObject& left, const BObject& right )
{
    if (!right.isa( BObjectImp::OTString ))
	{
		return BObjectRef(left.clone());
	}

	const String& varname = static_cast<const String&>(right.impref());

    return left.impref().operDotMinus( varname.data() );
}

BObjectRef Executor::checkmember( BObject& left, const BObject& right )
{
    if (!right.isa( BObjectImp::OTString ))
	{
		return BObjectRef(left.clone());
	}

	const String& varname = static_cast<const String&>(right.impref());

    return left.impref().operDotQMark( varname.data() );
}

#include "contiter.h"

ContIterator::ContIterator() : BObjectImp(BObjectImp::OTUnknown)
{
}
BObject* ContIterator::step()
{
    return NULL;
}
BObjectImp *ContIterator::copy( void ) const
{
	return NULL;
}
unsigned long ContIterator::sizeEstimate() const
{
    return sizeof(ContIterator);
}
string ContIterator::getStringRep() const
{
    return "<iterator>";
}

class ArrayIterator : public ContIterator
{
public:
    ArrayIterator( ObjArray* pArr, BObject* pIterVal );
    virtual BObject* step();
private:
    size_t m_Index;
    BObject m_Array;
    ObjArray* m_pArray;
    BObjectRef m_IterVal;
    BLong* m_pIterVal;
};
ArrayIterator::ArrayIterator( ObjArray* pArr, BObject* pIterVal ) :
    m_Index(0),
    m_Array( pArr ),
    m_pArray( pArr ),
    m_IterVal( pIterVal ),
    m_pIterVal( new BLong(0) )
{
    m_IterVal.get()->setimp( m_pIterVal );
}
BObject* ArrayIterator::step()
{
    m_pIterVal->increment();
    if (++m_Index > m_pArray->ref_arr.size())
        return NULL;
    
    BObjectRef& objref = m_pArray->ref_arr[ m_Index-1 ];
    BObject* elem = objref.get();
    if (elem == NULL)
    {
        elem = new BObject( UninitObject::create() );
        objref.set( elem );
    }
    return elem;
}

ContIterator* BObjectImp::createIterator( BObject* pIterVal )
{
    return new ContIterator();
}
ContIterator* ObjArray::createIterator( BObject* pIterVal )
{
    ArrayIterator* pItr = new ArrayIterator( this, pIterVal );
    return pItr;
}

/* Coming into initforeach, the expr to be iterated through is on the value stack.
   Initforeach must create three local variables:
     0. the iterator
     1. the expression
     2. the counter
   and remove the expression from the value stack.
   It then jumps to the STEPFOREACH instruction.
*/
void Executor::ins_initforeach( const Instruction& ins )
{
    Locals2->push_back( BObjectRef() ); // the iterator

	// this is almost like popParam, only we don't want a copy.
    BObjectRef objref = getObjRef();
    Locals2->push_back( BObjectRef() );
    Locals2->back().set( objref.get() );
    
    Locals2->push_back( BObjectRef() );
    Locals2->back().set( new BObject( new BLong(0) ) );
    
    PC = ins.token.lval;
}


void Executor::ins_stepforeach( const Instruction& ins )
{
    unsigned locsize = Locals2->size();
    ObjArray* arr = static_cast<ObjArray*>((*Locals2)[locsize-2]->impptr());
    if (!arr->isa( BObjectImp::OTArray ))
        return;
    BLong* blong = static_cast<BLong*>((*Locals2)[locsize-1]->impptr());
    
    if (blong->increment() > long(arr->ref_arr.size()))
        return;
    
    BObjectRef& objref = arr->ref_arr[blong->value()-1];
    BObject* elem = objref.get();
    if (elem == NULL)
    {
        elem = new BObject( UninitObject::create() );
        objref.set( elem );
    }
    (*Locals2)[locsize-3].set( elem );
    PC = ins.token.lval;
}

void Executor::ins_initforeach2( const Instruction& ins )
{
    Locals2->push_back( BObjectRef() ); // the iterator


    BObject* pIterVal = new BObject(UninitObject::create());

    // this is almost like popParam, only we don't want a copy.
    BObjectRef objref = getObjRef();
    Locals2->push_back( BObjectRef() );
    ContIterator* pIter = objref->impptr()->createIterator( pIterVal );
    Locals2->back().set( new BObject(pIter) );
    
    Locals2->push_back( BObjectRef() );
    Locals2->back().set( pIterVal );
    
    PC = ins.token.lval;
}


void Executor::ins_stepforeach2( const Instruction& ins )
{
    unsigned locsize = Locals2->size();
    ContIterator* pIter = static_cast<ContIterator*>((*Locals2)[locsize-2]->impptr());

    BObject* next = pIter->step();
    if (next != NULL)
    {
        (*Locals2)[locsize-3].set( next );
        PC = ins.token.lval;
    }
}

/*
	Coming into the INITFOR, there will be two values on the value stack:
		START VALUE
		END VALUE

	If START VALUE > END VALUE, we skip the whole for loop.
	(the INITFOR's lval has the instr to jump to)
*/

void Executor::ins_initfor( const Instruction& ins )
{
    BObjectRef endref = getObjRef();
    BObjectRef startref = getObjRef();
    if (*startref.get() > *endref.get())
    {
        PC = ins.token.lval;
        return;
    }

    Locals2->push_back( BObjectRef( startref->clone() ) ); // the iterator
    Locals2->push_back( BObjectRef( endref->clone() ) );
}

void Executor::ins_nextfor( const Instruction& ins )
{
    unsigned locsize = Locals2->size();
    

    BObjectImp* itr = (*Locals2)[locsize-2]->impptr();
    BObjectImp* end = (*Locals2)[locsize-1]->impptr();

    if (itr->isa( BObjectImp::OTLong ))
    {
        BLong* blong = static_cast<BLong*>(itr);
        blong->increment();
    }
    else if (itr->isa( BObjectImp::OTDouble ))
    {
        Double* dbl = static_cast<Double*>(itr);
        dbl->increment();
    }

    if (!end->isLessThan(*itr))
    {
        PC = ins.token.lval;
    }
}


int Executor::ins_casejmp_findlong( const Token& token, BLong* blong )
{
    const unsigned char* dataptr = token.dataptr;
    for (;;)
    {
        unsigned short offset = * (const unsigned short*) dataptr;
        dataptr += 2;
        unsigned char type = *dataptr;
        dataptr += 1;
        if (type == CASE_TYPE_LONG)
        {
            if (blong->value() == * (const long*) dataptr)
                return offset;
            dataptr += 4;
        }
        else if (type == CASE_TYPE_DEFAULT)
        {
            return offset;
        }
        else
        {
            dataptr += type;
        }
    }
}

int Executor::ins_casejmp_findstring( const Token& token, String* bstringimp )
{
    const string& bstring = bstringimp->value();
    const unsigned char* dataptr = token.dataptr;
    for (;;)
    {
        unsigned short offset = * (const unsigned short*) dataptr;
        dataptr += 2;
        unsigned char type = *dataptr;
        dataptr += 1;
        if (type == CASE_TYPE_LONG)
        {
            dataptr += 4;
        }
        else if (type == CASE_TYPE_DEFAULT)
        {
            return offset;
        }
        else
        {
            if (bstring.size() == type &&
                memcmp( bstring.data(), dataptr, type ) == 0)
            {
                return offset;
            }
            dataptr += type;
        }
    }
}

int Executor::ins_casejmp_finddefault( const Token& token )
{
    const unsigned char* dataptr = token.dataptr;
    for (;;)
    {
        unsigned short offset = * (const unsigned short*) dataptr;
        dataptr += 2;
        unsigned char type = *dataptr;
        dataptr += 1;
        if (type == CASE_TYPE_LONG)
        {
            dataptr += 4;
        }
        else if (type == CASE_TYPE_DEFAULT)
        {
            return offset;
        }
        else
        {
            dataptr += type;
        }
    }
}

void Executor::ins_casejmp( const Instruction& ins )
{
    BObjectRef& objref = ValueStack.top();
    BObjectImp* objimp = objref->impptr();
    if (objimp->isa( BObjectImp::OTLong ))
    {
        PC = ins_casejmp_findlong( ins.token, static_cast<BLong*>(objimp) );
    }
    else if (objimp->isa( BObjectImp::OTString ))
    {
        PC = ins_casejmp_findstring( ins.token, static_cast<String*>(objimp) );
    }
    else
    {
        PC = ins_casejmp_finddefault( ins.token );
    }
    ValueStack.pop();
}


void Executor::ins_jmpiftrue( const Instruction& ins )
{
    BObjectRef& objref = ValueStack.top();
	
    if (objref->impptr()->isTrue()) 
        PC = (unsigned) ins.token.lval;

    ValueStack.pop();
}

void Executor::ins_jmpiffalse( const Instruction& ins )
{
    BObjectRef& objref = ValueStack.top();
    
    if (!objref->impptr()->isTrue()) 
        PC = (unsigned) ins.token.lval;
    
    ValueStack.pop();
}


// case TOK_LOCALVAR: 
void Executor::ins_localvar( const Instruction& ins )
{
    ValueStack.push( (*Locals2)[ins.token.lval] );
}

// case RSV_GLOBAL:
// case TOK_GLOBALVAR:
void Executor::ins_globalvar( const Instruction& ins )
{
    ValueStack.push( Globals2[ins.token.lval] );
}

// case TOK_LONG:
void Executor::ins_long( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject(new BLong(ins.token.lval)) ) );
}

// case TOK_CONSUMER:
void Executor::ins_consume( const Instruction& ins )
{
    ValueStack.pop();
}

void Executor::ins_set_member( const Instruction& ins )
{
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        left.impref().set_member( ins.token.tokval(), &rightimpref );
    }
    else
    {
        BObjectImp* imp = rightimpref.copy();
        BObject obj(imp);
        left.impref().set_member( ins.token.tokval(), imp );
    }
}

void Executor::ins_set_member_id( const Instruction& ins )
{
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        left.impref().set_member_id( ins.token.lval, &rightimpref );
    }
    else
    {
        BObjectImp* imp = rightimpref.copy();
        BObject obj(imp);
        left.impref().set_member_id( ins.token.lval, imp );
    }
}

void Executor::ins_set_member_consume( const Instruction& ins )
{
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        left.impref().set_member( ins.token.tokval(), &rightimpref );
    }
    else
    {
        BObjectImp* imp = rightimpref.copy();
        BObject obj(imp);
        left.impref().set_member( ins.token.tokval(), imp );
    }
    ValueStack.pop();
}

void Executor::ins_set_member_id_consume( const Instruction& ins )
{
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        left.impref().set_member_id( ins.token.lval, &rightimpref );
    }
    else
    {
        BObjectImp* imp = rightimpref.copy();
        BObject obj(imp);
        left.impref().set_member_id( ins.token.lval, imp );
    }
    ValueStack.pop();
}

void Executor::ins_get_member( const Instruction& ins )
{
	BObjectRef& leftref = ValueStack.top();

    BObject& left = *leftref;
			 
    leftref = left->get_member( ins.token.tokval() );
}

void Executor::ins_get_member_id( const Instruction& ins )
{
	BObjectRef& leftref = ValueStack.top();

    BObject& left = *leftref;
	
    leftref = left->get_member_id( ins.token.lval );
}

void Executor::ins_assign_localvar( const Instruction& ins )
{
    BObjectRef& lvar = (*Locals2)[ins.token.lval];

	BObjectRef& rightref = ValueStack.top();

    BObject& right = *rightref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        lvar->setimp( &rightimpref);
    }
    else
    {
        lvar->setimp( rightimpref.copy() );
    }
    ValueStack.pop();
}
void Executor::ins_assign_globalvar( const Instruction& ins )
{
    BObjectRef& gvar = Globals2[ins.token.lval];
    
    BObjectRef& rightref = ValueStack.top();

    BObject& right = *rightref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        gvar->setimp( &rightimpref );
    }
    else
    {
        gvar->setimp( rightimpref.copy() );
    }
    ValueStack.pop();
}

// case INS_ASSIGN_CONSUME:
void Executor::ins_assign_consume( const Instruction& ins )
{
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        left.setimp( &rightimpref );
    }
    else
    {
        left.setimp( rightimpref.copy() );
    }
    ValueStack.pop();
}

void Executor::ins_assign( const Instruction& ins )
{
        /*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
         */
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    BObjectImp& rightimpref = right.impref();

    if (right.count() == 1 && rightimpref.count() == 1)
    {
        left.setimp( &rightimpref );
    }
    else
    {
        left.setimp( rightimpref.copy() );
    }
}

void Executor::ins_array_assign( const Instruction& ins )
{
/*
	on the value stack:
	x[i] := y;
	(top)
	y
	i
	x
	upon exit:
	(x[i])
*/
	BObjectRef y_ref = ValueStack.top();
    ValueStack.pop();
    BObjectRef i_ref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& x_ref = ValueStack.top();

    BObject& y = *y_ref;
    BObject& i = *i_ref;
    BObject& x = *x_ref;

    BObjectImp* result;
    

    if (y.count() == 1 && y->count() == 1)
    {
        result = x->array_assign( i.impptr(), y.impptr() );
    }
    else
    {
        result = x->array_assign( i.impptr(), y->copy() );
    }
    
    x_ref.set( new BObject(result) );
}
void Executor::ins_array_assign_consume( const Instruction& ins )
{
/*
	on the value stack:
	x[i] := y;
	(top)
	y
	i
	x
	upon exit:
	(x[i])
*/
	BObjectRef y_ref = ValueStack.top();
    ValueStack.pop();
    BObjectRef i_ref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& x_ref = ValueStack.top();

    BObject& y = *y_ref;
    BObject& i = *i_ref;
    BObject& x = *x_ref;

    BObjectImp* result;
    

    if (y.count() == 1 && y->count() == 1)
    {
        result = x->array_assign( i.impptr(), y.impptr() );
    }
    else
    {
        result = x->array_assign( i.impptr(), y->copy() );
    }
    
    BObject obj(result);
    ValueStack.pop();
}

// TOK_ADD:
void Executor::ins_add( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfPlusObjImp(right.impref() ) ) );
}

// TOK_SUBTRACT
void Executor::ins_subtract( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	leftref.set( new BObject( left.impref().selfMinusObjImp( right.impref() ) ) );
}

// TOK_MULT:
void Executor::ins_mult( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfTimesObjImp(right.impref() ) ) );
}
// TOK_DIV:
void Executor::ins_div( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfDividedByObjImp(right.impref() ) ) );
}
// TOK_MODULUS:
void Executor::ins_modulus( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfModulusObjImp(right.impref() ) ) );
}
// TOK_BSRIGHT:
void Executor::ins_bitshift_right( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfBitShiftRightObjImp(right.impref() ) ) );
}
// TOK_BSLEFT:
void Executor::ins_bitshift_left( const Instruction& ins )
{
	/*
		These each take two operands, and replace them with one.
		We'll leave the second one on the value stack, and
		just replace its object with the result
	*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfBitShiftLeftObjImp(right.impref() ) ) );
}
// TOK_BITAND:
void Executor::ins_bitwise_and( const Instruction& ins )
{
	/*
		These each take two operands, and replace them with one.
		We'll leave the second one on the value stack, and
		just replace its object with the result
	*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfBitAndObjImp(right.impref() ) ) );
}
// TOK_BITXOR:
void Executor::ins_bitwise_xor( const Instruction& ins )
{
	/*
		These each take two operands, and replace them with one.
		We'll leave the second one on the value stack, and
		just replace its object with the result
	*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfBitXorObjImp(right.impref() ) ) );
}
// TOK_BITOR:
void Executor::ins_bitwise_or( const Instruction& ins )
{
	/*
		These each take two operands, and replace them with one.
		We'll leave the second one on the value stack, and
		just replace its object with the result
	*/
    BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( left.impref().selfBitOrObjImp(right.impref() ) ) );
}

void Executor::ins_logical_and( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = (left.isTrue() && right.isTrue());
	leftref.set( new BObject( new BLong( _true ) ) );
}
void Executor::ins_logical_or( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = (left.isTrue() || right.isTrue());
	leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_notequal( const Instruction& ins )
{
	/*
		These each take two operands, and replace them with one.
		We'll leave the second one on the value stack, and
		just replace its object with the result
	*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = !(left->isEqual( right.impref() ) );
	leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_equal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = (left->isEqual( right.impref() ) );
	leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_lessthan( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = (left->isLT( right.impref() ) );
	leftref.set( new BObject( new BLong( _true ) ) );
}

void Executor::ins_lessequal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;
    int _true = (left->isLE(right.impref()));
	leftref.set( new BObject( new BLong( _true ) ) );
}
void Executor::ins_greaterthan( const Instruction& ins )
{
        /*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = (right->isLT( left.impref() ) );
	leftref.set( new BObject( new BLong( _true ) ) );
}
void Executor::ins_greaterequal( const Instruction& ins )
{
        /*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    int _true = (right->isLE( left.impref() ) );
	leftref.set( new BObject( new BLong( _true ) ) );
}

// case TOK_ARRAY_SUBSCRIPT:
void Executor::ins_arraysubscript( const Instruction& ins )
{
        /*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    leftref = (*leftref)->OperSubscript( *rightref );
}

void Executor::ins_multisubscript( const Instruction& ins )
{
    // the subscripts are on the value stack in right-to-left order, followed by the array itself
    stack<BObjectRef> indices;
    for( long i = 0; i < ins.token.lval; ++i )
    {
        indices.push( ValueStack.top() );
        ValueStack.pop();
    }

    BObjectRef& leftref = ValueStack.top();
    leftref = (*leftref)->OperMultiSubscript( indices );

}
void Executor::ins_multisubscript_assign( const Instruction& ins )
{
    BObjectRef target_ref = ValueStack.top();
    ValueStack.pop();
    // the subscripts are on the value stack in right-to-left order, followed by the array itself
    stack<BObjectRef> indices;
    for( long i = 0; i < ins.token.lval; ++i )
    {
        indices.push( ValueStack.top() );
        ValueStack.pop();
    }

    BObjectRef& leftref = ValueStack.top();
    leftref = (*leftref)->OperMultiSubscriptAssign( indices, target_ref->impptr() );

}

void Executor::ins_addmember( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;
			 
    leftref = addmember( left, right );
}

void Executor::ins_removemember( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;
			 
    leftref = removemember( left, right );
}

void Executor::ins_checkmember( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;
			 
    leftref = checkmember( left, right );
}

void Executor::ins_addmember2( const Instruction& ins )
{
	BObjectRef obref = ValueStack.top();

    BObject& ob = *obref;
			 
    ob.impref().operDotPlus( ins.token.tokval() );
}

void Executor::ins_addmember_assign( const Instruction& ins )
{
	BObjectRef valref = ValueStack.top();
    BObject& valob = *valref;
    BObjectImp* valimp = valref->impptr();

    ValueStack.pop();

    BObjectRef obref = ValueStack.top();
    BObject& ob = *obref;

    BObjectRef memref = ob.impref().operDotPlus( ins.token.tokval() );
    BObject& mem = *memref;

    if (valob.count() == 1 && valimp->count() == 1)
    {
        mem.setimp( valimp );
    }
    else
    {
        mem.setimp( valimp->copy() );
    }
    // the struct is at the top of the stack
}

void Executor::ins_dictionary_addmember( const Instruction& ins )
{
/*
	ENTRANCE: value stack:
		dictionary
		key
		value
	EXIT: value stack:
		dictionary
	FUNCTION:
		adds the (key, value) pair to the dictionary
*/

    BObjectRef valref = ValueStack.top();
    ValueStack.pop();
    BObject& valob = *valref;
    BObjectImp* valimp = valob.impptr();

    BObjectRef keyref = ValueStack.top();
    ValueStack.pop();
    BObject& keyob = *keyref;
    BObjectImp* keyimp = keyob.impptr();

    BObjectRef dictref = ValueStack.top();
    BObject& dictob = *dictref;
    BDictionary* dict = static_cast<BDictionary*>(dictob.impptr());

    if (keyob.count() != 1 || keyimp->count() != 1)
    {
        keyimp = keyimp->copy();
    }
    if (valob.count() != 1 || valimp->count() != 1)
    {
        valimp = valimp->copy();
    }

    dict->addMember( keyimp, valimp );
    // the dictionary remains at the top of the stack.
}

void Executor::ins_in( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

    leftref.set( new BObject( new BLong( right.impref().contains( left.impref() ) ) ) );
}

void Executor::ins_insert_into( const Instruction& ins )
{
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	left.impref().operInsertInto( left, right.impref() );
}

void Executor::ins_plusequal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	left.impref().operPlusEqual( left, right.impref() );
}

void Executor::ins_minusequal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	left.impref().operMinusEqual( left, right.impref() );
}

void Executor::ins_timesequal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	left.impref().operTimesEqual( left, right.impref() );
}

void Executor::ins_divideequal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	left.impref().operDivideEqual( left, right.impref() );
}

void Executor::ins_modulusequal( const Instruction& ins )
{
		/*
			These each take two operands, and replace them with one.
			We'll leave the second one on the value stack, and
			just replace its object with the result
		*/
	BObjectRef rightref = ValueStack.top();
    ValueStack.pop();
    BObjectRef& leftref = ValueStack.top();

    BObject& right = *rightref;
    BObject& left = *leftref;

	left.impref().operModulusEqual( left, right.impref() );
}

//case RSV_GOTO:
void Executor::ins_goto( const Instruction& ins )
{
    PC = (unsigned) ins.token.lval;
}

// TOK_FUNC:
void Executor::ins_func( const Instruction& ins )
{
	unsigned nparams = prog_->modules[ ins.token.module ]->functions[ ins.token.lval ]->nargs;
	getParams( nparams );
	execFunc(ins.token);
	cleanParams( );
    return;
}

void Executor::ins_call_method_id( const Instruction& ins )
{
    unsigned nparams = ins.token.type;
    getParams( nparams );

    BObjectRef& objref = ValueStack.top();
    BObjectImp* imp = objref->impptr()->call_method_id( ins.token.lval, *this );
    
    if (func_result_)
    {
        if (imp)
        {
            BObject obj(imp);
        }

        objref.set( new BObject( func_result_ ) );
        func_result_ = NULL;
    }
    else if (imp)
    {
        objref.set( new BObject( imp ) );
    }
    else
    {
        objref.set( new BObject( UninitObject::create() ) );
    }

    cleanParams();
    return;
}

void Executor::ins_call_method( const Instruction& ins )
{
    unsigned nparams = ins.token.lval;
    getParams( nparams );

    BObjectRef& objref = ValueStack.top();
    BObjectImp* imp = objref->impptr()->call_method( ins.token.tokval(), *this );
    
    if (func_result_)
    {
        if (imp)
        {
            BObject obj(imp);
        }

        objref.set( new BObject( func_result_ ) );
        func_result_ = NULL;
    }
    else if (imp)
    {
        objref.set( new BObject( imp ) );
    }
    else
    {
        objref.set( new BObject( UninitObject::create() ) );
    }

    cleanParams();
    return;
}

// CTRL_STATEMENTBEGIN:
void Executor::ins_statementbegin( const Instruction& ins )
{
	if (debug_level >= SOURCELINES && ins.token.tokval())
        cout << ins.token.tokval() << endl;
}

// case CTRL_PROGEND: 
void Executor::ins_progend( const Instruction& ins )
{
    done = 1;
    run_ok_ = false;
    PC = 0;
}


// case CTRL_MAKELOCAL:
void Executor::ins_makelocal( const Instruction& ins )
{
	if (Locals2) upperLocals2.push(Locals2);
	Locals2 = new BObjectRefVec;
}

// CTRL_JSR_USERFUNC:
void Executor::ins_jsr_userfunc( const Instruction& ins )
{
    ReturnContext rc;
    rc.PC = PC;
    rc.ValueStackDepth = ValueStack.size();
	ControlStack.push( rc );

	PC = (unsigned) ins.token.lval;
    if (ControlStack.size() >= escript_config.max_call_depth)
    {
        Log( "Script %s exceeded maximum call depth\n", scriptname().c_str() );
        Log( "Return path PCs: " );
        while (!ControlStack.empty())
        {
            rc = ControlStack.top();
            ControlStack.pop();
            Log( "%d ", rc.PC );
        }
        Log( "\n" );
        seterror( true );
    }
}

void Executor::ins_pop_param( const Instruction& ins )
{
    popParam( ins.token );
}

void Executor::ins_pop_param_byref( const Instruction& ins )
{
    popParamByRef( ins.token );
}
        
void Executor::ins_get_arg( const Instruction& ins )
{
    getArg( ins.token );
}

// CTRL_LEAVE_BLOCK: 
void Executor::ins_leave_block( const Instruction& ins )
{
    if (Locals2)
    {
        for( int i = 0; i < ins.token.lval; i++ )
            Locals2->pop_back();
    }
    else // at global level.  ick.
    {
        for( int i = 0; i < ins.token.lval; i++ )
		    Globals2.pop_back();
    }
}

void Executor::ins_gosub( const Instruction& ins )
{
    ReturnContext rc;
    rc.PC = PC;
    rc.ValueStackDepth = ValueStack.size();
	ControlStack.push( rc );
	if (Locals2) upperLocals2.push(Locals2);
	Locals2 = new BObjectRefVec;

    PC = (unsigned) ins.token.lval;
}

// case RSV_RETURN
void Executor::ins_return( const Instruction& ins )
{
    if (ControlStack.empty()) 
	{
		cerr << "Return without GOSUB!" << endl;

		seterror( true );
		return;
	}
	ReturnContext& rc = ControlStack.top();
    PC = rc.PC;
    // FIXME do something with rc.ValueStackDepth
    ControlStack.pop();

	if (Locals2) 
	{ 
		delete Locals2; 
		Locals2 = NULL; 
	}
	if (!upperLocals2.empty()) 
    {
        Locals2 = upperLocals2.top();
        upperLocals2.pop();
    }
}

void Executor::ins_exit( const Instruction& ins )
{
    done = 1;
    run_ok_ = false;
}

void Executor::ins_double( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject(new Double(ins.token.dval)) ) );
}
void Executor::ins_string( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject(new String(ins.token.tokval())) ) );
}
void Executor::ins_error( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject( new BError() ) ) );
}
void Executor::ins_struct( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject( new BStruct ) ) );
}
void Executor::ins_array( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject( new ObjArray ) ) );
}
void Executor::ins_dictionary( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject( new BDictionary ) ) );
}
void Executor::ins_uninit( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject( UninitObject::create() ) ) );
}
void Executor::ins_ident( const Instruction& ins )
{
    ValueStack.push( BObjectRef( new BObject( new BError( "Please recompile this script" ) ) ) );
}

// case TOK_UNMINUS:
void Executor::ins_unminus( const Instruction& ins )
{
	BObjectRef ref = getObjRef();
    BObjectImp *newobj;
	newobj = ref->impref().inverse();
    
	ValueStack.push( BObjectRef( new BObject(newobj) ) );
}

// case TOK_LOG_NOT:
void Executor::ins_logical_not( const Instruction& ins )
{
	BObjectRef ref = getObjRef();
	ValueStack.push( BObjectRef( new BObject(new BLong((long) !ref->impptr()->isTrue()))));
    return;
}

// case TOK_BITWISE_NOT:
void Executor::ins_bitwise_not( const Instruction& ins )
{
    BObjectRef ref = getObjRef();
    ValueStack.push( BObjectRef( new BObject( ref->impptr()->bitnot() ) ) );
    return;
}
		

void Executor::innerExec(const Instruction& ins)
{
    ++escript_execinstr_calls;
	const Token& token = ins.token;

    // this seems an ideal place for a table of function pointers ...
    Log( "Script %s used undefined token %d at PC %d\n",
            scriptname().c_str(),
            ins.token.id,
            PC );

    switch(ins.token.id) 
    {
        case TOK_USERFUNC:
            return;

        case TOK_FUNC: // ins_func
            {
		        unsigned nparams = prog_->modules[ ins.token.module ]->functions[ ins.token.lval ]->nargs;
		        //result = NULL;
		        getParams( nparams );
		        execFunc(token);
		        cleanParams( );
                return;
            }

        case INS_CALL_METHOD:   // ins_call_method
            {
                unsigned nparams = token.lval;
                getParams( nparams );

                BObjectRef& objref = ValueStack.top();
                BObjectImp* imp = objref->impptr()->call_method( token.tokval(), *this );
                
                if (func_result_)
                {
                    objref.set( new BObject( func_result_ ) );
                    func_result_ = NULL;
                }
                else if (imp)
                {
                    objref.set( new BObject( imp ) );
                }
                else
                {
                    objref.set( new BObject( UninitObject::create() ) );
                }

                cleanParams();
                return;
            }
        case INS_CALL_METHOD_ID:   // ins_call_method
            {
                unsigned nparams = token.type;
                getParams( nparams );

                BObjectRef& objref = ValueStack.top();
                BObjectImp* imp = objref->impptr()->call_method_id( token.lval, *this );
                
                if (func_result_)
                {
                    objref.set( new BObject( func_result_ ) );
                    func_result_ = NULL;
                }
                else if (imp)
                {
                    objref.set( new BObject( imp ) );
                }
                else
                {
                    objref.set( new BObject( UninitObject::create() ) );
                }

                cleanParams();
                return;
            }

	    case CTRL_STATEMENTBEGIN:   // ins_statementbegin
		    if (debug_level >= SOURCELINES && token.tokval())
                cout << token.tokval() << endl;
		    return;
	    case CTRL_PROGEND:  // ins_progend
		    done = 1;
            run_ok_ = false;
            PC = 0;
		    return;
	    case CTRL_MAKELOCAL:    // ins_makelocal
		    if (Locals2) upperLocals2.push(Locals2);
		    Locals2 = new BObjectRefVec;
		    return;
	    case CTRL_JSR_USERFUNC: // ins_jsr_userfunc
            ReturnContext rc;
            rc.PC = PC;
            rc.ValueStackDepth = ValueStack.size();
		    ControlStack.push( rc );
        
		    PC = (unsigned) token.lval;
            if (ControlStack.size() >= escript_config.max_call_depth)
            {
                Log( "Script %s exceeded maximum call depth\n", scriptname().c_str() );
                Log( "Return path PCs: " );
                while (!ControlStack.empty())
                {
                    rc = ControlStack.top();
                    ControlStack.pop();
                    Log( "%d ", rc.PC );
                }
                Log( "\n" );
                seterror( true );
            }
		    return;
	    case INS_POP_PARAM: // ins_pop_param
		    popParam( ins.token );
		    return;
	    case INS_POP_PARAM_BYREF:   // ins_pop_param_byref
		    popParamByRef( ins.token );
		    return;
        case INS_GET_ARG:       // ins_get_arg
            getArg( ins.token );
            return;
        case CTRL_LEAVE_BLOCK:  // ins_leave_block
//            if (1)
            {
                if (Locals2)
                {
                    for( int i = 0; i < ins.token.lval; i++ )
                        Locals2->pop_back();
                }
                else // at global level.  ick.
                {
                    for( int i = 0; i < ins.token.lval; i++ )
						Globals2.pop_back();
                }
            }
            return;

		case RSV_GOSUB: // ins_gosub
            {
                ReturnContext rc;
                rc.PC = PC;
                rc.ValueStackDepth = ValueStack.size();
			    ControlStack.push( rc );
			    if (Locals2) upperLocals2.push(Locals2);
			    Locals2 = new BObjectRefVec;
            }
			// NOTE fallthrough
		case RSV_GOTO:  // ins_goto
			PC = (unsigned) ins.token.lval;
			return;

        case RSV_RETURN:    // ins_return
			if (ControlStack.empty()) 
			{
				cerr << "Return without GOSUB!" << endl;

				seterror( true );
				return;
			}
			rc = ControlStack.top();
            ControlStack.pop();
            PC = rc.PC;
            // FIXME do something with rc.ValueStackDepth

			if (Locals2) 
			{ 
				delete Locals2; 
				Locals2 = NULL; 
			}
			if (!upperLocals2.empty()) 
            {
                Locals2 = upperLocals2.top();
                upperLocals2.pop();
            }
			return;
        
#ifdef NEVER
        case INS_INITFOREACH:   // ins_initforeach
            ins_initforeach( ins );
            return;
        case INS_STEPFOREACH:   // ins_stepforeach
            ins_stepforeach( ins );
            return;
#endif
        case INS_INITFOR:       // ins_initfor
            ins_initfor( ins );
            return;
        case INS_NEXTFOR:       // ins_nextfor
            ins_nextfor( ins );
            return;
        case INS_CASEJMP:       // ins_casejmp
            ins_casejmp( ins );
            return;

		case RSV_EXIT:          // ins_exit
			done = 1;
            run_ok_ = false;
			return;

		case RSV_JMPIFTRUE:     // ins_jmpiftrue
            // ins_jmpiftrue
            {
                BObjectRef& objref = ValueStack.top();
				
                if (objref->impptr()->isTrue()) 
                    PC = (unsigned) ins.token.lval;

                ValueStack.pop();
            }
			return;
        
		case RSV_JMPIFFALSE:    // ins_jmpiffalse
            {
                BObjectRef& objref = ValueStack.top();
                
    			if (!objref->impptr()->isTrue()) 
                    PC = (unsigned) ins.token.lval;
                
                ValueStack.pop();
            }
            return;

        case RSV_LOCAL:     // ins_makeLocal
			ins_makeLocal( ins );
            return;

        case INS_DECLARE_ARRAY: 
			ins_declareArray( ins );
            return;

        case TOK_LOCALVAR:  // ins_localvar
            passert( Locals2 );
            passert( token.lval < static_cast<long>(Locals2->size()) );

            ValueStack.push( (*Locals2)[token.lval] );
            return;

		case RSV_GLOBAL:        // ins_globalvar
        case TOK_GLOBALVAR:
            passert( token.lval < static_cast<long>(Globals2.size()) );
            ValueStack.push( Globals2[token.lval] );
            return;

        case TOK_LONG:  // ins_long
            ValueStack.push( BObjectRef( new BObject(new BLong(token.lval)) ) );
            return;
        case TOK_DOUBLE:    // ins_double
            ValueStack.push( BObjectRef( new BObject(new Double(token.dval)) ) );
            return;
        case TOK_STRING:
            ValueStack.push( BObjectRef( new BObject(new String(token.tokval())) ) );
            return;
        case TOK_ERROR:
            ValueStack.push( BObjectRef( new BObject( new BError() ) ) );
            return;
        case TOK_STRUCT:
            ValueStack.push( BObjectRef( new BObject( new BStruct ) ) );
            return;
        case TOK_ARRAY:
            ValueStack.push( BObjectRef( new BObject( new ObjArray ) ) );
            return;
        case TOK_DICTIONARY:
            ValueStack.push( BObjectRef( new BObject( new BDictionary ) ) );
            return;

        case TOK_IDENT:
			ValueStack.push( BObjectRef( new BObject( new BError( "Please recompile this script" ) ) ) );
            return;
		
        
	    case TOK_CONSUMER:  // ins_consumer
            ValueStack.pop(); // just consume
            return;

	    case TOK_UNMINUS:
            {
			    BObjectRef ref = getObjRef();
                BObjectImp *newobj;
				newobj = ref->impref().inverse();
                
				ValueStack.push( BObjectRef( new BObject(newobj) ) );
				return;
            }

	    case TOK_UNPLUS:        // unary plus doesn't actually do anything.
            return;

	    case TOK_LOG_NOT:       // ins_logical_not
            {
			    BObjectRef ref = getObjRef();
			    ValueStack.push( BObjectRef( new BObject(new BLong((long) !ref->impptr()->isTrue()))));
                return;
            }
        case TOK_BITWISE_NOT:   // ins_bitwise_not
            {
                BObjectRef ref = getObjRef();
                ValueStack.push( BObjectRef( new BObject( ref->impptr()->bitnot() ) ) );
                return;
            }


     	case INS_ASSIGN_CONSUME:    // ins_assign_consume
            {
			    BObjectRef rightref = ValueStack.top();
                ValueStack.pop();
                BObjectRef& leftref = ValueStack.top();

                BObject& right = *rightref;
                BObject& left = *leftref;

                BObjectImp& rightimpref = right.impref();

                if (right.count() == 1 && rightimpref.count() == 1)
                {
                    left.setimp( &rightimpref );
                }
                else
                {
                    left.setimp( rightimpref.copy() );
                }
                ValueStack.pop();

                return;
            }

        case TOK_ASSIGN:        // ins_assign
        case TOK_DIV:           // ins_div
        case TOK_SUBTRACT:      // ins_subtract
        case TOK_MULT:          // ins_mult
        case TOK_ADD:           // ins_add
        case TOK_MODULUS:       // ins_modulus

		case TOK_PLUSEQUAL:         // ins_plusequal
		case TOK_MINUSEQUAL:		// ins_minusequal
		case TOK_TIMESEQUAL:		// ins_timesequal
		case TOK_DIVIDEEQUAL:		// ins_divideequal
		case TOK_MODULUSEQUAL:		// ins_modulusequal

		case TOK_BSRIGHT:		// ins_bitshift_right
		case TOK_BSLEFT:		// ins_bitshift_left
        case TOK_BITAND:        // ins_bitwise_and
        case TOK_BITXOR:        // ins_bitwise_xor
        case TOK_BITOR:         // ins_bitwise_or

        case TOK_EQUAL:         // ins_equal
        case TOK_NEQ:           // ins_notequal
        case TOK_LESSTHAN:      // ins_lessthan
        case TOK_LESSEQ:        // ins_lessequal
        case TOK_GRTHAN:        // ins_greaterthan
        case TOK_GREQ:          // ins_greaterequal
        case TOK_AND:           // ins_logical_and
        case TOK_OR:            // ins_logical_or

        case TOK_IN:            // ins_in

        case TOK_ARRAY_SUBSCRIPT:   // ins_arraysubscript
        case TOK_DELMEMBER:         // ins_removemember
        case TOK_CHKMEMBER:         // ins_checkmember
        case TOK_ADDMEMBER:         // ins_addmember
        case TOK_MEMBER:            // ins_member
		{
			/*
				These each take two operands, and replace them with one.
				We'll leave the second one on the value stack, and
				just replace its object with the result
			*/
			BObjectRef rightref = ValueStack.top();
            ValueStack.pop();
            BObjectRef& leftref = ValueStack.top();

            BObject& right = *rightref;
            BObject& left = *leftref;

						// if (!quiet) cout << "LEFT isa " << left.value().isA() << endl;
						// if (!quiet) cout << "RIGHT isa " << right.value().isA() << endl;
			switch(token.id) 
            {
			  case TOK_ASSIGN:
                  {
                      BObjectImp& rightimpref = right.impref();

                      if (right.count() == 1 && rightimpref.count() == 1)
                      {
                          passert_always(0);
                      }
                      else
                      {
                          passert_always(0);
                      }
                      return;
                  }
                break;
			  case TOK_ADD:         // ins_add
				leftref.set( new BObject( left.impref().selfPlusObjImp(right.impref() ) ) );
                return;
			  case TOK_DIV:         // ins_div
				leftref.set( new BObject( left.impref().selfDividedByObjImp( right.impref() ) ) );
				return;
              case TOK_SUBTRACT:    // ins_subtract
				leftref.set( new BObject( left.impref().selfMinusObjImp( right.impref() ) ) );
				return;
			  case TOK_MULT:        // ins_mult
				leftref.set( new BObject( left.impref().selfTimesObjImp( right.impref() ) ) );
				return;
              case TOK_MODULUS:     // ins_modulus
                leftref.set( new BObject( left.impref().selfModulusObjImp( right.impref() ) ) );
                return;
			  case TOK_BSRIGHT:      // ins_bitshift_right
                leftref.set( new BObject( left.impref().selfBitShiftRightObjImp( right.impref() ) ) );
                return;
			  case TOK_BSLEFT:      // ins_bitshift_left
                leftref.set( new BObject( left.impref().selfBitShiftLeftObjImp( right.impref() ) ) );
                return;
              case TOK_BITAND:      // ins_bitwise_and
                leftref.set( new BObject( left.impref().selfBitAndObjImp( right.impref() ) ) );
                return;
              case TOK_BITXOR:      // ins_bitwise_xor
                leftref.set( new BObject( left.impref().selfBitXorObjImp( right.impref() ) ) );
                return;
              case TOK_BITOR:       // ins_bitwise_or
                leftref.set( new BObject( left.impref().selfBitOrObjImp( right.impref() ) ) );
                return;
			
			  case TOK_PLUSEQUAL:
				left.impref().operPlusEqual( left, right.impref() );
				return;
			  case TOK_MINUSEQUAL:
				left.impref().operMinusEqual( left, right.impref() );
				return;
			  case TOK_TIMESEQUAL:
				left.impref().operTimesEqual( left, right.impref() );
				return;
			  case TOK_DIVIDEEQUAL:
				left.impref().operDivideEqual( left, right.impref() );
				return;
			  case TOK_MODULUSEQUAL:
				left.impref().operModulusEqual( left, right.impref() );
				return;

				// Binary Logical/Comparison Operators
			  case TOK_EQUAL:
			  case TOK_NEQ:
			  case TOK_LESSTHAN:
			  case TOK_LESSEQ:
			  case TOK_GRTHAN:
			  case TOK_GREQ:
			  case TOK_AND:
			  case TOK_OR:
				int _true;
				_true = 0;
				switch(token.id) {
					case TOK_EQUAL:    _true = (left == right); break;
					case TOK_NEQ:      _true = (left != right); break;
					case TOK_LESSTHAN: _true = (left <  right); break;
					case TOK_LESSEQ:   _true = (left->isLE(right.impref())); break;
					case TOK_GRTHAN:   _true = (left >  right); break;
					case TOK_GREQ:     _true = (left >= right); break;
					case TOK_AND:      _true = (left.isTrue() &&
											    right.isTrue()); break;
					case TOK_OR:       _true = (left.isTrue() ||
											    right.isTrue()); break;
					default: passert(0); break;
				}
				leftref.set( new BObject( new BLong( _true ) ) );
                return;

              case TOK_IN:
                leftref.set( new BObject( new BLong( right.impref().contains( left.impref() ) ) ) );
                return;

              case TOK_ARRAY_SUBSCRIPT: // ins_array_subscript
                leftref = left.impptr()->OperSubscript( right );
                return;
			  case TOK_CHKMEMBER: // ins_checkmember
				  leftref = checkmember( left, right );
				  return;
			  case TOK_DELMEMBER: //ins_removemember
				  leftref = removemember( left, right );
				  return;
			  case TOK_ADDMEMBER:   // ins_addmember
                leftref = addmember( left, right );
                return;

              default: 
                  cerr << "Operator handling not defined in Executor::innerExec for " << token << endl;
                  seterror( true );
                  return;
            }
            // should be unreachable, see default above
            return;
		}

        default:
            cerr << "Execution error in " << scriptname() << ":" << endl;
            cerr << "Unhandled token " << token << " in Executor::innerExec" << endl;
            seterror( true );
		    return;
    }
    // also unreachable
	return;
}

void Executor::ins_nop( const Instruction& ins )
{
}

ExecInstrFunc Executor::GetInstrFunc( const Token& token )
{
    switch( token.id )
    {
    case INS_INITFOREACH:       return &Executor::ins_initforeach2;
    case INS_STEPFOREACH:       return &Executor::ins_stepforeach2;
    case INS_INITFOR:           return &Executor::ins_initfor;
    case INS_NEXTFOR:           return &Executor::ins_nextfor;
    case INS_CASEJMP:           return &Executor::ins_casejmp;
    case RSV_JMPIFTRUE:         return &Executor::ins_jmpiftrue;
    case RSV_JMPIFFALSE:        return &Executor::ins_jmpiffalse;
    case RSV_LOCAL:             return &Executor::ins_makeLocal;
    case RSV_GLOBAL:
    case TOK_GLOBALVAR:         return &Executor::ins_globalvar;
    case TOK_LOCALVAR:          return &Executor::ins_localvar;
    case TOK_LONG:              return &Executor::ins_long;
    case TOK_DOUBLE:            return &Executor::ins_double;
    case TOK_STRING:            return &Executor::ins_string;
    case TOK_ERROR:             return &Executor::ins_error;
    case TOK_STRUCT:            return &Executor::ins_struct;
    case TOK_ARRAY:            return &Executor::ins_array;
    case TOK_DICTIONARY:        return &Executor::ins_dictionary;
    case INS_UNINIT:            return &Executor::ins_uninit;
    case TOK_IDENT:             return &Executor::ins_ident;
    case INS_ASSIGN_GLOBALVAR:  return &Executor::ins_assign_globalvar;
    case INS_ASSIGN_LOCALVAR:   return &Executor::ins_assign_localvar;
    case INS_ASSIGN_CONSUME:    return &Executor::ins_assign_consume;
    case TOK_CONSUMER:          return &Executor::ins_consume;
    case TOK_ASSIGN:            return &Executor::ins_assign;
    case INS_SUBSCRIPT_ASSIGN:  return &Executor::ins_array_assign;
    case INS_SUBSCRIPT_ASSIGN_CONSUME: return &Executor::ins_array_assign_consume;
    case INS_MULTISUBSCRIPT:    return &Executor::ins_multisubscript;
    case INS_MULTISUBSCRIPT_ASSIGN:  return &Executor::ins_multisubscript_assign;
    case INS_GET_MEMBER:        return &Executor::ins_get_member;
    case INS_SET_MEMBER:        return &Executor::ins_set_member;
    case INS_SET_MEMBER_CONSUME: return &Executor::ins_set_member_consume;

    case INS_GET_MEMBER_ID:     return &Executor::ins_get_member_id; //test id
    case INS_SET_MEMBER_ID:     return &Executor::ins_set_member_id; //test id
    case INS_SET_MEMBER_ID_CONSUME:     return &Executor::ins_set_member_id_consume; //test id

    case TOK_ADD:               return &Executor::ins_add;
    case TOK_SUBTRACT:          return &Executor::ins_subtract;
	case TOK_DIV:               return &Executor::ins_div;
    case TOK_MULT:              return &Executor::ins_mult;
    case TOK_MODULUS:           return &Executor::ins_modulus;

	case TOK_INSERTINTO:		return &Executor::ins_insert_into;

	case TOK_PLUSEQUAL:         return &Executor::ins_plusequal;
	case TOK_MINUSEQUAL:		return &Executor::ins_minusequal;
	case TOK_TIMESEQUAL:		return &Executor::ins_timesequal;
	case TOK_DIVIDEEQUAL:		return &Executor::ins_divideequal;
	case TOK_MODULUSEQUAL:		return &Executor::ins_modulusequal;

    case TOK_LESSTHAN:          return &Executor::ins_lessthan;
    case TOK_LESSEQ:            return &Executor::ins_lessequal;
    case RSV_GOTO:              return &Executor::ins_goto;
    case TOK_ARRAY_SUBSCRIPT:   return &Executor::ins_arraysubscript;
    case TOK_EQUAL:             return &Executor::ins_equal;
    case TOK_FUNC:              return &Executor::ins_func;
    case INS_CALL_METHOD:       return &Executor::ins_call_method;
    case INS_CALL_METHOD_ID:    return &Executor::ins_call_method_id;
    case CTRL_STATEMENTBEGIN:   return &Executor::ins_statementbegin;
    case CTRL_MAKELOCAL:        return &Executor::ins_makelocal;
    case CTRL_JSR_USERFUNC:     return &Executor::ins_jsr_userfunc;
    case INS_POP_PARAM:         return &Executor::ins_pop_param;
    case INS_POP_PARAM_BYREF:   return &Executor::ins_pop_param_byref;
    case INS_GET_ARG:           return &Executor::ins_get_arg;
    case CTRL_LEAVE_BLOCK:      return &Executor::ins_leave_block;
    case RSV_GOSUB:             return &Executor::ins_gosub;
    case RSV_RETURN:            return &Executor::ins_return;
    case RSV_EXIT:              return &Executor::ins_exit;
    case INS_DECLARE_ARRAY:     return &Executor::ins_declareArray;
    case TOK_UNMINUS:           return &Executor::ins_unminus;
    case TOK_UNPLUS:            return &Executor::ins_nop;
    case TOK_LOG_NOT:           return &Executor::ins_logical_not;
    case TOK_BITWISE_NOT:       return &Executor::ins_bitwise_not;
	case TOK_BSRIGHT:           return &Executor::ins_bitshift_right;
	case TOK_BSLEFT:            return &Executor::ins_bitshift_left;
	case TOK_BITAND:            return &Executor::ins_bitwise_and;
    case TOK_BITXOR:            return &Executor::ins_bitwise_xor;
    case TOK_BITOR:             return &Executor::ins_bitwise_or;

    case TOK_NEQ:               return &Executor::ins_notequal;
    case TOK_GRTHAN:            return &Executor::ins_greaterthan;
    case TOK_GREQ:              return &Executor::ins_greaterequal;
    case TOK_AND:               return &Executor::ins_logical_and;
    case TOK_OR:                return &Executor::ins_logical_or;

    case TOK_ADDMEMBER:         return &Executor::ins_addmember;
	case TOK_DELMEMBER:      return &Executor::ins_removemember;
	case TOK_CHKMEMBER:      return &Executor::ins_checkmember;
    case INS_DICTIONARY_ADDMEMBER: return &Executor::ins_dictionary_addmember;
    case TOK_IN:                return &Executor::ins_in;
	case INS_ADDMEMBER2:        return &Executor::ins_addmember2;
    case INS_ADDMEMBER_ASSIGN:  return &Executor::ins_addmember_assign;
    case CTRL_PROGEND:          return &Executor::ins_progend;

    default:     
        throw std::runtime_error( "Undefined execution token " + tostring(token.id) );
        return &Executor::innerExec;

    }

}

void Executor::execInstr()
{
    unsigned onPC = PC;
    try 
    {    // this is really more of a class invariant.
        passert( run_ok_ );
        passert( PC < nLines );
        passert( !error_ );
        passert( !done );

#ifdef NDEBUG
        const Instruction& ins = prog_->instr[ PC ];
#else
        const Instruction& ins = prog_->instr.at( PC );
#endif
	    if (debug_level >= INSTRUCTIONS)
            cout << PC << ": " << ins.token << endl;

        if (debugging_)
        {
            if (debug_state_ == DEBUG_STATE_ATTACHING)
            {
                debug_state_ = DEBUG_STATE_ATTACHED;
                sethalt(true);
                return;
            }
            else if (debug_state_ == DEBUG_STATE_INS_TRACE)
            {
                // let this instruction through.
                debug_state_ = DEBUG_STATE_ATTACHED;
                sethalt(true);
                // but let this instruction execute.
            }
            else if (debug_state_ == DEBUG_STATE_STEP_INTO)
            {
                debug_state_ = DEBUG_STATE_STEPPING_INTO;
                // let this instruction execute.
            }
            else if (debug_state_ == DEBUG_STATE_STEPPING_INTO)
            {
                if (prog_->dbg_ins_statementbegin.size() > PC &&
                    prog_->dbg_ins_statementbegin[PC])
                {
                    tmpbreakpoints_.insert( PC );
                    // and let breakpoint processing catch it below.
                }
            }
            else if (debug_state_ == DEBUG_STATE_STEP_OVER)
            {
                unsigned breakPC = PC+1;
                while (prog_->dbg_ins_statementbegin.size() > breakPC)
                {
                    if (prog_->dbg_ins_statementbegin[breakPC])
                    {
                        tmpbreakpoints_.insert( breakPC );
                        debug_state_ = DEBUG_STATE_RUN;
                        break;
                    }
                    else
                    {
                        ++breakPC;
                    }
                }
            }
            else if (debug_state_ == DEBUG_STATE_RUN)
            {
                // do nothing
            }
            else if (debug_state_ == DEBUG_STATE_BREAK_INTO)
            {
                debug_state_ = DEBUG_STATE_ATTACHED;
                sethalt(true);
                return;
            }

            // check for breakpoints on this instruction
            if ((breakpoints_.count(PC) || tmpbreakpoints_.count(PC)) && bp_skip_ != PC && !halt())
            {
                tmpbreakpoints_.erase( PC );
                bp_skip_ = PC;
                debug_state_ = DEBUG_STATE_ATTACHED;
                sethalt(true);
                return;
            }
            bp_skip_ = ~0;
        }

        ++ins.cycles;
        ++prog_->instr_cycles;
        ++escript_instr_cycles;

        ++PC;

        (this->*(ins.func))(ins);
    }
    catch( exception& ex )
    {
        string err_string = "Exception in: ";
		err_string += prog_->name.c_str();
		err_string += " PC="+onPC;
		err_string += ": ";
		err_string += ex.what();
		err_string += "\n";
		if ( !run_ok_ )
			err_string += "run_ok_ = false\n";
		if ( PC < nLines )
		{
			err_string += " PC < nLines: (";
			err_string += PC+" < ";
			err_string += nLines+") \n";
		}
		if ( error_ )
			err_string += "error_ = true\n";
		if ( done )
			err_string += "done = true\n";

		seterror( true );
		cerr << err_string;
		Log("%s", err_string.c_str());

        show_context( onPC );
    }
#ifdef __unix__
    catch( ... )
    {
        seterror( true );
        cerr << "Exception in " << prog_->name.c_str() << ", PC=" << onPC
             << ": unclassified" << endl;

        Log( "Exception in %s, PC=%d\n",
             prog_->name.c_str(),
             onPC );
        
		show_context( onPC );
    }
#endif
}

string Executor::dbg_get_instruction( unsigned atPC ) const
{
    OSTRINGSTREAM os;
    os << ((atPC==PC)?">":" ") 
       << atPC 
       << (breakpoints_.count(atPC)?"*":":")
       << " " 
       << prog_->instr[ atPC ].token;
    return OSTRINGSTREAM_STR(os);
}

void Executor::show_context( unsigned atPC )
{
    unsigned start, end;
    if (atPC >= 5)
        start = atPC-5;
    else
        start = 0;

    end = atPC+5;

    if (end >= nLines)
        end = nLines-1;
    
    for( unsigned i = start; i <= end; ++i )
    {
        Log( "%d: %s\n", i, dbg_get_instruction(i).c_str() );
    }
}
void Executor::show_context( ostream& os, unsigned atPC )
{
    unsigned start, end;
    if (atPC >= 5)
        start = atPC-5;
    else
        start = 0;

    end = atPC+5;

    if (end >= nLines)
        end = nLines-1;
    
    for( unsigned i = start; i <= end; ++i )
    {
        os << dbg_get_instruction(i) << endl;
    }
}

bool Executor::exec()
{
    passert( prog_ok_ );
    passert( !error_ );

	scripts_thread_script = scriptname();

    while (runnable())
	{
		scripts_thread_scriptPC = PC;
        execInstr();
	}

    return !error_;
}

void Executor::reinitExec()
{
    PC = 0;
    done = 0;
    seterror( false );
    
	while (!ValueStack.empty())
		ValueStack.pop();

    delete Locals2;
    Locals2 = new BObjectRefVec;

    if (!prog_ok_)
    {
        seterror( true );
    }
}

void Executor::initForFnCall( unsigned in_PC )
{
#ifdef MEMORYLEAK
	bool data_shown = false;
#endif

    PC = in_PC;
    done = 0;
    seterror( false );

	while (!ValueStack.empty())
	{
#ifdef MEMORYLEAK
		if (memoryleak_debug)
		{
			if (!data_shown)
			{
				llog << "ValueStack... ";
				data_shown = true;
			}

			ValueStack.top()->impptr()->packonto(llog);
			llog << " [" << ValueStack.top()->impptr()->sizeEstimate() << "] ";
		}
#endif

		ValueStack.pop();
	}

#ifdef MEMORYLEAK
	if (memoryleak_debug)
		if (data_shown)
			llog << " ...deleted" << endl;
#endif

    delete Locals2;
    Locals2 = new BObjectRefVec;
}

void Executor::pushArg( BObjectImp* arg )
{
    passert_always( arg );
    ValueStack.push( BObjectRef(arg) );
}

void Executor::pushArg( const BObjectRef& arg )
{
    ValueStack.push( arg );
}

void Executor::addModule(ExecutorModule *module)
{
    availmodules.push_back(module);
}


ExecutorModule *Executor::findModule( const string& name )
{
	unsigned idx;
	for( idx = 0; idx < availmodules.size(); idx++ )
	{
		ExecutorModule* module = availmodules[ idx ];
		if (stricmp( module->moduleName, name.c_str() ) == 0)
			return module;
	}
	return NULL;
}

void Executor::attach_debugger()
{
    setdebugging(true);
    debug_state_ = DEBUG_STATE_ATTACHING;
}
void Executor::detach_debugger()
{
    setdebugging(false);
    debug_state_ = DEBUG_STATE_NONE;
    sethalt(false);
}
void Executor::dbg_ins_trace()
{
    debug_state_ = DEBUG_STATE_INS_TRACE;
    sethalt(false);
}
void Executor::dbg_step_into()
{
    debug_state_ = DEBUG_STATE_STEP_INTO;
    sethalt(false);
}
void Executor::dbg_step_over()
{
    debug_state_ = DEBUG_STATE_STEP_OVER;
    sethalt(false);
}
void Executor::dbg_run()
{
    debug_state_ = DEBUG_STATE_RUN;
    sethalt(false);
}
void Executor::dbg_break()
{
    debug_state_ = DEBUG_STATE_BREAK_INTO;
}

void Executor::dbg_setbp( unsigned atPC )
{
    breakpoints_.insert( atPC );
}
void Executor::dbg_clrbp( unsigned atPC )
{
    breakpoints_.erase( atPC );
}
void Executor::dbg_clrallbp()
{
    breakpoints_.clear();
}
