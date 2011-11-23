/*
 * auxclient.h
 *
 *  Created on: Nov 22, 2011
 *      Author: kevin
 */

#ifndef AUXCLIENT_H_
#define AUXCLIENT_H_

class AuxClientThread;

class AuxConnection : public BObjectImp
{
public:
    AuxConnection( AuxClientThread* auxclientthread , string ip) :
        BObjectImp( OTUnknown ),
        _auxclientthread( auxclientthread ),
        _ip( ip )
    {}

    virtual BObjectImp* copy() const;
    virtual bool isTrue() const;
    virtual std::string getStringRep() const;
    virtual unsigned int sizeEstimate() const;

    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
	virtual BObjectRef get_member( const char *membername );

    void disconnect();

private:
    AuxClientThread* _auxclientthread;
    string _ip;
};

class AuxService
{
public:
    AuxService( const Package* pkg, ConfigElem& elem );
    void run();

    const ScriptDef& scriptdef() const { return _scriptdef; }
	std::vector<unsigned int> _aux_ip_match;
    std::vector<unsigned int> _aux_ip_match_mask;
private:
    const Package* _pkg;
    ScriptDef _scriptdef;
    unsigned short _port;
};

class AuxClientThread : public SocketClientThread
{
public:
    AuxClientThread( AuxService* auxsvc, SocketListener& listener );
    AuxClientThread( ScriptDef scriptdef, Socket& sock );

    virtual void run();
    void transmit( const BObjectImp* imp );
    BObjectImp* get_ip();

private:
    bool init();
	bool ipAllowed(sockaddr MyPeer);

	AuxService* _auxservice;
	ScriptDef _scriptdef;
    ref_ptr<AuxConnection> _auxconnection;
    weak_ptr<UOExecutor> _uoexec;
};



#endif /* AUXCLIENT_H_ */
