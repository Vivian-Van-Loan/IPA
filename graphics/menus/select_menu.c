#include "select_menu.h"

#include "general.h"
#include "main_menu.h"

void draw_select_menu_bottom(matrix_client_t* client) {
    C2D_SceneBegin(bottom);
    struct ipa_graphics_state_t* graphics = client->graphics_state;

    if (find_in_graphics_queue_type(&graphics->queue, IPA_QUEUE_SYNC_DIRECTS)) {
        //todo: draw loading
    }

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
    //todo: please take a nice critical look at whatever this vrame image system is and see if it can be done better
    // because at the moment this isn't very good
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

    ipa_image_t* avatar = nullptr;
    for (ptrdiff_t i = start_idx; i < end_idx; i++) { //"make sure images are in the array" loop
        //loop that kills performance for some reason
        matrix_room_t* room = &client->rooms.data[i - start_idx];
        if (!room->avatar_url) {
            continue;
        }
        if (graphics->vram_images.count < i - start_idx + 1) { // || graphics->vram_images.data[i - start_idx].second != i - start_idx) {
            avatar = lookup_avatar(&client->graphics_state->image_map, room->avatar_url, BIG_AVATAR_SIZE, BIG_AVATAR_SIZE);
            if (avatar && avatar->data && !avatar->in_vram) {
                vector$pair$ipa_image_t_p$size_t$$_push_back(&graphics->vram_images, (pair$ipa_image_t_p$size_t$){avatar, i - start_idx});
                image_load_vram(avatar);
            } else if (!avatar) {
                queue_avatar_get(client, room->avatar_url, BIG_AVATAR_SIZE, BIG_AVATAR_SIZE);
                avatar = lookup_avatar(&client->graphics_state->image_map, room->avatar_url, BIG_AVATAR_SIZE, BIG_AVATAR_SIZE); //lazy but will exist
            }
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
        if (graphics->vram_images.count > i_0 && graphics->vram_images.data[i_0].first) {
            sprite = graphics->vram_images.data[i_0].first->sprite;
        }
        int button_x = 64;
        int button_y = i_0 * 32 + max(BOTTOM_HEIGHT - (graphics->time_on_menu - SPEED * i_0) * SPEED, button_y_start);
        draw_main_menu_button(sprite, button_x, button_y, room->name);
        if (avatar && !avatar->data) {
            int load_x = button_x + (BIG_AVATAR_SIZE - LOAD_SPRITE_WIDTH) / 2;
            int load_y = button_y + (BIG_AVATAR_SIZE - LOAD_SPRITE_WIDTH) / 2;
            draw_load_sprite(graphics, load_x, load_y, YELLOW, 0.05f);
        } else if (!sprite) {
            draw_battery_sprite(graphics, button_x + 8, button_y + 12, 0.05f); //todo: replace this with an ACTUAL sprite or something
        }
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

void setup_select_dm_menu(matrix_client_t* client) {
    ssize_t id = queue_matrix_client_sync_directs(client);
    add_to_graphics_queue(&client->graphics_state->queue, IPA_QUEUE_SYNC_DIRECTS, id);
    client->menu = MENU_DMS;
}

void setup_select_spaces_menu(matrix_client_t* client) {
    ssize_t id = queue_matrix_client_sync_spaces(client);
    add_to_graphics_queue(&client->graphics_state->queue, IPA_QUEUE_SYNC_SPACES, id);
    client->menu = MENU_SPACES;
}

void setup_select_rooms_menu(matrix_client_t* client) {
    ssize_t id = queue_matrix_client_sync_rooms(client);
    add_to_graphics_queue(&client->graphics_state->queue, IPA_QUEUE_SYNC_ROOMS, id);
    client->menu = MENU_ROOMS;
}
