#include "work_queue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

#include "utils.h"

static vector$ipa_queue_entry_t$ work_queue;
static vector$ipa_queue_entry_t$ finished_queue;
static ssize_t next_id = 0;

static pthread_t work_queue_thread;
static atomic_bool work_queue_thread_running = false;
static pthread_mutex_t work_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void* work_queue_loop(void* arg);

void destroy_queue_entry(ipa_queue_entry_t* entry) {
    free((void*)entry->url);
    if (entry->type == IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_FILE) {
        pair$str_const$str_const$ args = get_str_pair_args(entry->download_args);
        free((void*)args.first);
        free((void*)args.second);
    }
    free(entry->download_args);
    free(entry->args);
    free(entry->result);
    memset(entry, 0, sizeof(*entry));
}

int init_work_queue() {
    if (work_queue_thread_running) {
        return 0;
    }
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    work_queue = vector$ipa_queue_entry_t$_new();
    finished_queue = vector$ipa_queue_entry_t$_new();
    pthread_mutex_unlock(&work_queue_mutex);
    return 0;
}

int destroy_work_queue() {
    if (work_queue_thread_running) {
        return -1;
    }
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;

    vector$ipa_queue_entry_t$_free_callback(&work_queue, destroy_queue_entry);
    vector$ipa_queue_entry_t$_free_callback(&finished_queue, destroy_queue_entry);
    next_id = 0;

    pthread_mutex_unlock(&work_queue_mutex);
    return 0;
}

pair$str_const$str_const$ get_str_pair_args(void* args) {
    pair$str_const$str_const$ result = {0};
    result.first = ((char const**)args)[0];
    result.second = ((char const**)args)[1];
    return result;
}

void* set_str_pair_args(str_const str_1, str_const str_2) {
    void* args = malloc(sizeof(char*) * 2);
    ((char const**)args)[0] = strdup(str_1);
    ((char const**)args)[1] = strdup(str_2);
    return args;
}

ipa_queue_entry_t make_function_queue_entry(ipa_callback_t func, void* data, size_t data_size, void* args, size_t arg_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_FUNCTION;
    entry.args = args;
    entry.args_size = arg_size;
    entry.func = func;
    entry.result = data;
    entry.result_size = data_size;
    return entry;
}

ipa_queue_entry_t make_download_str_queue_entry_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_STR;
    entry.url = strdup(url);
    entry.func = callback;
    entry.args = args;
    entry.args_size = args_size;
    return entry;
}

ipa_queue_entry_t make_download_data_queue_entry_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_DATA;
    entry.url = strdup(url);
    entry.func = callback;
    entry.args = args;
    entry.args_size = args_size;
    return entry;
}

ipa_queue_entry_t make_download_file_queue_entry_callback(char const* url, char const* path, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_FILE;
    entry.url = strdup(url);
    entry.download_args = strdup(path);
    entry.func = callback;
    entry.args = args;
    entry.args_size = args_size;
    return entry;
}

ipa_queue_entry_t make_post_download_str_queue_entry_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_STR;
    entry.url = strdup(url);
    entry.download_args = strdup(post_data);
    entry.func = callback;
    entry.args = args;
    entry.args_size = args_size;
    return entry;
}

ipa_queue_entry_t make_post_download_data_queue_entry_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_DATA;
    entry.url = strdup(url);
    entry.download_args = strdup(post_data);
    entry.func = callback;
    entry.args = args;
    entry.args_size = args_size;
    return entry;
}

ipa_queue_entry_t make_post_download_file_queue_entry_callback(char const* url, char const* post_data, char const* path, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_FILE;
    entry.url = strdup(url);

    entry.download_args = set_str_pair_args(post_data, path);

    entry.func = callback;
    entry.args = args;
    entry.args_size = args_size;
    return entry;
}

