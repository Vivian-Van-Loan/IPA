#include "select_menu.h"

#include "main_menu.h"

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

    draw_main_bottom_shared(graphics);

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
            ipa_image_t* image = vector$pair$ipa_image_t_p$size_t$$_push_back(&graphics->vram_images, (pair$ipa_image_t_p$size_t$){get_avatar(client, room->avatar_url, BIG_AVATAR_SIZE, BIG_AVATAR_SIZE), i - start_idx})->first;
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
    draw_main_top_shared(client->graphics_state);
    draw_select_menu_bottom(client);
}
