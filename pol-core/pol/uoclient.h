/*
History
=======
2010/02/03 Turley:    MethodScript support for mobiles
2011/12/01 MuadDib:   Changed max_skills to unsigned short. 0xFF max in packets anyway. Probably never need to support over 255 skills anyway.

Notes
=======

*/

#ifndef UOCLIENT_H
#define UOCLIENT_H

#include "crypt/cryptkey.h"
namespace Pol {
  namespace Clib {
	class ConfigElem;
  }
  namespace Core {
	class ExportScript;

	class UoClientGeneral
	{
	public:
	  void check( std::string& var, const char* tag, const char* deflt );

	public:
	  ~UoClientGeneral();

	  class Mapping
	  {
	  public:
		bool any;
		std::string name;
		unsigned id;
	  };

	  Mapping strength;
	  Mapping intelligence;
	  Mapping dexterity;

	  Mapping hits;
	  Mapping stamina;
	  Mapping mana;
	  unsigned short maxskills; //dave changed 3/15/03, support configurable max skillid
	  ExportScript* method_script;
	};

	extern UoClientGeneral uoclient_general;

	class UoClientProtocol
	{
	public:
	  bool EnableFlowControlPackets;
	};
	extern UoClientProtocol uoclient_protocol;

	class UoClientListener
	{
	public:
	  UoClientListener( Clib::ConfigElem& elem );

	  Crypt::TCryptInfo encryption;
	  unsigned short port;
	  bool aosresist;
	  bool sticky;
	};

	typedef vector< UoClientListener > UoClientListeners;

	extern UoClientListeners uoclient_listeners;
  }
}
#endif
