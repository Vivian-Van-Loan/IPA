#include "matrix-utils.h"

#include <curl/curl.h>
#include <jansson.h>
#include <string.h>
#include <sys/unistd.h>

#include "../utils.h"

char* matrix_resolve_homeserver(char const* homeserver_base) {
    char buf[URL_BUFFER_SIZE];
    snprintf(buf, sizeof(buf), "%s/.well-known/matrix/client", homeserver_base);
    pair$alloc_str$long$ res = http_get_string(buf);
    if (res.second != 200) {
        free(res.first);
        return nullptr;
    }
    char* json_str = res.first;

    char* resolved = nullptr;
    json_error_t error;
    json_t* root = json_loads(json_str, 0, &error);
    free(json_str);

    if (!root)
        return resolved;
    json_t* container = json_object_get(root, "m.homeserver");
    if (!container) {
        efuncprintf("Failed to get m.homeserver\n");
        goto exit;
    }
    json_t* url = json_object_get(container, "base_url");
    if (!url || !json_is_string(url)) {
        efuncprintf("Failed to get base_url from m.homeserver\n");
        goto exit;
    }
    resolved = strdup(json_string_value(url));
    size_t len = strlen(resolved);
    if (resolved[len - 1] == '/') {
        resolved[len - 1] = '\0';
    }

    exit:
    json_decref(root);
    return resolved;
}

bool needs_refresh(json_t* json) {
    json_t* error = json_object_get(json, "errcode");
    if (!error || !json_is_string(error)) {
        json_decref(json);
        return false;
    }
    if (strcmp(json_string_value(error), "M_UNKNOWN_TOKEN") != 0) {
        json_decref(json);
        return false;
    }
    json_t* soft = json_object_get(json, "soft_logout");
    if (!soft || !json_is_boolean(soft)) {
        json_decref(json);
        return false;
    }
    bool val = json_boolean_value(soft);
    json_decref(json);
    return val;
}

char* matrix_get_string(matrix_login_t* login, char const* url) {
    bool retry = false;
    while (true) {
        pair$alloc_str$long$ ret = http_get_string(url);
        if (ret.second == 200) {
            return ret.first;
        }
        if (ret.second != 401 || retry) {
            efuncprintf("Failed to download page [%s], response code: %ld\n", url, ret.second);
            free(ret.first);
            return nullptr;
        }
        if (!needs_refresh(json_loads(ret.first, 0, nullptr))) {
            free(ret.first);
            return nullptr;
        }
        matrix_login_from_save(login, true);
        retry = true;
    }
}

pair$alloc_void$size_t$ matrix_get_data(matrix_login_t* login, char const* url) {
    pair$alloc_void$size_t$ ret = {nullptr, 0};

    bool retry = false;
    while (true) {
        pair$alloc_void$long$ pair = http_get_data(url);
        curl_off_t len;
        CURLcode c = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &len);
        if (c != CURLE_OK) {
            efuncprintf("Failed to get content length\n");
            free(pair.first);
            return ret;
        }
        if (pair.second == 200) {
            ret.first = pair.first;
            ret.second = len;
            return ret;
        }
        if (pair.second != 401 || retry) {
            efuncprintf("Failed to download page [%s], response code: %ld\n", url, pair.second);
            free(pair.first);
            return ret;
        }
        if (!needs_refresh(json_loads(pair.first, 0, nullptr))) {
            free(pair.first);
            return ret;
        }
        matrix_login_from_save(login, true);
        retry = true;
    }
}

int matrix_get_file(matrix_login_t* login, char const* url, char const* path) {
    bool retry = false;
    while (true) {
        long res = http_get_file(url, path);
        if (res == 200) {
            return 0;
        }
        if (res != 401 || retry) {
            return res;
        }
        if (!needs_refresh(json_load_file(path, 0, nullptr))) {
            unlink(path);
            return -1;
        }
        matrix_login_from_save(login, true);
        retry = true;
    }
}

char* matrix_post_json_string(matrix_login_t* login, char const* url, char const* json) {
    bool retry = false;
    while (true) {
        pair$alloc_str$long$ ret = post_json_string(url, json);
        if (ret.second == 200) {
            return ret.first;
        }
        if (ret.second != 401 || retry) {
            efuncprintf("Failed to download page [%s], response code: %ld\n", url, ret.second);
            free(ret.first);
            return nullptr;
        }
        if (!needs_refresh(json_loads(ret.first, 0, nullptr))) {
            free(ret.first);
            return nullptr;
        }
        matrix_login_from_save(login, true);
        retry = true;
    }
}

int matrix_post_json_file(matrix_login_t* login, char const* url, char const* json, char const* path) {
    bool retry = false;
    while (true) {
        long res = post_json_file(url, json, path);
        if (res == 200) {
            return 0;
        }
        if (res != 401 || retry) {
            return res;
        }
        if (!needs_refresh(json_load_file(path, 0, nullptr))) {
            unlink(path);
            return -1;
        }
            matrix_login_from_save(login, true);
            retry = true;
    }
}

pair$alloc_void$size_t$ matrix_download_mxc(matrix_client_t* client, char const* mxc_url) {
    pair$alloc_void$size_t$ ret = {nullptr, 0};

    char buf[URL_BUFFER_SIZE];
    char const* stripped = strstr(mxc_url, "mxc://");
    if (!stripped) {
        return ret;
    }
    stripped += sizeof("mxc://") - 1;
    snprintf(buf, sizeof(buf), "%s/_matrix/client/v1/media/download/%s", client->login.homeserver_resolved, stripped);
    ret = matrix_get_data(&client->login, buf);
    return ret;
}
