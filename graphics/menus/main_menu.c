#include "main_menu.h"

#include "general.h"

void draw_main_top_shared(struct ipa_graphics_state_t* graphics) {
    C2D_SceneBegin(top);
    draw_background(graphics, 40, 0, TOP_WIDTH, TOP_HEIGHT);

    draw_top_screen_left(graphics, 0.25f);
    draw_bevel_box(graphics, 360, 0, 40, 240, DARK_GREY, BLACK, ORANGE, BLACK, 0.25f);
    // draw_bevel_box(graphics, 50, 11, 100, 56, dark_grey, black, white, green);
    // draw_bevel_box(graphics, 150, 12, 100, 56, dark_grey, black, green, white);
}

void draw_main_menu_button(C2D_Sprite* button_sprite, float x, float y, char const* text) { //expects a 32x32 sprite
    C2D_Sprite* gradient = &gradient_sprites[1];
    u32 top_grey = C2D_Color32(0x79, 0x79, 0x79, 0xFF);

    C2D_DrawRectSolidFix(x + 1, y + 1, 0, 30, 30, WHITE); //avatar backdrop

    if (button_sprite) {
        C2D_SpriteSetPos(button_sprite, x, y);
        C2D_SpriteSetDepth(button_sprite, 0);
        C2D_DrawSprite(button_sprite);
    } else {
        //todo: idk some nice fallback or something
    }
    for (size_t j = 0; j < 79; j++) {
        C2D_SpriteSetPos(gradient, x + 32 + j * 2, y + 1);
        C2D_DrawSprite(gradient);
    }
    C2D_DrawLineFix(x, y, DARK_GREY, x, y + 32, DARK_GREY, 1, 0.05f); //avatar box
    C2D_DrawLineFix(x + 32, y, DARK_GREY, x + 32, y + 32, DARK_GREY, 1, 0.05f);
    C2D_DrawLineFix(x + 32, y + 31, BLACK, x, y + 31, BLACK, 1, 0.05f);
    C2D_DrawLineFix(x, y + 32, DARK_GREY, x, y, DARK_GREY, 1, 0.05f);

    C2D_DrawLineFix(x + 32, y, top_grey, x + 192, y, top_grey, 1, 0);
    C2D_DrawLineFix(x + 191, y + 1, DARK_GREY, x + 191, y + 32, DARK_GREY, 1, 0);
    C2D_DrawLineFix(x + 190, y + 1, WHITE, x + 190, y + 31, WHITE, 1, 0);
    C2D_DrawLineFix(x + 32, y + 30, WHITE, x + 190, y + 30, WHITE, 1, 0);
    C2D_DrawLineFix(x + 32, y + 31, BLACK, x + 191, y + 31, BLACK, 1, 0);

    draw_string(text, x + 36, y + 12, DARK_GREY, 0);
}

