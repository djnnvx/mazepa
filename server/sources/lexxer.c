
#include "server.h"
#include <getopt.h>
#include <stdio.h>

static void get_help(const char *bin_name) {
    printf("%s: mazepa keylogger server\n", bin_name);
    printf("USAGE:\n");
    printf("\t--listen-port [-p]: specifies server listening port\n");
    printf("\t--db-path     [-d]: specifies sqlite .db filepath\n");

    printf("\t--help        [-h]: triggers help mode\n");
    printf("\n\nwritten with <3 by djnn -- https://djnn.sh");
}

int parse_cli_arguments(
        server_t *instance,
        int ac,
        char **av,
        __attribute__((unused)) char **envp
    ) {

    int c = 0;
    int optind_digit = 0;

    // TODO

    return SUCCESSFUL;
}
