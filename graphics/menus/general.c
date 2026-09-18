#include "general.h"

ipa_string_t loading_string;

void draw_background(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, unsigned w, unsigned h) {
    for (size_t i = 0; i < h / 4; i++) {
        C2D_DrawLineFix(x, i * 4 + (y + 1), MEDIUM_GREY, x + w, i * 4 + (y + 1), MEDIUM_GREY, 3, 0); //thickness is from middle
        C2D_DrawLineFix(x, i * 4 + (y + 3), LIGHT_GREY, x + w, i * 4 + (y + 3), LIGHT_GREY, 1, 0);
    }
}

void draw_wifi_sprite(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, float depth) {
    u8 strength = osGetWifiStrength();
    C2D_Sprite* signal = main_sprites + SIGNAL_SPRITE_OFFSET + strength;
    C2D_SpriteSetPos(signal, x, y);
    C2D_SpriteSetDepth(signal, depth);
    C2D_DrawSprite(signal);
}

void draw_battery_sprite(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, float depth) {
    bool charging = 0;
    u8 level = 4;
    PTMU_GetAdapterState(&charging); //so it is possible for these to be invalid (such as on azahar), in which case it will not write to the pointer
    PTMU_GetBatteryLevel(&level); //As such the default values catch it. Enjoy the perpetually empty battery on emulator!
    C2D_Sprite* battery;
    if (charging) {
        battery = main_sprites + BATTERY_SPRITE_OFFSET + BATTERY_SPRITE_LEN - 1; //charging is last
    } else {
        battery = main_sprites + BATTERY_SPRITE_OFFSET + level;
    }
    C2D_SpriteSetPos(battery, x, y);
    C2D_SpriteSetDepth(battery, depth);
    C2D_DrawSprite(battery);
}

void draw_top_screen_left(struct ipa_graphics_state_t* graphics, float base_depth) {
    C2D_DrawRectSolidFix(0, 0, base_depth, 40, TOP_HEIGHT, WHITE);

    u32 green = C2D_Color32(0x00, 0x51, 0x00, 0xFF);
    for (size_t x = 1; x <= 37; x += 3) {
        C2D_DrawLineFix(x, 19, green, x + 2, 19, green, 1, base_depth + DEPTH_STEP_SMALL);
        C2D_DrawLineFix(x, 220, green, x + 2, 220, green, 1, base_depth + DEPTH_STEP_SMALL);
    }

    C2D_DrawRectSolidFix(7, 209, base_depth + DEPTH_STEP_SMALL, 26, 2, overlay_colour); //colour bar

    draw_wifi_sprite(graphics, 21, 1, base_depth + DEPTH_STEP_SMALL);

    draw_battery_sprite(graphics, 2, 4, base_depth + DEPTH_STEP_SMALL);
}

