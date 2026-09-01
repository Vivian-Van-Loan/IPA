#include "graphics.h"

#include "font.h"
#include "images.h"
#include "../utils.h"
#include "../matrix/matrix-client.h"
#include "../matrix/matrix-utils.h"

#include "menus/general.h"
#include "menus/main_menu.h"
#include "menus/select_menu.h"

C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
C2D_SpriteSheet main_sheet;
C2D_Sprite main_sprites[4];
C2D_SpriteSheet gradient_sheet;
C2D_Sprite gradient_sprites[2];

// C2D_ImageTint overlay_tint;
u32 overlay_colour;
u32 overlay_colour_dark;
u32 overlay_colour_light;

ipa_string_t loading_string;

unsigned char overlay_channel(unsigned char base, unsigned char overlay) {
    float b = (float)base / 255.0f;
    float o = (float)overlay / 255.0f;
    if (b < 0.5) {
        return (unsigned char)(2 * b * o * 255.0f);
    } else {
        return (unsigned char)((1 - 2 * (1 - b) * (1 - o)) * 255.0f);
    }
}

u32 C2D_Color_Overlay(u32 base, u32 overlay) {
    unsigned char r = overlay_channel(C2D_Color_get_red(base), C2D_Color_get_red(overlay));
    unsigned char g = overlay_channel(C2D_Color_get_green(base), C2D_Color_get_green(overlay));
    unsigned char b = overlay_channel(C2D_Color_get_blue(base), C2D_Color_get_blue(overlay));
    // unsigned char a = overlay_channel(C2D_Color_get_alpha(base), C2D_Color_get_alpha(overlay));
    return C2D_Color32(r, g, b, C2D_Color_get_alpha(base));
}

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
    overlay_colour_dark = C2D_Color_Overlay(DARK_GREY, overlay_colour);
    overlay_colour_light = C2D_Color_Overlay(LIGHT_GREY, overlay_colour);
    // C2D_PlainImageTint(&overlay_tint, overlay_colour, 0.5f);

    loading_string = ipa_string_convert("LOADING…");

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

void draw_thin_box(int x, int y, int w, int h, u32 outline, u32 inside, bool fill) {
    if (fill) {
        C2D_DrawRectSolidFix(x + 2, y + 2, 0, w - 4, h - 4, inside); //infill middle
        C2D_DrawRectSolidFix(x + 4, y + 1, 0, w - 7, h - 2, inside); //top & bot
        C2D_DrawRectSolidFix(x + 1, y + 4, 0, w - 2, h - 7, inside); //left & right
    }

    C2D_DrawLineFix(x + 4, y + 0, outline, x + w - 5, y + 0, outline, 1, 0); //top left to top right
    C2D_DrawLineFix(x + w - 5, y + 0, outline, x + w - 1, y + 4, outline, 1, 0); //top right corner
    C2D_DrawLineFix(x + w - 1, y + 4, outline, x + w - 1, y + h - 4, outline, 1, 0); //top right to bot right
    C2D_DrawLineFix(x + w - 1, y + h - 4, outline, x + w - 4, y + h - 1, outline, 1, 0); //bot right corner
    C2D_DrawLineFix(x + w - 4, y + h - 1, outline, x + 4, y + h - 1, outline, 1, 0); //bot right to bot left
    C2D_DrawLineFix(x + 5, y + h, outline, x + 0, y + h - 5, outline, 1, 0); //bot left corner
    C2D_DrawLineFix(x + 0, y + h - 5, outline, x + 0, y + 5, outline, 1, 0); //bot left to top left
    C2D_DrawLineFix(x + 0, y + 5, outline, x + 5,y + 0, outline, 1, 0); //top left corner
}

