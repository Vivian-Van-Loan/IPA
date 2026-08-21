#ifndef MATRIX_CLIENT_MATRIX_ROOM_T_H
#define MATRIX_CLIENT_MATRIX_ROOM_T_H

#include <time.h>
#include <string.h>
#include <jansson.h>

#include "event/matrix-event.h"

#define TEMPLATE_TYPE matrix_event_t
#include "../types/dequeue.h"

#define TEMPLATE_TYPE_K str_const
#define TEMPLATE_TYPE_V matrix_event_t
#define TEMPLATE_EQUAL_FUNC(lhs, rhs) (strcmp((lhs), (rhs)) == 0)
#include "../types/hash_map.h"

typedef struct matrix_room_t {
    char* id;
    hash_map$str_const$matrix_event_t$ users;
    dequeue$matrix_event_t$ events;
    char* name;
    int64_t name_ts;
    char* avatar_url;
    int64_t avatar_ts;
    char* topic;
    int64_t topic_ts;
    int64_t last_message_time;
    char* prev_batch;
    char* next_batch;
    char* end;
} matrix_room_t;
#define TEMPLATE_TYPE matrix_room_t
#include "../types/vector.h"

void matrix_room_destroy(matrix_room_t* room);
int matrix_room_order(void const* lhs_void, void const* rhs_void);
size_t matrix_room_get_user_count(matrix_room_t* room);
matrix_event_t* matrix_room_get_user(matrix_room_t* room, char const* user_id);
matrix_event_t* matrix_room_get_user_by_idx(matrix_room_t* room, size_t idx);
void matrix_room_build(matrix_room_t* room, json_t* events);
void matrix_room_add_events(matrix_room_t* room, json_t* events, bool reverse);

#endif //MATRIX_CLIENT_MATRIX_ROOM_T_H
