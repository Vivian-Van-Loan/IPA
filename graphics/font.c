#include "font.h"

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <citro2d.h>

#include "graphics.h"
#include "../utils.h"

C2D_SpriteSheet font_sheet;
ipa_fontchar_t* font = nullptr;
size_t font_size = 0;

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
    C2D_ImageTint tint;
    C2D_PlainImageTint(&tint, colour, 1);

    mbstate_t conv = {0};
    size_t len = strlen(str);
    while (*str) {
        wchar_t c;
        size_t offset = mbrtowc(&c, str, len, &conv);
        if (offset == (size_t)-1) {
            efuncprintf("Encoding error\n");
            return;
        }
        str += offset;
        len -= offset;

        ipa_fontchar_t* fontchar = get_fontchar(c);
        C2D_SpriteSetPos(&fontchar->glyph, x + fontchar->h_offset, y + fontchar->v_offset);
        C2D_SpriteSetDepth(&fontchar->glyph, depth);
        C2D_DrawSpriteTinted(&fontchar->glyph, &tint);
        x += fontchar->width + 1;
    }
}

int get_char_width(wchar_t c) {
    ipa_fontchar_t* fontchar = get_fontchar(c);
    return fontchar->width;
}
