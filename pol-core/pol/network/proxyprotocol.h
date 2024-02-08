/** @file
*
 * @par History
 * - 2024/02/07 Kukkino:  added proxy protocol v2 support
 */

#ifndef PROXYPROTOCOL_H
#define PROXYPROTOCOL_H

/*
 * The connection was established on purpose by the proxy
 * without being relayed. The connection endpoints are the sender and the
 * receiver. Such connections exist when the proxy sends health-checks to the
 * server. The receiver must accept this connection as valid and must use the
 * real connection endpoints and discard the protocol block including the
 * family which is ignored.
 */
#define PP_CMD_LOCAL 0x0

/*
 * The connection was established on behalf of another node,
 * and reflects the original connection endpoints. The receiver must then use
 * the information provided in the protocol block to get original the address.
 */
#define PP_CMD_PROXY 0x1

/*
 * The connection is forwarded for an unknown, unspecified
 * or unsupported protocol. The sender should use this family when sending
 * LOCAL commands or when dealing with unsupported protocol families. The
 * receiver is free to accept the connection anyway and use the real endpoint
 * addresses or to reject it. The receiver should ignore address information.
 */
#define PP_AF_UNSPEC 0x0
/*
 * The forwarded connection uses the AF_INET address family
 * (IPv4). The addresses are exactly 4 bytes each in network byte order,
 * followed by transport protocol information (typically ports).
 */
#define PP_AF_INET 0x1
/*
 * The forwarded connection uses the AF_INET6 address family
 * (IPv6). The addresses are exactly 16 bytes each in network byte order,
 * followed by transport protocol information (typically ports).
 */
#define PP_AF_INET6 0x2
/*
 * The forwarded connection uses the AF_UNIX address family
 * (UNIX). The addresses are exactly 108 bytes each.
 */
#define PP_AF_UNIX 0x3

/*
 * the connection is forwarded for an unknown, unspecified
 * or unsupported protocol. The sender should use this family when sending
 * LOCAL commands or when dealing with unsupported protocol families. The
 * receiver is free to accept the connection anyway and use the real endpoint
 * addresses or to reject it. The receiver should ignore address information.
 */
#define PP_TP_UNSPEC 0x0

/*
 * the forwarded connection uses a SOCK_STREAM protocol (eg:
 * TCP or UNIX_STREAM). When used with AF_INET/AF_INET6 (TCP), the addresses
 * are followed by the source and destination ports represented on 2 bytes
 * each in network byte order.
 */
#define PP_TP_STREAM 0x1

/*
 * The forwarded connection uses a SOCK_DGRAM protocol (eg:
 * UDP or UNIX_DGRAM). When used with AF_INET/AF_INET6 (UDP), the addresses
 * are followed by the source and destination ports represented on 2 bytes
 * each in network byte order.
 */
#define PP_TP_DGRAM 0x2

struct pp_header_v2
{
  /* hex 0D 0A 0D 0A 00 0D 0A 51 55 49 54 0A */
  uint8_t signaure[12];

private:
  /* protocol version and command */
  uint8_t version_command_value;
  /* protocol family and address */
  uint8_t family_protocol_value;
  /* number of following bytes part of the header */
  uint16_t payload_size_value;

public:
  uint8_t version() const { return version_command_value >> 4; }
  uint8_t command() const { return version_command_value & 0xf; }

  uint8_t address_family() const { return family_protocol_value >> 4; }
  uint8_t protocol() const { return family_protocol_value & 0xf; }

  uint16_t payload_size() const { return ntohs(payload_size_value); }

  bool has_valid_signature() const
  {
    return signaure[0] == 0x0D && signaure[1] == 0x0A &&
      signaure[2] == 0x0D && signaure[3] == 0x0A &&
      signaure[4] == 0x00 && signaure[5] == 0x0D &&
      signaure[6] == 0x0A && signaure[7] == 0x51 &&
      signaure[8] == 0x55 && signaure[9] == 0x49 &&
      signaure[10] == 0x54 && signaure[11] == 0x0A;
  }

  bool has_valid_version() const
  {
    return version() == 2;
  }

  bool has_valid_command() const
  {
    return command() == PP_CMD_LOCAL || command() == PP_CMD_PROXY;
  }

  bool has_valid_address_family() const
  {
    return address_family() == PP_AF_UNSPEC || address_family() == PP_AF_INET || address_family() == PP_AF_INET6 || address_family() == PP_AF_UNIX;
  }

  bool has_valid_protocol() const
  {
    return protocol() == PP_TP_UNSPEC || protocol() == PP_TP_STREAM || protocol() == PP_TP_DGRAM;
  }

  bool is_valid() const
  {
    return has_valid_signature() && has_valid_version() && has_valid_command() && has_valid_address_family() && has_valid_protocol();
  }

};

// sanity check
static_assert(sizeof(pp_header_v2) == 16);

union pp_payload_v2 {
  /* for TCP/UDP over IPv4, len = 12 */
  struct {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint16_t src_port;
    uint16_t dst_port;
  } ipv4_addr;
  /* for TCP/UDP over IPv6, len = 36 */
  struct {
    uint8_t  src_addr[16];
    uint8_t  dst_addr[16];
    uint16_t src_port;
    uint16_t dst_port;
  } ipv6_addr;
  /* for AF_UNIX sockets, len = 216 */
  struct {
    uint8_t src_addr[108];
    uint8_t dst_addr[108];
  } unix_addr;
};

// sanity check
static_assert(sizeof(pp_payload_v2) == 216);

#endif //PROXYPROTOCOL_H
