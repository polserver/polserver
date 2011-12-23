/*
History
=======
2005/12/07 MuadDib: Added "Recompile required" to Bad version reports.

Notes
=======

*/



#include "../clib/stl_inc.h"

#include <stdio.h>

#include "../clib/strutil.h"

#include "filefmt.h"

#include "eprog.h"
#include "escriptv.h"
#include "options.h"
#include "executor.h"
#include "objmethods.h"

int EScriptProgram::read(const char *fname)
{
    FILE* fp = NULL;

    try
    {
        name = fname;

        fp = fopen(fname, "rb");
	    if (!fp)
            throw runtime_error( string("Unable to open ") + fname + " for reading." );

	    BSCRIPT_FILE_HDR hdr;
        if (fread( &hdr, sizeof hdr, 1, fp ) != 1)
	    {
            cerr << "Error loading script " << fname
                 << ": error reading header" << endl;
		    fclose(fp);
		    return -1;
	    }
	    if (hdr.magic2[0] != BSCRIPT_FILE_MAGIC0 ||
            hdr.magic2[1] != BSCRIPT_FILE_MAGIC1)
	    {
            cerr << "Error loading script " << fname 
                 << ": bad magic value '" << hdr.magic2[0] << hdr.magic2[1] << "'" << endl;
		    fclose(fp);
		    return -1;
	    }
		// auto-check for latest version (see filefmt.h for setting)
	    if (hdr.version != ESCRIPT_FILE_VER_CURRENT)
	    {
            cerr << "Error loading script " << fname 
                 << ": Recompile required. Bad version number " << hdr.version << endl;
		    fclose(fp);
		    return -1;
	    }
        version = hdr.version;
        nglobals = hdr.globals;
	    BSCRIPT_SECTION_HDR sechdr;
	    while (fread( &sechdr, sizeof sechdr, 1, fp ) == 1)
	    {
		    switch( sechdr.type )
		    {
                case BSCRIPT_SECTION_PROGDEF:
                    if (read_progdef_hdr(fp))
                    {
                        cerr << "Error loading script " << fname
                             << ": error reading progdef section" << endl;
                        fclose(fp);
                        return -1;
                    }
                    break;

			    case BSCRIPT_SECTION_MODULE:
				    if (read_module(fp))
				    {
                        cerr << "Error loading script " << fname
                             << ": error reading module section" << endl;
					    fclose(fp);
					    return -1;
				    }
				    break;
			    case BSCRIPT_SECTION_CODE:
				    tokens.read(fp);
				    break;
			    case BSCRIPT_SECTION_SYMBOLS:
				    symbols.read(fp);
				    break;
                case BSCRIPT_SECTION_GLOBALVARNAMES:
                    if (read_globalvarnames(fp))
                    {
                        cerr << "Error loading script " << fname
                             << ": error reading global variable name section" << endl;
					    fclose(fp);
					    return -1;
                    }
                    break;
                case BSCRIPT_SECTION_EXPORTED_FUNCTIONS:
                    if (read_exported_functions(fp, &sechdr))
                    {
                        cerr << "Error loading script " << fname
                             << ": error reading exported functions section" << endl;
                        fclose(fp);
                        return -1;
                    }
                    break;
			    default:
                        cerr << "Error loading script " << fname
                             << ": unknown section type " << int(sechdr.type) << endl;
				    fclose(fp);
				    return -1;
		    }
	    }
        fclose(fp);

        return create_instructions();
    }
    catch( std::exception& ex )
    {
        cerr << "Exception caught while loading script " << fname << ": " << ex.what() << endl;
        if (fp != NULL)
            fclose(fp);
        return -1;
    }
#ifndef WIN32
    catch(...)
    {
        cerr << "Exception caught while loading script " << fname << endl;
        if (fp != NULL)
            fclose(fp);
        return -1;
    }
#endif
}
int EScriptProgram::create_instructions()
{
        int nLines = tokens.length() / sizeof(StoredToken);
	    instr.resize( nLines ); // = new Instruction[ nLines ];

        for( int i = 0; i < nLines; i++ )
	    {
            Instruction& ins = instr[i];
		    if (_readToken( ins.token, i ))
                return -1;

            // executor only:
            ins.func = Executor::GetInstrFunc( ins.token );
        }
        return 0;
}

