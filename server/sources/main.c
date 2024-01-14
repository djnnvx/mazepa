
#include <unistd.h>
#include "server.h"

int main(int ac, char **av, char **envp) {

    server_t srv = {0};

    if (!parse_cli_arguments(&srv, ac, av, envp)) {
#ifdef DEBUG
        DEBUG_LOG("[%s]: invalid arguments. exiting on startup.", av[0]);
#endif
        _exit(1);
    }

    srv.sockfd = init_remote_connection(&srv);
#ifdef DEBUG
    DEBUG_LOG("[+] socket initiated....preparing DB connection");
#endif

    srv.dbfd = init_csv_dbfd((char const *)srv.options.db_filepath);
}
