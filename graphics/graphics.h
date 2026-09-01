#ifndef MATRIX_CLIENT_GRAPHICS_H
#define MATRIX_CLIENT_GRAPHICS_H

#include <citro2d.h>

#include "images.h"
#include "messages.h"
#include "../matrix/matrix-client.h"

// struct matrix_client_t; //can't include the client due to circular refs

typedef ipa_image_t* ipa_image_t_p;
#define TEMPLATE_TYPE_K ipa_image_t_p
#define TEMPLATE_TYPE_V size_t
#include "../types/pair.h"

#define TEMPLATE_TYPE pair$ipa_image_t_p$size_t$
#include "../types/vector.h"

#define TEMPLATE_TYPE ipa_message_t
#include "../types/dequeue.h"

enum ipa_graphics_queue_type_t {
    IPA_QUEUE_NONE,
    IPA_QUEUE_LOGIN,
    IPA_QUEUE_IMAGE,
    IPA_QUEUE_AVATAR,
    IPA_QUEUE_SYNC_MAIN,
    IPA_QUEUE_SYNC_DIRECTS,
    IPA_QUEUE_LOAD_ROOM,
    IPA_QUEUE_FUNCTION,
};

typedef struct ipa_graphics_queue_t {
    enum ipa_graphics_queue_type_t type;
    ssize_t id;
} ipa_graphics_queue_t;

#define TEMPLATE_TYPE ipa_graphics_queue_t
#include "../types/vector.h"

struct ipa_graphics_state_t {
    ptrdiff_t time_on_menu;
    matrix_menu_t menu;
    ptrdiff_t selection;
    ptrdiff_t selection_num;
    ptrdiff_t scroll_pos;
    ptrdiff_t scroll_to_pos;
    ptrdiff_t anim_start_time;
    hash_map$img_path$ipa_image_t$ image_map;
    vector$pair$ipa_image_t_p$size_t$$ vram_images;
    vector$ipa_graphics_queue_t$ queue;
};

#define TOP_HEIGHT 240
#define TOP_WIDTH 400
#define BOTTOM_HEIGHT 240
#define BOTTOM_WIDTH 320

#define C2D_Color32c(r, g, b, a) ((u32)r | ((u32)g << (u32)8) | ((u32)b << (u32)16) | ((u32)a << (u32)24))
#define C2D_Color_get_red(c) ((c) & 0xFF)
#define C2D_Color_get_green(c) ((c >> 8) & 0xFF)
#define C2D_Color_get_blue(c) ((c >> 16) & 0xFF)
#define C2D_Color_get_alpha(c) ((c >> 24) & 0xFF)
#define C2D_Color_new_alpha(c, alpha) (C2D_Color32c(C2D_Color_get_red(overlay_colour), C2D_Color_get_green(overlay_colour), C2D_Color_get_blue(overlay_colour), (alpha)));
constexpr u32 WHITE = C2D_Color32c(0xFB, 0xFB, 0xFB, 0xFF);
constexpr u32 BLACK = C2D_Color32c(0x00, 0x00, 0x00, 0xFF);
constexpr u32 SUPER_DARK_GREY = C2D_Color32c(0x28, 0x28, 0x28, 0xFF);
constexpr u32 DARK_GREY = C2D_Color32c(0x49, 0x49, 0x49, 0xFF);
constexpr u32 MEDIUM_GREY = C2D_Color32c(0xAA, 0xAA, 0xAA, 0xFF);
constexpr u32 LIGHT_GREY = C2D_Color32c(0xBA, 0xBA, 0xBA, 0xFF);
constexpr u32 SUPER_LIGHT_GREY = C2D_Color32c(0xEB, 0xEB, 0xEB, 0xFF);
constexpr u32 SILVER = C2D_Color32c(0xD3, 0xCB, 0xC3, 0xFF);
constexpr u32 ORANGE = C2D_Color32c(0xFB, 0x69, 0x00, 0xFF);

constexpr float FADE_TOP = 0.25f;
constexpr float FADE_BOTTOM = 1;
constexpr int SPEED = 3;
constexpr int BIG_AVATAR_SIZE = 32;
constexpr int SMALL_AVATAR_SIZE = 16;
constexpr float DEPTH_STEP_SMALL = 1.0f / 65536;
constexpr float DEPTH_STEP_BIG = 1.0f / 128;

extern C3D_RenderTarget* top;
extern C3D_RenderTarget* bottom;
extern C2D_SpriteSheet main_sheet;
extern C2D_Sprite main_sprites[4];
extern C2D_SpriteSheet gradient_sheet;
extern C2D_Sprite gradient_sprites[2];

// extern C2D_ImageTint overlay_tint;
extern u32 overlay_colour;
extern u32 overlay_colour_dark;
extern u32 overlay_colour_light;

#define COORD_OFFSET 0.0625f

#define C2D_DrawLineFix(x0, y0, clr0, x1, y1, clr1, thickness, depth) C2D_DrawLine(x0 + COORD_OFFSET, y0 + COORD_OFFSET, clr0, x1 + COORD_OFFSET, y1 + COORD_OFFSET, clr1, thickness, depth)
#define C2D_DrawRectSolidFix(x, y, z, w, h, clr) C2D_DrawRectSolid(x + COORD_OFFSET, y + COORD_OFFSET, z, w, h, clr)

u32 C2D_Color_Overlay(u32 base, u32 overlay);

int init_graphics();

void draw_and_input(struct matrix_client_t* client);

#endif //MATRIX_CLIENT_GRAPHICS_H
