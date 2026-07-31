#ifndef MATRIX_CLIENT_MATRIX_MESSAGE_H
#define MATRIX_CLIENT_MATRIX_MESSAGE_H

#include <jansson.h>

#include "../../types/general_types.h"

typedef enum matrix_message_type_t {
    MATRIX_MSG_UNKNOWN,
    MATRIX_MSG_TEXT,
    MATRIX_MSG_EMOTE,
    MATRIX_MSG_NOTICE,
    MATRIX_MSG_IMAGE,
    MATRIX_MSG_FILE,
    MATRIX_MSG_AUDIO,
    MATRIX_MSG_LOCATION, //no
    MATRIX_MSG_VIDEO, //ditto
    // MATRIX_MSG_KEY_VER_REQ, //ditto ditto
    MATRIX_MSG_STICKER, //technically its own thing, which is stupid
} matrix_message_type_t;

matrix_message_type_t matrix_message_type_from_str(char const* str);
char const* matrix_message_type_to_str(matrix_message_type_t type);

#define TEMPLATE_TYPE_K matrix_message_type_t
#define TEMPLATE_TYPE_V alloc_str
#include "../../types/pair.h"

typedef struct matrix_message_t {
    pair$matrix_message_type_t$alloc_str$ type; //string is only allocated (non-null) IF type is unknown
    char* body;
    char* url;
    char* filename;
    char* mime;
    //todo: figure out if we're dealing with formatted body, unlikely for a while
} matrix_message_t;

matrix_message_t matrix_make_message(json_t* event_json);
void matrix_message_destroy(matrix_message_t* message);

#endif //MATRIX_CLIENT_MATRIX_MESSAGE_H
