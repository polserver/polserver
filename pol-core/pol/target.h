/*
History
=======


Notes
=======

*/

#ifndef __TARGET_H
#define __TARGET_H

#include "../clib/rawtypes.h"

#include "pktboth.h"
namespace Pol {
  namespace Mobile {
    class Character;
  }
  namespace Network {
    class Client;
  }
  namespace Core {
	class UObject;

	// The TargetCursor self-registers with the "PKTBI_6C" handler. 

	class TargetCursor
	{
	public:
	  explicit TargetCursor( bool inform_on_cancel );
	  virtual ~TargetCursor() {};

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
	public:
      FullMsgTargetCursor( void( *func )( Mobile::Character*, PKTBI_6C* ) );
	  virtual ~FullMsgTargetCursor() {};

      virtual void on_target_cursor( Mobile::Character* targetter, PKTBI_6C* msg );
	private:
      void( *func )( Mobile::Character* targetter, PKTBI_6C* msg );
	};



	/******************************************************/
	class LosCheckedTargetCursor : public TargetCursor
	{
	public:
      LosCheckedTargetCursor( void( *func )( Mobile::Character*, UObject* ),
							  bool inform_on_cancel = false );
	  virtual ~LosCheckedTargetCursor() {};

      virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg );
	private:
      void( *func )( Mobile::Character*, UObject* targetted );
	};
	/******************************************************/



	/******************************************************/
	class NoLosCheckedTargetCursor : public TargetCursor
	{
	public:
      NoLosCheckedTargetCursor( void( *func )( Mobile::Character*, UObject* ),
								bool inform_on_cancel = false );
	  virtual ~NoLosCheckedTargetCursor() {};

      virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg );
	private:
      void( *func )( Mobile::Character*, UObject* targetted );
	};
	/******************************************************/



	/******************************************************/
	class LosCheckedCoordCursor : public TargetCursor
	{
	public:
      LosCheckedCoordCursor( void( *func )( Mobile::Character*, PKTBI_6C* msg ),
							 bool inform_on_cancel = false );
	  virtual ~LosCheckedCoordCursor() {};

	  bool send_coord_cursor( Network::Client* client );
      virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg );
	private:
      void( *func_ )( Mobile::Character*, PKTBI_6C* );
	};
	/*******************************************************/



	/******************************************************/
	class MultiPlacementCursor : public TargetCursor
	{
	public:
      MultiPlacementCursor( void( *func )( Mobile::Character*, PKTBI_6C* msg ) );
	  virtual ~MultiPlacementCursor() {};

	  virtual void send_placemulti( Network::Client* client, unsigned int objtype, int flags, s16 xoffset, s16 yoffset, u32 hue );
      virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg );
	private:
      void( *func_ )( Mobile::Character*, PKTBI_6C* );
	};
	/*******************************************************/




	/******************************************************/
	/* NoLosCharacterCursor - not used anymore            */
	class NoLosCharacterCursor : public TargetCursor
	{
	public:
      NoLosCharacterCursor( void( *func )( Mobile::Character* targetter, Mobile::Character* targetted ) );
	  virtual ~NoLosCharacterCursor() {};
      virtual void on_target_cursor( Mobile::Character*, PKTBI_6C* msg );
	private:
      void( *func )( Mobile::Character* targetter, Mobile::Character* targetted );
	};
	/******************************************************/




	/******************************************************/
	class NoLosUObjectCursor : public TargetCursor
	{
	public:
      NoLosUObjectCursor( void( *func )( Mobile::Character*, UObject* ),
						  bool inform_on_cancel = false );
	  virtual ~NoLosUObjectCursor() {};
      virtual void on_target_cursor( Mobile::Character* chr, PKTBI_6C* msg );
	private:
      void( *func )( Mobile::Character*, UObject* obj );
	};
	/******************************************************/

  }
}
#endif
