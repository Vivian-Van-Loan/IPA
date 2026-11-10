#include "graphics.h"

#include "font.h"
#include "images.h"
#include "../utils.h"
#include "../matrix/matrix-client.h"
#include "../matrix/matrix-utils.h"

#include "menus/general.h"
#include "menus/main_menu.h"
#include "menus/select_menu.h"
#include "menus/chat_menu.h"

C3D_RenderTarget* top;
C3D_RenderTarget* bottom;
C2D_SpriteSheet main_sheet;
C2D_Sprite main_sprites[NUM_SPRITES];
C2D_SpriteSheet gradient_sheet;
C2D_Sprite gradient_sprites[2];

// C2D_ImageTint overlay_tint;
u32 overlay_colour;
u32 overlay_colour_dark;
u32 overlay_colour_light;

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

void update_events(struct ipa_graphics_state_t* graphics, matrix_room_t* room) {
    dequeue$matrix_event_t$* events = &room->events;
    dequeue$ipa_message_t$* messages = &graphics->messages;
    if (events->count == 0) {
        return;
    }
    while (messages->count && strcmp(events->data[0].room_id, messages->data[0].event_id) != 0) {
        ipa_message_destroy(&messages->data[0]);
        dequeue$ipa_message_t$_remove(messages, 0);
    }
    for (size_t i = 0; i < events->count; i++) {
        matrix_event_t* event = &events->data[i];
        if (event->relates_to.rel_type == MATRIX_REL_TYPE_REPLACE) {
            //todo: have fun with that
            // also need to validate against sender
        } else if (event->type.first == EVENT_ROOM_REDACTION) {
            for (size_t j = 0; j < messages->count; j++) {
                ipa_message_t* message = &messages->data[j];
                if (strcmp(message->event_id, event->room.redaction.redacts) == 0) {
                    ipa_message_destroy(message);
                    dequeue$ipa_message_t$_remove(messages, j);
                    break;
                }
            }
        } else if (event_can_be_message(event)) {
            matrix_event_t* member_event = matrix_room_get_user(room, event->sender);
            matrix_room_member_t* member = nullptr;
            if (member_event && member_event->type.first == EVENT_ROOM_MEMBER) {
                member = &member_event->room.member;
            }
            pair$int$int$ widths = get_nameplate_width(member->display_name, member->avatar_url);
            ipa_message_t message = make_ipa_message(event, 320 - 3 - widths.second, 320 - 3);
            dequeue$ipa_message_t$_push_back(messages, message);
        }
    }
}

void draw_menu_none(matrix_client_t* client) {
    struct ipa_graphics_state_t* graphics = client->graphics_state;

    draw_main_top_shared(graphics);
    C2D_SceneBegin(bottom);
    draw_main_bottom_shared(graphics);

    if (graphics->anim_id == IPA_ANIM_NONE) {
        graphics->anim_id = IPA_ANIM_LOAD_IN;
        graphics->anim_start_time = graphics->time_on_menu;
        graphics->next_anim_id = IPA_ANIM_MENU_1;
    }
    if (graphics->anim_id == IPA_ANIM_LOAD_IN || graphics->anim_id == IPA_ANIM_LOAD_OUT) {
        draw_load_box(graphics);
        if (graphics->anim_id == IPA_ANIM_LOAD_IN) {
            ipa_graphics_queue_t* graphic_entry = find_in_graphics_queue_type(&graphics->queue, IPA_QUEUE_LOGIN);
            if (!graphic_entry) {
                ssize_t id = queue_matrix_login_from_save(&client->login, false);
                add_to_graphics_queue(&graphics->queue, IPA_QUEUE_LOGIN, id);
            } else {
                if (graphic_entry->finished) {
                    destroy_queue_entry(&graphic_entry->finished_entry);
                    remove_from_graphics_queue(&graphics->queue, graphic_entry->id);
                    graphics->anim_start_time = graphics->time_on_menu;
                    graphics->anim_id = IPA_ANIM_LOAD_OUT;
                }
            }
        }
    } else if (graphics->anim_id == IPA_ANIM_MENU_1) {
        client->menu = MENU_MAIN;
    }
}

void reset_graphics_state(struct ipa_graphics_state_t* graphics) {
    graphics->time_on_menu = 0;
    graphics->selection = 0;
    graphics->selection_num = 0;
    graphics->selected = false;
    graphics->scroll_pos = 0;
    graphics->scroll_to_pos = 0;
    graphics->anim_start_time = 0;
    graphics->anim_id = 0;
    graphics->next_anim_id = 0;
    graphics->load_y = BOTTOM_HEIGHT;
    for (size_t i = 0; i < lengthof(graphics->track_y); i++) {
        graphics->track_y[i] = 0;
    }

    while (graphics->vram_images.count) {
        ipa_image_t* image = vector$pair$ipa_image_t_p$size_t$$_get(&graphics->vram_images, 0)->first;
        image_unload_vram(image);
        vector$pair$ipa_image_t_p$size_t$$_remove(&graphics->vram_images, 0);
    }
    hash_map$img_path$ipa_image_t$_free_callback(&graphics->image_map, nullptr, destroy_image);
}

