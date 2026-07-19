#include "graphics.h"

#include "font.h"
#include "images.h"
#include "../utils.h"
#include "../matrix/matrix-client.h"
#include "../matrix/matrix-utils.h"

struct ipa_graphics_state_t {
    ptrdiff_t time_on_menu;
    matrix_menu_t menu;
    ptrdiff_t selection;
    ptrdiff_t selection_num;
    ptrdiff_t scroll_pos;
    ptrdiff_t scroll_to_pos;
    ptrdiff_t anim_start_time;
    hash_map$img_path$ipa_image_t$ image_map;
    vector$pair$ipa_image_t_p$size_t$$ vram_images;
};

constexpr float FADE_TOP = 0.25f;
constexpr float FADE_BOTTOM = 1;
constexpr int SPEED = 3;
constexpr int BIG_AVATAR_SIZE = 32;
constexpr int SMALL_AVATAR_SIZE = 16;

C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
C2D_SpriteSheet main_sheet;
C2D_Sprite main_sprites[4];
C2D_SpriteSheet gradient_sheet;
C2D_Sprite gradient_sprites[2];

// C2D_ImageTint overlay_tint;
u32 overlay_colour;

int init_graphics() {
    make_dirs(IMG_CACHE_DIR);
    make_dirs(AVATAR_CACHE_DIR);
    img_path buf;
    snprintf(buf, sizeof(buf), "%s%zux%zu/", AVATAR_CACHE_DIR, BIG_AVATAR_SIZE, BIG_AVATAR_SIZE);
    make_dirs(buf);
    snprintf(buf, sizeof(buf), "%s%zux%zu/", AVATAR_CACHE_DIR, SMALL_AVATAR_SIZE, SMALL_AVATAR_SIZE);
    make_dirs(buf);

    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C3D_FrameRate(60);
    C2D_Prepare();

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

    overlay_colour = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    // C2D_PlainImageTint(&overlay_tint, overlay_colour, 0.5f);

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

pair$alloc_void$size_t$ download_image_data(matrix_client_t* client, char const* url, enum image_format* format_out) {
    pair$alloc_void$size_t$ out = matrix_download_mxc(client, url);
    if (!out.first) {
        out.second = 0;
        return out;
    }
    struct curl_header* header;
    CURLHcode h = curl_easy_header(curl_handle, "Content-Type", 0, CURLH_HEADER, -1, &header);
    if (h != CURLHE_OK) {
        efuncprintf("Failed to get content type\n");
        free(out.first);
        out.first = nullptr;
        out.second = 0;
        return out;
    }

    *format_out = IMAGE_FORMAT_UNKNOWN;
    if (strcasecmp(header->value, "image/png") == 0) {
        *format_out = IMAGE_FORMAT_PNG;
    } else if (strcasecmp(header->value, "image/jpeg") == 0) {
        *format_out = IMAGE_FORMAT_JPG;
    } else if (strcasecmp(header->value, "image/gif") == 0) {
        *format_out = IMAGE_FORMAT_GIF;
    } else if (strcasecmp(header->value, "image/webp") == 0) {
        *format_out = IMAGE_FORMAT_WEBP;
    } else if (strcasecmp(header->value, "image/bmp") == 0) {
        *format_out = IMAGE_FORMAT_BMP;
    }

    return out;
}

ipa_image_t* get_avatar(matrix_client_t* client, char const* mxc_url, size_t width, size_t height) {
    if (!mxc_url) {
        return nullptr;
    }

    ipa_image_t* image = lookup_avatar(&client->graphics_state->image_map, mxc_url, width, height);
    if (!image) {
        img_path path;
        ipa_image_t temp;
        form_avatar_path(path, strrchr(mxc_url, '/') + 1, width, height);
        FILE* f = fopen(path, "rb"); //let's go disk cache baby
        if (f) {
            off_t size = get_file_size_fd(fileno(f));
            unsigned char* data = malloc(size);
            // if (fread(data, 1, size, f) != size) {
            //     eprintf("Failed to read avatar image\n");
            //     free(data);
            //     fclose(f);
            //     return nullptr;
            // }
            fread(data, 1, size, f);
            fclose(f);
            temp = load_etc_image(data, width, height, IMAGE_FORMAT_ETC1A4);
            free(data);
            image = hash_map$img_path$ipa_image_t$_add(&client->graphics_state->image_map, path, temp);
            if (!image || !image->data) {
                destroy_image(&temp);
                return nullptr;
            }
            return image;
        }

        enum image_format format;
        pair$alloc_void$size_t$ data = download_image_data(client, mxc_url, &format);
        if (!data.first) {
            return nullptr;
        }
        temp = load_resize_compress_save(data.first, data.second, format, width, height, path);
        free(data.first);
        if (!temp.data) {
            return nullptr;
        }
        image = hash_map$img_path$ipa_image_t$_add(&client->graphics_state->image_map, path, temp);
        if (!image || !image->data) {
            destroy_image(&temp);
            return nullptr;
        }
    }
    return image;
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

    C2D_DrawRectSolid(7, 209, 0, 26, 2, overlay_colour); //colour bar

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

    draw_string(" !\"#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`", 2, 50, BLACK, 0);
    draw_string("abcdefghijklmnopqrstuvwxyz{|}~æ", 2, 75, BLACK, 0);
}

void draw_main_menu_button(C2D_Sprite* button_sprite, float x, float y, char const* text) { //expects a 32x32 sprite
    C2D_Sprite* gradient = &gradient_sprites[1];
    u32 top_grey = C2D_Color32(0x79, 0x79, 0x79, 0xFF);

    C2D_DrawRectSolid(x + 1, y + 1, 0, 30, 30, WHITE); //avatar backdrop

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
    C2D_DrawLine(x, y, DARK_GREY, x, y + 32, DARK_GREY, 1, 0.05f); //avatar box
    C2D_DrawLine(x + 32, y, DARK_GREY, x + 32, y + 32, DARK_GREY, 1, 0.05f);
    C2D_DrawLine(x + 32, y + 31, BLACK, x, y + 31, BLACK, 1, 0.05f);
    C2D_DrawLine(x, y + 32, DARK_GREY, x, y, DARK_GREY, 1, 0.05f);

    C2D_DrawLine(x + 32, y, top_grey, x + 192, y, top_grey, 1, 0);
    C2D_DrawLine(x + 191, y + 1, DARK_GREY, x + 191, y + 32, DARK_GREY, 1, 0);
    C2D_DrawLine(x + 190, y + 1, WHITE, x + 190, y + 31, WHITE, 1, 0);
    C2D_DrawLine(x + 32, y + 30, WHITE, x + 190, y + 30, WHITE, 1, 0);
    C2D_DrawLine(x + 32, y + 31, BLACK, x + 191, y + 31, BLACK, 1, 0);

    draw_string(text, x + 36, y + 12, DARK_GREY, 0);
}

void draw_main_menu_select(float x, float y) {
    u32 overlay_colour_75 = C2D_Color_new_alpha(overlay_colour, 0xBF);

    C2D_DrawLine(x + 1, y + 0, WHITE, x + 11, y + 0, WHITE, 1, 0.1f); //top left
    C2D_DrawLine(x + 10, y + 0, WHITE, x + 10, y + 5, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 10, y + 4, WHITE, x + 4, y + 4, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 4, y + 4, WHITE, x + 4, y + 11, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 4, y + 10, WHITE, x + 0, y + 10, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 0, y + 11, WHITE, x + 0, y + 1, WHITE, 1, 0.1f);
    C2D_DrawRectSolid(x + 1, y + 1, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolid(x + 1, y + 1, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolid(x + 1, y + 4, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolid(x + 1, y + 4, 0.1f, 3, 6, overlay_colour_75);

    C2D_DrawLine(x + 191, y + 0, WHITE, x + 201, y + 0, WHITE, 1, 0.1f); //top right
    C2D_DrawLine(x + 201, y + 1, WHITE, x + 201, y + 11, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 201, y + 10, WHITE, x + 197, y + 10, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 197, y + 11, WHITE, x + 197, y + 4, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 197, y + 4, WHITE, x + 191, y + 4, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 191, y + 5, WHITE, x + 191, y + 0, WHITE, 1, 0.1f);
    C2D_DrawRectSolid(x + 192, y + 1, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolid(x + 192, y + 1, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolid(x + 198, y + 4, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolid(x + 198, y + 4, 0.1f, 3, 6, overlay_colour_75);

    C2D_DrawLine(x + 197, y + 31, WHITE, x + 201, y + 31, WHITE, 1, 0.1f); //bot right
    C2D_DrawLine(x + 201, y + 31, WHITE, x + 201, y + 41, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 201, y + 41, WHITE, x + 191, y + 41, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 191, y + 41, WHITE, x + 191, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 191, y + 37, WHITE, x + 197, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 197, y + 38, WHITE, x + 197, y + 31, WHITE, 1, 0.1f);
    C2D_DrawRectSolid(x + 192, y + 38, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolid(x + 192, y + 38, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolid(x + 198, y + 32, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolid(x + 198, y + 32, 0.1f, 3, 6, overlay_colour_75);

    C2D_DrawLine(x + 0, y + 31, WHITE, x + 4, y + 31, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 4, y + 31, WHITE, x + 4, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 4, y + 37, WHITE, x + 10, y + 37, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 10, y + 37, WHITE, x + 10, y + 42, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 10, y + 41, WHITE, x + 1, y + 41, WHITE, 1, 0.1f);
    C2D_DrawLine(x + 0, y + 41, WHITE, x + 0, y + 31, WHITE, 1, 0.1f);
    C2D_DrawRectSolid(x + 1, y + 38, 0.1f, 9, 3, DARK_GREY);
    C2D_DrawRectSolid(x + 1, y + 38, 0.1f, 9, 3, overlay_colour_75);
    C2D_DrawRectSolid(x + 1, y + 32, 0.1f, 3, 6, DARK_GREY);
    C2D_DrawRectSolid(x + 1, y + 32, 0.1f, 3, 6, overlay_colour_75);
}

void draw_main_shared(struct ipa_graphics_state_t* graphics) {
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
    C2D_DrawLine(0, 23, black, BOTTOM_WIDTH, 23, black, 1, 0.25f);
    C2D_DrawLine(0, 216, black, BOTTOM_WIDTH, 216, black, 1, 0.25f);

    draw_string("Choose an Option.", 117, 6, BLACK, 0.25f);
}

void draw_main_menu_bottom(struct ipa_graphics_state_t* graphics) {
    C2D_SceneBegin(bottom);
    draw_main_shared(graphics);

    // for (size_t i = 0; i < 4; i++) {
    //     draw_main_menu_button(i, 64, i * 32 + max(BOTTOM_HEIGHT - graphics->time_on_menu * 3, 56));
    //     // draw_main_menu_button(i, 64, i * 32 + 56);
    // }
    draw_main_menu_button(main_sprites + 0, 64, 0 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - SPEED * 0) * SPEED, 56), "Direct Messages"); //the 4 main menu icons are first in the tex array
    draw_main_menu_button(main_sprites + 1, 64, 1 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - SPEED * 1) * SPEED, 56), "Spaces");
    draw_main_menu_button(main_sprites + 2, 64, 2 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - SPEED * 2) * SPEED, 56), "Rooms");
    draw_main_menu_button(main_sprites + 3, 64, 3 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - SPEED * 3) * SPEED, 56), "Settings");
    draw_main_menu_select(59, graphics->selection * 32 + max(BOTTOM_HEIGHT - 5 - (graphics->time_on_menu - SPEED * graphics->selection) * SPEED, 51));
}

void draw_main_menu(struct ipa_graphics_state_t* graphics) {
    graphics->selection_num = 4;

    draw_main_menu_top(graphics);
    draw_main_menu_bottom(graphics);
}

void draw_select_menu_bottom(matrix_client_t* client) {
    C2D_SceneBegin(bottom);

    struct ipa_graphics_state_t* graphics = client->graphics_state;
    ptrdiff_t const num_rooms = client->rooms.count;
    if (graphics->selection_num == 0) {
        graphics->selection_num = num_rooms;
    }
    // if (graphics->selection < 0) {
    //     graphics->selection = 0;
    //     graphics->scroll_pos = 0;
    //     graphics->scroll_to_pos = 0;
    //     // graphics->scroll_to_pos = min(num_rooms * AVATAR_SIZE, 6 * AVATAR_SIZE);
    // }
    if (graphics->scroll_pos > graphics->scroll_to_pos) {
        graphics->scroll_pos -= SPEED;
        if (graphics->scroll_pos < graphics->scroll_to_pos) {
            graphics->scroll_pos = graphics->scroll_to_pos;
        }
    } else if (graphics->scroll_pos < graphics->scroll_to_pos) {
        graphics->scroll_pos += SPEED;
        if (graphics->scroll_pos > graphics->scroll_to_pos) {
            graphics->scroll_pos = graphics->scroll_to_pos;
        }
    }

    draw_main_shared(graphics);

    ptrdiff_t const min_scroll_idx = max(0, graphics->scroll_pos - (6 * BIG_AVATAR_SIZE));
    while (graphics->vram_images.count && graphics->vram_images.data[0].second < min_scroll_idx) {
        ipa_image_t* image = vector$pair$ipa_image_t_p$size_t$$_get(&graphics->vram_images, 0)->first;
        image_unload_vram(image);
        vector$pair$ipa_image_t_p$size_t$$_remove(&graphics->vram_images, 0); //image is still backed by the hashmap so other data need not be freed
    }

    //a maximum of 6 buttons fit within margins (exact fit as well)
    //thus if we need to display a room with idx >= 6, we need to scroll the list
    //multiply graphics state scroll_to_pos by height of the buttons (32px)
    //scroll to at speed, thus the first idx to be drawn would be scroll_pos / height
    //gonna have to special case < 6 for vert centring but that's fine
    ptrdiff_t start_idx = max(0, graphics->scroll_pos / 32);
    ptrdiff_t end_idx = min(num_rooms, graphics->scroll_pos / 32 + 6); //todo: because of things scrolling off it will be possible to have 7, gotta either use that or case it
    if (start_idx > end_idx) {
        ptrdiff_t temp = start_idx;
        start_idx = end_idx;
        end_idx = temp;
    }
    for (ptrdiff_t i = start_idx; i < end_idx; i++) { //"make sure images are in the array" loop
        matrix_room_t* room = &client->rooms.data[i - start_idx];
        if (!room->avatar_url) {
            continue;
        }
        if (graphics->vram_images.count < i - start_idx + 1) { // || graphics->vram_images.data[i - start_idx].second != i - start_idx) {
            ipa_image_t* image = vector$pair$ipa_image_t_p$size_t$$_push(&graphics->vram_images, (pair$ipa_image_t_p$size_t$){get_avatar(client, room->avatar_url, BIG_AVATAR_SIZE, BIG_AVATAR_SIZE), i - start_idx})->first;
            image_load_vram(image);
        }
    }

    int button_y_start = 24;
    if (num_rooms < 6) {
        button_y_start = 24 + (6 - num_rooms) * 32 / 2;
    }
    for (ptrdiff_t i = start_idx; i < end_idx; i++) { //draw loop
        ptrdiff_t i_0 = i - start_idx;
        matrix_room_t* room = &client->rooms.data[i_0];
        C2D_Sprite* sprite = nullptr;
        if (graphics->vram_images.data[i_0].first) {
            sprite = graphics->vram_images.data[i_0].first->sprite;
        }
        draw_main_menu_button(sprite, 64, i_0 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - SPEED * i_0) * SPEED, button_y_start), room->name);
    }
    if (graphics->selection >= start_idx && graphics->selection < end_idx) {
        ptrdiff_t idx = graphics->selection - start_idx;
        draw_main_menu_select(59, idx * 32 + max(BOTTOM_HEIGHT - 5 - (graphics->time_on_menu - SPEED * graphics->selection) * SPEED, button_y_start - 5));
    }
}

