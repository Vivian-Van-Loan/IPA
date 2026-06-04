#include "login.h"

#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#include "matrix-utils.h"
#include "../utils.h"

void matrix_login_destroy(matrix_login_t* login);

//DOES NOT logout, just frees and clears
matrix_login_t matrix_login_pass(char const* homeserver_base, char const* user, char const* pass, char const* device_id_str) {
    matrix_login_t login = {0};
    json_t* root = nullptr;
    json_t* response = nullptr;
    char* json_str = nullptr;
    char* response_str = nullptr;
    login.homeserver_resolved = matrix_resolve_homeserver(homeserver_base);
    if (!login.homeserver_resolved) {
        efuncprintf("Failed to resolve homeserver to a matrix location");
        goto exit;
    }

    root = json_object();
    json_object_set_new(root, "type", json_string("m.login.password"));
    json_object_set_new(root, "password", json_string(pass));
    if (device_id_str) {
        json_object_set_new(root, "device_id", json_string(device_id_str));
    }
    json_object_set_new(root, "identifier", json_object());
    json_t* identifier = json_object_get(root, "identifier");
    json_object_set_new(identifier, "type", json_string("m.id.user"));
    json_object_set_new(identifier, "user", json_string(user));
    json_str = json_dumps(root, JSON_COMPACT);
    // printf("Login: %s\n", json_str);

    char buf[256];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/login", login.homeserver_resolved);
    response_str = post_json_string(buf, json_str);
    printf("Response: %s\n", response_str);
    if (!response_str) {
        efuncprintf("Failed to POST and download login\n");
        goto exit;
    }
    json_error_t json_error;
    response = json_loads(response_str, 0, &json_error);
    if (!response) {
        efuncprintf("Failed to parse json response\n");
        goto exit;
    }
    json_t* user_id = json_object_get(response, "user_id");
    json_t* access_token = json_object_get(response, "access_token");
    json_t* homeserver = json_object_get(response, "home_server");
    json_t* device_id = json_object_get(response, "device_id");
    if (!user_id || !json_is_string(user_id)) {
        efuncprintf("user_id failed\n");
        goto exit;
    } else if (!access_token || !json_is_string(access_token)) {
        efuncprintf("access_token failed\n");
        goto exit;
    } else if (!homeserver || !json_is_string(homeserver)) {
        efuncprintf("homeserver failed\n");
        goto exit;
    } else if (!device_id || !json_is_string(device_id)) {
        efuncprintf("device_id failed\n");
        goto exit;
    }
    login.user_id = strdup(json_string_value(user_id));
    login.access_token = strdup(json_string_value(access_token));
    login.homeserver = strdup(json_string_value(homeserver));
    login.device_id = strdup(json_string_value(device_id));

    snprintf(buf, sizeof(buf), "Authorization: Bearer %s", login.access_token);
    int res = curl_add_header(buf);
    if (res) {
        efuncprintf("Failed to add access token to curl headers\n");
        goto exit;
    }

    login.logged_in = true;
exit:
    json_decref(root);
    json_decref(response);
    free(json_str);
    free(response_str);
    if (!login.logged_in)
        matrix_login_destroy(&login);
    return login;
}

void matrix_login_destroy(matrix_login_t* login) {
    free(login->user_id);
    free(login->homeserver);
    free(login->homeserver_resolved);
    free(login->device_id);
    free(login->access_token);
    memset(login, 0, sizeof(*login));
}
