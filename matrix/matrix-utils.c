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

struct ipa_process_args_t* ipa_process_args_new(matrix_login_t* login, bool retry, ipa_callback_t next_callback, void* next_args, size_t next_args_size) {
    struct ipa_process_args_t* args = malloc(sizeof(struct ipa_process_args_t));
    args->login = login;
    args->retry = retry;
    args->next_callback = next_callback;
    args->next_args = next_args;
    args->next_args_size = next_args_size;
    return args;
}

triple$bool$alloc_void$size_t$ matrix_process_data(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {false, nullptr, 0};
    struct ipa_process_args_t args = *(struct ipa_process_args_t*)entry.args;
    bool retry = args.retry;
    if (entry.status == 200) {
        ret.second = entry.result;
        ret.third = entry.result_size;
        if (args.next_callback) {
            ret.first = false;
            void* data = malloc(entry.result_size);
            memcpy(data, entry.result, entry.result_size);
            ipa_queue_entry_t next = make_function_queue_entry(args.next_callback, data, entry.result_size, args.next_args, args.next_args_size);
            add_to_work_queue_with_id(next, entry.id);
        } else {
            ret.first = true;
        }
        return ret;
    }
    if (entry.status != 401 || retry) {
        efuncprintf("Failed to download page [%s], response code: %ld\n", entry.url, entry.status);
        ret.first = true;
        ret.second = nullptr;
        ret.third = 0;
        return ret;
    }
    if (!needs_refresh(json_loads(entry.result, 0, nullptr))) {
        ret.first = true;
        ret.second = nullptr;
        ret.third = 0;
        return ret;
    }

    struct ipa_process_args_t* new_args = malloc(sizeof(args));
    *new_args = args;
    new_args->retry = true;
    ipa_queue_entry_t new_entry = make_download_str_queue_entry_callback(entry.url, entry.func, new_args, sizeof(args));
    prepend_to_work_queue(new_entry, entry.id);

    matrix_login_from_save(args.login, true);

    return ret;
}

triple$bool$alloc_void$size_t$ matrix_process_file(ipa_queue_entry_t entry) {
    triple$bool$alloc_void$size_t$ ret = {false, nullptr, 0};
    struct ipa_process_args_t args = *(struct ipa_process_args_t*)entry.args;
    bool retry = args.retry;
    if (entry.status == 200) {
        ret.second = malloc(entry.result_size);
        memcpy(ret.second, entry.result, entry.result_size);
        ret.third = entry.result_size;
        if (args.next_callback) {
            ret.first = false;
            void* data = malloc(entry.result_size);
            memcpy(data, entry.result, entry.result_size);
            ipa_queue_entry_t next = make_function_queue_entry(args.next_callback, data, entry.result_size, args.next_args, args.next_args_size);
            add_to_work_queue_with_id(next, entry.id);
        } else {
            ret.first = true;
        }
        return ret;
    }
    if (entry.status != 401 || retry) {
        return ret;
    }
    if (!needs_refresh(json_load_file(entry.result, 0, nullptr))) {
        unlink(entry.result);
        return ret;
    }

    struct ipa_process_args_t* new_args = malloc(sizeof(args));
    *new_args = args;
    new_args->retry = true;
    ipa_queue_entry_t new_entry = make_download_str_queue_entry_callback(entry.url, entry.func, new_args, sizeof(args));
    prepend_to_work_queue(new_entry, entry.id);

    matrix_login_from_save(args.login, true);

    return ret;
}

triple$bool$alloc_void$size_t$ matrix_process_post_data(ipa_queue_entry_t entry) {
    return matrix_process_data(entry); //atm it's the same
}

triple$bool$alloc_void$size_t$ matrix_process_post_file(ipa_queue_entry_t entry) {
    return matrix_process_file(entry); //ditto on it being the same now
}

int matrix_get_mxc_url(matrix_login_t* login, char const* mxc_url, char out_url[URL_BUFFER_SIZE]) {
    char const* stripped = strstr(mxc_url, "mxc://");
    if (!stripped) {
        return -1;
    }
    stripped += sizeof("mxc://") - 1;
    pthread_mutex_lock(&login->mutex);
    snprintf(out_url, sizeof(char[URL_BUFFER_SIZE]), "%s/_matrix/client/v1/media/download/%s", login->homeserver_resolved, stripped);
    pthread_mutex_unlock(&login->mutex);
    return 0;
}
