#include "matrix-client.h"

#include <jansson.h>
#include <stdio.h>
#include <string.h>

#include "../utils.h"

char* filter_id = nullptr;

int make_filter(matrix_client_t const* client) { //rendered useless, keeping for testing or building future filters
    int status = -1;

    json_t* filter_json = json_object();
    json_object_set_new(filter_json, "room", json_object());

    json_t* room_filter = json_object_get(filter_json, "room");
    // json_object_set_new(room_filter, "timeline", json_object());
    // json_object_set_new(room_filter, "state", json_object());
    json_object_set_new(room_filter, "rooms", json_array());

    // json_t* timeline_filter = json_object_get(room_filter, "timeline");
    // // json_object_set_new(timeline_filter, "limit", json_integer(1));
    // json_object_set_new(timeline_filter, "types", json_array());
    // json_object_set_new(timeline_filter, "lazy_load_members", json_true());
    //
    // json_t* state_filter = json_object_get(room_filter, "state");
    // // json_object_set_new(state_filter, "limit", json_integer(1));
    // json_object_set_new(state_filter, "types", json_array());
    // json_object_set_new(state_filter, "lazy_load_members", json_true());

    json_object_set_new(filter_json, "presence", json_object());
    json_t* presence_filter = json_object_get(filter_json, "presence");
    json_object_set_new(presence_filter, "types", json_array());

    json_object_set_new(filter_json, "account_data", json_object());
    json_t* account_filter = json_object_get(filter_json, "account_data");
    json_object_set_new(account_filter, "not_types", json_array());
    json_t* not_types = json_object_get(account_filter, "not_types");
    json_array_append_new(not_types, json_string("m.direct"));
    json_array_append_new(not_types, json_string("m.push_rules"));

    // json_dump_file(filter_json, SAVE_DIR"whatever", JSON_COMPACT);
    char* json_str = json_dumps(filter_json, JSON_COMPACT);
    printf("%s\n", json_str);

    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/user/%s/filter", client->login.homeserver_resolved, client->login.user_id);

    char* response_str = post_json_string(buf, json_str);
    free(json_str);
    json_decref(filter_json);
    json_t* response = nullptr;

    if (!response_str) {
        efuncprintf("Failed to set filter\n");
        goto exit;
    }
    response = json_loads(response_str, 0, nullptr);
    if (!response) {
        efuncprintf("Failed to load filter response json\n");
        goto exit;
    }
    json_t* filter_id_json = json_object_get(response, "filter_id");
    if (!filter_id_json || !json_is_string(filter_id_json)) {
        efuncprintf("Failed to retrieve filter_id\n");
        goto exit;
    }
    filter_id = strdup(json_string_value(filter_id_json));

    status = 0;

exit:
    json_decref(response);
    free(response_str);
    return status;
}

int matrix_client_sync_account_data(matrix_client_t* client) {
    int res;
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

    char* account_data_str = http_get_string(buf);
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

void destroy_client(matrix_client_t* client) {
    matrix_login_destroy(&client->login);
    json_decref(client->core_account_data);
}
