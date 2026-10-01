#pragma once

#include <cstddef>
#include <vector>

namespace Pol::Network
{
class Client;
void transmit( Client* client, const void* data, int len );

// Compresses data with the server-to-client Huffman code, ending with the flush code and padded
// to a whole byte. out is resized to the result, which can be up to 11/8 of len.
void huffman_compress( const unsigned char* data, size_t len, std::vector<unsigned char>& out );
}  // namespace Pol::Network
