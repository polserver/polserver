/** @file
 *
 * @par History
 */


#ifndef __TARGET_H
#define __TARGET_H

#include <array>

#include "../clib/compilerspecifics.h"
#include "../clib/rawtypes.h"
#include "pktboth.h"

namespace Pol
{
namespace Mobile
{
class Character;
}
namespace Network
{
class Client;
}
namespace Core
{
class UObject;

// The TargetCursor self-registers with the "PKTBI_6C" handler.

class TargetCursor
{
  friend struct Cursors;

protected:
  explicit TargetCursor( bool inform_on_cancel );

public:
  virtual ~TargetCursor(){};

  bool send_object_cursor( Network::Client* client,
                           PKTBI_6C::CURSOR_TYPE crstype = PKTBI_6C::CURSOR_TYPE_NEUTRAL );

  void cancel( Mobile::Character* chr );


  void handle_target_cursor( Mobile::Character* targetter, PKTBI_6C* msg );
  virtual void on_target_cursor( Mobile::Character* targetter, PKTBI_6C* msg ) = 0;
  u32 cursorid_;

protected:
  bool inform_on_cancel_;
};

class FullMsgTargetCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  FullMsgTargetCursor( void ( *func )( Mobile::Character*, PKTBI_6C* ) );

public:
  virtual ~FullMsgTargetCursor(){};

  virtual void on_target_cursor( Mobile::Character* targetter, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func )( Mobile::Character* targetter, PKTBI_6C* msg );
};


/******************************************************/
class LosCheckedTargetCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  LosCheckedTargetCursor( void ( *func )( Mobile::Character*, UObject* ),
                          bool inform_on_cancel = false );

public:
  virtual ~LosCheckedTargetCursor(){};

  virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func )( Mobile::Character*, UObject* targetted );
};
/******************************************************/


/******************************************************/
class NoLosCheckedTargetCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  NoLosCheckedTargetCursor( void ( *func )( Mobile::Character*, UObject* ),
                            bool inform_on_cancel = false );

public:
  virtual ~NoLosCheckedTargetCursor(){};

  virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func )( Mobile::Character*, UObject* targetted );
};
/******************************************************/


/******************************************************/
class LosCheckedCoordCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  LosCheckedCoordCursor( void ( *func )( Mobile::Character*, PKTBI_6C* msg ),
                         bool inform_on_cancel = false );

public:
  virtual ~LosCheckedCoordCursor(){};

  bool send_coord_cursor( Network::Client* client );
  virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func_ )( Mobile::Character*, PKTBI_6C* );
};
/*******************************************************/


/******************************************************/
class MultiPlacementCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  MultiPlacementCursor( void ( *func )( Mobile::Character*, PKTBI_6C* msg ) );

public:
  virtual ~MultiPlacementCursor(){};

  void send_placemulti( Network::Client* client, unsigned int objtype, int flags, s16 xoffset,
                        s16 yoffset, u32 hue );
  virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func_ )( Mobile::Character*, PKTBI_6C* );
};
/*******************************************************/


/******************************************************/
/* NoLosCharacterCursor - not used anymore            */
class NoLosCharacterCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  NoLosCharacterCursor( void ( *func )( Mobile::Character* targetter,
                                        Mobile::Character* targetted ) );

public:
  virtual ~NoLosCharacterCursor(){};
  virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func )( Mobile::Character* targetter, Mobile::Character* targetted );
};
/******************************************************/


/******************************************************/
class NoLosUObjectCursor : public TargetCursor
{
  friend struct Cursors;

protected:
  NoLosUObjectCursor( void ( *func )( Mobile::Character*, UObject* ),
                      bool inform_on_cancel = false );

public:
  virtual ~NoLosUObjectCursor(){};
  virtual void on_target_cursor( Mobile::Character* chr, PKTBI_6C* msg ) POL_OVERRIDE;

private:
  void ( *func )( Mobile::Character*, UObject* obj );
};
/******************************************************/

}  // namespace Core
namespace Module
{
void handle_script_cursor( Mobile::Character* chr, Core::UObject* obj );
void handle_coord_cursor( Mobile::Character* chr, Core::PKTBI_6C* msg );
}
namespace Core
{
void handle_add_member_cursor( Mobile::Character* chr, PKTBI_6C* msgin );
void handle_remove_member_cursor( Mobile::Character* chr, PKTBI_6C* msgin );
void handle_ident_cursor( Mobile::Character* chr, PKTBI_6C* msgin );
void start_packetlog( Mobile::Character* looker, Mobile::Character* mob );
void stop_packetlog( Mobile::Character* looker, Mobile::Character* mob );
void show_repdata( Mobile::Character* looker, Mobile::Character* mob );

struct Cursors
{
private:
  // Every TargetCursor will be statically created once,
  // and gets a id aka index in array to find the corresponding cursor
  // from the client pkt. Sounds a bit wierd?
  std::array<TargetCursor*, 10> _target_cursors;
  u32 _cursorid_count;

  friend class TargetCursor;
  friend void handle_target_cursor( Network::Client* client, PKTBI_6C* msg );
  friend class GameState;

protected:
  Cursors();

public:
  LosCheckedTargetCursor los_checked_script_cursor;
  NoLosCheckedTargetCursor nolos_checked_script_cursor;

  FullMsgTargetCursor add_member_cursor;
  FullMsgTargetCursor remove_member_cursor;

  FullMsgTargetCursor ident_cursor;
  LosCheckedCoordCursor script_cursor2;
  MultiPlacementCursor multi_placement_cursor;

  NoLosCharacterCursor repdata_cursor;
  NoLosCharacterCursor startlog_cursor;
  NoLosCharacterCursor stoplog_cursor;
};
}
}
#endif
