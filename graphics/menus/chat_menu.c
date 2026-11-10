#include "chat_menu.h"

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

pair$int$int$ get_nameplate_width(char const* name, char const* avatar_url) {
    int avatar_width = 16;
    if (!avatar_url) {
        avatar_width = 0;
    }

    int string_width = min(get_string_width(name), MAX_NAME_WIDTH);

    int inner_plate_width = 2 + avatar_width + 1 + string_width + 2; //actually the inner portion, not the outer
    if (avatar_width == 0 && string_width == 0) {
        inner_plate_width = 0;
    }

    int outer_plate_width = inner_plate_width + 3;
    return (pair$int$int$){inner_plate_width, outer_plate_width};
}

int draw_message_box_text(matrix_client_t* client, char const* name, char const* text, char const* avatar_url, int x, int y, int w, u32 dark, u32 light, u32 back, u32 text_c, int max_lines) {
    pair$int$int$ nameplate_width = get_nameplate_width(name, avatar_url);

    int avatar_width = 16;
    int avatar_left = x + 4;
    int avatar_top = y + 3;
    ipa_image_t* avatar = nullptr;
    if (avatar_url) {
        avatar = lookup_avatar(&client->graphics_state->image_map, avatar_url, avatar_width, avatar_width);
        if (!avatar) {
            queue_avatar_get(client, avatar_url, avatar_width, avatar_width);
        } else if (!avatar->in_vram) {
            image_load_vram(avatar);
        }
    } else {
        avatar_left = x;
        avatar_width = 0;
    }

    ipa_string_t name_ipa = ipa_string_conv_crop(name, MAX_NAME_WIDTH, L'…'); //honestly the allocation and crop shouldn't matter

    int const v_text_offset = 7;
    int const w_text_pad = 5 + 2; //5 pixels from box edge, 2 pixels for box edge
    int text_y = y + v_text_offset;
    int text_x = x + nameplate_width.second + 2;
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

    if (nameplate_width.first) {
        C2D_DrawRectSolidFix(x + 3, y + 2, 0, nameplate_width.first - 2, 17, light);
        if (num_lines > 1) {
            C2D_DrawLineFix(x + 2, y + 4, light, x + 2, y + 20, light, 1, 0);
            C2D_DrawLineFix(x + 2, y + 19, light, x + nameplate_width.first - 1, y + 19, light, 1, 0);
            C2D_DrawLineFix(x + 1, y + 20, dark, x + nameplate_width.first - 1, y + 20, dark, 1, 0);
        } else {
            C2D_DrawLineFix(x + 2, y + 4, light, x + 2, y + 18, light, 1, 0);
            C2D_DrawLineFix(x + 4, y + 19, light, x + nameplate_width.first - 1, y + 19, light, 1, 0);
            C2D_DrawLineFix(x + 4, y + 20, dark, x + nameplate_width.first - 1, y + 20, dark, 1, 0);
        }
        C2D_DrawLineFix(x + 1, y + 5, dark, x + 5, y + 1, dark, 1, 0); //recover the top left line
        C2D_DrawLineFix(x + nameplate_width.first - 1, y + 20, dark, x + nameplate_width.first + 2, y + 17, dark, 1, 0);
        C2D_DrawLineFix(x + nameplate_width.first + 1, y + 18, dark, x + nameplate_width.first + 1, y + 2, dark, 1, 0);
    }

    if (avatar && avatar->in_vram) {
        C2D_SpriteSetPos(avatar->sprite, avatar_left, avatar_top);
        C2D_SpriteSetDepth(avatar->sprite, 0);
        C2D_DrawSprite(avatar->sprite);
    } else if (avatar_width != 0) {
        draw_load_sprite(client->graphics_state, avatar_left, avatar_top, dark, 0);
    }

    draw_ipa_string(name_ipa, avatar_left + avatar_width + 1, y + v_text_offset, dark, 0);
    ipa_string_destroy(&name_ipa);

    return h;
}

