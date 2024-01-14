/*
   program's imput handling is handled here.
*/

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "implant.h"

static void
help_mode(char const *binary_path) {
#ifdef DEBUG
    printf("%s USAGE:\n\t-i\tCallback IP Address\n\t-p\tCallback "
           "Port\n\t-n\tDisable network connection (for debug)\n",
           binary_path);
#else
    (void)binary_path;
#endif
}

int parse_user_input(int ac, char **av, implant_t *settings) {
    int c = 0;

#ifdef DEBUG
    char const *options = "i:p:hn";
#else
    char const *options = "i:p:h";
#endif

    do {
        c = getopt(ac, av, options);
        switch (c) {

#ifdef DEBUG
        case 'n':
            settings->disable_net = 1;
            break;
#endif
        case 'i':
            strncpy((char *)&settings->ip, optarg, 255);
            break;

        case 'p':
            settings->port = (unsigned short)strtoul(optarg, NULL, 0);
            break;

        case 'h':
            help_mode(av[0]);
            exit(0);
            /* will exit on help_mode() anyway */

        case '?':
            help_mode(av[0]);
            return ERROR;

        default:
            break;
        }
    } while (c != -1);
    return SUCCESSFUL;
}
