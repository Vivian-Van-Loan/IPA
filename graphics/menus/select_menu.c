#include "select_menu.h"

#include "general.h"
#include "main_menu.h"

void draw_select_menu_bottom(matrix_client_t* client) {
    C2D_SceneBegin(bottom);
    struct ipa_graphics_state_t* graphics = client->graphics_state;

    draw_main_bottom_shared(graphics);

    if (graphics->anim_id == IPA_ANIM_NONE) {
        graphics->anim_id = IPA_ANIM_LOAD_IN;
        graphics->next_anim_id = IPA_ANIM_MENU_1;
        graphics->anim_start_time = graphics->time_on_menu;
    }
    if (graphics->anim_id == IPA_ANIM_LOAD_IN || graphics->anim_id == IPA_ANIM_LOAD_OUT) {
        draw_load_box(graphics);
        if (graphics->anim_id == IPA_ANIM_LOAD_IN) {
            if (!find_in_graphics_queue_type(&graphics->queue, IPA_QUEUE_SYNC_DIRECTS)) {
                graphics->anim_id = IPA_ANIM_LOAD_OUT;
                graphics->anim_start_time = graphics->time_on_menu;
            }
        }
        return;
    }

    ptrdiff_t const num_rooms = client->rooms.count;
    if (graphics->selection_num == 0) {
        graphics->selection_num = num_rooms;
    }
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
        matrix_room_t* room = &client->rooms.data[i - start_idx];
        if (!room->avatar_url) {
            continue;
        }
        if (graphics->vram_images.count < i - start_idx + 1) {
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

    ptrdiff_t anim_time = graphics->time_on_menu - graphics->anim_start_time;

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
        if (graphics->anim_id != IPA_ANIM_MENU_CHANGE) {
            graphics->track_y[i_0] = i_0 * 32 + max(BOTTOM_HEIGHT - (anim_time - SPEED * i_0) * SPEED, button_y_start);
        } else {
            if (anim_time - SPEED * (3 - i_0) >= 0) {
                graphics->track_y[i_0] += SPEED;
            }
        }
        draw_main_menu_button(sprite, button_x, graphics->track_y[i_0], room->name);
        if (avatar && !sprite) {
            int load_x = button_x + (BIG_AVATAR_SIZE - LOAD_SPRITE_WIDTH) / 2;
            int load_y = graphics->track_y[i_0] + (BIG_AVATAR_SIZE - LOAD_SPRITE_WIDTH) / 2;
            draw_load_sprite(graphics, load_x, load_y, YELLOW, 0.05f);
        } else if (!sprite) {
            draw_battery_sprite(graphics, button_x + 8, graphics->track_y[i_0] + 12, 0.05f); //todo: replace this with an ACTUAL sprite or something
        }
    }
    if (graphics->selection >= start_idx && graphics->selection < end_idx) {
        ptrdiff_t i_0 = graphics->selection - start_idx;
        draw_main_menu_select(59, graphics->selection * 32 + graphics->track_y[i_0] - 5);
    }
}

void draw_select_menu(matrix_client_t* client) {
    struct ipa_graphics_state_t* graphics = client->graphics_state;
    draw_main_top_shared(graphics);
    draw_select_menu_bottom(client);

    if (graphics->selected && graphics->anim_id != IPA_ANIM_MENU_CHANGE) {
        graphics->anim_id = IPA_ANIM_MENU_CHANGE;
        graphics->anim_start_time = graphics->time_on_menu;
    } else if (graphics->selected && graphics->track_y[0] >= BOTTOM_HEIGHT) {
        if (client->menu == MENU_DMS) {
            ssize_t id = queue_matrix_client_load_room(client, client->rooms.data[graphics->selection].id, true);
            add_to_graphics_queue(&client->graphics_state->queue, IPA_QUEUE_LOAD_ROOM, id);
            client->menu = MENU_CHAT;
        } else if (client->menu == MENU_SPACES) {
            //todo
        } else if (client->menu == MENU_ROOMS) {
            //also todo
        }
    }
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
