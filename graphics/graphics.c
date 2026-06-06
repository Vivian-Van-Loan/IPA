#include "graphics.h"

#include "../utils.h"
#include "../matrix/matrix-client.h"

struct graphics_state_t {
    ptrdiff_t time_on_menu;
    matrix_menu_t menu;
};

constexpr u32 white = C2D_Color32c(0xFB, 0xFB, 0xFB, 0xFF);
constexpr u32 black = C2D_Color32c(0x00, 0x00, 0x00, 0xFF);
constexpr u32 dark_grey = C2D_Color32c(0x49, 0x49, 0x49, 0xFF);
constexpr u32 medium_grey = C2D_Color32c(0xAA, 0xAA, 0xAA, 0xFF);
constexpr u32 light_grey = C2D_Color32c(0xBA, 0xBA, 0xBA, 0xFF);
constexpr u32 orange = C2D_Color32c(0xFB, 0x69, 0x00, 0xFF);

C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
C2D_SpriteSheet main_sheet;
C2D_Sprite main_sprites[4];
C2D_SpriteSheet gradient_sheet;
C2D_Sprite gradient_sprites[2];
C2D_ImageTint overlay_tint;
int init_graphics() {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // consoleInit(GFX_TOP, nullptr);
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    main_sheet = C2D_SpriteSheetLoad("romfs:/gfx/IPA_TEXTURES_MAIN.t3x");
    if (!main_sheet) {
        efuncprintf("Failed to load spritesheet\n");
        return -1;
    }
    for (size_t i = 0; i < lengthof(main_sprites); i++) {
        C2D_SpriteFromSheet(main_sprites + i, main_sheet, i);
    }
    // C2D_SpriteSheetFree(main_sheet);

    C2D_SpriteSetPos(main_sprites + 0, 64, 56);
    C2D_SpriteSetPos(main_sprites + 1, 64, 88);
    C2D_SpriteSetPos(main_sprites + 2, 64, 120);
    C2D_SpriteSetPos(main_sprites + 3, 64, 152);

    gradient_sheet = C2D_SpriteSheetLoad("romfs:/gfx/IPA_TEXTURES_GRADIENT.t3x");
    if (!gradient_sheet) {
        efuncprintf("Failed to load spritesheet\n");
        return -1;
    }
    for (size_t i = 0; i < lengthof(gradient_sprites); i++) {
        C2D_SpriteFromSheet(gradient_sprites + i, gradient_sheet, i);
    }
    // C2D_SpriteSheetFree(gradient_sheet);

    u32 overlay_colour = C2D_Color32(128, 0, 255, 255);
    C2D_PlainImageTint(&overlay_tint, overlay_colour, 0.5f);

    return 0;
}

void draw_background(struct graphics_state_t* graphics, unsigned x, unsigned y, unsigned w, unsigned h) {
    for (size_t i = 0; i < h / 4; i++) {
        C2D_DrawLine(x, i * 4 + (y + 1), medium_grey, x + w, i * 4 + (y + 1), medium_grey, 3, 0); //thickness is from middle
        C2D_DrawLine(x, i * 4 + (y + 3), light_grey, x + w, i * 4 + (y + 3), light_grey, 1, 0);
    }
}

void draw_top_screen_left(struct graphics_state_t* graphics) {
    C2D_DrawRectSolid(7, 209, 0, 26, 2, overlay_tint.corners->color); //colour bar

    C2D_DrawRectSolid(2, 4, 0, 17, 11, black); //todo: battery
    C2D_DrawRectSolid(21, 1, 0, 16, 16, black); //todo: wireless state
}

void draw_top_screen_right(struct graphics_state_t* graphics) {
    for (size_t i = 0; i < 234 / 2; i++) {
        C2D_DrawLine(364, i * 2 + 3, dark_grey, 396, i * 2 + 3, dark_grey, 1, 0);
        C2D_DrawLine(364, i * 2 + 4, black, 396, i * 2 + 4, black, 1, 0);
    }

    C2D_DrawLine(365, 0, black, 394, 0, black, 1, 0);
    C2D_DrawLine(365, 2, orange, 394, 2, orange, 2, 0); //top
    C2D_DrawLine(365, 3, black, 394, 3, black, 1, 0);

    C2D_DrawLine(360, 5, black, 365, 0, black, 1, 0);
    C2D_DrawLine(360, 6, black, 366, 0, black, 1, 0);
    C2D_DrawLine(366, 2, orange, 362, 6, orange, 2, 0); //top left
    C2D_DrawLine(363, 6, black, 366, 3, black, 1, 0);

    C2D_DrawLine(394, -1, black, 400, 5, black, 1, 0);
    C2D_DrawLine(393, -1, black, 400, 6, black, 1, 0);
    C2D_DrawLine(393, 1, orange, 398, 6, orange, 2, 0); //top right
    C2D_DrawLine(393, 3, black, 396, 6, black, 1, 0);

    C2D_DrawLine(360, 6, black, 360, 234, black, 1, 0);
    C2D_DrawLine(362, 5, orange, 362, 235, orange, 2, 0); //left side
    C2D_DrawLine(363, 6, black, 363, 234, black, 1, 0);

    C2D_DrawLine(396, 6, black, 396, 234, black, 1, 0);
    C2D_DrawLine(398, 5, orange, 398, 235, orange, 2, 0); //right side
    C2D_DrawLine(399, 6, black, 399, 234, black, 1, 0);

    C2D_DrawLine(365, 236, black, 394, 236, black, 1, 0);
    C2D_DrawLine(365, 238, orange, 394, 238, orange, 2, 0); //bottom
    C2D_DrawLine(365, 239, black, 394, 239, black, 1, 0);

    C2D_DrawLine(360, 235, black, 365, 240, black, 1, 0);
    C2D_DrawLine(360, 234, black, 365, 239, black, 1, 0);
    C2D_DrawLine(362, 234, orange, 366, 238, orange, 2, 0); //bottom left
    C2D_DrawLine(363, 233,  black, 366, 236, black, 1, 0);

    C2D_DrawLine(393, 240, black, 400, 233, black, 1, 0);
    C2D_DrawLine(394, 240, black, 400, 234, black, 1, 0);
    C2D_DrawLine(393, 238, orange, 398, 233, orange, 2, 0); //bottom right
    C2D_DrawLine(393, 236, black, 396, 233, black, 1, 0);
}