void draw_select_menu(matrix_client_t* client) {
    draw_main_menu_top(client->graphics_state);
    draw_select_menu_bottom(client);
}

void draw_thin_box(int x, int y, int w, int h, u32 outline, u32 inside, bool fill) {
    if (fill) {
        C2D_DrawRectSolid(x + 2, y + 2, 0, w - 4, h - 4, inside); //infill middle
        C2D_DrawRectSolid(x + 4, y + 1, 0, w - 7, h - 2, inside); //top & bot
        C2D_DrawRectSolid(x + 1, y + 4, 0, w - 2, h - 7, inside); //left & right
    }

    C2D_DrawLine(x + 4, y + 0, outline, x + w - 5, y + 0, outline, 1, 0); //top left to top right
    C2D_DrawLine(x + w - 5, y + 0, outline, x + w - 1, y + 4, outline, 1, 0); //top right corner
    C2D_DrawLine(x + w - 1, y + 4, outline, x + w - 1, y + h - 4, outline, 1, 0); //top right to bot right
    C2D_DrawLine(x + w - 1, y + h - 4, outline, x + w - 4, y + h - 1, outline, 1, 0); //bot right corner
    C2D_DrawLine(x + w - 4, y + h - 1, outline, x + 4, y + h - 1, outline, 1, 0); //bot right to bot left
    C2D_DrawLine(x + 5, y + h, outline, x + 0, y + h - 5, outline, 1, 0); //bot left corner
    C2D_DrawLine(x + 0, y + h - 5, outline, x + 0, y + 5, outline, 1, 0); //bot left to top left
    C2D_DrawLine(x + 0, y + 5, outline, x + 5,y + 0, outline, 1, 0); //top left corner
}

