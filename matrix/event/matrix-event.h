#ifndef MATRIX_CLIENT_MATRIX_EVENT_H
#define MATRIX_CLIENT_MATRIX_EVENT_H

#include <stdint.h>

#include "../../types/general_types.h"
#include "matrix-event-message.h"
#include "matrix-event-room.h"
#include "matrix-event-type.h"

#define TEMPLATE_TYPE_K matrix_event_type_t
#define TEMPLATE_TYPE_V alloc_str
#include "../../types/pair.h"

typedef struct matrix_encrypted_t {
    char* algorithm;
    char* ciphertext;
    char* ciphertext_type; //for Olm
    char* device_id;
    char* session_id;
} matrix_encrypted_t;

typedef struct matrix_event_unsigned_data_t {
    int64_t age;
    char* membership;
    char* replaces;
    char* transaction_id;
    //do we bother with redaction info? I don't think so honestly
} matrix_event_unsigned_data_t;

typedef struct matrix_event_t {
    pair$matrix_event_type_t$alloc_str$ type; //the string will ONLY be valid IF event is EVENT_UNKNOWN
    char* id;
    int64_t origin_server_ts;
    char* sender;
    char* room_id;
    char* state_key;
    matrix_event_unsigned_data_t unsigned_data;
    union {
        matrix_event_room_t room;
        matrix_message_t message;
        matrix_encrypted_t encrypted;
    };
} matrix_event_t;

void matrix_event_destroy(matrix_event_t* event);

#endif //MATRIX_CLIENT_MATRIX_EVENT_H