void draw_main_menu_top(struct graphics_state_t* graphics) {
    C2D_SceneBegin(top);
    draw_background(graphics, 40, 0, TOP_WIDTH, TOP_HEIGHT);

    C2D_DrawRectSolid(0, 0, 0, 40, TOP_HEIGHT, white);

    u32 green = C2D_Color32(0x00, 0x51, 0x00, 0xFF);
    for (size_t x = 1; x <= 37; x += 3) {
        C2D_DrawLine(x, 19, green, x + 2, 19, green, 1, 0);
        C2D_DrawLine(x, 220, green, x + 2, 220, green, 1, 0);
    }

    draw_top_screen_left(graphics);
    draw_top_screen_right(graphics);
}

void draw_main_menu_button(size_t idx, float x, float y) {
    C2D_Sprite* gradient = &gradient_sprites[1];
    u32 top_grey = C2D_Color32(0x79, 0x79, 0x79, 0xFF);

    C2D_Sprite* button = main_sprites + idx;
    C2D_SpriteSetPos(button, x, y);
    C2D_DrawSprite(button);
    for (size_t j = 0; j < 79; j++) {
        C2D_SpriteSetPos(gradient, x + 32 + j * 2, y + 1);
        C2D_DrawSprite(gradient);
    }

    C2D_DrawLine(x + 32, y, top_grey, x + 192, y, top_grey, 1, 0);
    C2D_DrawLine(x + 191, y + 1, dark_grey, x + 191, y + 32, dark_grey, 1, 0);
    C2D_DrawLine(x + 190, y + 1, white, x + 190, y + 31, white, 1, 0);
    C2D_DrawLine(x + 32, y + 30, white, x + 190, y + 30, white, 1, 0);
    C2D_DrawLine(x + 32, y + 31, black, x + 191, y + 31, black, 1, 0);
}

void draw_main_menu_bottom(struct graphics_state_t* graphics) {
    C2D_SceneBegin(bottom);
    draw_background(graphics, 0, 24, BOTTOM_WIDTH, BOTTOM_HEIGHT);

    // for (size_t i = 0; i < 4; i++) {
    //     draw_main_menu_button(i, 64, i * 32 + max(BOTTOM_HEIGHT - graphics->time_on_menu * 3, 56));
    //     // draw_main_menu_button(i, 64, i * 32 + 56);
    // }
    int const speed = 6;
    draw_main_menu_button(0, 64, 0 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - speed * 0) * speed, 56));
    draw_main_menu_button(1, 64, 1 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - speed * 1) * speed, 56));
    draw_main_menu_button(2, 64, 2 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - speed * 2) * speed, 56));
    draw_main_menu_button(3, 64, 3 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - speed * 3) * speed, 56));

    C2D_Sprite* gradient = &gradient_sprites[0];
    for (size_t i = 0; i < BOTTOM_WIDTH / 2; i++) { //gradient top and bottom
        int y = 0;
        C2D_SpriteSetScale(gradient, 1, 1);
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);

        y = 217;
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_SpriteSetScale(gradient, 1, -1);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);
    }
    u32 black = C2D_Color32(0, 0, 0, 0xFF);
    C2D_DrawLine(0, 23, black, BOTTOM_WIDTH, 23, black, 1, 0);
    C2D_DrawLine(0, 216, black, BOTTOM_WIDTH, 216, black, 1, 0);
}

void draw_main_menu(struct graphics_state_t* graphics) {
    draw_main_menu_top(graphics);
    draw_main_menu_bottom(graphics);
}

void draw(matrix_client_t* client) {
    if (!client->graphics_state) {
        client->graphics_state = malloc(sizeof(*client->graphics_state));
    }

    struct graphics_state_t* graphics = client->graphics_state;
    if (client->menu != graphics->menu) {
        graphics->menu = client->menu;
        graphics->time_on_menu = 0;
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(top, C2D_Color32(0xFF, 0, 0, 0xFF));
    C2D_TargetClear(bottom, C2D_Color32(0xFF, 0, 0, 0xFF));

    switch (client->menu) {
        case MENU_MAIN:
            draw_main_menu(graphics);
            break;
        case MENU_DMS:
            break;
        case MENU_SPACES:
            break;
        case MENU_ROOMS:
            break;
        case MENU_SETTINGS:
            break;
        case MENU_CHAT:
            break;
        case MENU_SPACE:
            break;
    }

    C3D_FrameEnd(0);
    graphics->time_on_menu++;
}
