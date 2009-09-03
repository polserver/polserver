/*
History
=======


Notes
=======

*/

#ifndef __TARGET_H
#define __TARGET_H

#include "clib/rawtypes.h"

#include "pktboth.h"

class Character;
class Client;
class UObject;

// The TargetCursor self-registers with the "PKTBI_6C" handler. 
 
class TargetCursor
{
public:
	explicit TargetCursor( bool inform_on_cancel );
	virtual ~TargetCursor() {};

    bool send_object_cursor( Client* client,
        PKTBI_6C::CURSOR_TYPE crstype = PKTBI_6C::CURSOR_TYPE_NEUTRAL );
    
    void cancel(Character* chr);


    void handle_target_cursor( Character* targetter, PKTBI_6C* msg );
    virtual void on_target_cursor( Character* targetter, PKTBI_6C* msg ) = 0;
	u32 cursorid_;
	
protected:
    bool inform_on_cancel_;
};

class FullMsgTargetCursor : public TargetCursor
{
public:
    FullMsgTargetCursor( void (*func)(Character*, PKTBI_6C*) );
	virtual ~FullMsgTargetCursor() {};

    virtual void on_target_cursor( Character* targetter, PKTBI_6C* msg );
private:
    void (*func)(Character* targetter, PKTBI_6C* msg);
};



/******************************************************/
class LosCheckedTargetCursor : public TargetCursor
{
public:
    LosCheckedTargetCursor( void (*func)(Character*, UObject*),
                            bool inform_on_cancel = false );
	virtual ~LosCheckedTargetCursor() {};

    virtual void on_target_cursor( Character*, PKTBI_6C* msg );
private:
    void (*func)(Character*, UObject* targetted);
};
/******************************************************/



/******************************************************/
class NoLosCheckedTargetCursor : public TargetCursor
{
public:
    NoLosCheckedTargetCursor( void (*func)(Character*, UObject*),
                            bool inform_on_cancel = false );
	virtual ~NoLosCheckedTargetCursor() {};

    virtual void on_target_cursor( Character*, PKTBI_6C* msg );
private:
    void (*func)(Character*, UObject* targetted);
};
/******************************************************/



/******************************************************/
class LosCheckedCoordCursor : public TargetCursor
{
public:
    LosCheckedCoordCursor( void (*func)(Character*,PKTBI_6C* msg),
                           bool inform_on_cancel = false );
	virtual ~LosCheckedCoordCursor() {};

    bool send_coord_cursor( Client* client ); 
    virtual void on_target_cursor( Character*, PKTBI_6C* msg );
private:
    void (*func_)(Character*,PKTBI_6C*);
};
/*******************************************************/



/******************************************************/
class MultiPlacementCursor : public TargetCursor
{
public:
    MultiPlacementCursor( void (*func)(Character*,PKTBI_6C* msg) );
	virtual ~MultiPlacementCursor() {};

    virtual void send_placemulti( Client* client, unsigned short objtype, long flags, s16 xoffset, s16 yoffset ); 
    virtual void on_target_cursor( Character*, PKTBI_6C* msg );
private:
    void (*func_)(Character*,PKTBI_6C*);
};
/*******************************************************/




/******************************************************/
/* NoLosCharacterCursor - not used anymore            */
class NoLosCharacterCursor : public TargetCursor
{
public:
    NoLosCharacterCursor( void (*func)(Character* targetter, Character* targetted) );
	virtual ~NoLosCharacterCursor() {};
    virtual void on_target_cursor( Character*, PKTBI_6C* msg );
private:
    void (*func)(Character* targetter, Character* targetted);
};
/******************************************************/




/******************************************************/
class NoLosUObjectCursor : public TargetCursor
{
public:
    NoLosUObjectCursor( void (*func)(Character*, UObject*),
                        bool inform_on_cancel = false );
	virtual ~NoLosUObjectCursor() {};
    virtual void on_target_cursor( Character* chr, PKTBI_6C* msg );
private:
    void (*func)(Character*, UObject* obj);
};
/******************************************************/


#endif
