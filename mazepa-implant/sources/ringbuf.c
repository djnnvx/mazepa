#include "implant.h"

#include <string.h>

void ringbuf_init(ringbuf_t *rb) {
    if (!rb)
        return;
    memset(rb->data, 0, RINGBUF_SIZE);
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

uint8_t
ringbuf_push(ringbuf_t *rb, uint8_t byte) {
    if (!rb || rb->count >= RINGBUF_SIZE)
        return ERROR;

    rb->data[rb->head] = byte;
    rb->head = (rb->head + 1) % RINGBUF_SIZE;
    rb->count++;
    return SUCCESSFUL;
}

size_t
ringbuf_read(ringbuf_t *rb, uint8_t *dst, size_t len) {
    size_t bytes_read = 0;

    if (!rb || !dst)
        return 0;

    while (bytes_read < len && rb->count > 0) {
        dst[bytes_read] = rb->data[rb->tail];
        rb->tail = (rb->tail + 1) % RINGBUF_SIZE;
        rb->count--;
        bytes_read++;
    }
    return bytes_read;
}

int ringbuf_should_flush(ringbuf_t *rb) {
    if (!rb)
        return 0;
    return rb->count >= RINGBUF_FLUSH_THRESHOLD;
}
