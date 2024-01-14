/*
    this code handles the main server loop


*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "server.h"

int init_remote_connection(server_t *instance) {

#ifdef DEBUG
    DEBUG_LOG("Initiating remote connection with (localhost:%d)",
              instance->options.listen_port);
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
    struct sockaddr_in addr = {.sin_family = AF_INET,
                               .sin_port = htons(instance->options.listen_port),
                               .sin_addr.s_addr = inet_addr("0.0.0.0")};

    /* setting socket options to make connection more convenient */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, /* allowing to reuse ADDR or PORT
                                                                       (useful for testing) */
                   &is_enabled,
                   sizeof(int)) < 0) {

#ifdef DEBUG
        DEBUG_LOG("setsockopt send_key_description: %s", strerror(errno));
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

    if (listen(sockfd, MAX_NB_CLIENT) < 0) {
#ifdef DEBUG
        DEBUG_LOG("listen: %s", strerror(errno));
#endif

        return -1;
    }

    return sockfd;
}
