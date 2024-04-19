
#include <string.h>
#include "server.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <xkbcommon/xkbcommon.h>

static const translated_key_t KEYS[] = {
    {"space", ' '},
    {"comma", ','},
    {"period", '.'},
    {"slash", '/'},
    {"questionmark", '?'},
    {"semicolon", ';'},
    {"apostrophe", '\''},
    {"bracketleft", '['},
    {"bracketright", ']'},
    {"backslash", '\\'},
    {"pipe", '|'},
    {"Tab", '\t'},
    {"minus", '-'},
    {"plus", '+'},
    {"equal", '='},
    {"dollar", '$'},
    {"percentage", '%'},
    {"star", '*'},
    {"parenthesisleft", '('},
    {"parenthesisright", ')'},
    {"underscore", '_'},
    {"ampersand", '&'},
    {"pound", '#'},
    {"arobase", '@'},
    {"tild", '~'},
    {NULL, 0},
};


static char *
retrieve_translated_key(char *key_desc) {
    for (size_t ctr = 0; KEYS[ctr].description != NULL; ctr++) {
        if (!strncmp(KEYS[ctr].description, key_desc, strlen(KEYS[ctr].description))) {

            memset(key_desc, 0, STRING_BUFFER_SIZE);
            key_desc[0] = KEYS[ctr].representation;

            break;
        }
    }
    return key_desc;
}