void draw_bevel_box(struct ipa_graphics_state_t* graphics, int x, int y, int w, int h, u32 bar1, u32 bar2, u32 border_in, u32 border_out, float base_depth) {
    u32 const bars[] = {bar1, bar2};
    for (size_t i = 0; i < (h - 6) / 2; i++) {
        int y1 = y + i * 2 + 3;
        int y2 = y + i * 2 + 4;
        C2D_DrawLineFix(x + 4, y1, bars[y1 % 2], x + w - 4, y1, bars[y1 % 2], 1, base_depth + DEPTH_STEP_SMALL);
        C2D_DrawLineFix(x + 4, y2, bars[y2 % 2], x + w - 4, y2, bars[y2 % 2], 1, base_depth + DEPTH_STEP_SMALL);
    }

    C2D_DrawLineFix(x + 5, y + 0, border_out, x + w - 4, y + 0, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + 5, y + 2, border_in, x + w - 5, y + 2, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //top
    C2D_DrawLineFix(x + 5, y + 3, border_out, x + w - 4, y + 3, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x + 0, y + 5, border_out, x + 5, y + 0, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + 0, y + 6, border_out, x + 6, y + 0, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + 2, y + 6, border_in, x + 7, y + 1, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //top left
    C2D_DrawLineFix(x + 3, y + 7, border_out, x + 7, y + 3, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x + w - 6, y + 0, border_out, x + w, y + 6, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + w - 5, y + 0, border_out, x + w, y + 5, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + w - 7, y + 1, border_in, x + w - 2, y + 6, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //top right
    C2D_DrawLineFix(x + w - 7, y + 3, border_out, x + w - 3, y + 7, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x, y + 6, border_out, x, y + h - 6, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + 2, y + 5, border_in, x + 2, y + h - 5, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //left side
    C2D_DrawLineFix(x + 3, y + 6, border_out, x + 3, y + h - 6, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x + w - 4, y + 6, border_out, x + w - 4, y + h - 6, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + w - 2, y + 5, border_in, x + w - 2, y + h - 5, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //right side
    C2D_DrawLineFix(x + w - 1, y + 6, border_out, x + w - 1, y + h - 6, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x + 5, y + h - 4, border_out, x + w - 4, y + h - 4, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + 5, y + h - 2, border_in, x + w - 5, y + h - 2, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //bottom
    C2D_DrawLineFix(x + 5, y + h - 1, border_out, x + w - 4, y + h - 1, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x, y + h - 5, border_out, x + 5, y + h, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x, y + h - 6, border_out, x + 6, y + h, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + 2, y + h - 6, border_in, x + 6, y + h - 2, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //bottom left
    C2D_DrawLineFix(x + 3, y + h - 7, border_out, x + 7, y + h - 3, border_out, 1, base_depth + DEPTH_STEP_BIG);

    C2D_DrawLineFix(x + w - 5, y + h, border_out, x + w, y + h - 5, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + w - 6, y + h, border_out, x + w, y + h - 6, border_out, 1, base_depth + DEPTH_STEP_BIG);
    C2D_DrawLineFix(x + w - 6, y + h - 2, border_in, x + w - 2, y + h - 6, border_in, 2, base_depth + DEPTH_STEP_BIG + DEPTH_STEP_SMALL); //bottom right
    C2D_DrawLineFix(x + w - 7, y + h - 3, border_out, x + w - 3, y + h - 7, border_out, 1, base_depth + DEPTH_STEP_BIG);
}

void draw_load_sprite(struct ipa_graphics_state_t* graphics, int x, int y, u32 colour, float depth) {
    C2D_ImageTint load_tint;
    C2D_PlainImageTint(&load_tint, colour, 1);

    int load_frame = ((graphics->time_on_menu) / 4) % LOAD_SPRITE_LEN;
    C2D_Sprite* load_sprite = &main_sprites[LOAD_SPRITE_OFFSET + load_frame];
    C2D_SpriteSetDepth(load_sprite, depth);
    C2D_SpriteSetPos(load_sprite, x, y);
    C2D_DrawSpriteTinted(load_sprite, &load_tint);
}

void draw_load_box(struct ipa_graphics_state_t* graphics) {
    if (graphics->anim_id != IPA_ANIM_LOAD_IN && graphics->anim_id != IPA_ANIM_LOAD_OUT) {
        return;
    }
    if (!loading_string.str) {
        loading_string = ipa_string_convert("LOADING…");
    }
    int y;
    int x;
    constexpr int box_width = 287;
    constexpr int box_height = 70;
    constexpr int speed_mod = 2;
    if (graphics->anim_id == IPA_ANIM_LOAD_IN) {
        y = max(BOTTOM_HEIGHT - (graphics->time_on_menu - graphics->anim_start_time) * SPEED * speed_mod, (BOTTOM_HEIGHT - box_height) / 2);
        graphics->load_y = y;
    } else if (graphics->anim_id == IPA_ANIM_LOAD_OUT) {
        graphics->load_y += SPEED * speed_mod;
        y = graphics->load_y;
        if (graphics->load_y >= BOTTOM_HEIGHT) {
            graphics->anim_id = graphics->next_anim_id;
            graphics->anim_start_time = graphics->time_on_menu;
            graphics->load_y = BOTTOM_HEIGHT;
        }
    }
    x = (BOTTOM_WIDTH - box_width) / 2;
    int y_text = y + (box_height - CHAR_MAX_HEIGHT) / 2;
    int x_text = x + (box_width - loading_string.width) / 2;
    draw_bevel_box(graphics, x, y, box_width, box_height, DARK_GREY, BLACK, ORANGE, BLACK, 0.05f);
    draw_ipa_string(loading_string, x_text, y_text, WHITE, 0.05f);

    int y_sprite = y + (box_height - LOAD_SPRITE_WIDTH) / 2;
    int x_sprite_offset = 8;
    int x_sprite = x + x_sprite_offset;

    draw_load_sprite(graphics, x_sprite, y_sprite, YELLOW, 0.05f);

    x_sprite = x + box_width - LOAD_SPRITE_WIDTH - x_sprite_offset;
    draw_load_sprite(graphics, x_sprite, y_sprite, YELLOW, 0.05f);
}
