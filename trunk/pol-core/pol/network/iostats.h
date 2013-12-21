/*
History
=======


Notes
=======

*/
namespace Pol {
  namespace Network {
	class IOStats
	{
	public:
	  IOStats();

	  struct Packet
	  {
		unsigned int count;
		unsigned int bytes;
	  };

	  Packet sent[256];
	  Packet received[256];
	};

	extern IOStats iostats;
	extern IOStats queuedmode_iostats;
  }
}