ssize_t prepend_to_work_queue(ipa_queue_entry_t entry, ssize_t forced_id) {
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    if (forced_id != -1) {
        entry.id = forced_id;
    } else {
        entry.id = next_id++;
    }

    vector$ipa_queue_entry_t$_insert_at(&work_queue, 0, entry);
    if (!work_queue_thread_running) {
        work_queue_thread_running = true;
        pthread_create(&work_queue_thread, nullptr, work_queue_loop, nullptr);
    }
    pthread_mutex_unlock(&work_queue_mutex);
    return entry.id;
}

ssize_t add_to_work_queue(ipa_queue_entry_t entry) {
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    entry.id = next_id++;
    vector$ipa_queue_entry_t$_push_back(&work_queue, entry);
    if (!work_queue_thread_running) {
        work_queue_thread_running = true;
        pthread_create(&work_queue_thread, nullptr, work_queue_loop, nullptr);
    }
    pthread_mutex_unlock(&work_queue_mutex);
    return entry.id;
}

ssize_t add_to_work_queue_with_id(ipa_queue_entry_t entry, ssize_t forced_id) {
    if (forced_id == -1) {
        return add_to_work_queue(entry);
    }
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    entry.id = forced_id;
    vector$ipa_queue_entry_t$_push_back(&work_queue, entry);
    if (!work_queue_thread_running) {
        work_queue_thread_running = true;
        pthread_create(&work_queue_thread, nullptr, work_queue_loop, nullptr);
    }
    pthread_mutex_unlock(&work_queue_mutex);
    return entry.id;
}

ssize_t add_function_to_work_queue(ipa_callback_t func, void* data, size_t data_size, void* args, size_t arg_size) {
    ipa_queue_entry_t entry = make_function_queue_entry(func, data, data_size, args, arg_size);
    return add_to_work_queue(entry);
}

ssize_t add_download_str_to_work_queue(char const* url) {
    return add_download_str_to_work_queue_callback(url, nullptr, nullptr, 0);
}

ssize_t add_download_data_to_work_queue(char const* url) {
    return add_download_data_to_work_queue_callback(url, nullptr, nullptr, 0);
}

ssize_t add_download_file_to_work_queue(char const* url, char const* path) {
    return add_download_file_to_work_queue_callback(url, path, nullptr, nullptr, 0);
}

ssize_t add_post_download_str_to_work_queue(char const* url, char const* post_data) {
    return add_post_download_str_to_work_queue_callback(url, post_data, nullptr, nullptr, 0);
}

ssize_t add_post_download_data_to_work_queue(char const* url, char const* post_data) {
    return add_post_download_data_to_work_queue_callback(url, post_data, nullptr, nullptr, 0);
}

ssize_t add_post_download_file_to_work_queue(char const* url, char const* post_data, char const* path) {
    return add_post_download_file_to_work_queue_callback(url, post_data, path, nullptr, nullptr, 0);
}

ssize_t add_download_str_to_work_queue_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = make_download_str_queue_entry_callback(url, callback, args, args_size);
    return add_to_work_queue(entry);
}

ssize_t add_download_data_to_work_queue_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = make_download_data_queue_entry_callback(url, callback, args, args_size);
    return add_to_work_queue(entry);
}

ssize_t add_download_file_to_work_queue_callback(char const* url, char const* path, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = make_download_file_queue_entry_callback(url, path, callback, args, args_size);
    return add_to_work_queue(entry);
}

ssize_t add_post_download_str_to_work_queue_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = make_post_download_str_queue_entry_callback(url, post_data, callback, args, args_size);
    return add_to_work_queue(entry);
}

ssize_t add_post_download_data_to_work_queue_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = make_post_download_data_queue_entry_callback(url, post_data, callback, args, args_size);
    return add_to_work_queue(entry);
}

ssize_t add_post_download_file_to_work_queue_callback(char const* url, char const* post_data, char const* path, ipa_callback_t callback, void* args, size_t args_size) {
    ipa_queue_entry_t entry = make_post_download_file_queue_entry_callback(url, post_data, path, callback, args, args_size);
    return add_to_work_queue(entry);
}

size_t get_work_queue_size() {
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    size_t size = work_queue.count;
    pthread_mutex_unlock(&work_queue_mutex);
    return size;
}

