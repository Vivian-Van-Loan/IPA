#include "matrix_room_t.h"

#include "../utils.h"
#include "event/matrix-event-room.h"

void matrix_room_destroy(matrix_room_t* room) {
    free(room->id);
    hash_map$str_const$matrix_event_t$_free_callback(&room->users, nullptr, matrix_event_destroy);
    dequeue$matrix_event_t$_free_callback(&room->events, matrix_event_destroy);
    free(room->name);
    free(room->avatar_url);
    free(room->topic);
    memset(room, 0, sizeof(*room));
}

int matrix_room_order(void const* lhs_void, void const* rhs_void) {
    matrix_room_t const* lhs = lhs_void;
    matrix_room_t const* rhs = rhs_void;
    //so this is actually backwards, however it is more useful to have larger values seen as lesser and as such earlier in the array
    //nice comment previous me, could have just said "order newer timestamps as lesser" or something
    if (lhs->last_message_time > rhs->last_message_time) {
        return -1;
    } else if (lhs->last_message_time < rhs->last_message_time) {
        return 1;
    }
    return 0;
}

size_t matrix_room_get_user_count(matrix_room_t* room) {
    if (!room) {
        return 0;
    }
    return room->users.num_items;
}

matrix_event_t* matrix_room_get_user(matrix_room_t* room, char const* user_id) {
    if (!room || !user_id) {
        return nullptr;
    }

    return hash_map$str_const$matrix_event_t$_find(&room->users, user_id);
}

matrix_event_t* matrix_room_get_user_by_idx(matrix_room_t* room, size_t idx) {
    if (!room) {
        return nullptr;
    }
    return hash_map$str_const$matrix_event_t$_get_by_idx(&room->users, idx);
}

bool matrix_room_add_user(matrix_room_t* room, matrix_event_t* user_event) { //returns true if the user event was added
    if (!user_event || user_event->type.first != EVENT_ROOM_MEMBER) {
        return false;
    }
    matrix_event_t* exist = matrix_room_get_user(room, user_event->sender);
    if (exist && exist->origin_server_ts < user_event->origin_server_ts) { //again probably unneeded on this sweep which is building members, but will be useful for future sweeps
        return false;
    } else if (exist) {
        hash_map$str_const$matrix_event_t$_remove(&room->users, user_event->sender);
    }
    hash_map$str_const$matrix_event_t$_add(&room->users, user_event->sender, *user_event);
    return true;
}

void matrix_room_build(matrix_room_t* room, json_t* events) {
    size_t index_i;
    json_t* value_inner;
    json_array_foreach(events, index_i, value_inner) { //are you kidding me with this format? "yeah object containing only an array." JUST MAKE THE OBJECT THE ARRAY
        matrix_event_t event = matrix_make_event(value_inner);
        bool destroy_event = true;
        switch (event.type.first) {
            case EVENT_ROOM_NAME:
                room->name = strdup(event.room.name.name);
                room->name_ts = event.origin_server_ts;
                break;
            case EVENT_ROOM_AVATAR:
                room->avatar_url = strdup(event.room.avatar.url);
                room->avatar_ts = event.origin_server_ts;
                break;
            case EVENT_ROOM_TOPIC:
                room->topic = strdup(event.room.topic.topic);
                room->topic_ts = event.origin_server_ts;
                break;
            default:
                break;
            case EVENT_ROOM_MEMBER:
                if (matrix_room_add_user(room, &event)) {
                    destroy_event = false; //data is now kept in the user map
                }
        }
        if (destroy_event) {
            matrix_event_destroy(&event);
        }
    }
}

void matrix_room_add_events(matrix_room_t* room, json_t* events, bool reverse) {
    if (!json_is_array(events)) {
        efuncprintf("provided with non-array event json!");
        return;
    }
    size_t size = json_array_size(events);

    size_t index_i;
    json_t* value_inner;
    json_array_foreach(events, index_i, value_inner) {
        bool destroy_event = false;
        matrix_event_t event = matrix_make_event(value_inner);
        if (event.type.first == EVENT_ROOM_MEMBER) {
            if (!matrix_room_add_user(room, &event)) {
                destroy_event = true;
            }
        } else if (event.type.first == EVENT_ROOM_TOPIC) {
            room->topic = strdup(event.room.topic.topic);
            room->topic_ts = event.origin_server_ts;
            destroy_event = true;
        } else if (event.type.first == EVENT_ROOM_NAME) {
            room->name = strdup(event.room.name.name);
            room->name_ts = event.origin_server_ts;
            destroy_event = true;
        } else if (event.type.first == EVENT_ROOM_AVATAR) {
            room->avatar_url = strdup(event.room.avatar.url);
            room->avatar_ts = event.origin_server_ts;
            destroy_event = true;
        } else if (event.type.first == EVENT_ROOM_REDACTION) {
            matrix_room_redaction_t* redaction = &event.room.redaction;
            for (size_t i = 0; i < room->events.count; i++) {
                matrix_event_t* e = &room->events.data[i];
                if (strcmp(e->id, redaction->redacts) == 0) {
                    matrix_event_destroy(e);
                    dequeue$matrix_event_t$_remove(&room->events, i);
                    break;
                }
            }
            destroy_event = true;
        }

        if (destroy_event) {
            matrix_event_destroy(&event);
        } else {
            if (reverse) {
                dequeue$matrix_event_t$_push(&room->events, event);
            } else {
                dequeue$matrix_event_t$_push_back(&room->events, event);
            }
            if (room->events.count > 500) { //shrink if we have too many events
                if (reverse) {
                    dequeue$matrix_event_t$_resize_keep_front(&room->events, 500);
                } else {
                    dequeue$matrix_event_t$_resize_keep_back(&room->events, 500);
                }
            }
        }
    }
}