int draw_message_box(struct ipa_graphics_state_t* graphics, int x, int y, int w, int num_lines, bool draw_lines, u32 dark, u32 light, u32 back) {
    int h;
    if (num_lines == 1) {
        h = 22;
    } else {
        h = 40 + (num_lines - 2) * 18 + 1;
    }

    draw_thin_box(x, y, w, h, back, back, true);

    //dark border
    C2D_DrawLineFix(x + 4, y + 1, dark, x + w - 5, y + 1, dark, 1, 0); //top left to top right
    C2D_DrawLineFix(x + w - 5, y + 1, dark, x + w - 2, y + 4, dark, 1, 0); //top right corner
    C2D_DrawLineFix(x + w - 2, y + 4, dark, x + w - 2, y + h - 4, dark, 1, 0); //top right to bot right
    C2D_DrawLineFix(x + w - 2, y + h - 4, dark, x + w - 4, y + h - 2, dark, 1, 0); //bot right corner
    C2D_DrawLineFix(x + w - 4, y + h - 2, dark, x + 4, y + h - 2, dark, 1, 0); //bot right to bot left
    C2D_DrawLineFix(x + 5, y + h - 1, dark, x + 1, y + h - 5, dark, 1, 0); //bot left corner
    C2D_DrawLineFix(x + 1, y + h - 5, dark, x + 1, y + 5, dark, 1, 0); //bot left to top left
    C2D_DrawLineFix(x + 1, y + 5, dark, x + 5,y + 1, dark, 1, 0); //top left corner

    if (draw_lines && num_lines > 1) {
        C2D_DrawLineFix(x + 3, y + 20, light, x + w - 3, y + 20, light, 1, 0);
        for (int i = 0; i < num_lines - 2; i++) {
            C2D_DrawLineFix(x + 3, y + 38 + i * 18, light, x + w - 3, y + 38 + i * 18, light, 1, 0);
        }
    }

    return h;
}

int draw_message_box_text(matrix_client_t* client, char const* name, char const* text, char const* avatar_url, int x, int y, int w, u32 dark, u32 light, u32 back, u32 text_c, int max_lines) {
    int avatar_width = 16;
    int avatar_left = x + 4;
    int avatar_top = y + 3;
    ipa_image_t* avatar = get_avatar(&client->graphics_state->image_map, avatar_url, avatar_width, avatar_width);
    if (!avatar) {
        // avatar_left = x;
        // avatar_width = 0;
        ipa_graphics_queue_t queue_entry = {IPA_QUEUE_AVATAR};
        queue_entry.id = queue_avatar_download(client, avatar_url, avatar_width, avatar_width);
    } else {
        image_load_vram(avatar);
    }

    ipa_string_t name_ipa = ipa_string_conv_crop(name, MAX_NAME_WIDTH, L'…'); //honestly the allocation and crop shouldn't matter

    int const v_text_offset = 7;
    int box_width = (avatar_left - x) + avatar_width + 1 + name_ipa.width; //actually the inner portion, not the outer
    if (avatar_width == 0 && name_ipa.width == 0) {
        box_width = 0;
    }

    int const w_text_pad = 5 + 2; //5 pixels from box edge, 2 pixels for box edge
    int text_y = y + v_text_offset;
    int text_x = x + box_width + 5;
    int num_lines = 0;
    int text_width = x + (w - w_text_pad - text_x);
    size_t offset = 0;
    size_t len = strlen(text);
    while (text_y < BOTTOM_HEIGHT && (num_lines < max_lines || max_lines <= 0) && offset < len) {
        C2D_DrawRectSolidFix(text_x, text_y, 0, text_width, 14, back); //cover up an alpha-depth compositing problem
        pair$int$size_t$ result = draw_string_max_width(text + offset, text_x, text_y, text_c, 0.05f, text_width, true);
        offset += result.second;
        text_y += 18;
        text_x = x + w_text_pad;
        text_width = (w - w_text_pad * 2); //since w is entire box width we have to take the padding from both sides off
        num_lines++;
    }
    int h = draw_message_box(client->graphics_state, x, y, w, num_lines, true, dark, light, back);

    if (box_width) {
        C2D_DrawRectSolidFix(x + 3, y + 2, 0, box_width - 2, 17, light);
        if (num_lines > 1) {
            C2D_DrawLineFix(x + 2, y + 4, light, x + 2, y + 20, light, 1, 0);
            C2D_DrawLineFix(x + 2, y + 19, light, x + box_width - 1, y + 19, light, 1, 0);
            C2D_DrawLineFix(x + 1, y + 20, dark, x + box_width - 1, y + 20, dark, 1, 0);
        } else {
            C2D_DrawLineFix(x + 2, y + 4, light, x + 2, y + 18, light, 1, 0);
            C2D_DrawLineFix(x + 4, y + 19, light, x + box_width - 1, y + 19, light, 1, 0);
            C2D_DrawLineFix(x + 4, y + 20, dark, x + box_width - 1, y + 20, dark, 1, 0);
        }
        C2D_DrawLineFix(x + 1, y + 5, dark, x + 5, y + 1, dark, 1, 0); //recover the top left line
        C2D_DrawLineFix(x + box_width - 1, y + 20, dark, x + box_width + 2, y + 17, dark, 1, 0);
        C2D_DrawLineFix(x + box_width + 1, y + 18, dark, x + box_width + 1, y + 2, dark, 1, 0);
    }

    if (avatar) {
        C2D_SpriteSetPos(avatar->sprite, avatar_left, avatar_top);
        C2D_SpriteSetDepth(avatar->sprite, 0);
        C2D_DrawSprite(avatar->sprite);
    }

    draw_ipa_string(name_ipa, avatar_left + avatar_width + 1, y + v_text_offset, dark, 0);
    ipa_string_destroy(&name_ipa);

    return h;
}

