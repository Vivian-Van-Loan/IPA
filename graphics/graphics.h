#ifndef MATRIX_CLIENT_GRAPHICS_H
#define MATRIX_CLIENT_GRAPHICS_H

#include <citro2d.h>

#include "images.h"

struct matrix_client_t; //can't include the client due to circular refs

typedef ipa_image_t* ipa_image_t_p;
#define TEMPLATE_TYPE_K ipa_image_t_p
#define TEMPLATE_TYPE_V size_t
#include "../types/pair.h"

#define TEMPLATE_TYPE pair$ipa_image_t_p$size_t$
#include "../types/vector.h"

#define TOP_HEIGHT 240
#define TOP_WIDTH 400
#define BOTTOM_HEIGHT 240
#define BOTTOM_WIDTH 320

#define C2D_Color32c(r, g, b, a) ((u32)r | ((u32)g << (u32)8) | ((u32)b << (u32)16) | ((u32)a << (u32)24))
#define C2D_Color_get_red(c) ((c) & 0xFF)
#define C2D_Color_get_green(c) ((c >> 8) & 0xFF)
#define C2D_Color_get_blue(c) ((c >> 16) & 0xFF)
#define C2D_Color_get_alpha(c) ((c >> 24) & 0xFF)
#define C2D_Color_new_alpha(c, alpha) (C2D_Color32c(C2D_Color_get_red(overlay_colour), C2D_Color_get_green(overlay_colour), C2D_Color_get_blue(overlay_colour), (alpha)))
constexpr u32 WHITE = C2D_Color32c(0xFB, 0xFB, 0xFB, 0xFF);
constexpr u32 BLACK = C2D_Color32c(0x00, 0x00, 0x00, 0xFF);
constexpr u32 SUPER_DARK_GREY = C2D_Color32c(0x28, 0x28, 0x28, 0xFF);
constexpr u32 DARK_GREY = C2D_Color32c(0x49, 0x49, 0x49, 0xFF);
constexpr u32 MEDIUM_GREY = C2D_Color32c(0xAA, 0xAA, 0xAA, 0xFF);
constexpr u32 LIGHT_GREY = C2D_Color32c(0xBA, 0xBA, 0xBA, 0xFF);
constexpr u32 SUPER_LIGHT_GREY = C2D_Color32c(0xEB, 0xEB, 0xEB, 0xFF);
constexpr u32 ORANGE = C2D_Color32c(0xFB, 0x69, 0x00, 0xFF);

int init_graphics();

void draw(struct matrix_client_t* client);

#endif //MATRIX_CLIENT_GRAPHICS_H
