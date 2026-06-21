#include "matrix_room_t.h"

void matrix_room_destroy(matrix_room_t* room) {
    free(room->id);
    vector$matrix_user_t$_free_callback(&room->associated_users, matrix_user_destroy);
    free(room->name);
    free(room->avatar_url);
    memset(room, 0, sizeof(*room));
}

matrix_user_t* matrix_room_get_user(matrix_room_t* room, char const* user_id) {
    if (!room || !user_id) {
        return nullptr;
    }

    for (size_t i = 0; i < room->associated_users.count; i++) {
        if (strcmp(room->associated_users.data[i].id, user_id) == 0) {
            return &room->associated_users.data[i];
        }
    }
    return nullptr;
}

void matrix_room_build(matrix_room_t* room, json_t* events) {
    size_t index_i;
    json_t* value_inner;
    json_array_foreach(events, index_i, value_inner) {
        char const* type = json_string_value(json_object_get(value_inner, "type"));
        json_t* content = json_object_get(value_inner, "content");
        if (strcmp(type, "m.room.name") == 0) {
            room->name = strdup(json_string_value(json_object_get(content, "name")));
        } else if (strcmp(type, "m.room.avatar") == 0) {
            room->avatar_url = strdup(json_string_value(json_object_get(content, "url")));
        } else if (strcmp(type, "m.room.member") == 0) {
            char const* user_id = json_string_value(json_object_get(value_inner, "state_key"));
            matrix_user_t* user = matrix_room_get_user(room, user_id);
            if (!user) {
                user = vector$matrix_user_t$_push_empty(&room->associated_users);
            }
            user->display_name = strdup(json_string_value(json_object_get(content, "displayname")));
            user->avatar_url = strdup(json_string_value(json_object_get(content, "avatar_url")));
            user->id = strdup(user_id);
        }
    }
}
