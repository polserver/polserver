/*
History
=======


Notes
=======

*/

#ifndef POLVAR_H
#define POLVAR_H
namespace Pol {
  namespace Core {
	class PolVar
	{
	public:
	  unsigned short DataWrittenBy;
	};

	extern PolVar polvar;
	extern unsigned int stored_last_item_serial;
	extern unsigned int stored_last_char_serial;
  }
}
#endif
