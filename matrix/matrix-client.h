#ifndef MATRIX_CLIENT_MATRIX_CLIENT_H
#define MATRIX_CLIENT_MATRIX_CLIENT_H

#include <jansson.h>

#include "login.h"
#include "matrix_user_t.h"
#include "matrix_room_t.h"

typedef enum matrix_menu_t {
    MENU_MAIN = 0,
    MENU_DMS,
    MENU_SPACES,
    MENU_ROOMS,
    MENU_SETTINGS,
    MENU_CHAT,
    MENU_SPACE,

    //todo: all the settings menus
} matrix_menu_t;

typedef struct matrix_client_t {
    matrix_login_t login;
    json_t* core_account_data;
    matrix_menu_t menu;
    struct ipa_graphics_state_t* graphics_state;
    // vector$matrix_user_t$ users;
    vector$matrix_room_t$ rooms;
} matrix_client_t;

int matrix_client_sync_account_data(matrix_client_t* client);
int matrix_client_sync_directs(matrix_client_t* client);
int matrix_client_sync_dump(matrix_client_t* client);
matrix_client_t make_client();
void destroy_client(matrix_client_t* client);

#endif //MATRIX_CLIENT_MATRIX_CLIENT_H