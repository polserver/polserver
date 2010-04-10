/*
History
=======
2005/01/24 Shinigami: added get-/setspyonclient2 to support packet 0xd9 (Spy on Client 2)
2005/04/03 Shinigami: added UOExpansionFlag for Samurai Empire
2005/08/29 Shinigami: character.spyonclient2 renamed to character.clientinfo
                      get-/setspyonclient2 renamed to get-/setclientinfo
2006/05/16 Shinigami: added UOExpansionFlag for Mondain's Legacy
                      added GENDER/RACE flag (e.g. used inside ClientCreateChar())
2007/07/09 Shinigami: added isUOKR [bool] - UO:KR client used?
2009/08/10 MuadDib:   Added CLIENT_VER_50000 for v5.0.0x clients.
2009/08/19 Turley:    Added u32 UOExpansionFlagClient
2009/09/06 Turley:    Added u8 ClientType + FlagEnum
                      Removed is*
2009/12/02 Turley:    added SA expansion - Tomi
2009/12/04 Turley:    Crypto cleanup - Tomi
2010/01/22 Turley:    Speedhack Prevention System

Notes
=======

*/

#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdio.h> // for that FILE fpLog down there :(
#include <memory>
#include <string>

#include "../../clib/rawtypes.h"
#include "../../clib/wallclock.h"

#include "../../bscript/bstruct.h"

#include "../pktin.h"
#include "../sockets.h"
#include "../ucfg.h"
#include "../crypt/cryptengine.h"
#include "../crypt/cryptkey.h"

class MessageTypeFilter;
class Account;
class Character;
class UContainer;
struct XmitBuffer;
class ClientGameData;
class ClientInterface;
class UOClientInterface;

extern UOClientInterface uo_client_interface;

const u16 T2A = 0x01;
const u16 LBR = 0x02;
const u16 AOS = 0x04;
const u16 SE  = 0x08; // set AOS-Flag in send_feature_enable() too for needed checks
const u16 ML  = 0x10; // set SE- and AOS-Flag in send_feature_enable() too for needed checks
const u16 KR  = 0x20; // set KR- and ML- and SE- and AOS-Flag in send_feature_enable() too for needed checks
const u16 SA  = 0x40; // set SA- and KR- and SE- and AOS-Flag in send_feature_enable() too for needed checks

const u8 FLAG_GENDER = 0x01;
const u8 FLAG_RACE   = 0x02;


struct VersionDetailStruct
{
	int major;
    int minor;
	int rev;
	int patch;
};

const struct VersionDetailStruct CLIENT_VER_4000 ={4,0,0,0};
const struct VersionDetailStruct CLIENT_VER_4070 ={4,0,7,0};
const struct VersionDetailStruct CLIENT_VER_5000 ={5,0,0,0};
const struct VersionDetailStruct CLIENT_VER_5020 ={5,0,2,0};
const struct VersionDetailStruct CLIENT_VER_6017 ={6,0,1,7};
const struct VersionDetailStruct CLIENT_VER_60142={6,0,14,2};
const struct VersionDetailStruct CLIENT_VER_7000 ={7,0,0,0};

enum ClientTypeFlag
{
    CLIENTTYPE_4000  = 0x1,  // 4.0.0a   (new spellbookcontent packet 0xbf:0x1b)
    CLIENTTYPE_4070  = 0x2,  // 4.0.7a   (new damage packet 0x0b instead of 0xbf:0x22)
	CLIENTTYPE_5000  = 0x4,  // 5.0.0a   (compressed gumps)
    CLIENTTYPE_5020  = 0x8,  // 5.0.2a   (Buff/Debuff 0xdf)
	CLIENTTYPE_6017  = 0x10, // 6.0.1.7  (Grid locs)
	CLIENTTYPE_60142 = 0x20, // 6.0.14.2 (feature enable 0xb9 size change)
	CLIENTTYPE_UOKR  = 0x40,
    CLIENTTYPE_7000  = 0x80, // 7.0.0.0  (Gargoyle race)
	CLIENTTYPE_UOSA  = 0x100
};

typedef struct
{
	unsigned char pktbuffer[PKTIN_02_SIZE];
}PacketThrottler;

