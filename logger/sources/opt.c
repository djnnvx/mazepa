/*
   program's imput handling is handled here.
   */


#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "implant.h"


int parse_user_input(int ac, char **av, implant_t *settings) {
    int c = 0;

    do {
        c = getopt(ac, av, "i:p:d");
        switch (c) {
            case 'i':
                /* we got an IP address */
                strncpy((char *)&settings->ip, optarg, 16);
                break;

            case 'p':
                /* we got a port */
                 settings->port = (unsigned short)strtoul(optarg, NULL, 0);
                 break;

            case 'd':
                 settings->debug_enabled = 1;
                 break;

            case '?':
                 return ERROR;

            default:
                 break;
        }
    } while (c != -1);
    return SUCCESSFUL;
}
