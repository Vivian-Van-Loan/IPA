#include "login.h"

#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#include "matrix-utils.h"
#include "../utils.h"

#define REFRESH_SAVE SAVE_DIR"access.txt"

void matrix_login_dump_refresh(matrix_login_t const* login) {
    if (!login->refresh_token || !login->user_id || !login->access_token) {
        efuncprintf("Missing login requirements\n");
        return;
    }
    FILE* f = fopen(REFRESH_SAVE, "w");
    if (!f) {
        efuncprintf("Failed to open access.txt\n");
        return;
    }
    fprintf(f, "%s\n%s\n%s\n%s\n", login->user_id, login->access_token, login->refresh_token, login->device_id);
    fclose(f);
}

void matrix_login_from_save(matrix_login_t* login) {
    FILE* f = fopen(REFRESH_SAVE, "r");
    if (!f) {
        efuncprintf("Failed to open access.txt\n");
        return;
    }
    char* lineptr = nullptr;
    size_t line = 0;
    size_t n;
    ssize_t line_len;
    char* homeserver = nullptr;
    char* user = nullptr;
    char* refresh_token = nullptr;
    // char* access_token = nullptr;
    char* device_id = nullptr;
    bool failed = false;
    while ((line_len = __getline(&lineptr, &n, f)) != -1) {
        if (line_len == 0 || lineptr[0] == '\n') {
            failed = true;
            break;
        }
        lineptr[line_len - 1] = '\0';
        if (line == 0) {
            user = strdup(lineptr);
            char* colon = strchr(lineptr, ':');
            if (colon) {
                *colon = '\0';
                colon++;
                homeserver = strdup(colon);
            }
        } else if (line == 1) {
            // access_token = strdup(lineptr);
        } else if (line == 2) {
            refresh_token = strdup(lineptr);
        } else if (line == 3) {
            device_id = strdup(lineptr);
        }
        line++;
    }
    if (line <= 3) {
        failed = true;
    }
    free(lineptr);
    fclose(f);
    if (!failed) {
        login->device_id = strdup(device_id);
        matrix_login_refresh(login, homeserver, user, refresh_token);
    } else {
        login->logged_in = false;
    }
    free(homeserver);
    free(user);
    free(refresh_token);
    // free(access_token);
    free(device_id);
}

void matrix_login_refresh(matrix_login_t* login, char const* homeserver_base, char const* user, char const* refresh_token) {
    login->logged_in = false;
    login->homeserver_resolved = matrix_resolve_homeserver(homeserver_base);
    if (!login->homeserver_resolved) {
        efuncprintf("Failed to resolve homeserver to a matrix location\n");
        return;
    }
    login->user_id = strdup(user);
    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/refresh", login->homeserver_resolved);
    json_auto_t* root = json_object();
    json_object_set_new(root, "refresh_token", json_string(refresh_token));
    char* json_str = json_dumps(root, JSON_COMPACT);
    char* response_str = post_json_string(buf, json_str);
    json_auto_t* response = json_loads(response_str, 0, nullptr);
    free(json_str);
    free(response_str);
    if (!response) {
        efuncprintf("Failed to parse refresh response\n");
        matrix_login_destroy(login);
        return;
    }
    json_t* access_token = json_object_get(response, "access_token");
    json_t* refresh_token_new = json_object_get(response, "refresh_token");
    login->access_token = strdup(json_string_value(access_token));
    login->refresh_token = strdup(json_string_value(refresh_token_new));
    matrix_login_dump_refresh(login);
    snprintf(buf, sizeof(buf), "Authorization: Bearer %s", login->access_token);
    int res = curl_add_header(buf);
    if (res) {
        efuncprintf("Failed to add access token to curl headers\n");
        matrix_login_destroy(login);
        return;
    }
    login->logged_in = true;
}

void matrix_login_pass(matrix_login_t* login, char const* homeserver_base, char const* user, char const* pass, char const* device_id_str) {
    login->logged_in = false;
    json_t* root = nullptr;
    json_t* response = nullptr;
    char* json_str = nullptr;
    char* response_str = nullptr;
    login->homeserver_resolved = matrix_resolve_homeserver(homeserver_base);
    if (!login->homeserver_resolved) {
        efuncprintf("Failed to resolve homeserver to a matrix location\n");
        goto exit;
    }

    root = json_object();
    json_object_set_new(root, "type", json_string("m.login.password"));
    json_object_set_new(root, "password", json_string(pass));
    json_object_set_new(root, "refresh_token", json_true());
    if (device_id_str) {
        json_object_set_new(root, "device_id", json_string(device_id_str));
    }
    json_object_set_new(root, "identifier", json_object());
    json_t* identifier = json_object_get(root, "identifier");
    json_object_set_new(identifier, "type", json_string("m.id.user"));
    json_object_set_new(identifier, "user", json_string(user));
    json_str = json_dumps(root, JSON_COMPACT);
    // printf("Login: %s\n", json_str);

    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v3/login", login->homeserver_resolved);
    response_str = post_json_string(buf, json_str);
    // printf("Response: %s\n", response_str);
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
    json_t* refresh_token = json_object_get(response, "refresh_token");
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
    login->user_id = strdup(json_string_value(user_id));
    login->access_token = strdup(json_string_value(access_token));
    login->homeserver = strdup(json_string_value(homeserver));
    login->device_id = strdup(json_string_value(device_id));
    if (refresh_token && json_is_string(refresh_token)) {
        login->refresh_token = strdup(json_string_value(refresh_token));
        matrix_login_dump_refresh(login);
    } else {
        login->refresh_token = nullptr;
    }

    snprintf(buf, sizeof(buf), "Authorization: Bearer %s", login->access_token);
    int res = curl_add_header(buf);
    if (res) {
        efuncprintf("Failed to add access token to curl headers\n");
        goto exit;
    }

    login->logged_in = true;
exit:
    json_decref(root);
    json_decref(response);
    free(json_str);
    free(response_str);
    if (!login->logged_in) {
        matrix_login_destroy(login);
    }
}

//DOES NOT logout, just frees and clears
void matrix_login_destroy(matrix_login_t* login) {
    free(login->user_id);
    free(login->homeserver);
    free(login->homeserver_resolved);
    free(login->device_id);
    free(login->access_token);
    memset(login, 0, sizeof(*login));
}
