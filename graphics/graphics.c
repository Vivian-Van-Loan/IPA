#include "graphics.h"

#include "font.h"
#include "../utils.h"
#include "../matrix/matrix-client.h"

struct ipa_graphics_state_t {
    ptrdiff_t time_on_menu;
    matrix_menu_t menu;
};

constexpr float FADE_TOP = 0.25f;
constexpr float FADE_BOTTOM = 1;

C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
C2D_SpriteSheet main_sheet;
C2D_Sprite main_sprites[4];
C2D_SpriteSheet gradient_sheet;
C2D_Sprite gradient_sprites[2];

// C2D_Font font;
// C2D_TextBuf text_buf;
// C2D_Text text;

C2D_ImageTint overlay_tint;

int init_graphics() {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // font = C2D_FontLoad("romfs:/resources/font/NDS_FONT.bcfnt");
    // if (!font) {
    //     return -1;
    // }
    // text_buf = C2D_TextBufNew(8192); //todo: perhaps overkill, adjust later
    // C2D_TextFontParse(&text, font, text_buf, "Test text to be rendered.\nThe Quick Brown Fox Jumps Over The Lazy Dog");
    // C2D_TextOptimize(&text);

    // consoleInit(GFX_TOP, nullptr);
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    main_sheet = C2D_SpriteSheetLoad("romfs:/resources/gfx/IPA_TEXTURES_MAIN.t3x");
    if (!main_sheet) {
        efuncprintf("Failed to load spritesheet\n");
        return -1;
    }
    for (size_t i = 0; i < lengthof(main_sprites); i++) {
        C2D_SpriteFromSheet(main_sprites + i, main_sheet, i);
    }
    // C2D_SpriteSheetFree(main_sheet);

    gradient_sheet = C2D_SpriteSheetLoad("romfs:/resources/gfx/IPA_TEXTURES_GRADIENT.t3x");
    if (!gradient_sheet) {
        efuncprintf("Failed to load spritesheet\n");
        return -1;
    }
    for (size_t i = 0; i < lengthof(gradient_sprites); i++) {
        C2D_SpriteFromSheet(gradient_sprites + i, gradient_sheet, i);
    }
    // C2D_SpriteSheetFree(gradient_sheet);

    int res = init_font();
    if (res) {
        efuncprintf("Failed to load font\n");
        return -1;
    }

    // u32 overlay_colour = C2D_Color32(128, 0, 255, 255);
    u32 overlay_colour = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    C2D_PlainImageTint(&overlay_tint, overlay_colour, 0.5f);

    return 0;
}

