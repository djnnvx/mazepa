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
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"


static void
sig_handler(int code) {
    if (code == SIGINT)
        should_gracefully_exit(code);
}


void loop(server_t *instance) {

    int dbfd = init_csv_dbfd(instance->options.db_filepath);
    if (dbfd < 0 && instance->options.db_filepath[0] != 0)
        return;

    signal(SIGINT, sig_handler);
    TAILQ_INIT(&instance->clients);
    while (should_gracefully_exit(0)) {

        icmp_msg_t msg = {0};

        /* poll for new packet */
        if (net_icmp_recv(instance, &msg) < 0) {
            goto CLEANUP_ROUTINE;
        }

        /* have we received a new message ?
         * (we should always have one but lets check if something fucked up) */
        if (msg.addr[0] == 0) {
            usleep(2500);
            continue;
        }

        /* find related client in client-list, then update state */
        client_t *it = NULL;
        int found_client = 0;
        TAILQ_FOREACH(it, &instance->clients, clients) {
            if (strncmp(it->ipaddr, msg.addr, STRING_BUFFER_SIZE) == 0) {
                /* update state ! */
                found_client = 1;
            }
        }

        /* no client found, find least active client & kick it. then, add it */
        if (!found_client) {


        }


    }

CLEANUP_ROUTINE:

    /* TODO(djnn): dump all client state to database & close server */


    close(dbfd);
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
