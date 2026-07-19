#ifndef MATRIX_CLIENT_MATRIX_ROOM_T_H
#define MATRIX_CLIENT_MATRIX_ROOM_T_H

#include <time.h>
#include <string.h>
#include <jansson.h>

#include "matrix_user_t.h"

#define TEMPLATE_TYPE matrix_user_t
#include "../types/vector.h"

#define TEMPLATE_TYPE_K str_const
#define TEMPLATE_TYPE_V matrix_user_t
#define TEMPLATE_EQUAL_FUNC(lhs, rhs) (strcmp((lhs), (rhs)) == 0)
#include "../types/hash_map.h"

typedef struct matrix_room_t {
    char* id;
    // vector$matrix_user_t$ associated_users;
    hash_map$str_const$matrix_user_t$ users;
    char* name;
    char* avatar_url;
    char* topic;
    time_t last_message_time;
} matrix_room_t;
#define TEMPLATE_TYPE matrix_room_t
#include "../types/vector.h"

void matrix_room_destroy(matrix_room_t* room);
int matrix_room_order(void const* lhs_void, void const* rhs_void);
size_t matrix_room_get_user_count(matrix_room_t* room);
matrix_user_t* matrix_room_get_user(matrix_room_t* room, char const* user_id);
matrix_user_t* matrix_room_get_user_by_idx(matrix_room_t* room, size_t idx);
void matrix_room_build(matrix_room_t* room, json_t* events);

#endif //MATRIX_CLIENT_MATRIX_ROOM_T_H
