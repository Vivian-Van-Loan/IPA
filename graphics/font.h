#ifndef MATRIX_CLIENT_FONT_H
#define MATRIX_CLIENT_FONT_H

#include <stdint.h>
#include <string.h>
#include <wchar.h>

#include <citro2d.h>

#include "../../types/general_types.h"

typedef struct ipa_string_t {
    wchar_t* str;
    int width;
} ipa_string_t;

extern int EM10_WIDTH;

int init_font();
void destroy_font();
void draw_string(char const* str, int x, int y, u32 colour, float depth);
void draw_string_until(char const* str, int x, int y, u32 colour, float depth, size_t until);
pair$int$size_t$ draw_string_max_width(char const* str, int x, int y, u32 colour, float depth, int max_width); //returns the width it did draw

int get_char_width(wchar_t c);
int get_char_full_width(wchar_t c);
int get_string_width(char const* str);
int get_string_width_itr(char const* str, size_t* offset, int* width, mbstate_t* conv);

ipa_string_t convert_string(char const* str);
void ipa_string_destroy(ipa_string_t* str);
void ipa_string_crop(ipa_string_t* str, int max_width);
ipa_string_t ipa_string_conv_crop(char const* str, int max_width);
void draw_ipa_string(ipa_string_t str, int x, int y, u32 colour, float depth);

#endif //MATRIX_CLIENT_FONT_H
