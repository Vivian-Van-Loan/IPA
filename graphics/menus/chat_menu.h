#ifndef MATRIX_CLIENT_CHAT_MENU_H
#define MATRIX_CLIENT_CHAT_MENU_H

#include "general.h"
#include "../types/general_types.h"

void draw_chat_room(matrix_client_t* client);

int draw_message_box(struct ipa_graphics_state_t* graphics, int x, int y, int w, int num_lines, bool draw_lines, u32 dark, u32 light, u32 back);
pair$int$int$ get_nameplate_width(char const* name, char const* avatar_url);
int draw_message(matrix_client_t* client, ipa_message_t const* message, int x, int y);
void draw_keyboard();

#endif //MATRIX_CLIENT_CHAT_MENU_H
