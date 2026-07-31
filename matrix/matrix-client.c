#include "matrix-client.h"

#include <jansson.h>
#include <stdio.h>
#include <string.h>

#include "matrix-utils.h"
#include "../utils.h"

char* account_direct_filter_id = nullptr;

json_t* make_empty_filter() {
    json_t* filter_json = json_object();

    json_object_set_new(filter_json, "room", json_object());
    json_t* room_filter = json_object_get(filter_json, "room");
    json_object_set_new(room_filter, "rooms", json_array());

    json_object_set_new(filter_json, "presence", json_object());
    json_t* presence_filter = json_object_get(filter_json, "presence");
    json_object_set_new(presence_filter, "types", json_array());

    json_object_set_new(filter_json, "account_data", json_object());
    json_t* account_filter = json_object_get(filter_json, "account_data");
    json_object_set_new(account_filter, "types", json_array());

    return filter_json;
}

char* install_filter(matrix_client_t* client, json_t const* filter_json) {
    char* ret = nullptr;
    char* json_str = json_dumps(filter_json, JSON_COMPACT);
    printf("%s\n", json_str);

    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/user/%s/filter", client->login.homeserver_resolved, client->login.user_id);
    char* response_str = matrix_post_json_string(&client->login, buf, json_str);
    free(json_str);
    json_t* response = nullptr;
    if (!response_str) {
        efuncprintf("Failed to set filter\n");
        goto exit;
    }
    response = json_loads(response_str, 0, nullptr);
    free(response_str);
    if (!response) {
        efuncprintf("Failed to load filter response json\n");
        goto exit;
    }
    json_t* filter_id_json = json_object_get(response, "filter_id");
    if (!filter_id_json || !json_is_string(filter_id_json)) {
        efuncprintf("Failed to retrieve filter_id\n");
        goto exit;
    }
    ret = strdup(json_string_value(filter_id_json));

    exit:
    json_decref(response);
    return ret;
}

int make_account_direct_filter(matrix_client_t* client) { //rendered useless, keeping for testing or building future filters
    json_t* filter_json = make_empty_filter();
    json_t* account_filter = json_object_get(filter_json, "account_data");
    json_object_set_new(account_filter, "types", json_array());
    json_t* types = json_object_get(account_filter, "types");
    json_array_append_new(types, json_string("m.direct"));
    // json_array_append_new(types, json_string("m.push_rules"));
    account_direct_filter_id = install_filter(client, filter_json);
    json_decref(filter_json);
    if (!account_direct_filter_id) {
        efuncprintf("Failed to install direct filter\n");
        return -1;
    }
    return 0;
}

int matrix_client_sync_account_data(matrix_client_t* client) {
    // int res;
    // if (!filter_id) {
    //     res = make_filter(client);
    //     if (res) {
    //         efuncprintf("Failed to build filter\n");
    //         return res;
    //     }
    // }

    char buf[URL_BUFFER_SIZE];
    char* filter_url = curl_easy_escape(curl_handle, "{\"room\":{\"rooms\":[]},\"presence\":{\"types\":[]},\"account_data\":{\"not_types\":[\"m.direct\",\"m.push_rules\"]}}", 0); //https://spec.matrix.org/v1.18/client-server-api/#filtering
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync?filter=%s", client->login.homeserver_resolved, filter_url);
    free(filter_url);

    char* account_data_str = matrix_get_string(&client->login, buf);
    if (!account_data_str) {
        efuncprintf("Failed to download sync json\n");
        return -1;
    }
    client->core_account_data = json_loads(account_data_str, 0, nullptr);
    free(account_data_str);
    if (!client->core_account_data) {
        efuncprintf("Failed to parse account data\n");
        return -1;
    }

    return 0;
}

void wipe_remake_client_temps(matrix_client_t* client) {
    // vector$matrix_user_t$_free_callback(&client->users, matrix_user_destroy);
    vector$matrix_room_t$_free_callback(&client->rooms, matrix_room_destroy);
    client->rooms = vector$matrix_room_t$_new();
}

