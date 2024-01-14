/*
    Manages .csv logging here, so that it is easily sync'd up.

TODO:
    - add optional support for sqlite (maybe at compile-time) with an option
    - build little abstraction that will call the under-layered functions
underneath

    simple layout will be:
    uuid, name, ip-addr, linux-version, locale, dump
*/

#include <fcntl.h>
#include <unistd.h>
#include "server.h"

int init_csv_dbfd(char const *filepath) {

    if (!filepath || (!(*filepath)))
        return -1;

    /* obviously, we need to have a read-writable file */
    int access_status = access(filepath, R_OK | W_OK);

    /* file probably does not exist, let's create a file and return fd */
    if (access_status == -1) {
        int fd = open(filepath, O_CREAT | O_RDWR);
#ifdef DEBUG
        DEBUG_LOG("[+] created dbfd: %d\n", fd);
#endif
        return fd;
    }

    /* all good, lets just open & return */
    if (access_status == 0) {
        int fd = open(filepath, O_RDWR);
#ifdef DEBUG
        DEBUG_LOG("[+] opened existing dbfd: %d\n", fd);
#endif
        return fd;
    }

    /* incorrect perms is most likely */
#ifdef DEBUG
    DEBUG_LOG("[!] could not open %s: access() result: %d\n", filepath, access_status);
    DEBUG_LOG("incorrect file permissions is the most likely issue...\n");
#endif
    return -1;
}