int draw_event(matrix_client_t* client, matrix_event_t* event, int x, int y, int w) {
    if (event_is_message(event->type.first)) {
        matrix_message_t message = event->message;
        if (!message.body) {
            return 0;
        }
        matrix_event_t* sender = matrix_room_get_user(client->current_room, event->sender);
        char const* name = nullptr;
        char const* url = nullptr;
        if (sender && sender->type.first == EVENT_ROOM_MEMBER) {
            name = sender->room.member.display_name;
            url = sender->room.member.avatar_url;
        }
        return draw_message_box_text(client, name, message.body, url, x, y, w, overlay_colour_dark, overlay_colour_light, WHITE, BLACK, 0);
    } else if (event_is_room(event->type.first)) {
        char const* text;
        u32 back = BLACK;
        u32 text_c = SILVER;
        u32 light = WHITE; //never actually rendered
        u32 dark = WHITE;
        switch (event->type.first) {
            case EVENT_ROOM_CREATE:
                text = "Room created";
                break;
            case EVENT_ROOM_NAME:
                text = "Room name changed";
                break;
            case EVENT_ROOM_AVATAR:
                text = "Room avatar changed";
                break;
            case EVENT_ROOM_TOPIC:
                text = "Room topic changed";
                break;
            case EVENT_ROOM_MEMBER:
                text = "User joined";
                break;
            case EVENT_ROOM_POWER_LEVELS:
            case EVENT_ROOM_REDACTION:
            case EVENT_ROOM_JOIN_RULES:
            case EVENT_ROOM_CANON_ALIAS:
            case EVENT_ROOM_ENCRYPT:
            default:
                return 0;
        }

        return draw_message_box_text(client, nullptr, text, nullptr, x, y, w, dark, light, back, text_c, 1);
    }
    return 0;
}

void draw_chat_room_top(struct matrix_client_t* client) {
    C2D_SceneBegin(top);
    draw_background(client->graphics_state, 40, 0, TOP_WIDTH, TOP_HEIGHT);
    draw_top_screen_left(client->graphics_state, 0.25f);
    draw_bevel_box(client->graphics_state, 360, 0, 40, 240, DARK_GREY, BLACK, ORANGE, BLACK, 0.25f);

    dequeue$matrix_event_t$* events = &client->current_room->events;
    int y = 1;
    for (size_t i = 0; i < events->count; i++) {
        matrix_event_t* event = &events->data[i];
        int offset = draw_event(client, event, 40 + 2, y, 320 - 3);
        if (offset) {
            y += offset + 2;
        }
    }
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
        C2D_DrawRectSolidFix(89 + i * 16, 152, 0, 15, 14, SUPER_LIGHT_GREY);
        draw_keyboard_key(row0[i], 89 + i * 16, 155);
    }
    for (size_t i = 0; i < 10; i++) {
        C2D_DrawRectSolidFix(98 + i * 16, 168, 0, 15, 15, SUPER_LIGHT_GREY);
        draw_keyboard_key(row1[i], 98 + i * 16, 171);
    }
    C2D_DrawRectSolidFix(258, 168, 0, 25, 15, LIGHT_GREY);
    C2D_DrawRectSolidFix(87, 184, 0, 17, 15, LIGHT_GREY);
    for (size_t i = 0; i < 9; i++) {
        C2D_DrawRectSolidFix(105 + i * 16, 184, 0, 15, 15, SUPER_LIGHT_GREY);
        draw_keyboard_key(row2[i], 105 + i * 16, 187);
    }
    C2D_DrawRectSolidFix(249, 184, 0, 34, 15, LIGHT_GREY);
    C2D_DrawRectSolidFix(87, 200, 0, 25, 15, LIGHT_GREY);
    for (size_t i = 0; i < 10; i++) {
        C2D_DrawRectSolidFix(113 + i * 16, 200, 0, 15, 15, SUPER_LIGHT_GREY);
        draw_keyboard_key(row3[i], 113 + i * 16, 203);
    }

    C2D_DrawRectSolidFix(121, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[0], 121, 219);
    C2D_DrawRectSolidFix(137, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[1], 137, 219);
    C2D_DrawRectSolidFix(153, 216, 0, 79, 15, LIGHT_GREY);
    C2D_DrawRectSolidFix(233, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[2], 233, 219);
    C2D_DrawRectSolidFix(249, 216, 0, 15, 15, SUPER_LIGHT_GREY);
    draw_keyboard_key(row4[3], 249, 219);
}

