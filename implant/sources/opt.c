/*
   program's imput handling is handled here.
   */


#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "implant.h"

static void help_mode(char const *binary_path) {
    printf("%s USAGE:\n\t-i\tCallback IP Address\n\t-p\tCallback Port\n", binary_path);
}


int parse_user_input(int ac, char **av, implant_t *settings) {
    int c = 0;

    do {
        c = getopt(ac, av, "i:p:h");
        switch (c) {
            case 'i':
                /* we got an IP address */
                strncpy((char *)&settings->ip, optarg, 255);
                break;

            case 'p':
                /* we got a port */
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
