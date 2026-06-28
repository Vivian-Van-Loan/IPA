#ifndef MATRIX_CLIENT_FONT_H
#define MATRIX_CLIENT_FONT_H

#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include <citro2d.h>

typedef struct ipa_fontchar_t {
    wchar_t character;
    int8_t v_offset;
    int8_t width;
    int8_t h_offset;
    C2D_Sprite glyph;
} ipa_fontchar_t;

extern ipa_fontchar_t* font;
extern size_t font_size;

int init_font();
void destroy_font();
void draw_string(char const* str, int x, int y, u32 colour, float depth);

int get_char_width(wchar_t c);

#endif //MATRIX_CLIENT_FONT_H
