#include "general.h"

void draw_background(struct ipa_graphics_state_t* graphics, unsigned x, unsigned y, unsigned w, unsigned h) {
    for (size_t i = 0; i < h / 4; i++) {
        C2D_DrawLineFix(x, i * 4 + (y + 1), MEDIUM_GREY, x + w, i * 4 + (y + 1), MEDIUM_GREY, 3, 0); //thickness is from middle
        C2D_DrawLineFix(x, i * 4 + (y + 3), LIGHT_GREY, x + w, i * 4 + (y + 3), LIGHT_GREY, 1, 0);
    }
}

void draw_top_screen_left(struct ipa_graphics_state_t* graphics, float base_depth) {
    C2D_DrawRectSolidFix(0, 0, base_depth, 40, TOP_HEIGHT, WHITE);

    u32 green = C2D_Color32(0x00, 0x51, 0x00, 0xFF);
    for (size_t x = 1; x <= 37; x += 3) {
        C2D_DrawLineFix(x, 19, green, x + 2, 19, green, 1, base_depth + DEPTH_STEP_SMALL);
        C2D_DrawLineFix(x, 220, green, x + 2, 220, green, 1, base_depth + DEPTH_STEP_SMALL);
    }

    C2D_DrawRectSolidFix(7, 209, base_depth + DEPTH_STEP_SMALL, 26, 2, overlay_colour); //colour bar

    C2D_DrawRectSolidFix(2, 4, base_depth + DEPTH_STEP_SMALL, 17, 11, BLACK); //todo: battery
    C2D_DrawRectSolidFix(21, 1, base_depth + DEPTH_STEP_SMALL, 16, 16, BLACK); //todo: wireless state
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