void handle_queue(matrix_client_t* client) {
    struct ipa_graphics_state_t* graphics = client->graphics_state;
    optional$ipa_queue_entry_t$ entry = get_from_finished_queue(); //only do one per frame to not lag
    bool destroy = true;
    if (entry.is_valid) {
        ipa_graphics_queue_t* graphic_entry = find_in_graphics_queue_id(&graphics->queue, entry.value.id);
        if (graphic_entry) {
            if (graphic_entry->type == IPA_QUEUE_SYNC_DIRECTS) {
                client->rooms = *(vector$matrix_room_t$*)entry.value.result;
                remove_from_graphics_queue(&graphics->queue, graphic_entry->id);
                destroy_queue_entry(&entry.value);
            } else if (graphic_entry->type == IPA_QUEUE_LOAD_ROOM) {
                matrix_room_info_t* info = entry.value.result;
                matrix_room_t* room = matrix_client_get_room(client, info->id);
                matrix_room_add_events(room, &info->events, info->reverse);
                update_events(graphics, room);
                dequeue$matrix_event_t$_free(&info->events);
                if (client->current_room != room) {
                    client->current_room = room;
                }
            } else if (graphic_entry->type == IPA_QUEUE_AVATAR || graphic_entry->type == IPA_QUEUE_IMAGE) {
                ipa_image_t* image = entry.value.result;
                ipa_image_t* mapped_image = nullptr;
                for (size_t i = 0; i < graphics->image_map.num_items; i++) {
                    ipa_image_t* temp = hash_map$img_path$ipa_image_t$_get_by_idx(&graphics->image_map, i);
                    if (temp->id == entry.value.id) {
                        mapped_image = temp;
                        break;
                    }
                }
                if (mapped_image && image) {
                    *mapped_image = *image;
                }
                //todo: gotta figure out a way to put them into the queue without an entry existing
            } else {
                graphic_entry->finished = true;
                graphic_entry->finished_entry = entry.value;
                destroy = false;
            }
        }
        if (destroy) {
            if (graphic_entry) {
                remove_from_graphics_queue(&graphics->queue, graphic_entry->id);
            }
            destroy_queue_entry(&entry.value);
        }
    }
}

void draw_and_input(matrix_client_t* client) {
    if (!client->graphics_state) {
        client->graphics_state = calloc(1, sizeof(*client->graphics_state));
        if (!client->graphics_state) {
            efuncprintf("failed to allocate graphics state, whatever, exiting");
            exit(67); //there, now the linter can stop yelling that it can be null
        }
    }

    struct ipa_graphics_state_t* graphics = client->graphics_state;
    if (client->menu != graphics->menu) {
        graphics->menu = client->menu;
        reset_graphics_state(graphics);
    }

    handle_queue(client);

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
            draw_main_menu(client);
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

    if (!graphics->selected) {
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
        } else if (key_presses & KEY_A) {
            graphics->selected = true;
        }
    }

    C3D_FrameEnd(0);
    graphics->time_on_menu++;
}

void add_to_graphics_queue(vector$ipa_graphics_queue_t$* queue, enum ipa_graphics_queue_type_t type, ssize_t id) {
    vector$ipa_graphics_queue_t$_push_back(queue, (ipa_graphics_queue_t){type, id});
}

ipa_graphics_queue_t* find_in_graphics_queue_id(vector$ipa_graphics_queue_t$* queue, ssize_t id) {
    for (size_t i = 0; i < queue->count; i++) {
        if (queue->data[i].id == id) {
            return &queue->data[i];
        }
    }
    return nullptr;
}

ipa_graphics_queue_t* find_in_graphics_queue_type(vector$ipa_graphics_queue_t$* queue, enum ipa_graphics_queue_type_t type) {
    for (size_t i = 0; i < queue->count; i++) {
        if (queue->data[i].type == type) {
            return &queue->data[i];
        }
    }
    return nullptr;
}

void remove_from_graphics_queue(vector$ipa_graphics_queue_t$* queue, ssize_t id) {
    size_t index = 0;
    for (index = 0; index < queue->count; index++) {
        if (queue->data[index].id == id) {
            break;
        }
    }
    if (index == queue->count) {
        return;
    }
    vector$ipa_graphics_queue_t$_remove(queue, index);
}
