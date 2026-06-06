#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include <malloc.h>
#include <citro2d.h>

#include <curl/curl.h>
#include <jansson.h>

#include "utils.h"
#include "matrix/matrix-client.h"
#include "matrix/matrix-utils.h"
#include "matrix/login.h"

C3D_RenderTarget* bottom;
C2D_SpriteSheet main_sheet;
C2D_Sprite main_sprites[4];
C2D_SpriteSheet gradient_sheet;
C2D_Sprite gradient_sprites[2];
C2D_ImageTint overlay_tint;
int init_sprites() { //todo: MOVE ALL OF THIS TO DEDICATED GRAPHICS CODE
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

void draw_main_menu() {
    //background
    u32 dark_grey = C2D_Color32(0xAA, 0xAA, 0xAA, 0xFF);
    u32 light_grey = C2D_Color32(0xBA, 0xBA, 0xBA, 0xFF);
    for (size_t i = 0; i < 48; i++) {
        C2D_DrawLine(0, i * 4 + 25, dark_grey, 320, i * 4 + 25, dark_grey, 3, 0); //thickness is from middle
        C2D_DrawLine(0, i * 4 + 27, light_grey, 320, i * 4 + 27, light_grey, 1, 0);
    }

    C2D_Sprite* gradient = &gradient_sprites[1];
    u32 top_grey = C2D_Color32(0x79, 0x79, 0x79, 0xFF);
    u32 side_grey = C2D_Color32(0x49, 0x49, 0x49, 0xFF);
    u32 black = C2D_Color32(0, 0, 0, 0xFF);
    u32 white = C2D_Color32(0xFB, 0xFB, 0xFB, 0xFF);
    for (size_t i = 0; i < 4; i++) { //main buttons
        C2D_DrawSprite(main_sprites + i);
        for (size_t j = 0; j < 79; j++) {
            C2D_SpriteSetPos(gradient, j * 2 + 96, i * 32 + 57);
            C2D_DrawSprite(gradient);
        }

        C2D_DrawLine(96, i * 32 + 56, top_grey, 256, i * 32 + 56, top_grey, 1, 0);
        C2D_DrawLine(255, i * 32 + 57, side_grey, 255, i * 32 + 88, side_grey, 1, 0.5f);
        C2D_DrawLine(254, i * 32 + 57, white, 254, i * 32 + 87, white, 1, 0.5f);
        C2D_DrawLine(96, i * 32 + 86, white, 254, i * 32 + 86, white, 1, 0);
        C2D_DrawLine(96, i * 32 + 87, black, 255, i * 32 + 87, black, 1, 0);
    }

    gradient = &gradient_sprites[0];
    for (size_t i = 0; i < 320 / 2; i++) { //gradient top and bottom
        int y = 0;
        C2D_SpriteSetScale(gradient, 1, 1);
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);

        y = 217;
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_SpriteSetScale(gradient, 1, -1);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);
    }
    C2D_DrawLine(0, 23, black, 320, 23, black, 1, 0);
    C2D_DrawLine(0, 216, black, 320, 216, black, 1, 0);
}

int main(int argc, char** argv) {
    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_TOP, nullptr);

    int res = init_general();
    if (res) {
        return res;
    }

    res = init_curl();
    if (res) {
        return res;
    }

    // matrix_login_t login = matrix_login_pass("server", "user", "pass", "IPA-testing");
    // matrix_client_t client = {.login = login};
    // matrix_client_sync_account_data(&client);
    // printf("Logged in and synced, good luck reading it\n");

    res = init_sprites();
    if (res) {
        return res;
    }

    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        // Flush and swap framebuffers
        // gfxFlushBuffers();
        // gfxSwapBuffers();

        // Render the scene
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(bottom, C2D_Color32(0xFF, 0, 0, 0xFF));
        C2D_SceneBegin(bottom);

        draw_main_menu();

        C3D_FrameEnd(0);

        gspWaitForVBlank();
    }

    destroy_curl();
    gfxExit();
    return 0;
}
