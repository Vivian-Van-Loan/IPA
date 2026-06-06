#ifndef MATRIX_CLIENT_MATRIX_CLIENT_H
#define MATRIX_CLIENT_MATRIX_CLIENT_H

#include <jansson.h>

#include "login.h"

typedef struct matrix_client_t {
    matrix_login_t login;
    json_t* core_account_data;
} matrix_client_t;

int matrix_client_sync_account_data(matrix_client_t* client);
void destroy_client(matrix_client_t* client);

#endif //MATRIX_CLIENT_MATRIX_CLIENT_H