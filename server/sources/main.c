
#include "server.h"
#include <unistd.h>

int main(int ac, char **av, char **envp) {

    server_t srv = {0};

    if (!parse_cli_arguments(&srv, ac, av, envp)) {
        DEBUG_LOG("[%s]: invalid arguments. exiting on startup.", av[0]);
        _exit(1);
    }

}