int EScriptProgram::read_progdef_hdr( FILE *fp )
{
    BSCRIPT_PROGDEF_HDR hdr;
    if (fread( &hdr, sizeof hdr, 1, fp ) != 1)
        return -1;
 
    haveProgram = true;
    expectedArgs = hdr.expectedArgs;
    return 0;
}

int EScriptProgram::read_module( FILE *fp )
{
	BSCRIPT_MODULE_HDR hdr;
	if (fread( &hdr, sizeof hdr, 1, fp ) != 1)
		return -1;
	FunctionalityModule *fm = new FunctionalityModule( hdr.modulename );
	for( unsigned i = 0; i < hdr.nfuncs; i++ )
	{
		BSCRIPT_MODULE_FUNCTION func;
		if (fread( &func, sizeof func, 1, fp ) != 1)
		{
			delete fm;
			return -1;
		}
		fm->addFunction( func.funcname, func.nargs );
	}
	modules.push_back( fm );
	return 0;
}

/* Note: This function is ONLY used from Executor::read(). */
int EScriptProgram::_readToken(Token& token, unsigned position) const
{
    StoredToken st;
    tokens.atGet1( position, st );
    // StoredToken& st = tokens[position];

    token.module = (ModuleID) st.module;
    token.id = static_cast<BTokenId>(st.id);
    token.type = static_cast<BTokenType>(st.type);
    token.lval = st.offset;

    token.nulStr();

    // FIXME: USED to set lval to 0 for TYP_FUNC.  Not sure if needed anymore.
    // FIXME: Doesn't seem to be, but must be sure before removal
    switch( st.id )
    {
        case INS_CASEJMP:
            if (st.offset >= symbols.length())
            {
                throw runtime_error( "Symbol offset of " + decint(st.offset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
            }
            token.dataptr = reinterpret_cast<const unsigned char*>(symbols.array()+st.offset);
            return 0;
        case TOK_LONG:
            if (st.offset >= symbols.length())
            {
                throw runtime_error( "Symbol offset of " + decint(st.offset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
            }
            token.lval = * (int *) (symbols.array()+st.offset);
            return 0;
        case TOK_DOUBLE:
            if (st.offset >= symbols.length())
            {
                throw runtime_error( "Symbol offset of " + decint(st.offset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
            }
            token.dval = * (double *) (symbols.array()+st.offset);
            return 0;

        case CTRL_STATEMENTBEGIN:
            if (st.offset) 
            {
                if (st.offset >= symbols.length())
                {
                    throw runtime_error( "Symbol offset of " + decint(st.offset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
                    return -1;
                }
                DebugToken *dt = (DebugToken *) (symbols.array()+st.offset);
                token.sourceFile = dt->sourceFile;
                token.lval = dt->offset;

                if (dt->strOffset >= symbols.length())
                {
                    throw runtime_error( "Symbol offset of " + decint(dt->strOffset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
                    return -1;
                }
                if (dt->strOffset) token.setStr(symbols.array() + dt->strOffset);
            }
            return 0;

        case INS_INITFOREACH:
        case INS_STEPFOREACH:
        case INS_INITFOR:
        case INS_NEXTFOR:
        case TOK_GLOBALVAR:
        case TOK_LOCALVAR:
        case CTRL_JSR_USERFUNC:
        case CTRL_LEAVE_BLOCK:
        case TOK_ARRAY_SUBSCRIPT:
        case RSV_JMPIFFALSE:
        case RSV_JMPIFTRUE:
        case RSV_GOTO:
        case RSV_LOCAL:
        case RSV_GLOBAL:
        case INS_ASSIGN_GLOBALVAR:
        case INS_ASSIGN_LOCALVAR:
        case INS_GET_MEMBER_ID:
        case INS_SET_MEMBER_ID:
        case INS_SET_MEMBER_ID_CONSUME:
        case INS_CALL_METHOD_ID:
		    token.lval = st.offset;
		    return 0;

        case INS_CALL_METHOD:
        case TOK_FUNC:
        case TOK_USERFUNC:
            token.lval = st.type;
            token.type = (st.id == TOK_FUNC) ? TYP_FUNC : TYP_USERFUNC;
            if (st.offset)
            {
                if (st.offset >= symbols.length())
                {
                    throw runtime_error( "Symbol offset of " + decint(st.offset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
                }
                token.setStr(symbols.array()+st.offset);
            }
            else if (token.type == TYP_FUNC)
            {
                FunctionalityModule* modl = modules[token.module];
                const ModuleFunction* modfunc;

                // executor only:
                modfunc = modl->functions.at( token.lval );

                token.setStr( modfunc->name.c_str() );
            }
            return 0;

        default:
          if (st.offset) 
          {
              if (st.offset >= symbols.length())
              {
                  throw runtime_error( "Symbol offset of " + decint(st.offset) + " exceeds symbol store length of " + decint(symbols.length()) + " at PC=" + decint(position));
              }
              token.setStr(symbols.array()+st.offset);
          }
          return 0;
    }
}

int EScriptProgram::read_globalvarnames( FILE* fp )
{
    BSCRIPT_GLOBALVARNAMES_HDR hdr;
    if (fread( &hdr, sizeof hdr, 1, fp ) != 1)
        return -1;
    int res = 0;
    unsigned bufalloc = 20;
    char* buffer = new char[bufalloc];
    for( unsigned idx = 0; idx < hdr.nGlobalVars; ++idx )
    {
        BSCRIPT_GLOBALVARNAME_HDR ghdr;
        if (fread( &ghdr, sizeof ghdr, 1, fp ) != 1)
        {
            res = -1;
            break;
        }
        if (ghdr.namelen >= bufalloc)
        {
            bufalloc = ghdr.namelen + 5;
            delete[] buffer;
            buffer = new char[bufalloc];
        }
        if (fread( buffer, ghdr.namelen+1, 1, fp ) != 1)
        {
            res = -1;
            break;
        }
        globalvarnames.push_back( string(buffer) );
    }
    delete[] buffer;
    buffer = NULL;
    return res;
}

int EScriptProgram::read_exported_functions( FILE* fp, BSCRIPT_SECTION_HDR* hdr )
{
    BSCRIPT_EXPORTED_FUNCTION bef;
    ObjMethod* mth;

    unsigned nexports = hdr->length / sizeof bef;
    while (nexports--)
    {
        if (fread( &bef, sizeof bef, 1, fp ) != 1)
            return -1;
        EPExportedFunction ef;
        ef.name = bef.funcname;
        ef.nargs = bef.nargs;
        ef.PC = bef.PC;
        exported_functions.push_back( ef );
        if( (mth = getKnownObjMethod(ef.name.c_str())) != NULL )
            mth->overridden = true;
    }
    return 0;
}

int EScriptProgram::read_dbg_file()
{
    if (debug_loaded)
        return 0;

    string mname = name;
    mname.replace( mname.size()-3, 3, "dbg" );
    FILE* fp = fopen( mname.c_str(), "rb" );
    if (!fp)
    {
        cerr << "Unable to open " << mname << endl;
        return -1;
    }

    u32 version;
    size_t fread_res=fread( &version, sizeof version, 1, fp );
    if (version != 2 && version != 3)
    {
        cerr << "Recompile required. Bad version " << version << " in " << mname << ", expected version 2" << endl;
        fclose(fp);
        return -1;
    }
    
    size_t bufalloc = 20;
    char* buffer = new char[bufalloc];
    int res = 0;

    u32 count;
    fread_res=fread( &count, sizeof count, 1, fp );
    dbg_filenames.resize( count );
    for( unsigned i = 0; i < dbg_filenames.size(); ++i )
    {
        fread_res=fread( &count, sizeof count, 1, fp );
        if (count >= bufalloc)
        {
            delete[] buffer;
            bufalloc = count*2;
            buffer = new char[bufalloc];
        }
        fread_res=fread( buffer, count, 1, fp );
        dbg_filenames[i] = buffer;
    }

    fread_res=fread( &count, sizeof count, 1, fp );
    globalvarnames.resize( count );
    for( unsigned i = 0; i < globalvarnames.size(); ++i )
    {
        fread_res=fread( &count, sizeof count, 1, fp );
        if (count >= bufalloc)
        {
            delete[] buffer;
            bufalloc = count*2;
            buffer = new char[bufalloc];
        }
        fread_res=fread( buffer, count, 1, fp );
        globalvarnames[i] = buffer;
    }

    fread_res=fread( &count, sizeof count, 1, fp );
    dbg_filenum.resize( count );
    dbg_linenum.resize( count );
    dbg_ins_blocks.resize( count );
    dbg_ins_statementbegin.resize( count );
    for( unsigned i = 0; i < tokens.count(); ++i )
    {
        BSCRIPT_DBG_INSTRUCTION ins;
        fread_res=fread( &ins, sizeof ins, 1, fp );
        dbg_filenum[i] = ins.filenum;
        dbg_linenum[i] = ins.linenum;
        dbg_ins_blocks[i] = ins.blocknum;
        dbg_ins_statementbegin[i] = ins.statementbegin?true:false;
    }
    fread_res=fread( &count, sizeof count, 1, fp );
    blocks.resize( count );
    for( unsigned i = 0; i < blocks.size(); ++i )
    {
        EPDbgBlock& block = blocks[i];
        u32 tmp;
        
        fread_res=fread( &tmp, sizeof tmp, 1, fp );
        block.parentblockidx = tmp;

        fread_res=fread( &tmp, sizeof tmp, 1, fp );
        block.parentvariables = tmp;

        fread_res=fread( &tmp, sizeof tmp, 1, fp );
        block.localvarnames.resize( tmp );

        for( unsigned j = 0; j < block.localvarnames.size(); ++j )
        {
            fread_res=fread( &count, sizeof count, 1, fp );
            if (count >= bufalloc)
            {
                delete[] buffer;
                bufalloc = count*2;
                buffer = new char[bufalloc];
            }
            fread_res=fread( buffer, count, 1, fp );
            block.localvarnames[j] = buffer;
        }
    }
    if (version >= 3)
    {
        fread_res=fread( &count, sizeof count, 1, fp );
        dbg_functions.resize( count );
        for( unsigned i = 0; i < dbg_functions.size(); ++i )
        {
            EPDbgFunction& func = dbg_functions[i];
            u32 tmp;
            fread_res=fread( &tmp, sizeof tmp, 1, fp );
            if (tmp >= bufalloc)
            {
                delete[] buffer;
                bufalloc = tmp*2;
                buffer = new char[bufalloc];
            }
            fread_res=fread( buffer, tmp, 1, fp );
            func.name = buffer;

            fread_res=fread( &tmp, sizeof tmp, 1, fp );
            func.firstPC = tmp;
            fread_res=fread( &tmp, sizeof tmp, 1, fp );
            func.lastPC = tmp;
        }
    }
    
    delete[] buffer;
    buffer = NULL;

    fclose(fp);
    debug_loaded = true;
	if (fread_res) // FIXME senseless check so fread_res is used
		res=0;
    return res;
}
