#include "matrix_room_t.h"

void matrix_room_destroy(matrix_room_t* room) {
    free(room->id);
    // vector$matrix_user_t$_free_callback(&room->associated_users, matrix_user_destroy);
    hash_map$str_const$matrix_user_t$_free_callback(&room->users, nullptr, matrix_user_destroy);
    free(room->name);
    free(room->avatar_url);
    free(room->topic);
    memset(room, 0, sizeof(*room));
}

int matrix_room_order(void const* lhs_void, void const* rhs_void) {
    matrix_room_t const* lhs = lhs_void;
    matrix_room_t const* rhs = rhs_void;
    //so this is actually backwards, however it is more useful to have larger values seen as lesser and as such earlier in the array
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

matrix_user_t* matrix_room_get_user(matrix_room_t* room, char const* user_id) {
    if (!room || !user_id) {
        return nullptr;
    }

    // for (size_t i = 0; i < room->associated_users.count; i++) {
    //     if (strcmp(room->associated_users.data[i].id, user_id) == 0) {
    //         return &room->associated_users.data[i];
    //     }
    // }
    return hash_map$str_const$matrix_user_t$_find(&room->users, user_id);
}

matrix_user_t* matrix_room_get_user_by_idx(matrix_room_t* room, size_t idx) {
    if (!room) {
        return nullptr;
    }
    return hash_map$str_const$matrix_user_t$_get_by_idx(&room->users, idx);
}

void matrix_room_build(matrix_room_t* room, json_t* events) {
    size_t index_i;
    json_t* value_inner;
    json_array_foreach(events, index_i, value_inner) { //are you kidding me with this format? "yeah object containing only an array." JUST MAKE THE OBJECT THE ARRAY
        char const* type = json_string_value(json_object_get(value_inner, "type"));
        json_t* content = json_object_get(value_inner, "content");
        if (strcmp(type, "m.room.name") == 0) {
            room->name = strdup(json_string_value(json_object_get(content, "name")));
        } else if (strcmp(type, "m.room.avatar") == 0) {
            room->avatar_url = strdup(json_string_value(json_object_get(content, "url")));
        } else if (strcmp(type, "m.room.topic") == 0) {
            room->topic = strdup(json_string_value(json_object_get(content, "topic")));
        } else if (strcmp(type, "m.room.member") == 0) {
            char const* user_id = json_string_value(json_object_get(value_inner, "state_key"));
            matrix_user_t* user = matrix_room_get_user(room, user_id);
            if (user) {
                user->display_name = strdup(json_string_value(json_object_get(content, "displayname")));
                user->avatar_url = strdup(json_string_value(json_object_get(content, "avatar_url")));
                user->id = strdup(user_id);
            } else {
                matrix_user_t new_user;
                new_user.display_name = strdup(json_string_value(json_object_get(content, "displayname")));
                new_user.avatar_url = strdup(json_string_value(json_object_get(content, "avatar_url")));
                new_user.id = strdup(user_id);
                hash_map$str_const$matrix_user_t$_add(&room->users, new_user.id, new_user);
            }
        }
    }
}