int matrix_client_sync_directs(matrix_client_t* client) {
    wipe_remake_client_temps(client);

    char buf[URL_BUFFER_SIZE];
    if (!account_direct_filter_id) {
        make_account_direct_filter(client);
    }
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync?filter=%s", client->login.homeserver_resolved, account_direct_filter_id);
    // snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync", client->login.homeserver_resolved);

    char* response_str = matrix_get_string(&client->login, buf);
    if (!response_str) {
        efuncprintf("Failed to download sync json\n");
        return -1;
    }
    json_auto_t* response_json = json_loads(response_str, 0, nullptr);
    free(response_str);
    if (!response_json) {
        efuncprintf("Failed to parse sync json\n");
        return -1;
    }
    json_auto_t* filter_json = make_empty_filter(); //todo: rather than making a new one each time we should just cache the first one, and then occasionally call sync with the since param to see if there's new ones
    json_t* room_filter = json_object_get(filter_json, "room");

    json_object_set_new(room_filter, "state", json_object());
    json_t* state_filter = json_object_get(room_filter, "state");
    json_object_set_new(state_filter, "lazy_load_members", json_boolean(true));
    json_object_set_new(state_filter, "types", json_array());
    json_t* types = json_object_get(state_filter, "types");
    json_array_append_new(types, json_string("m.room.member"));
    json_array_append_new(types, json_string("m.room.name"));
    json_array_append_new(types, json_string("m.room.avatar"));
    // json_object_set_new(state_filter, "not_senders", json_array());
    // json_t* not_senders = json_object_get(state_filter, "not_senders");
    // json_array_append_new(not_senders, json_string(client->login.user_id));

    // json_object_set(room_filter, "timeline", state_filter);
    json_object_set_new(room_filter, "timeline", json_object());
    json_t* timeline_filter = json_object_get(room_filter, "timeline");
    json_object_set_new(timeline_filter, "types", json_array());
    json_object_set_new(timeline_filter, "limit", json_integer(1));
    types = json_object_get(timeline_filter, "types");
    json_array_append_new(types, json_string("m.room.message"));

    json_object_set_new(room_filter, "ephemeral", json_object());
    json_t* ephemeral_filter = json_object_get(room_filter, "ephemeral");
    json_object_set_new(ephemeral_filter, "types", json_array());
    json_t* rooms = json_object_get(room_filter, "rooms");

    json_t* account_data = json_object_get(response_json, "account_data");
    json_t* events = json_object_get(account_data, "events");
    if (!events) {
        efuncprintf("Failed to get events from account data\n");
        return -1;
    }
    size_t index_i;
    char const* key;
    size_t index_j;
    json_t* value_outer;
    json_t* value_inner;
    json_t* value_room_id;
    json_array_foreach(events, index_i, value_outer) {
        json_t* content = json_object_get(value_outer, "content");
        json_object_foreach(content, key, value_inner) { //key in this event is user id
            json_array_foreach(value_inner, index_j, value_room_id) {
                json_array_append(rooms, value_room_id);
                //this was used for autofilling the list of associated users for DMs, but uhhhhhh, that is done already and this allows non-existent rooms.
                //So now we just use the list to build the filter of allowed rooms which will be DMs
                // matrix_room_t* room = vector$matrix_room_t$_push(&client->rooms, (matrix_room_t){strdup(json_string_value(value_room_id)), vector$matrix_user_t$_new()});
                // vector$matrix_user_t$_push(&room->associated_users, (matrix_user_t){.id = strdup(key)});
            }
        }
    }
    char* filter_value = install_filter(client, filter_json);
    if (!filter_value) {
        efuncprintf("Failed to install filter\n");
        return -1;
    }
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync?filter=%s", client->login.homeserver_resolved, filter_value);
    free(filter_value);

    char const* ROOMS_PATH = SAVE_DIR"directs_rooms.json";
    int res = matrix_get_file(&client->login, buf, ROOMS_PATH);
    if (res) {
        efuncprintf("Failed to download directs rooms json\n");
        return res;
    }
    json_auto_t* rooms_json = json_load_file(ROOMS_PATH, 0, nullptr);
    if (!rooms_json) {
        efuncprintf("Failed to parse directs rooms json\n");
        return -1;
    }
    json_t* rooms_obj = json_object_get(rooms_json, "rooms");
    json_t* join_obj = json_object_get(rooms_obj, "join");
    json_object_foreach(join_obj, key, value_outer) { //key in this event is room id
        // matrix_room_t* room = nullptr; //same deal as above
        // for (size_t i = 0; i < client->rooms.count; i++) {
        //     if (strcmp(key, client->rooms.data[i].id) == 0) {
        //         room = &client->rooms.data[i];
        //         break;
        //     }
        // }
        // if (!room) {
        //     continue;
        // }
        // matrix_room_t* room = vector$matrix_room_t$_push(&client->rooms, (matrix_room_t){.id = strdup(key), .associated_users = vector$matrix_user_t$_new()});
        matrix_room_t* room = vector$matrix_room_t$_push(&client->rooms, (matrix_room_t){.id = strdup(key), .users = hash_map$str_const$matrix_user_t$_new()});
        json_t* state_events = json_object_get(json_object_get(value_outer, "state"), "events");
        json_t* timeline_events = json_object_get(json_object_get(value_outer, "timeline"), "events");
        matrix_room_build(room, state_events);

        json_array_foreach(timeline_events, index_i, value_inner) {
            json_t* origin_ts = json_object_get(value_inner, "origin_server_ts");
            room->last_message_time = json_integer_value(origin_ts);
        }
    }

    for (size_t i = 0; i < client->rooms.count; i++) {
        matrix_room_t* room = &client->rooms.data[i];
        if (room->name && room->avatar_url) {
            continue;
        }
        bool set_name = false;
        bool set_avatar = false;
        for (size_t j = 0; j < matrix_room_get_user_count(room); j++) { //todo: does not handle groups properly, fix eventually I guess
            matrix_user_t* user = matrix_room_get_user_by_idx(room, j);
            if (!user || strcmp(user->id, client->login.user_id) == 0) {
                continue;
            }
            if (user->avatar_url && !room->avatar_url && !set_avatar) {
                room->avatar_url = strdup(user->avatar_url);
                set_avatar = true;
            }
            if (!room->name && !set_name) {
                if (user->display_name) {
                    room->name = strdup(user->display_name);
                } else {
                    room->name = strdup(user->id);
                }
                set_name = true;
            }
            if (set_name && set_avatar) {
                break;
            }
        }
    }
    qsort(client->rooms.data, client->rooms.count, sizeof(matrix_room_t), matrix_room_order);

    return 0;
}

int matrix_client_sync_dump(matrix_client_t* client) {
    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync", client->login.homeserver_resolved);

    int res = matrix_get_file(&client->login, buf, SAVE_DIR"sync_dump.json");
    return res;
}

matrix_client_t make_client() {
    matrix_client_t client = {0};
    return client;
}

void destroy_client(matrix_client_t* client) {
    matrix_login_destroy(&client->login);
    json_decref(client->core_account_data);
    wipe_remake_client_temps(client);
    vector$matrix_event_t$_free_callback(&client->events, matrix_event_destroy);
}