class Client : public ref_counted
{
public:
	Client( ClientInterface& aInterface, TCryptInfo& encryption );
    static void Delete( Client* client );
    friend class GCCHelper;
	virtual ~Client();

private:
    void PreDelete();

public:
    void Disconnect();
    void transmit( const void *data, int len ); // for entire message or header only
    void transmitmore( const void *data, int len ); // for stuff after a header

    void recv_remaining( int total_expected );
    void recv_remaining_nocrypt( int total_expected );

    void setversion( const std::string& ver ) { version_ = ver; }
    const std::string& getversion() const { return version_; }
	VersionDetailStruct getversiondetail() const { return versiondetail_; }
	void setversiondetail( VersionDetailStruct& detail ) { versiondetail_ = detail; }
	void itemizeclientversion( const std::string& ver, VersionDetailStruct& detail );
	bool compareVersion( const std::string& ver );
	bool compareVersion(const VersionDetailStruct& ver2);
	void setClientType(ClientTypeFlag type);

    void setclientinfo( const PKTIN_D9 *msg ) { memcpy( &clientinfo_, msg, sizeof(clientinfo_) ); }
    BStruct* getclientinfo() const;

    Account* acct;
	Character* chr;
    ClientInterface& Interface;

	bool ready;			// all initialization stuff has been sent, ready for general use.


//
    bool have_queued_data() const;
	void send_queued_data();

	SOCKET csocket;		// socket to client ACK  - requires header inclusion.
    unsigned short listen_port;
	bool aosresist; // UOClient.Cfg Entry

	bool disconnect;		// if 1, disconnect this client

	enum e_recv_states {
			RECV_STATE_CRYPTSEED_WAIT,
			RECV_STATE_MSGTYPE_WAIT,
			RECV_STATE_MSGLEN_WAIT,
			RECV_STATE_MSGDATA_WAIT
	} recv_state;

    unsigned char bufcheck1_AA;
	unsigned char buffer[ MAXBUFFER ];
	unsigned char xoutbuffer[ 0xFFFF ]; // moved from global transmit_encrypted (threadsafe)
    unsigned char bufcheck2_55;
	unsigned int bytes_received;		// how many bytes have been received into the buffer.
	unsigned int message_length;		// how many bytes are expected for this message

	struct sockaddr ipaddr;

    std::auto_ptr<CCryptBase> cryptengine;

	bool encrypt_server_stream;				// encrypt the server stream (data sent to client)?

	const MessageTypeFilter *msgtype_filter;

    FILE* fpLog;

    std::string status() const;

    void send_pause(bool bForce = false);
    void send_restart(bool bForce = false);

    void pause();
	void restart();
    void restart2();
	int pause_count;

    std::string ipaddrAsString() const;

	bool SpeedHackPrevention(bool add = true);
	BObjectImp* make_ref();

protected:

	XmitBuffer *first_xmit_buffer;
	XmitBuffer *last_xmit_buffer;
	int n_queued;
    int queued_bytes_counter; // only used for monitoring

	// we may want to track how many bytes total are outstanding,
	// and boot clients that are too far behind.
	void queue_data( const void *data, unsigned short datalen );
    void transmit_encrypted( const void *data, int len );
	void xmit( const void *data, unsigned short datalen );

public:
    ClientGameData* gd;
    unsigned int instance_;
    static unsigned int instance_counter_;
    int checkpoint;//CNXBUG
    unsigned char last_msgtype;
    int thread_pid;
    u16 UOExpansionFlag;
	u32 UOExpansionFlagClient;
	u16 ClientType;
	queue<PacketThrottler> movementqueue;
	wallclock_t next_movement;
	
private:
    struct {
        unsigned int bytes_transmitted;
        unsigned int bytes_received;
    } counters;
    std::string version_;
    PKTIN_D9 clientinfo_;
    bool paused_;
    VersionDetailStruct versiondetail_;
    // hidden:
    Client( const Client& x );
    Client& operator=( const Client& x );
};

inline bool Client::have_queued_data() const
{
	return (first_xmit_buffer != NULL);
}

#endif