void draw_message_box(struct ipa_graphics_state_t* graphics, int x, int y, int w, int num_lines, bool draw_lines, u32 dark, u32 light) {
    int h;
    if (num_lines == 1) {
        h = 22;
    } else {
        h = 40 + (num_lines - 2) * 18 + 1;
    }

    draw_thin_box(x, y, w, h, WHITE, WHITE, true);

    //dark border
    C2D_DrawLine(x + 4, y + 1, dark, x + w - 5, y + 1, dark, 1, 0); //top left to top right
    C2D_DrawLine(x + w - 5, y + 1, dark, x + w - 2, y + 4, dark, 1, 0); //top right corner
    C2D_DrawLine(x + w - 2, y + 4, dark, x + w - 2, y + h - 4, dark, 1, 0); //top right to bot right
    C2D_DrawLine(x + w - 2, y + h - 4, dark, x + w - 4, y + h - 2, dark, 1, 0); //bot right corner
    C2D_DrawLine(x + w - 4, y + h - 2, dark, x + 4, y + h - 2, dark, 1, 0); //bot right to bot left
    C2D_DrawLine(x + 5, y + h - 1, dark, x + 1, y + h - 5, dark, 1, 0); //bot left corner
    C2D_DrawLine(x + 1, y + h - 5, dark, x + 1, y + 5, dark, 1, 0); //bot left to top left
    C2D_DrawLine(x + 1, y + 5, dark, x + 5,y + 1, dark, 1, 0); //top left corner

    if (draw_lines && num_lines > 1) {
        C2D_DrawLine(x + 3, y + 20, light, x + w - 3, y + 20, light, 1, 0);
        for (int i = 0; i < num_lines - 2; i++) {
            C2D_DrawLine(x + 3, y + 38 + i * 18, light, x + w - 3, y + 38 + i * 18, light, 1, 0);
        }
    }
}

