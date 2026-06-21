#ifndef MATRIX_CLIENT_MATRIX_ROOM_T_H
#define MATRIX_CLIENT_MATRIX_ROOM_T_H

#include <jansson.h>

#include "matrix_user_t.h"

typedef struct matrix_room_t {
    char* id;
    vector$matrix_user_t$ associated_users;
    char* name;
    char* avatar_url;
} matrix_room_t;
#define TEMPLATE_TYPE matrix_room_t
#include "../types/vector.h"

void matrix_room_destroy(matrix_room_t* room);
matrix_user_t* matrix_room_get_user(matrix_room_t* room, char const* user_id);
void matrix_room_build(matrix_room_t* room, json_t* events);

#endif //MATRIX_CLIENT_MATRIX_ROOM_T_H
