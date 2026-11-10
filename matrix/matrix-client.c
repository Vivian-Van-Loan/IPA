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

triple$bool$alloc_void$size_t$ install_filter_response(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {true};
    struct ipa_process_args_t* args = (struct ipa_process_args_t*)entry.args;
    if (!entry.result) {
        efuncprintf("Failed to install filter\n");
    }

    json_t* response = json_loads(entry.result, 0, nullptr);
    if (!response) {
        efuncprintf("Failed to load filter response json\n");
        json_decref(response);
        return ret;
    }
    json_t* filter_id_json = json_object_get(response, "filter_id");
    if (!filter_id_json || !json_is_string(filter_id_json)) {
        efuncprintf("Failed to retrieve filter_id\n");
        json_decref(response);
        return ret;
    }
    ret.second = strdup(json_string_value(filter_id_json));
    ret.third = strlen(ret.second) + 1;

    struct ipa_process_args_t* new_args = ipa_process_args_new(args->login, false, nullptr, nullptr, 0);
    ipa_queue_entry_t new_entry = make_function_queue_entry(args->next_callback, strdup(ret.second), ret.third, new_args, sizeof(*new_args));
    add_to_work_queue_with_id(new_entry, entry.id);
    ret.first = false;
    return ret;
}

void install_filter(struct ipa_process_args_t* args, json_t const* filter_json, void* next_args, size_t next_args_size, ssize_t id) {
    char* json_str = json_dumps(filter_json, JSON_COMPACT);
    // printf("%s\n", json_str);

    char buf[URL_BUFFER_SIZE];
    pthread_mutex_lock(&args->login->mutex);
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/user/%s/filter", args->login->homeserver_resolved, args->login->user_id);
    pthread_mutex_unlock(&args->login->mutex);

    ipa_queue_entry_t new_entry = make_post_download_str_queue_entry_callback(buf, json_str, install_filter_response, next_args, next_args_size);
    add_to_work_queue_with_id(new_entry, id);
    free(json_str);
}

triple$bool$alloc_void$size_t$ process_matrix_client_sync_account_data(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {true};
    if (!entry.result) {
        efuncprintf("Failed to download sync json\n");
        return ret;
    }
    json_t* core_account_data = json_loads(entry.result, 0, nullptr);
    if (!core_account_data) {
        efuncprintf("Failed to parse account data\n");
        return ret;
    }
    ret.second = core_account_data;
    ret.third = sizeof(json_t*);

    return ret;
}

ssize_t queue_matrix_client_sync_account_data(matrix_client_t* client) {
    if (!client) {
        return -1;
    }
    char buf[URL_BUFFER_SIZE];
    CURL* curl = curl_easy_init();
    char* filter_url = curl_easy_escape(curl, "{\"room\":{\"rooms\":[]},\"presence\":{\"types\":[]},\"account_data\":{\"not_types\":[\"m.direct\",\"m.push_rules\"]}}", 0); //https://spec.matrix.org/v1.18/client-server-api/#filtering
    curl_easy_cleanup(curl);
    pthread_mutex_lock(&client->login.mutex);
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync?filter=%s", client->login.homeserver_resolved, filter_url);
    pthread_mutex_unlock(&client->login.mutex);
    curl_free(filter_url);

    struct ipa_process_args_t* args = ipa_process_args_new(&client->login, false, process_matrix_client_sync_account_data, nullptr, 0);
    return add_download_str_to_work_queue_callback(buf, matrix_process_data, args, sizeof(*args));
}

void wipe_remake_client_temps(matrix_client_t* client) {
    vector$matrix_room_t$_free_callback(&client->rooms, matrix_room_destroy);
    client->rooms = vector$matrix_room_t$_new();
}

