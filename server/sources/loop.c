/*
    stores main loop logic (that is managing entering / ending connections &
    logging them keys)
*/

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <unistd.h>
#include "server.h"

/*
    - call select() to know which available (net) sockets are ready to
    do something
    - accept incoming connections if any & assign new socket for each
    - read from all available sockets
    - if new keys are dumped, lets get xkbcontext & read the new keys
    - check which items are to be stored in database
    - store it by bulk updates
        -> if anything fucks up at any point just trigger cleanup routine
*/
void loop(server_t *instance) {
    fd_set rs;
    fd_set ws;
    fd_set es;

    TAILQ_INIT(&instance->clients);
    while (should_gracefully_exit(0)) {

        // TODO: check for new connections here

        int fdsize = get_fd_size();
        if (fdsize == -1)
            break;

        fdset_setup(&ws, &rs, &es, instance);

#ifdef DEBUG
        struct timeval timeout = {.tv_usec = 5000, .tv_sec = 0};
        int sstat = select(fdsize, &rs, &ws, &es, &timeout);
        if (sstat == -1) {
            DEBUG_LOG("select: %s", strerror(errno));
#else
        int sstat = select(fdsize, &rs, &ws, &es, NULL);
        if (sstat == -1) {
#endif
            break;
        }
    }
}

void fdset_setup(fd_set *ws, fd_set *rs, fd_set *es, server_t *instance) {
    FD_ZERO(es);
    FD_ZERO(ws);
    FD_ZERO(rs);

    client_t *it = NULL;
    TAILQ_FOREACH(it, &instance->clients, clients) {
        FD_SET(it->sockfd, es);
        FD_SET(it->sockfd, ws);
        FD_SET(it->sockfd, rs);
    }
}

int should_gracefully_exit(int code) {
    static int should_exit = 0;

    if (code != 0 || should_exit) {
#ifdef DEBUG
        DEBUG_LOG("[+] caught code: %d\n", code)
#endif
        should_exit = 1;
    }
    return should_exit;
}

static void
sig_handler(int code) {
    if (code == SIGINT)
        should_gracefully_exit(code);
}

int get_fd_size(void) {

    const char fdsize[] = "FDSize:";
    char buf[4096] = {0};
    int statfd = open("/proc/self/status", O_RDONLY);

    if (statfd < 0) {
#ifdef DEBUG
        DEBUG_LOG("open statfd: %s", strerror(errno));
#endif

        return -1;
    }

    const ssize_t num_read = read(statfd, buf, sizeof(buf) - 1);
    if (num_read <= 0) {
#ifdef DEBUG
        DEBUG_LOG("read statfd: %s", strerror(errno));
#endif

        return -1;
    }

    buf[num_read] = '\0';
    close(statfd);

    const char *fdsize_ptr = strstr(buf, fdsize_ptr);
    if (!fdsize_ptr)
        return 256;

    for (const char *char_ptr = fdsize_ptr + sizeof(fdsize) - 1; char_ptr <= buf + num_read; ++char_ptr) {
        if (isspace(*char_ptr))
            continue;
        else if (*char_ptr == 0)
            return 256;

        return atoi(char_ptr);
    }

    return 256;
}