void graphics_destroy() {
    C2D_SpriteSheetFree(main_sheet);
    C2D_SpriteSheetFree(gradient_sheet);
    destroy_font();
    C3D_RenderTargetDelete(top);
    C3D_RenderTargetDelete(bottom);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

void draw_background(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, unsigned w, unsigned h) {
    for (size_t i = 0; i < h / 4; i++) {
        C2D_DrawLine(x, i * 4 + (y + 1), MEDIUM_GREY, x + w, i * 4 + (y + 1), MEDIUM_GREY, 3, 0); //thickness is from middle
        C2D_DrawLine(x, i * 4 + (y + 3), LIGHT_GREY, x + w, i * 4 + (y + 3), LIGHT_GREY, 1, 0);
    }
}

void draw_top_screen_left(struct ipa_graphics_state_t* graphics) {
    C2D_DrawRectSolid(0, 0, 0, 40, TOP_HEIGHT, WHITE);

    u32 green = C2D_Color32(0x00, 0x51, 0x00, 0xFF);
    for (size_t x = 1; x <= 37; x += 3) {
        C2D_DrawLine(x, 19, green, x + 2, 19, green, 1, 0);
        C2D_DrawLine(x, 220, green, x + 2, 220, green, 1, 0);
    }

    C2D_DrawRectSolid(7, 209, 0, 26, 2, overlay_tint.corners->color); //colour bar

    C2D_DrawRectSolid(2, 4, 0, 17, 11, BLACK); //todo: battery
    C2D_DrawRectSolid(21, 1, 0, 16, 16, BLACK); //todo: wireless state
}

void draw_bevel_box(struct ipa_graphics_state_t* graphics, int x, int y, int w, int h, u32 bar1, u32 bar2, u32 border_in, u32 border_out) {
    u32 const bars[] = {bar1, bar2};
    for (size_t i = 0; i < (h - 6) / 2; i++) {
        int y1 = y + i * 2 + 3;
        int y2 = y + i * 2 + 4;
        C2D_DrawLine(x + 4, y1, bars[y1 % 2], x + w - 4, y1, bars[y1 % 2], 1, 0);
        C2D_DrawLine(x + 4, y2, bars[y2 % 2], x + w - 4, y2, bars[y2 % 2], 1, 0);
    }

    C2D_DrawLine(x + 5, y + 0, border_out, x + w - 4, y + 0, border_out, 1, 0);
    C2D_DrawLine(x + 5, y + 2, border_in, x + w - 5, y + 2, border_in, 2, 0); //top
    C2D_DrawLine(x + 5, y + 3, border_out, x + w - 4, y + 3, border_out, 1, 0);

    C2D_DrawLine(x + 0, y + 5, border_out, x + 5, y + 0, border_out, 1, 0);
    C2D_DrawLine(x + 0, y + 6, border_out, x + 6, y + 0, border_out, 1, 0);
    C2D_DrawLine(x + 2, y + 6, border_in, x + 7, y + 1, border_in, 2, 0); //top left
    C2D_DrawLine(x + 3, y + 7, border_out, x + 7, y + 3, border_out, 1, 0);

    C2D_DrawLine(x + w - 6, y + 0, border_out, x + w, y + 6, border_out, 1, 0);
    C2D_DrawLine(x + w - 5, y + 0, border_out, x + w, y + 5, border_out, 1, 0);
    C2D_DrawLine(x + w - 7, y + 1, border_in, x + w - 2, y + 6, border_in, 2, 0); //top right
    C2D_DrawLine(x + w - 7, y + 3, border_out, x + w - 3, y + 7, border_out, 1, 0);

    C2D_DrawLine(x, y + 6, border_out, x, y + h - 6, border_out, 1, 0);
    C2D_DrawLine(x + 2, y + 5, border_in, x + 2, y + h - 5, border_in, 2, 0); //left side
    C2D_DrawLine(x + 3, y + 6, border_out, x + 3, y + h - 6, border_out, 1, 0);

    C2D_DrawLine(x + w - 4, y + 6, border_out, x + w - 4, y + h - 6, border_out, 1, 0);
    C2D_DrawLine(x + w - 2, y + 5, border_in, x + w - 2, y + h - 5, border_in, 2, 0); //right side
    C2D_DrawLine(x + w - 1, y + 6, border_out, x + w - 1, y + h - 6, border_out, 1, 0);

    C2D_DrawLine(x + 5, y + h - 4, border_out, x + w - 4, y + h - 4, border_out, 1, 0);
    C2D_DrawLine(x + 5, y + h - 2, border_in, x + w - 5, y + h - 2, border_in, 2, 0); //bottom
    C2D_DrawLine(x + 5, y + h - 1, border_out, x + w - 4, y + h - 1, border_out, 1, 0);

    C2D_DrawLine(x, y + h - 5, border_out, x + 5, y + h, border_out, 1, 0);
    C2D_DrawLine(x, y + h - 6, border_out, x + 6, y + h, border_out, 1, 0);
    C2D_DrawLine(x + 2, y + h - 6, border_in, x + 6, y + h - 2, border_in, 2, 0); //bottom left
    C2D_DrawLine(x + 3, y + h - 7, border_out, x + 7, y + h - 3, border_out, 1, 0);

    C2D_DrawLine(x + w - 5, y + h, border_out, x + w, y + h - 5, border_out, 1, 0);
    C2D_DrawLine(x + w - 6, y + h, border_out, x + w, y + h - 6, border_out, 1, 0);
    C2D_DrawLine(x + w - 6, y + h - 2, border_in, x + w - 2, y + h - 6, border_in, 2, 0); //bottom right
    C2D_DrawLine(x + w - 7, y + h - 3, border_out, x + w - 3, y + h - 7, border_out, 1, 0);
}

void draw_main_menu_top(struct ipa_graphics_state_t* graphics) {
    C2D_SceneBegin(top);
    draw_background(graphics, 40, 0, TOP_WIDTH, TOP_HEIGHT);

    draw_top_screen_left(graphics);
    draw_bevel_box(graphics, 360, 0, 40, 240, DARK_GREY, BLACK, ORANGE, BLACK);
    // draw_bevel_box(graphics, 50, 11, 100, 56, dark_grey, black, white, green);
    // draw_bevel_box(graphics, 150, 12, 100, 56, dark_grey, black, green, white);

    draw_string(" !\"#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`", 2, 50);
    draw_string("abcdefghijklmnopqrstuvwxyz{|}~æ", 2, 75);
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
    C2D_DrawLine(x + 191, y + 1, DARK_GREY, x + 191, y + 32, DARK_GREY, 1, 0);
    C2D_DrawLine(x + 190, y + 1, WHITE, x + 190, y + 31, WHITE, 1, 0);
    C2D_DrawLine(x + 32, y + 30, WHITE, x + 190, y + 30, WHITE, 1, 0);
    C2D_DrawLine(x + 32, y + 31, BLACK, x + 191, y + 31, BLACK, 1, 0);
}

void draw_main_menu_bottom(struct ipa_graphics_state_t* graphics) {
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
        C2D_TopImageTint(&overlay_tint, overlay_tint.corners->color, FADE_TOP);
        C2D_BottomImageTint(&overlay_tint, overlay_tint.corners->color, FADE_BOTTOM);
        C2D_SpriteSetScale(gradient, 1, 1);
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);

        y = 217;
        C2D_TopImageTint(&overlay_tint, overlay_tint.corners->color, FADE_BOTTOM);
        C2D_BottomImageTint(&overlay_tint, overlay_tint.corners->color, FADE_TOP);
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_SpriteSetScale(gradient, 1, -1);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);
    }
    u32 black = C2D_Color32(0, 0, 0, 0xFF);
    C2D_DrawLine(0, 23, black, BOTTOM_WIDTH, 23, black, 1, 0);
    C2D_DrawLine(0, 216, black, BOTTOM_WIDTH, 216, black, 1, 0);
}

void draw_main_menu(struct ipa_graphics_state_t* graphics) {
    draw_main_menu_top(graphics);
    draw_main_menu_bottom(graphics);
}

void draw(matrix_client_t* client) {
    if (!client->graphics_state) {
        client->graphics_state = malloc(sizeof(*client->graphics_state));
    }

    struct ipa_graphics_state_t* graphics = client->graphics_state;
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