triple$bool$alloc_void$size_t$ process_sync_directs_sync_response(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {true};
    struct ipa_process_args_t args = *(struct ipa_process_args_t*)entry.args;

    ret.third = sizeof(vector$matrix_room_t$);
    ret.second = calloc(ret.third, 1);
    vector$matrix_room_t$* vec = ret.second;

    if (!entry.result) {
        efuncprintf("Failed to download directs rooms json\n");
        return ret;
    }
    json_auto_t* rooms_json = json_load_file(entry.result, 0, nullptr);
    if (!rooms_json) {
        efuncprintf("Failed to parse directs rooms json\n");
        return ret;
    }
    char const* key;
    json_t* value_outer;
    json_t* rooms_obj = json_object_get(rooms_json, "rooms");
    json_t* join_obj = json_object_get(rooms_obj, "join");
    json_object_foreach(join_obj, key, value_outer) { //key in this event is room id
        matrix_room_t* room = vector$matrix_room_t$_push_back(vec, (matrix_room_t){.id = strdup(key), .users = hash_map$str_const$matrix_event_t$_new()});
        json_t* state_events = json_object_get(json_object_get(value_outer, "state"), "events");
        json_t* timeline_events = json_object_get(json_object_get(value_outer, "timeline"), "events");
        matrix_room_build(room, state_events);

        size_t index_i;
        json_t* value_inner;
        json_array_foreach(timeline_events, index_i, value_inner) {
            json_t* origin_ts = json_object_get(value_inner, "origin_server_ts");
            room->last_message_time = json_integer_value(origin_ts);
        }
    }

    pthread_mutex_lock(&args.login->mutex);
    char* user_id_copy = strdup(args.login->user_id);
    pthread_mutex_unlock(&args.login->mutex);
    for (size_t i = 0; i < vec->count; i++) {
        matrix_room_t* room = &vec->data[i];
        if (room->name && room->avatar_url) {
            continue;
        }
        bool set_name = false;
        bool set_avatar = false;
        for (size_t j = 0; j < matrix_room_get_user_count(room); j++) { //todo: does not handle groups properly, fix eventually I guess
            matrix_event_t* user_event = matrix_room_get_user_by_idx(room, j);
            if (!user_event || strcmp(user_event->id, user_id_copy) == 0) {
                continue;
            }
            matrix_room_member_t* member = &user_event->room.member;
            if (member->avatar_url && !room->avatar_url && !set_avatar) {
                room->avatar_url = strdup(member->avatar_url);
                set_avatar = true;
            }
            if (!room->name && !set_name) {
                if (member->display_name) {
                    room->name = strdup(member->display_name);
                } else {
                    room->name = strdup(user_event->sender);
                }
                set_name = true;
            }
            if (set_name && set_avatar) {
                break;
            }
        }
    }
    free(user_id_copy);
    qsort(vec->data, vec->count, sizeof(matrix_room_t), matrix_room_order);

    return ret;
}

triple$bool$alloc_void$size_t$ process_sync_directs_install_filter_response(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {false};
    struct ipa_process_args_t args = *(struct ipa_process_args_t*)entry.args;
    if (!entry.result) {
        efuncprintf("Failed to install filter\n");
    }
    char buf[URL_BUFFER_SIZE];
    pthread_mutex_lock(&args.login->mutex);
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync?filter=%s", args.login->homeserver_resolved, (char const*)entry.result);
    pthread_mutex_unlock(&args.login->mutex);

    char const* ROOMS_PATH = SAVE_DIR"directs_rooms.json";

    struct ipa_process_args_t* next_args = ipa_process_args_new(args.login, false, nullptr, nullptr, 0);
    ipa_queue_entry_t new_entry = make_download_file_queue_entry_callback(buf, ROOMS_PATH, process_sync_directs_sync_response, next_args, sizeof(*next_args));
    add_to_work_queue_with_id(new_entry, entry.id);
    return ret;
}

triple$bool$alloc_void$size_t$ process_matrix_client_sync_directs(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {true};
    struct ipa_process_args_t* args = entry.args;
    if (!entry.result) {
        efuncprintf("Failed to download sync json\n");
        return ret;
    }
    json_auto_t* response_json = json_loads(entry.result, 0, nullptr);
    if (!response_json) {
        efuncprintf("Failed to parse sync json\n");
        return ret;
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
        return ret;
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
            }
        }
    }

    ret.first = false;
    struct ipa_process_args_t* new_args = ipa_process_args_new(args->login, false, process_sync_directs_install_filter_response, nullptr, 0);
    install_filter(args, filter_json, new_args, sizeof(*new_args), entry.id);
    return ret;
}

ssize_t queue_matrix_client_sync_directs(matrix_client_t* client) {
    if (!client) {
        return -1;
    }

    wipe_remake_client_temps(client);

    char buf[URL_BUFFER_SIZE];
    // if (!account_direct_filter_id) {
    //     make_account_direct_filter(client);
    // }
    CURL* curl = curl_easy_init();
    char* filter_url = curl_easy_escape(curl, "{\"account_data\":{\"types\":[\"m.direct\"]}}", 0); //https://spec.matrix.org/v1.18/client-server-api/#filtering
    curl_easy_cleanup(curl);
    pthread_mutex_lock(&client->login.mutex);
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync?filter=%s", client->login.homeserver_resolved, filter_url);
    pthread_mutex_unlock(&client->login.mutex);
    curl_free(filter_url);

    struct ipa_process_args_t* next_args = ipa_process_args_new(&client->login, false, nullptr, nullptr, 0);
    struct ipa_process_args_t* args = ipa_process_args_new(&client->login, false, process_matrix_client_sync_directs, next_args, sizeof(*next_args));
    return add_download_str_to_work_queue_callback(buf, matrix_process_data, args, sizeof(*args));
}

ssize_t queue_matrix_client_sync_spaces(matrix_client_t* client) {
    efuncprintf("was called before it was implemented");
    exit(1);
}

ssize_t queue_matrix_client_sync_rooms(matrix_client_t* client) {
    efuncprintf("was called before it was implemented");
    exit(1);
}

ssize_t queue_matrix_client_sync_dump(matrix_client_t* client) {
    if (!client) {
        return -1;
    }
    pthread_mutex_lock(&client->login.mutex);
    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/sync", client->login.homeserver_resolved);
    pthread_mutex_unlock(&client->login.mutex);

    struct ipa_process_args_t* args = ipa_process_args_new(&client->login, false, nullptr, nullptr, 0);
    return add_download_file_to_work_queue_callback(buf, SAVE_DIR"sync_dump.json", matrix_process_file, args, sizeof(*args));
}