size_t get_finished_queue_size() {
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    size_t size = finished_queue.count;
    pthread_mutex_unlock(&work_queue_mutex);
    return size;
}

optional$ipa_queue_entry_t$ get_from_finished_queue() {
    optional$ipa_queue_entry_t$ result = {0};
    if (!pthread_mutex_lock(&work_queue_mutex))
        return result;
    if (finished_queue.count == 0) {
        pthread_mutex_unlock(&work_queue_mutex);
        return result;
    }
    result.value = vector$ipa_queue_entry_t$_pop(&finished_queue);
    result.is_valid = true;

    pthread_mutex_unlock(&work_queue_mutex);
    return result;
}

void* work_queue_loop(void* arg) {
    while (work_queue_thread_running) {
        if (!pthread_mutex_lock(&work_queue_mutex)) {
            efuncprintf("Mutex unlock returned error!\n");
            break;
        }
        if (work_queue.count == 0) {
            work_queue_thread_running = false;
            pthread_mutex_unlock(&work_queue_mutex);
            break;
        }
        ipa_queue_entry_t entry = vector$ipa_queue_entry_t$_pop(&work_queue);
        pthread_mutex_unlock(&work_queue_mutex);

        switch (entry.type) {
            case IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_STR: {
                pair$alloc_str$long$ result = http_get_string(entry.url);
                entry.status = result.second;
                entry.result = result.first;
                break;
            }
            case IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_DATA: {
                pair$alloc_void$long$ result = http_get_data(entry.url);
                entry.status = result.second;
                entry.result = result.first;
                break;
            }
            case IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_FILE: {
                char const* path = entry.args;
                long status = http_get_file(entry.url, path);
                entry.status = status;
                entry.result = strdup(path);
                break;
            }
            case IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_STR: {
                pair$alloc_str$long$ result = http_post_get_string(entry.url, entry.args);
                entry.status = result.second;
                entry.result = result.first;
                break;
            }
            case IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_DATA: {
                pair$alloc_void$long$ result = http_post_get_data(entry.url, entry.args);
                entry.status = result.second;
                entry.result = result.first;
                break;
            }
            case IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_FILE: {
                pair$str_const$str_const$ args = get_str_pair_args(entry.args);
                char const* post_data = args.first;
                char const* path = args.second;
                long status = http_post_get_file(entry.url, post_data, path);
                entry.status = status;
                entry.result = strdup(path);
                break;
            }
            case IPA_QUEUE_ENTRY_TYPE_FUNCTION:
                triple$bool$alloc_void$size_t$ result = entry.func(entry);
                entry.result = result.second;
                entry.result_size = result.third;
                break;
        }

        if (entry.type == IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_STR || entry.type == IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_STR) {
            entry.result_size = strlen(entry.result) + 1;
        } else if (entry.type != IPA_QUEUE_ENTRY_TYPE_FUNCTION) {
            long len;
            CURLcode c = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &len); //really shouldn't be able to fail but fine
            if (c != CURLE_OK) {
                efuncprintf("Failed to get content length\n");
                free(entry.result);
                entry.result = nullptr;
                entry.result_size = 0;
            } else {
                entry.result_size = len;
            }
        }

        bool add_to_finished_queue = true;
        if (entry.type != IPA_QUEUE_ENTRY_TYPE_FUNCTION && entry.func) {
            triple$bool$alloc_void$size_t$ result = entry.func(entry);
            free(entry.result);
            entry.result = result.second;
            entry.result_size = result.third;
            if (!result.first) {
                add_to_finished_queue = false;
                destroy_queue_entry(&entry);
            }
        }

        if (add_to_finished_queue) {
            if (!pthread_mutex_lock(&work_queue_mutex)) {
                efuncprintf("Mutex unlock returned error!\n");
                break;
            }
            vector$ipa_queue_entry_t$_push_back(&finished_queue, entry);
            pthread_mutex_unlock(&work_queue_mutex);
        }
    }

    return nullptr;
}
