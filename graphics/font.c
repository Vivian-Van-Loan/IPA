#include "font.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <wctype.h>

#include <citro2d.h>

#include "graphics.h"
#include "../utils.h"

typedef struct ipa_fontchar_t {
    wchar_t character;
    int8_t v_offset;
    int8_t width;
    int8_t h_offset;
    C2D_Sprite glyph;
} ipa_fontchar_t;

C2D_SpriteSheet font_sheet;
ipa_fontchar_t* font = nullptr;
size_t font_size = 0;

int EM10_WIDTH = 0;

int init_font() {
    int res = -1;
    FILE* info = nullptr;
    char* lineptr = nullptr;
    font_sheet = C2D_SpriteSheetLoad("romfs:/resources/gfx/IPA_TEXTURES_FONT.t3x");
    if (!font_sheet) {
        efuncprintf("Failed to load font sheet\n");
        goto error;
    }
    font_size = C2D_SpriteSheetCount(font_sheet);
    font = malloc(sizeof(*font) * font_size);
    if (!font) {
        efuncprintf("Failed to malloc font array\n");
        goto error;
    }
    info = fopen("romfs:/resources/gfx/font_info.txt", "r");
    if (!info) {
        efuncprintf("Failed to open font info file\n");
        goto error;
    }
    size_t n;
    ssize_t line_len;
    size_t chars_parsed = 0;
    mbstate_t conv = {0};
    while ((line_len = __getline(&lineptr, &n, info)) != -1) {
        if ((lineptr[0] == '/' && lineptr[1] == '/') || lineptr[0] == '\n') {
            continue;
        }
        if (chars_parsed >= font_size) {
            font_size++;
            void* new = realloc(font, sizeof(*font) * font_size);
            if (!new) {
                efuncprintf("Failed to realloc font buffer\n");
                goto error;
            }
            font = new;
        }
        ipa_fontchar_t* font_entry = &font[chars_parsed];
        C2D_SpriteFromSheet(&font_entry->glyph, font_sheet, chars_parsed);
        size_t offset;
        if (strstr(lineptr, "unknown") == lineptr) {
            offset = strlen("unknown");
            font_entry->character = WCHAR_MAX;
        }  else {
            offset = mbrtowc(&font_entry->character, lineptr, line_len, &conv);
        }
        char* lineptr_adjusted = lineptr + offset;
        char* end;
        font_entry->v_offset = strtol(lineptr_adjusted, &end, 10);
        lineptr_adjusted = end;
        font_entry->width = strtol(lineptr_adjusted, &end, 10);
        lineptr_adjusted = end;
        font_entry->h_offset = strtol(lineptr_adjusted, &end, 10);
        lineptr_adjusted = end;

        chars_parsed++;
    }
    free(lineptr);
    fclose(info);

    res = 0;
    EM10_WIDTH = get_string_width("mmmmmmmmmm");
    return res;

error:
    res = -1;
    free(lineptr);
    fclose(info);
    free(font);
    font = nullptr;
    font_size = 0;
    C2D_SpriteSheetFree(font_sheet);
    return res;
}

void destroy_font() {
    free(font);
    font = nullptr;
    font_size = 0;
    C2D_SpriteSheetFree(font_sheet);
}

ipa_fontchar_t* get_fontchar(wchar_t c) {
    for (size_t i = 0; i < font_size; i++) {
        if (font[i].character == c) {
            return &font[i];
        }
    }
    return &font[font_size - 1]; //should always be the unknown char
}

void draw_string(char const* str, int x, int y, u32 colour, float depth) {
    draw_string_until(str, x, y, colour, depth, strlen(str));
}

void draw_string_until(char const* str, int x, int y, u32 colour, float depth, size_t until) {
    C2D_ImageTint tint;
    C2D_PlainImageTint(&tint, colour, 1);

    mbstate_t conv = {0};
    while (*str && until > 0) {
        wchar_t c;
        size_t offset = mbrtowc(&c, str, until, &conv);
        if (offset == (size_t)-1) {
            efuncprintf("Encoding error\n");
            return;
        }
        str += offset;
        until -= offset;

        ipa_fontchar_t* fontchar = get_fontchar(c);
        C2D_SpriteSetPos(&fontchar->glyph, x + fontchar->h_offset, y + fontchar->v_offset);
        C2D_SpriteSetDepth(&fontchar->glyph, depth);
        C2D_DrawSpriteTinted(&fontchar->glyph, &tint);
        x += fontchar->width + 1;
    }
}

pair$int$size_t$ string_find_max_width(char const* str, int max_width) {
    mbstate_t conv = {0};
    size_t len = strlen(str);
    pair$int$size_t$ width_processed = {0};
    while (*str) {
        wchar_t c;
        size_t offset = mbrtowc(&c, str, len, &conv);
        if (offset == (size_t)-1) {
            efuncprintf("Encoding error\n");
            return width_processed;
        }

        ipa_fontchar_t* fontchar = get_fontchar(c);
        int full_width = get_char_full_width(fontchar->character);
        if (width_processed.first + full_width > max_width) {
            return width_processed;
        }
        width_processed.first += full_width;
        str += offset;
        len -= offset;
        width_processed.second += offset;
    }
    return width_processed;
}