void draw_chat_room_bottom(struct matrix_client_t* client) {
    C2D_SceneBegin(bottom);
    draw_background(client->graphics_state, 0, 48, 320, 192);
    C2D_DrawRectSolidFix(0, 0, 0, 320, 16, WHITE);
    C2D_DrawRectSolidFix(0, 48, 0, 37, 192, WHITE);
    C2D_DrawRectSolidFix(0, 236, 0, 320, 4, WHITE);
    C2D_DrawLineFix(0, 48, WHITE, 320, 48, WHITE, 1, 0);

    C2D_DrawRectSolidFix(36, 49, 0, 4, 2, WHITE); //fill some corners
    C2D_DrawRectSolidFix(36, 51, 0, 2, 2, WHITE);
    C2D_DrawRectSolidFix(37, 234, 0, 2, 4, WHITE);
    C2D_DrawLineFix(40, 49, DARK_GREY, 320, 49, DARK_GREY, 1, 0);
    C2D_DrawLineFix(36, 54, DARK_GREY, 41, 49, DARK_GREY, 1, 0);
    C2D_DrawLineFix(36, 53, DARK_GREY, 36, 232, DARK_GREY, 1, 0);
    C2D_DrawLineFix(36, 232, DARK_GREY, 41, 237, DARK_GREY, 1, 0);
    C2D_DrawLineFix(40, 236, DARK_GREY, 320, 236, DARK_GREY, 1, 0);

    // draw_message_box(client->graphics_state, 39, 52, 280, 5, true, BLACK, ORANGE);
    pthread_mutex_lock(&client->login.mutex);
    matrix_event_t* user = matrix_room_get_user(client->current_room, client->login.user_id);
    pthread_mutex_unlock(&client->login.mutex);
    draw_message_box_text(client, user->room.member.display_name,
        "Hello everybody my name is Markiplier and welcome to Five Nights at Freddies, an indie horror game that you guys suggested in mass, and I saw that Yamimash played it and he said that it was really really good; so I'm very eager to see what is up - and that is a terrifying animatronic bear reads off script family pizzeria looking for security guard to work the night shift. Oh, 12:00 A.M, the first night. If I didn’t want to stay the first night, why would I stay any more than five? Why would I say anymore than two - hello. Okay...Hello? Hello - oh, ah I can’t move. That’s a creepy skull...There’s creepy things on the wall - Oh, hello.",
        user->room.member.avatar_url, 39, 52, 280, overlay_colour_dark, overlay_colour_light, WHITE, BLACK, 5);

    draw_thin_box(39, 150, 44, 52, DARK_GREY, WHITE, true);
    draw_thin_box(39, 207, 44, 26, DARK_GREY, WHITE, true);
    draw_thin_box(85, 150, 200, 83, DARK_GREY, WHITE, true);
    draw_thin_box(288, 150, 36, 83, DARK_GREY, WHITE, true);
    draw_keyboard();

    C2D_DrawRectSolidFix(2, 86, 0, 14, 13, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 86, 0, 14, 13, MEDIUM_GREY);
    C2D_DrawRectSolidFix(2, 100, 0, 14, 13, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 100, 0, 14, 13, MEDIUM_GREY);

    C2D_DrawRectSolidFix(2, 119, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 119, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(2, 134, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 134, 0, 14, 14, MEDIUM_GREY);

    C2D_DrawRectSolidFix(2, 154, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 154, 0, 14, 14, DARK_GREY);
    C2D_DrawRectSolidFix(21, 155, 0, 12, 12, ORANGE);
    C2D_DrawRectSolidFix(2, 171, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 171, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(21, 172, 0, 12, 12, overlay_colour);
    C2D_DrawRectSolidFix(2, 188, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 188, 0, 14, 7, MEDIUM_GREY);

    C2D_DrawRectSolidFix(20, 205, 0, 14, 14, MEDIUM_GREY);
    C2D_DrawRectSolidFix(20, 222, 0, 14, 14, MEDIUM_GREY);

    for (size_t i = 0; i < 192/2; i++) {
        C2D_DrawLineFix(17, 48 + i * 2, DARK_GREY, 19, 50 + i * 2, DARK_GREY, 1, 0);
    }
    for (size_t i = 0; i < 16 / 2; i++) {
        C2D_DrawLineFix(0 + i * 2, 81, DARK_GREY, 1 + i * 2, 81, DARK_GREY, 1, 0);
        C2D_DrawLineFix(21 + i * 2, 81, DARK_GREY, 22 + i * 2, 81, DARK_GREY, 1, 0);
        C2D_DrawLineFix(0 + i * 2, 150, DARK_GREY, 1 + i * 2, 150, DARK_GREY, 1, 0);
        C2D_DrawLineFix(21 + i * 2, 150, DARK_GREY, 22 + i * 2, 150, DARK_GREY, 1, 0);
    }
}

void draw_chat_room(matrix_client_t* client) {
    draw_chat_room_top(client);
    draw_chat_room_bottom(client);
}

void draw_menu_none(matrix_client_t* client) {
    struct ipa_graphics_state_t* graphics = client->graphics_state;
    if (!graphics->anim_start_time) {
        if (!graphics->queue.count || graphics->queue.data[0].type != IPA_QUEUE_LOGIN) {
            ssize_t id = queue_matrix_login_from_save(&client->login, false);
            vector$ipa_graphics_queue_t$_push_back(&graphics->queue, (ipa_graphics_queue_t){IPA_QUEUE_LOGIN, id});
        } else {
            if (get_finished_queue_size()) {
                optional$ipa_queue_entry_t$ entry = get_from_finished_queue();
                if (entry.is_valid && graphics->queue.data[0].id == entry.value.id) {
                    vector$ipa_graphics_queue_t$_remove(&graphics->queue, 0);
                    // client->menu = MENU_MAIN;
                    graphics->anim_start_time = graphics->time_on_menu;
                }
            }
        }
    } else if (graphics->time_on_menu - graphics->anim_start_time >= 60) {
        client->menu = MENU_MAIN;
    }

    draw_main_top_shared(graphics);
    C2D_SceneBegin(bottom);
    draw_main_bottom_shared(graphics);

    constexpr int box_width = 287;
    constexpr int box_height = 70;
    int y;
    if (!graphics->anim_start_time) {
        y = max(BOTTOM_HEIGHT - (graphics->time_on_menu) * SPEED * 2, (BOTTOM_HEIGHT - box_height) / 2);
    } else {
        y = min(((BOTTOM_HEIGHT - box_height) / 2) + (graphics->time_on_menu - graphics->anim_start_time) * SPEED * 2, BOTTOM_HEIGHT);
    }
    int x = (BOTTOM_WIDTH - box_width) / 2;
    int y_text = y + (box_height - 7) / 2;
    int x_text = x + (box_width - loading_string.width) / 2;
    draw_bevel_box(graphics, x, y, box_width, box_height, DARK_GREY, BLACK, ORANGE, BLACK, 0.05f);
    draw_ipa_string(loading_string, x_text, y_text, WHITE, 0.05f);
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

        while (graphics->vram_images.count) {
            ipa_image_t* image = vector$pair$ipa_image_t_p$size_t$$_get(&graphics->vram_images, 0)->first;
            image_unload_vram(image);
            vector$pair$ipa_image_t_p$size_t$$_remove(&graphics->vram_images, 0);
        }
        hash_map$img_path$ipa_image_t$_free_callback(&graphics->image_map, nullptr, destroy_image);
    }

    while (!C3D_FrameBegin(C3D_FRAME_NONBLOCK)) {
        //gotta love a busy wait
        //needed to actually get beyond 30fps for reasons I do not understand
    }
    C2D_TargetClear(top, C2D_Color32(0xFF, 0, 0xFF, 0xFF));
    C2D_TargetClear(bottom, C2D_Color32(0xFF, 0, 0xFF, 0xFF));

    switch (client->menu) {
        case MENU_NONE:
            draw_menu_none(client);
            break;
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
