#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "implant.h"

void daemon_setup(void) {
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    if (pid != 0 || setsid() < 0)
        exit(EXIT_FAILURE);

    /* FIXME(djnn): implement cleaner signal handler here */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    for (long x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close((int)x);
    }

    openlog(CLIENT_ID, LOG_PID, LOG_DAEMON);
}
