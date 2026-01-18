/*
   program's input handling is handled here.
*/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "implant.h"

static void help_mode(void) {
#ifdef DEBUG
    printf("mazepa_%s USAGE:\n\t-i\tCallback IP Address\n\t-p\tCallback "
           "Port\n\t-n\tDisable network connection (for debug)\n",
           CLIENT_ID);
#endif
}

int run_lexer(int ac, char **av, implant_t *settings) {
    int c = 0;

    char const *options = "i:hn";

    do {
        c = getopt(ac, av, options);
        switch (c) {

        case 'n':
            settings->disable_net = 1;
            break;

        case 'i':
            strncpy((char *)settings->ip, optarg, STRING_BUFFER_SIZE - 1);
            settings->ip[STRING_BUFFER_SIZE - 1] = '\0';
            break;

        case '?':
            help_mode();
            return ERROR;

        case 'h':
            help_mode();
            exit(0);

        default:
            break;
        }
    } while (c != -1);
    return SUCCESSFUL;
}