triple$bool$alloc_void$size_t$ process_matrix_client_load_room(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {true};
    json_t* room_json = nullptr;
    if (!entry.result) {
        efuncprintf("Failed to download room json\n");
        goto error;
    }
    room_json = json_loads(entry.result, 0, nullptr);
    if (!room_json) {
        efuncprintf("Failed to parse room json\n");
        goto error;
    }
    json_t* chunk = json_object_get(room_json, "chunk");
    if (!chunk || !json_is_array(chunk)) {
        efuncprintf("Failed to get chunk from sync json\n");
        goto error;
    }

    matrix_room_info_t room_info = {0};
    pair$bool$alloc_str$* pair = entry.args;
    room_info.reverse = pair->first;
    room_info.id = pair->second;
    room_info.events = dequeue$matrix_event_t$_new();
    size_t index_i;
    json_t* value_inner;
    json_array_foreach(chunk, index_i, value_inner) {
        matrix_event_t event = matrix_make_event(value_inner);
        if (room_info.reverse) {
            dequeue$matrix_event_t$_push(&room_info.events, event);
        } else {
            dequeue$matrix_event_t$_push_back(&room_info.events, event);
        }
    }

    json_t* end = json_object_get(room_json, "end");
    if (end && json_is_string(end)) {
        room_info.end = strdup(json_string_value(end));
    } else {
        room_info.end = nullptr;
    }
    json_decref(room_json);

    ret.first = true;
    ret.second = malloc(sizeof(room_info));
    memcpy(ret.second, &room_info, sizeof(room_info));
    ret.third = sizeof(room_info);

    return ret;

    error:
    efuncprintf("Failed to load room\n");
    json_decref(room_json);
    return ret;
}

ssize_t queue_matrix_client_load_room(matrix_client_t* client, char const* room_id, bool reverse) {
    if (!client || !room_id) {
        return -1;
    }
    matrix_room_t* room = matrix_client_get_room(client, room_id);
    if (!room) {
        efuncprintf("Failed to find room with id %s\n", room_id);
        return -1;
    }

    char buf[URL_BUFFER_SIZE];
    pthread_mutex_lock(&client->login.mutex);
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/rooms/%s/messages", client->login.homeserver_resolved, room->id);
    pthread_mutex_unlock(&client->login.mutex);
    CURLUcode res = 0;
    CURLU* url = curl_url();
    res = curl_url_set(url, CURLUPART_URL, buf, 0);
    if (res != CURLUE_OK)
        goto url_error;
    if (reverse) {
        res = curl_url_set(url, CURLUPART_QUERY, "dir=b", CURLU_APPENDQUERY);
    } else {
        res = curl_url_set(url, CURLUPART_QUERY, "dir=f", CURLU_APPENDQUERY);
    }
    if (res != CURLUE_OK)
        goto url_error;
    res = curl_url_set(url, CURLUPART_QUERY, "limit=20", CURLU_APPENDQUERY);
    if (res != CURLUE_OK)
        goto url_error;
    if (room->end) {
        snprintf(buf, sizeof(buf), "from=%s", room->end);
        res = curl_url_set(url, CURLUPART_QUERY, buf, CURLU_APPENDQUERY);
        if (res != CURLUE_OK) {
            goto url_error;
        }
    }
    char* complete_url;
    res = curl_url_get(url, CURLUPART_URL, &complete_url, 0);
    if (res != CURLUE_OK) {
        goto url_error;
    }
    curl_url_cleanup(url);

    pair$bool$alloc_str$* next = malloc(sizeof(pair$bool$alloc_str$));
    next->first = reverse;
    next->second = strdup(room_id);
    struct ipa_process_args_t* args = ipa_process_args_new(&client->login, false, process_matrix_client_load_room, next, sizeof(pair$bool$alloc_str$));
    ssize_t id = add_download_str_to_work_queue_callback(complete_url, matrix_process_data, args, sizeof(*args));
    curl_free(complete_url);
    return id;

    url_error:
    efuncprintf("Failed to set url\n");
    curl_url_cleanup(url);
    return -1;
}

matrix_room_t* matrix_client_get_room(matrix_client_t* client, char const* room_id) {
    matrix_room_t* room = nullptr;
    for (size_t i = 0; i < client->rooms.count; i++) {
        matrix_room_t* r = &client->rooms.data[i];
        if (strcmp(r->id, room_id) == 0) {
            room = r;
            break;
        }
    }
    return room;
}

matrix_client_t make_client() {
    matrix_client_t client = {0};
    client.login = make_login();
    return client;
}

void destroy_client(matrix_client_t* client) {
    pthread_mutex_lock(&client->login.mutex); //it is destroyed after this
    matrix_login_destroy(&client->login);
    json_decref(client->core_account_data);
    wipe_remake_client_temps(client);
}
