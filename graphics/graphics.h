#ifndef MATRIX_CLIENT_GRAPHICS_H
#define MATRIX_CLIENT_GRAPHICS_H

#include <citro2d.h>

struct matrix_client_t; //can't include the client due to circular refs

#define TOP_HEIGHT 240
#define TOP_WIDTH 400
#define BOTTOM_HEIGHT 240
#define BOTTOM_WIDTH 320

#define C2D_Color32c(r, g, b, a) ((u32)r | ((u32)g << (u32)8) | ((u32)b << (u32)16) | ((u32)a << (u32)24))
constexpr u32 WHITE = C2D_Color32c(0xFB, 0xFB, 0xFB, 0xFF);
constexpr u32 BLACK = C2D_Color32c(0x00, 0x00, 0x00, 0xFF);
constexpr u32 DARK_GREY = C2D_Color32c(0x49, 0x49, 0x49, 0xFF);
constexpr u32 MEDIUM_GREY = C2D_Color32c(0xAA, 0xAA, 0xAA, 0xFF);
constexpr u32 LIGHT_GREY = C2D_Color32c(0xBA, 0xBA, 0xBA, 0xFF);
constexpr u32 ORANGE = C2D_Color32c(0xFB, 0x69, 0x00, 0xFF);

int init_graphics();

void draw(struct matrix_client_t* client);

#endif //MATRIX_CLIENT_GRAPHICS_H
