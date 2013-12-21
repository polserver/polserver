/*
History
=======


Notes
=======

*/
namespace Pol {
  namespace Core {
	class MessageTypeFilter
	{
	public:
	  enum MessageDefault { NormallyDisabled, NormallyEnabled };
	  // terminate exceptions list with -1
	  MessageTypeFilter( MessageDefault def, int *exceptions );

	  unsigned char msgtype_allowed[256];
	};

	extern MessageTypeFilter login_filter;
	extern MessageTypeFilter game_filter;
	extern MessageTypeFilter disconnected_filter;
  }
}