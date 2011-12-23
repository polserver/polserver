/*
History
=======
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../../bscript/bobject.h"
#include "../../bscript/berror.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/impstr.h"

#include "../../clib/cfgelem.h"
#include "../../clib/esignal.h"
#include "../../clib/sckutil.h"
#include "../../clib/socketsvc.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"
#include "../../clib/threadhelp.h"
#include "../../clib/weakptr.h"

#include "../../plib/pkg.h"

#include "../core.h"
#include "../polsem.h"
#include "../scrdef.h"
#include "../scrsched.h"
#include "../sockets.h"
#include "../module/uomod.h"
#include "../uoexec.h"
#include "auxclient.h"

BObjectImp* AuxConnection::copy() const
{
    return const_cast<AuxConnection*>(this);
}

std::string AuxConnection::getStringRep() const
{
    return "<AuxConnection>";
}

size_t AuxConnection::sizeEstimate() const
{
    return sizeof(AuxConnection);
}

bool AuxConnection::isTrue() const
{
    return (_auxclientthread != NULL);
}

BObjectRef AuxConnection::get_member( const char *membername )
{
    if (stricmp( membername, "ip" ) == 0)
    {
        return BObjectRef(new String(_ip));
    }
    return BObjectRef(UninitObject::create());
}

BObjectImp* AuxConnection::call_method( const char* methodname, Executor& ex )
{
    if (stricmp( methodname, "transmit" ) == 0)
    {
        if (ex.numParams()==1)
        {
            if (_auxclientthread != NULL)
            {
                BObjectImp* value = ex.getParamImp(0);
                // FIXME this can block!
                _auxclientthread->transmit( value );
            }
            else
            {
                return new BError( "Client has disconnected" );
            }
        }
        else
        {
            return new BError( "1 parameter expected" );
        }
    }
    return NULL;
}

void AuxConnection::disconnect()
{
    _auxclientthread = NULL;
}

AuxClientThread::AuxClientThread( AuxService* auxsvc, SocketListener& listener ) : 
    SocketClientThread(listener), 
    _auxservice(auxsvc),
    _uoexec(0)
{
}
AuxClientThread::AuxClientThread( ScriptDef scriptdef, Socket& sock ) :
    SocketClientThread(sock),
    _auxservice(0),
    _scriptdef(scriptdef),
    _uoexec(0)
{
}

bool AuxClientThread::init()
{
    PolLock lock;
	struct sockaddr ConnectingIP = _sck.peer_address();
	if (ipAllowed(ConnectingIP))
	{
        _auxconnection.set( new AuxConnection( this, _sck.getpeername() ) );
        UOExecutorModule* uoemod;
        if (_auxservice)
        	uoemod = start_script( _auxservice->scriptdef(), _auxconnection.get() );
        else
        	uoemod = start_script( _scriptdef, _auxconnection.get() );
		_uoexec = uoemod->uoexec.weakptr;
		return true;
	}
	else
	{
		return false;
	}
}

bool AuxClientThread::ipAllowed(sockaddr MyPeer)
{
	if (!_auxservice || _auxservice->_aux_ip_match.empty())
	{
		return true;
	}
	for( unsigned j = 0; j < _auxservice->_aux_ip_match.size(); ++j )
	{
		unsigned int addr1part, addr2part;
		struct sockaddr_in* sockin = reinterpret_cast<struct sockaddr_in*>(&MyPeer);

		addr1part = _auxservice->_aux_ip_match[j]      & _auxservice->_aux_ip_match_mask[j];
#ifdef _WIN32
		addr2part = sockin->sin_addr.S_un.S_addr & _auxservice->_aux_ip_match_mask[j];
#else
		addr2part = sockin->sin_addr.s_addr      & _auxservice->_aux_ip_match_mask[j];
#endif
		if (addr1part == addr2part)
			return true;
	}
    return false;
}
void AuxClientThread::run()
{
    if(!init())
	{
		if (_sck.connected())
		{
            writeline( _sck, "Connection closed" );
			_sck.close();
		}
		_auxconnection.clear();
		return;
	}

    string tmp;
    bool result,timeout_exit;
    for(;;)
    {
        result=readline( _sck, tmp, &timeout_exit, 5 );
        if(!result && !timeout_exit)
            break;

        PolLock lock;

        if (_uoexec.exists())
        {
            if(result)
            {
				istringstream is(tmp);
				BObjectImp* value = _uoexec->auxsvc_assume_string ? new String(tmp) : BObjectImp::unpack( is );

				auto_ptr<BStruct> event (new BStruct);
				event->addMember( "type", new String( "recv" ) );
				event->addMember( "value", value );
				_uoexec->os_module->signal_event( event.release() );
            }
        }
        else
        {   // the controlling script dropped its last reference to the connection,
            // by exiting or otherwise.
            break;
        }
    }
    
    PolLock lock;
    _auxconnection->disconnect();
    // the auxconnection is probably referenced by another ref_ptr, 
    // so its deletion must be protected by the lock.  
    // Clear our reference:
    _auxconnection.clear();
}

void AuxClientThread::transmit( const BObjectImp* value )
{
    string tmp = _uoexec->auxsvc_assume_string ? value->getStringRep() : value->pack() ;
    writeline( _sck, tmp );
}

AuxService::AuxService( const Package* pkg, ConfigElem& elem ) :
    _pkg(pkg),
    _scriptdef( elem.remove_string( "SCRIPT" ), _pkg ),
    _port( elem.remove_ushort( "PORT" ) )
{
	string iptext;
	while (elem.remove_prop( "IPMATCH", &iptext ))
	{
		string::size_type delim = iptext.find_first_of( "/" );
		if (delim != string::npos)
		{
			string ipaddr_str = iptext.substr( 0, delim );
			string ipmask_str = iptext.substr( delim+1 );
			unsigned int ipaddr = inet_addr( ipaddr_str.c_str() );
			unsigned int ipmask = inet_addr( ipmask_str.c_str() );
			_aux_ip_match.push_back( ipaddr );
			_aux_ip_match_mask.push_back( ipmask );
		}
		else
		{
			unsigned int ipaddr = inet_addr( iptext.c_str() );
			_aux_ip_match.push_back( ipaddr );
			_aux_ip_match_mask.push_back( 0xFFffFFffLu );
		}
	}
}

void AuxService::run()
{
    atomic_cout( "Starting Aux Listener (" + _scriptdef.relativename() + ", port " + decint(_port) + ")" );

    SocketListener listener( _port );
    while (!exit_signalled)
    {
        if (listener.GetConnection( 5 ))
        {
			// Shinigami: Just 4 Debugging. We got Crashes here...
            #ifdef PERGON
			cerr << "Aux Listener (" << _scriptdef.relativename() << ", port " << decint(_port) << ") - create Thread" << endl;
            #endif
			SocketClientThread* clientthread = new AuxClientThread( this, listener );
            clientthread->start(); 
            // note SocketClientThread::start deletes the SocketClientThread upon thread exit.
        }
    }
}


typedef vector< AuxService* > AuxServices;
AuxServices auxservices;

void aux_service_thread_stub( void* arg )
{
    AuxService* as = static_cast<AuxService*>(arg);
    as->run();
}

void start_aux_services()
{
    for( unsigned i = 0; i < auxservices.size(); ++i )
    {
        threadhelp::start_thread( aux_service_thread_stub, "AuxService", auxservices[i] );
    }
}

void load_auxservice_entry( const Package* pkg, ConfigElem& elem )
{
    auxservices.push_back( new AuxService(pkg,elem) );
}

void load_aux_services()
{
    load_packaged_cfgs( "auxsvc.cfg", "AuxService", load_auxservice_entry );
}

void unload_aux_services()
{
    delete_all( auxservices );
}