int draw_message(matrix_client_t* client, ipa_message_t const* message, int x, int y) {
    int w = 320 - 3;

    u32 dark;
    u32 light;
    u32 back;
    u32 text_c;
    char const* name = nullptr;
    char const* avatar_url = nullptr;
    if (message->type == IPA_MSG_MSG) {
        dark = overlay_colour_dark;
        light = overlay_colour_light;
        back = WHITE;
        text_c = BLACK;
        matrix_event_t* user = matrix_room_get_user(client->current_room, message->user);
        if (user && user->type.first == EVENT_ROOM_MEMBER) {
            name = user->room.member.display_name;
            avatar_url = user->room.member.avatar_url;
        }
    } else if (message->type == IPA_MSG_ANNOUNCEMENT) {
        dark = WHITE;
        light = WHITE;
        back = BLACK;
        text_c = SILVER;
    } else {
        dark = WHITE;
        light = WHITE;
        back = BLACK;
        text_c = SILVER;
    }

    pair$int$int$ nameplate_width = get_nameplate_width(name, avatar_url);

    int avatar_width = 16;
    int avatar_left = x + 4;
    int avatar_top = y + 3;
    ipa_image_t* avatar = nullptr;
    if (avatar_url) {
        avatar = lookup_avatar(&client->graphics_state->image_map, avatar_url, avatar_width, avatar_width);
        if (!avatar) {
            queue_avatar_get(client, avatar_url, avatar_width, avatar_width);
        } else if (!avatar->in_vram) {
            image_load_vram(avatar);
        }
    } else {
        avatar_left = x;
        avatar_width = 0;
    }

    int h = draw_message_box(client->graphics_state, x, y, w, message->lines.count, true, dark, light, back);

    if (nameplate_width.first) {
        C2D_DrawRectSolidFix(x + 3, y + 2, 0, nameplate_width.first - 2, 17, light);
        if (message->lines.count > 1) {
            C2D_DrawLineFix(x + 2, y + 4, light, x + 2, y + 20, light, 1, 0);
            C2D_DrawLineFix(x + 2, y + 19, light, x + nameplate_width.first - 1, y + 19, light, 1, 0);
            C2D_DrawLineFix(x + 1, y + 20, dark, x + nameplate_width.first - 1, y + 20, dark, 1, 0);
        } else {
            C2D_DrawLineFix(x + 2, y + 4, light, x + 2, y + 18, light, 1, 0);
            C2D_DrawLineFix(x + 4, y + 19, light, x + nameplate_width.first - 1, y + 19, light, 1, 0);
            C2D_DrawLineFix(x + 4, y + 20, dark, x + nameplate_width.first - 1, y + 20, dark, 1, 0);
        }
        C2D_DrawLineFix(x + 1, y + 5, dark, x + 5, y + 1, dark, 1, 0); //recover the top left line
        C2D_DrawLineFix(x + nameplate_width.first - 1, y + 20, dark, x + nameplate_width.first + 2, y + 17, dark, 1, 0);
        C2D_DrawLineFix(x + nameplate_width.first + 1, y + 18, dark, x + nameplate_width.first + 1, y + 2, dark, 1, 0);
    }

    ipa_string_t name_ipa = ipa_string_conv_crop(name, MAX_NAME_WIDTH, L'…'); //honestly the allocation and crop shouldn't matter

    int const v_text_offset = 7;
    int const w_text_pad = 5 + 2; //5 pixels from box edge, 2 pixels for box edge
    int text_y = y + v_text_offset;
    int text_x = x + nameplate_width.second + 2;
    int text_width = x + (w - w_text_pad - text_x);
    for (size_t i = 0; i < message->lines.count; i++) {
        C2D_DrawRectSolidFix(text_x, text_y, 0, text_width, 13, back); //cover up an alpha-depth compositing problem
        draw_ipa_string(message->lines.data[i], text_x, text_y, text_c, 0.05f);
        text_y += 18;
        text_x = x + w_text_pad;
        text_width = w - w_text_pad * 2; //since w is entire box width we have to take the padding from both sides off
    }

    if (avatar && avatar->in_vram) {
        C2D_SpriteSetPos(avatar->sprite, avatar_left, avatar_top);
        C2D_SpriteSetDepth(avatar->sprite, 0);
        C2D_DrawSprite(avatar->sprite);
    } else if (avatar_width != 0) {
        draw_load_sprite(client->graphics_state, avatar_left, avatar_top, dark, 0);
    }

    draw_ipa_string(name_ipa, avatar_left + avatar_width + 1, y + v_text_offset, dark, 0);
    ipa_string_destroy(&name_ipa);

    return h;
}

void draw_chat_room_top(struct matrix_client_t* client) {
    C2D_SceneBegin(top);
    draw_background(client->graphics_state, 40, 0, TOP_WIDTH, TOP_HEIGHT);
    draw_top_screen_left(client->graphics_state, 0.25f);
    draw_bevel_box(client->graphics_state, 360, 0, 40, 240, DARK_GREY, BLACK, ORANGE, BLACK, 0.25f);

    if (client->current_room) {
        dequeue$ipa_message_t$* messages = &client->graphics_state->messages;
        int y = 1;
        for (ptrdiff_t i = messages->count - 1; i >= 0; i--) {
            //todo: check if the message is actually on screen or not, shouldn't be too hard
            int offset = draw_message(client, &messages->data[i], 40 + 2, y);
            if (offset) {
                y += offset + 2;
            }
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
    char const* name = "NULL POINTER";
    char const* avatar = nullptr;
    if (user) {
        name = user->room.member.display_name;
        avatar = user->room.member.avatar_url;
    }
    draw_message_box_text(client, name,
        "Hello everybody my name is Markiplier and welcome to Five Nights at Freddies, an indie horror game that you guys suggested in mass, and I saw that Yamimash played it and he said that it was really really good; so I'm very eager to see what is up - and that is a terrifying animatronic bear reads off script family pizzeria looking for security guard to work the night shift. Oh, 12:00 A.M, the first night. If I didn’t want to stay the first night, why would I stay any more than five? Why would I say anymore than two - hello. Okay...Hello? Hello - oh, ah I can’t move. That’s a creepy skull...There’s creepy things on the wall - Oh, hello.",
        avatar, 39, 52, 280, overlay_colour_dark, overlay_colour_light, WHITE, BLACK, 5);

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
