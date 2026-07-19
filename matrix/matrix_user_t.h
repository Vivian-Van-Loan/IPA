#ifndef MATRIX_CLIENT_MATRIX_USER_T_H
#define MATRIX_CLIENT_MATRIX_USER_T_H

#include "../types/general_types.h"

typedef struct matrix_user_t {
    char* id;
    char* display_name;
    char* avatar_url;
} matrix_user_t;

void matrix_user_destroy(matrix_user_t* user);

#endif //MATRIX_CLIENT_MATRIX_USER_T_H