void draw_main_menu_select(float x, float y) {
    u32 overlay_colour_75 = C2D_Color_new_alpha(overlay_colour, 0xBF);

    C2D_DrawLineFix(x + 1, y + 0, WHITE, x + 11, y + 0, WHITE, 1, 0.1f); //top left
    C2D_DrawLineFix(x + 10, y + 0, WHITE, x + 10, y + 5, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 10, y + 4, WHITE, x + 4, y + 4, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 4, y + 4, WHITE, x + 4, y + 11, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 4, y + 10, WHITE, x + 0, y + 10, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 0, y + 11, WHITE, x + 0, y + 1, WHITE, 1, 0.1f);
    C2D_DrawRectSolidFix(x + 1, y + 1, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolidFix(x + 1, y + 1, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolidFix(x + 1, y + 4, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolidFix(x + 1, y + 4, 0.1f, 3, 6, overlay_colour_75);

    C2D_DrawLineFix(x + 191, y + 0, WHITE, x + 201, y + 0, WHITE, 1, 0.1f); //top right
    C2D_DrawLineFix(x + 201, y + 1, WHITE, x + 201, y + 11, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 201, y + 10, WHITE, x + 197, y + 10, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 197, y + 11, WHITE, x + 197, y + 4, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 197, y + 4, WHITE, x + 191, y + 4, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 191, y + 5, WHITE, x + 191, y + 0, WHITE, 1, 0.1f);
    C2D_DrawRectSolidFix(x + 192, y + 1, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolidFix(x + 192, y + 1, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolidFix(x + 198, y + 4, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolidFix(x + 198, y + 4, 0.1f, 3, 6, overlay_colour_75);

    C2D_DrawLineFix(x + 197, y + 31, WHITE, x + 201, y + 31, WHITE, 1, 0.1f); //bot right
    C2D_DrawLineFix(x + 201, y + 31, WHITE, x + 201, y + 41, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 201, y + 41, WHITE, x + 191, y + 41, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 191, y + 41, WHITE, x + 191, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 191, y + 37, WHITE, x + 197, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 197, y + 38, WHITE, x + 197, y + 31, WHITE, 1, 0.1f);
    C2D_DrawRectSolidFix(x + 192, y + 38, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolidFix(x + 192, y + 38, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolidFix(x + 198, y + 32, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolidFix(x + 198, y + 32, 0.1f, 3, 6, overlay_colour_75);

    C2D_DrawLineFix(x + 0, y + 31, WHITE, x + 4, y + 31, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 4, y + 31, WHITE, x + 4, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 4, y + 37, WHITE, x + 10, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 10, y + 37, WHITE, x + 10, y + 42, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 10, y + 41, WHITE, x + 1, y + 41, WHITE, 1, 0.1f);
    C2D_DrawLineFix(x + 0, y + 41, WHITE, x + 0, y + 31, WHITE, 1, 0.1f);
    C2D_DrawRectSolidFix(x + 1, y + 38, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolidFix(x + 1, y + 38, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolidFix(x + 1, y + 32, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolidFix(x + 1, y + 32, 0.1f, 3, 6, overlay_colour_75);
}

void draw_main_bottom_shared(struct ipa_graphics_state_t* graphics) {
    draw_background(graphics, 0, 24, BOTTOM_WIDTH, BOTTOM_HEIGHT);

    C2D_Sprite* gradient = &gradient_sprites[0];
    C2D_ImageTint overlay_tint;
    for (size_t i = 0; i < BOTTOM_WIDTH / 2; i++) { //gradient top and bottom
        int y = 0;
        C2D_TopImageTint(&overlay_tint, overlay_colour, FADE_TOP);
        C2D_BottomImageTint(&overlay_tint, overlay_colour, FADE_BOTTOM);
        C2D_SpriteSetScale(gradient, 1, 1);
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_SpriteSetDepth(gradient, 0.25f);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);

        y = 217;
        C2D_TopImageTint(&overlay_tint, overlay_colour, FADE_BOTTOM);
        C2D_BottomImageTint(&overlay_tint, overlay_colour, FADE_TOP);
        C2D_SpriteSetPos(gradient, i * 2, y);
        C2D_SpriteSetScale(gradient, 1, -1);
        C2D_SpriteSetDepth(gradient, 0.25f);
        C2D_DrawSpriteTinted(gradient, &overlay_tint);
    }
    u32 black = C2D_Color32(0, 0, 0, 0xFF);
    C2D_DrawLineFix(0, 23, black, BOTTOM_WIDTH, 23, black, 1, 0.25f);
    C2D_DrawLineFix(0, 216, black, BOTTOM_WIDTH, 216, black, 1, 0.25f);

    draw_string("Choose an Option.", 117, 6, BLACK, 0.25f);
}

void draw_main_menu_bottom(struct ipa_graphics_state_t* graphics) {
    C2D_SceneBegin(bottom);
    draw_main_bottom_shared(graphics);

    // for (size_t i = 0; i < 4; i++) {
    //     draw_main_menu_button(i, 64, i * 32 + max(BOTTOM_HEIGHT - graphics->time_on_menu * 3, 56));
    //     // draw_main_menu_button(i, 64, i * 32 + 56);
    // }
    ptrdiff_t anim_time = graphics->time_on_menu - graphics->anim_start_time;
    draw_main_menu_button(main_sprites + 0, 64, 0 * 32 + max(BOTTOM_HEIGHT - (anim_time - SPEED * 0) * SPEED, 56), "Direct Messages"); //the 4 main menu icons are first in the tex array
    draw_main_menu_button(main_sprites + 1, 64, 1 * 32 + max(BOTTOM_HEIGHT - (anim_time - SPEED * 1) * SPEED, 56), "Spaces");
    draw_main_menu_button(main_sprites + 2, 64, 2 * 32 + max(BOTTOM_HEIGHT - (anim_time - SPEED * 2) * SPEED, 56), "Rooms");
    draw_main_menu_button(main_sprites + 3, 64, 3 * 32 + max(BOTTOM_HEIGHT - (anim_time - SPEED * 3) * SPEED, 56), "Settings");
    draw_main_menu_select(59, graphics->selection * 32 + max(BOTTOM_HEIGHT - 5 - (anim_time - SPEED * graphics->selection) * SPEED, 51));
}

void draw_main_menu(struct ipa_graphics_state_t* graphics) {
    graphics->selection_num = 4;

    draw_main_top_shared(graphics);
    draw_main_menu_bottom(graphics);
}