#ifndef MATRIX_CLIENT_SELECT_MENU_H
#define MATRIX_CLIENT_SELECT_MENU_H

#include "../graphics.h"

void draw_select_menu(matrix_client_t* client);

void setup_select_dm_menu(matrix_client_t* client);
void setup_select_spaces_menu(matrix_client_t* client);
void setup_select_rooms_menu(matrix_client_t* client);

#endif //MATRIX_CLIENT_SELECT_MENU_H
