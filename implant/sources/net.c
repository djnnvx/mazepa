
/*
    this file holds all of the functions needed for communicating with the
    remote server


*/


#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "implant.h"

int init_remote_connection(implant_t *instance) {

#ifdef DEBUG
    DEBUG_LOG("Initiating remote connection with (%s:%d)", instance->ip, instance->port);
#endif

    /*
       creating initial socket...

       AF_INET          -> remote connection    (for local, use AF_UNIX)
       SOCK_STREAM      -> TCP connection       (for UDP, use SOCK_DGRAM)
       IPPROTO_TCP      -> Specify protocol     (for UDP, use IPPROTO_UDP)

    */
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {

#ifdef DEBUG
        perror("socket");
#endif

        close(sockfd);
        return -1;
    }

    /* items required to bind socket to remote address */
    int is_enabled = 1;
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(instance->port),
        .sin_addr.s_addr = inet_addr(instance->ip)
    };


    /* setting socket options to make connection more convenient */
    if (setsockopt(
            sockfd,
            SOL_SOCKET,
            SO_REUSEADDR | SO_REUSEPORT, /* allowing to reuse ADDR or PORT (useful for testing) */
            &is_enabled,
            sizeof(int)
        ) < 0) {

#ifdef DEBUG
        perror("setsockopt");
#endif

        close(sockfd);
        return -1;
    }

    /* connecting to remote server */
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

#ifdef DEBUG
        perror("setsockopt");
#endif

        close(sockfd);
        return -1;
    }

    return sockfd;
}
