
/**
 * This file contains a collection of utility functions for various tasks.
 *
 * `file_get_contents`: Reads a file and returns its contents as a null-terminated string.
 * The file path is passed as the first argument, and a pointer to the file contents is returned as the second argument.
 *
 * `remove_repeating_whitespaces`: Removes consecutive whitespace characters from a string and returns the modified string.
 *
 * `array_get_size`: Returns the size of a dynamically allocated array of int8_t values.
 * The array is passed as a pointer to the first element.
 *
 * `array_free`: Frees a dynamically allocated array of int8_t values.
 * The array is passed as a pointer to the first element.
 *
 * `array_from_string`: Generates a table of strings from a given prefix and suffix.
 * The prefix and suffix are passed as strings, and the function returns a dynamically allocated array of strings.
 */

#include "implant.h"

#include <ctype.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int8_t file_get_contents(int8_t const *path, int8_t **buffer) {
    struct stat st = {0};
    int fd = 0;
    int64_t filesize = 0;
    int64_t bytes_read = 0;

    if (NULL == path) {
#ifdef DEBUG
        DEBUG_LOG("[%s] %s is NULL\n", CLIENT_ID, path);
#endif
        return ERROR;
    }

    fd = open((const char *)path, O_RDONLY);
    if (0 > fd) {
#ifdef DEBUG
        DEBUG_LOG("[%s] Could not open %s: %s\n", CLIENT_ID, path, strerror(errno));
#endif
        return ERROR;
    }

    /* could use fseek but stat is posix so let's use that */
    if (0 > fstat(fd, &st)) {
#ifdef DEBUG
        DEBUG_LOG("[%s] Could not call fstat on %s: %s\n", CLIENT_ID, path, strerror(errno));
#endif
        return ERROR;
    }

    filesize = (int64_t)st.st_size;

    if (!buffer) {
#ifdef DEBUG
        DEBUG_LOG("[%s] file_get_contents: buffer addr is NULL\n", CLIENT_ID);
#endif
        return ERROR;
    }

    *buffer = malloc(sizeof(int8_t) * (uint64_t)(filesize + 1));
    if (!buffer || !(*buffer)) {
#ifdef DEBUG
        DEBUG_LOG("[%s] file_get_contents: could not allocate memory for %s: %s", CLIENT_ID, path, strerror(errno));
#endif
        return ERROR;
    }

    /* FIXME(djnn):

       read file by chunks, append to circular-buffer data structure.
       we only read small files with this for now though, less syscalls is better probablies.
       */

    bytes_read = (int64_t)read(fd, *buffer, (uint64_t)filesize);
    if (bytes_read != filesize) {
#ifdef DEBUG
        DEBUG_LOG("[%s] Should have read %ld bytes but read %ld (filepath: %s)\n",
                  CLIENT_ID,
                  st.st_size,
                  bytes_read,
                  path);
#endif
    }

    if (bytes_read < 0) {
#ifdef DEBUG
        DEBUG_LOG("[%s] read: %s", CLIENT_ID, strerror(errno));
#endif
        if (NULL != buffer) {
            free(*buffer);
        }
        return ERROR;
    }

    if (buffer != NULL)
        *buffer[bytes_read] = '\0';
    close(fd);

    return SUCCESSFUL;
}

static size_t
get_nb_cols(const int8_t *str, int n, int8_t separator) {
    size_t i = (size_t)n;
    size_t res = 0;

    if (str == NULL || str[0] == '\0')
        return 0;
    for (; str[i] && str[i] != separator; i++, res++)
        ;
    return (res);
}

static size_t
get_nb_rows(const int8_t *str, int8_t separator) {
    int i;
    size_t res = 0;

    if (str == NULL || str[0] == '\0')
        return 0;
    if (str == NULL || str[0] == '\0')
        return (0);
    for (i = 0; str[i] != '\0'; i++)
        if (str[i] == separator)
            res++;
    return (res + 1);
}

int8_t **
array_from_string(const int8_t *str, int8_t separator) {
    size_t i = 0;
    size_t malloc_size = 0;
    size_t index_str = 0;
    int8_t **res = NULL;

    if (str == NULL || *str == 0)
        return NULL;

    res = malloc(sizeof(int8_t *) * (get_nb_rows(str, separator) + 1));
    for (; i < get_nb_rows(str, separator); i++) {
        malloc_size = get_nb_cols(&str[index_str], 0, separator);

        res[i] = malloc(sizeof(int8_t) * (malloc_size + 4));
        if (!res[i]) {
            res[i] = NULL;
            array_free((int8_t **)res);
            return NULL;
        }

        res[i] = (int8_t *)strncpy((char *)res[i], (const char *)&str[index_str], malloc_size);
        res[i][malloc_size] = '\0';

        index_str = index_str + malloc_size + 1;
    }

    res[i] = NULL;
    return (res);
}

void array_free(int8_t **tab) {
    for (size_t ctr = 0; tab[ctr] != NULL; ctr++)
        free(tab[ctr]);
    free(tab);
}

uint32_t array_get_size(int8_t **array) {
    uint32_t ctr = 0;

    if (!array)
        return 0;
    while (array[ctr++])
        ;
    return ctr;
}

int8_t *remove_repeating_whitespaces(int8_t *s) {
    size_t i = 0;
    size_t x = 0;

    if (!s || !(*s))
        return NULL;

    for (; s[i]; ++i)
        if (!isspace(s[i]) || (i > 0 && !isspace(s[i - 1])))
            s[x++] = s[i];

    s[x] = '\0';
    return s;
}

int8_t ascii_to_hex(char input[STRING_BUFFER_SIZE / 2], char output[STRING_BUFFER_SIZE]) {
    int8_t ctr = -1;

    bzero(output, STRING_BUFFER_SIZE);
    if (!input)
        return ERROR;

    while (input[++ctr] != 0 && ctr < STRING_BUFFER_SIZE / 4) {
        snprintf(output + ctr * 2, STRING_BUFFER_SIZE - (ctr * 2), "%02x", (int8_t)input[ctr]);
    }
    output[STRING_BUFFER_SIZE - 1] = 0;
    return SUCCESSFUL;
}

int8_t hex_to_ascii(char input[STRING_BUFFER_SIZE], char output[STRING_BUFFER_SIZE]) {
    int8_t ctr = -1;

    bzero(output, STRING_BUFFER_SIZE);
    if (!input)
        return ERROR;

    while (input[++ctr] != 0) {
        output[ctr / 2] = (char)strtol(input + ctr, NULL, 16);
    }
    return SUCCESSFUL;
}
