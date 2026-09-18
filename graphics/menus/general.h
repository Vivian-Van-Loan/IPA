#ifndef MATRIX_CLIENT_GENERAL_H
#define MATRIX_CLIENT_GENERAL_H

#include "../graphics.h"

void draw_background(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, unsigned w, unsigned h);
void draw_wifi_sprite(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, float depth);
void draw_battery_sprite(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, float depth);
void draw_top_screen_left(struct ipa_graphics_state_t* graphics, float base_depth);
void draw_bevel_box(struct ipa_graphics_state_t* graphics, int x, int y, int w, int h, u32 bar1, u32 bar2, u32 border_in, u32 border_out, float base_depth);
void draw_load_sprite(struct ipa_graphics_state_t* graphics, int x, int y, u32 colour, float depth);
void draw_load_box(struct ipa_graphics_state_t* graphics);

#endif //MATRIX_CLIENT_GENERAL_H
