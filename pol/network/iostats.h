/*
History
=======


Notes
=======

*/

class IOStats
{
public:
    IOStats();

    struct Packet {
        unsigned long count;
        unsigned long bytes;
    };
    
    Packet sent[ 256 ];
    Packet received[ 256 ];
};

extern IOStats iostats;
extern IOStats queuedmode_iostats;