void draw_chat_room_top(struct matrix_client_t* client) {
    C2D_SceneBegin(top);
    draw_background(client->graphics_state, 40, 0, TOP_WIDTH, TOP_HEIGHT);
    draw_top_screen_left(client->graphics_state);
    draw_bevel_box(client->graphics_state, 360, 0, 40, 240, DARK_GREY, BLACK, ORANGE, BLACK);
}

void draw_keyboard_key(char c, int x, int y) {
    int char_width = get_char_width(c);
    draw_string((char const[]){c, 0}, x + (15 - char_width) / 2, y, SUPER_DARK_GREY, 0);
}

void draw_keyboard() { //todo: add a keyboard enum
    u32 overlay_colour_half = C2D_Color_new_alpha(overlay_colour, 0x80); //todo: see if a key is held and change its colour (draw rect over it with this) if so
    char constexpr row0[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '='};
    char constexpr row1[] = {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
    char constexpr row2[] = {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'};
    char constexpr row3[] = {'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'};
    char constexpr row4[] = {';', '\'', '[', ']'};
    for (size_t i = 0; i < 12; i++) {
        C2D_DrawRectSolid(89 + i * 16, 152, 0, 15, 14, SUPER_LIGHT_GREY);
        draw_keyboard_key(row0[i], 89 + i * 16, 155);
    }
    for (size_t i = 0; i < 10; i++) {
        C2D_DrawRectSolid(98 + i * 16, 168, 0, 15, 15, SUPER_LIGHT_GREY);
        draw_keyboard_key(row1[i], 98 + i * 16, 171);
    }
    C2D_DrawRectSolid(258, 168, 0, 25, 15, LIGHT_GREY);
    C2D_DrawRectSolid(87, 184, 0, 17, 15, LIGHT_GREY);
    for (size_t i = 0; i < 9; i++) {
        C2D_DrawRectSolid(105 + i * 16, 184, 0, 15, 15, SUPER_LIGHT_GREY);
        draw_keyboard_key(row2[i], 105 + i * 16, 187);
    }
    C2D_DrawRectSolid(249, 184, 0, 34, 15, LIGHT_GREY);
    C2D_DrawRectSolid(87, 200, 0, 25, 15, LIGHT_GREY);
    for (size_t i = 0; i < 10; i++) {
        C2D_DrawRectSolid(113 + i * 16, 200, 0, 15, 15, SUPER_LIGHT_GREY);
        draw_keyboard_key(row3[i], 113 + i * 16, 203);
    }

    C2D_DrawRectSolid(121, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[0], 121, 219);
    C2D_DrawRectSolid(137, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[1], 137, 219);
    C2D_DrawRectSolid(153, 216, 0, 79, 15, LIGHT_GREY);
    C2D_DrawRectSolid(233, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[2], 233, 219);
    C2D_DrawRectSolid(249, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[3], 249, 219);
}

void draw_chat_room_bottom(struct matrix_client_t* client) {
    C2D_SceneBegin(bottom);
    draw_background(client->graphics_state, 0, 48, 320, 192);
    C2D_DrawRectSolid(0, 0, 0, 320, 16, WHITE);
    C2D_DrawRectSolid(0, 48, 0, 37, 192, WHITE);
    C2D_DrawRectSolid(0, 236, 0, 320, 4, WHITE);
    C2D_DrawLine(0, 48, WHITE, 320, 48, WHITE, 1, 0);

    C2D_DrawRectSolid(36, 49, 0, 4, 2, WHITE); //fill some corners
    C2D_DrawRectSolid(36, 51, 0, 2, 2, WHITE);
    C2D_DrawRectSolid(37, 234, 0, 2, 4, WHITE);
    C2D_DrawLine(40, 49, DARK_GREY, 320, 49, DARK_GREY, 1, 0);
    C2D_DrawLine(36, 54, DARK_GREY, 41, 49, DARK_GREY, 1, 0);
    C2D_DrawLine(36, 53, DARK_GREY, 36, 232, DARK_GREY, 1, 0);
    C2D_DrawLine(36, 232, DARK_GREY, 41, 237, DARK_GREY, 1, 0);
    C2D_DrawLine(40, 236, DARK_GREY, 320, 236, DARK_GREY, 1, 0);

    draw_message_box(client->graphics_state, 39, 52, 280, 5, true, BLACK, ORANGE);

    draw_thin_box(39, 150, 44, 52, DARK_GREY, WHITE, true);
    draw_thin_box(39, 207, 44, 26, DARK_GREY, WHITE, true);
    draw_thin_box(85, 150, 200, 83, DARK_GREY, WHITE, true);
    draw_thin_box(288, 150, 36, 83, DARK_GREY, WHITE, true);
    draw_keyboard();

    C2D_DrawRectSolid(2, 86, 0, 14, 13, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 86, 0, 14, 13, MEDIUM_GREY);
    C2D_DrawRectSolid(2, 100, 0, 14, 13, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 100, 0, 14, 13, MEDIUM_GREY);

    C2D_DrawRectSolid(2, 119, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 119, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(2, 134, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 134, 0, 14, 14, MEDIUM_GREY);

    C2D_DrawRectSolid(2, 154, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 154, 0, 14, 14, DARK_GREY);
    C2D_DrawRectSolid(21, 155, 0, 12, 12, ORANGE);
    C2D_DrawRectSolid(2, 171, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 171, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(21, 172, 0, 12, 12, overlay_colour);
    C2D_DrawRectSolid(2, 188, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 188, 0, 14, 7, MEDIUM_GREY);

    C2D_DrawRectSolid(20, 205, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolid(20, 222, 0, 14, 14, MEDIUM_GREY);

    for (size_t i = 0; i < 192/2; i++) {
        C2D_DrawLine(17, 48 + i * 2, DARK_GREY, 19, 50 + i * 2, DARK_GREY, 1, 0);
    }
    for (size_t i = 0; i < 16 / 2; i++) {
        C2D_DrawLine(0 + i * 2, 81, DARK_GREY, 1 + i * 2, 81, DARK_GREY, 1, 0);
        C2D_DrawLine(21 + i * 2, 81, DARK_GREY, 22 + i * 2, 81, DARK_GREY, 1, 0);
        C2D_DrawLine(0 + i * 2, 150, DARK_GREY, 1 + i * 2, 150, DARK_GREY, 1, 0);
        C2D_DrawLine(21 + i * 2, 150, DARK_GREY, 22 + i * 2, 150, DARK_GREY, 1, 0);
    }
}

void draw_chat_room(matrix_client_t* client) {
    draw_chat_room_top(client);
    draw_chat_room_bottom(client);
}

void draw_and_input(matrix_client_t* client) {
    if (!client->graphics_state) {
        client->graphics_state = calloc(1, sizeof(*client->graphics_state));
    }

    struct ipa_graphics_state_t* graphics = client->graphics_state;
    if (client->menu != graphics->menu) {
        graphics->menu = client->menu;
        graphics->time_on_menu = 0;
        graphics->selection = 0;
        graphics->selection_num = 0;
        graphics->scroll_pos = 0;
        graphics->scroll_to_pos = 0;
        graphics->anim_start_time = 0;

        //todo: consider what to do about the hashmap
        while (graphics->vram_images.count) {
            ipa_image_t* image = vector$pair$ipa_image_t_p$size_t$$_get(&graphics->vram_images, 0)->first;
            image_unload_vram(image);
            vector$pair$ipa_image_t_p$size_t$$_remove(&graphics->vram_images, 0);
        }
    }

    while (!C3D_FrameBegin(C3D_FRAME_NONBLOCK)) {
        //gotta love a busy wait
        //needed to actually get beyond 30fps for reasons I do not understand
    }
    C2D_TargetClear(top, C2D_Color32(0xFF, 0, 0xFF, 0xFF));
    C2D_TargetClear(bottom, C2D_Color32(0xFF, 0, 0xFF, 0xFF));

    switch (client->menu) {
        case MENU_MAIN:
            draw_main_menu(graphics);
            break;
        case MENU_DMS:
        case MENU_SPACES:
        case MENU_ROOMS:
            draw_select_menu(client);
            break;
        case MENU_SETTINGS:
            break;
        case MENU_CHAT:
            draw_chat_room(client);
            break;
        case MENU_SPACE:
            break;
    }

    hidScanInput();

    u32 key_presses = hidKeysDown();

    if (key_presses & KEY_START)
        exit(0);
    if (key_presses & KEY_DOWN) {
        graphics->selection++;
        if (graphics->selection >= graphics->selection_num) {
            graphics->selection = graphics->selection_num - 1;
        }
    } else if (key_presses & KEY_UP) {
        graphics->selection--;
        if (graphics->selection < 0) {
            graphics->selection = 0;
        }
    }

    C3D_FrameEnd(0);
    graphics->time_on_menu++;
}