size_t crop_leading_whitespace(char const* str) {
    mbstate_t conv = {0};
    size_t len = strlen(str);
    size_t offset = 0;
    while (*str) {
        wchar_t c;
        size_t offset_temp = mbrtowc(&c, str, len, &conv);
        if (offset_temp == (size_t)-1) {
            efuncprintf("Encoding error\n");
            return (size_t)-1;
        }
        if (iswspace(c)) {
            offset += offset_temp;
            str += offset;
            len -= offset;
            continue;
        }
        break;
    }
    return offset;
}

pair$int$size_t$ draw_string_max_width(char const* str, int x, int y, u32 colour, float depth, int max_width) {
    pair$int$size_t$ result = {0};

    size_t whitespace_offset = crop_leading_whitespace(str);
    if (whitespace_offset == (size_t)-1) {
        return result;
    }
    str += whitespace_offset;

    result = string_find_max_width(str, max_width); //find amount of chars after whitespace
    draw_string_until(str, x, y, colour, depth, result.second);
    result.second += whitespace_offset; //add whitespace skipped chars back into the skipped count
    return result;
}

int get_char_width(wchar_t c) {
    ipa_fontchar_t* fontchar = get_fontchar(c);
    return fontchar->width;
}

int get_char_full_width(wchar_t c) {
    ipa_fontchar_t* fontchar = get_fontchar(c);
    return fontchar->width + 1; //width, and 1 extra to get the full width (ie. where the next char would start).
}

int get_string_width(char const* str) {
    int width = 0;
    mbstate_t conv = {0};
    size_t len = strlen(str);
    while (*str) {
        wchar_t c;
        size_t offset = mbrtowc(&c, str, len, &conv);
        if (offset == (size_t)-1) {
            efuncprintf("Encoding error\n");
            return -1;
        }

        str += offset;
        len -= offset;

        width += get_char_full_width(c);
    }
    return width;
}

int get_string_width_itr(char const* str, size_t* offset, int* width, mbstate_t* conv) {
    if (*str == '\0') {
        return -1;
    }
    size_t len = strlen(str);
    wchar_t c;
    size_t num = mbrtowc(&c, str + *offset, len - *offset, conv);
    if (num == (size_t)-1) {
        efuncprintf("Encoding error\n");
        return -2;
    }
    *width += get_char_full_width(c);
    *offset += num;
    return 0;
}

ipa_string_t convert_string(char const* str) {
    ipa_string_t result = {0};
    mbstate_t conv = {0};
    char const* str2 = str;
    size_t nchars = mbsrtowcs(nullptr, &str2, 0, &conv);
    if (nchars == (size_t)-1) {
        efuncprintf("Failed to convert string\n");
        return result;
    }
    result.str = malloc(sizeof(wchar_t) * (nchars + 1));
    conv = (mbstate_t){0};
    str2 = str;
    mbsrtowcs(result.str, &str2, nchars + 1, &conv);
    if (str2) {
        efuncprintf("Failed to convert string\n");
        free(result.str);
        result.str = nullptr;
    }
    for (size_t i = 0; i < nchars; i++) {
        result.width += get_char_full_width(result.str[i]);
    }
    return result;
}

void ipa_string_destroy(ipa_string_t* str) {
    free(str->str);
    str->str = nullptr;
    str->width = 0;
}

//note: because of how we calc width and lazily crop, we can actually go over max width by a few. Shouldn't really matter in practice, just note its not exact
void ipa_string_crop(ipa_string_t* str, int max_width) {
    wchar_t crop_c = L'…';
    wchar_t nul_c = L'\0';

    if (str->width <= max_width) {
        return;
    }
    size_t len = wcslen(str->str);
    int width = 0;
    size_t i = 0;
    while (i < len) {
        int full_width = get_char_full_width(str->str[i]);
        width += full_width;
        if (width > max_width) {
            break;
        }
        i++;
    }
    while (i > 0) {
        wchar_t c = str->str[i];
        width -= get_char_full_width(c);
        str->str[i] = crop_c;
        str->str[i + 1] = nul_c;
        int crop_width = width + get_char_full_width(crop_c);
        if (crop_width <= max_width) {
            width = crop_width;
            break;
        }
        i--;
    }

    str->width = width;
    str->str = realloc(str->str, (wcslen(str->str) + 1) * sizeof(wchar_t));
}

ipa_string_t ipa_string_conv_crop(char const* str, int max_width) {
    ipa_string_t result = convert_string(str);
    ipa_string_crop(&result, max_width);
    return result;
}

void draw_ipa_string(ipa_string_t str, int x, int y, u32 colour, float depth) {
    C2D_ImageTint tint;
    C2D_PlainImageTint(&tint, colour, 1);

    size_t len = wcslen(str.str);
    for (size_t i = 0; i < len; i++) {
        ipa_fontchar_t* fontchar = get_fontchar(str.str[i]);
        C2D_SpriteSetPos(&fontchar->glyph, x + fontchar->h_offset, y + fontchar->v_offset);
        C2D_SpriteSetDepth(&fontchar->glyph, depth);
        C2D_DrawSpriteTinted(&fontchar->glyph, &tint);
        x += fontchar->width + 1;
    }
}
