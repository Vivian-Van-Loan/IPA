#ifndef MATRIX_CLIENT_MAIN_MENU_H
#define MATRIX_CLIENT_MAIN_MENU_H

#include "../graphics.h"

void draw_main_menu(matrix_client_t* client);

void draw_main_top_shared(struct ipa_graphics_state_t* graphics);
void draw_main_menu_button(C2D_Sprite* button_sprite, float x, float y, char const* text);
void draw_main_menu_select(float x, float y);
void draw_main_bottom_shared(struct ipa_graphics_state_t* graphics);

#endif //MATRIX_CLIENT_MAIN_MENU_H
