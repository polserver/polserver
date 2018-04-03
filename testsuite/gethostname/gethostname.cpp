#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main( int argc, char* argv[] )
{
    if (argc <= 1)
    {
        cerr << "Usage: gethostname [hostname]" << endl;
        return 1;
    }


    char* hostname = argv[1];

    struct hostent host_ret;
    struct hostent* host_result = NULL;
    char tmp_buf[ 1024 ];
    int my_h_errno;
    int res = gethostbyname_r( hostname, &host_ret, tmp_buf,
                               sizeof tmp_buf, &host_result, &my_h_errno );
    if (res == 0 && host_result && host_result->h_addr_list[0])
    {
        char* addr = host_result->h_addr_list[0];
        char* addrstr = inet_ntoa( * (struct in_addr *) addr );

        cout << "Address: " << addrstr << endl;
    }
    else
    {
        cout << "Warning: gethostbyname_r failed for server"
              << hostname << ": "
                     << my_h_errno << endl;
    }
    return 0;
}
