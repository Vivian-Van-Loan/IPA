#include "work_queue.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

static vector$ipa_queue_entry_t$ work_queue;
static vector$ipa_queue_entry_t$ finished_queue;
static ssize_t next_id = 0;

static pthread_t work_queue_thread;
static bool work_queue_thread_running = false;
static pthread_mutex_t work_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void destroy_queue_entry(ipa_queue_entry_t* entry) {
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
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    if (work_queue_thread_running) {
        pthread_mutex_unlock(&work_queue_mutex);
        return -1;
    }

    vector$ipa_queue_entry_t$_free_callback(&work_queue, destroy_queue_entry);
    vector$ipa_queue_entry_t$_free_callback(&finished_queue, destroy_queue_entry);
    next_id = 0;

    pthread_mutex_unlock(&work_queue_mutex);
    return 0;
}

ssize_t add_to_work_queue(ipa_queue_entry_t entry) {
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    entry.id = next_id++;
    vector$ipa_queue_entry_t$_push_back(&work_queue, entry);
    pthread_mutex_unlock(&work_queue_mutex);
    return next_id;
}

ssize_t add_download_str_to_work_queue(char const* url) {
    return add_download_str_to_work_queue_callback(url, nullptr, 0);
}

ssize_t add_download_data_to_work_queue(char const* url) {
    return add_download_data_to_work_queue_callback(url, nullptr, 0);
}

ssize_t add_download_file_to_work_queue(char const* url, char const* path) {
    return add_download_file_to_work_queue_callback(url, path, nullptr, 0);
}

ssize_t add_post_download_str_to_work_queue(char const* url, char const* post_data) {
    return add_post_download_str_to_work_queue_callback(url, post_data, nullptr, 0);
}

ssize_t add_post_download_data_to_work_queue(char const* url, char const* post_data) {
    return add_post_download_data_to_work_queue_callback(url, post_data, nullptr, 0);
}

ssize_t add_post_download_file_to_work_queue(char const* url, char const* post_data, char const* path) {
    return add_post_download_file_to_work_queue_callback(url, post_data, path, nullptr, 0);
}

ssize_t add_download_str_to_work_queue_callback(char const* url, void* (*callback)(void*), size_t result_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_STR;
    entry.url = strdup(url);
    entry.func = callback;
    entry.result_size = result_size;
    return add_to_work_queue(entry);
}

ssize_t add_download_data_to_work_queue_callback(char const* url, void* (*callback)(void*), size_t result_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_DATA;
    entry.url = strdup(url);
    entry.func = callback;
    entry.result_size = result_size;
    return add_to_work_queue(entry);
}

ssize_t add_download_file_to_work_queue_callback(char const* url, char const* path, void* (*callback)(void*), size_t result_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_FILE;
    entry.url = strdup(url);
    entry.args = strdup(path);
    entry.func = callback;
    entry.result_size = result_size;
    return add_to_work_queue(entry);
}

ssize_t add_post_download_str_to_work_queue_callback(char const* url, char const* post_data, void* (*callback)(void*), size_t result_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_STR;
    entry.url = strdup(url);
    entry.args = strdup(post_data);
    entry.func = callback;
    entry.result_size = result_size;
    return add_to_work_queue(entry);
}

ssize_t add_post_download_data_to_work_queue_callback(char const* url, char const* post_data, void* (*callback)(void*), size_t result_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_DATA;
    entry.url = strdup(url);
    entry.args = strdup(post_data);
    entry.func = callback;
    entry.result_size = result_size;
    return add_to_work_queue(entry);
}

ssize_t add_post_download_file_to_work_queue_callback(char const* url, char const* post_data, char const* path, void* (*callback)(void*), size_t result_size) {
    ipa_queue_entry_t entry = {0};
    entry.type = IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_FILE;
    entry.url = strdup(url);

    entry.args = malloc(sizeof(char*) * 2);
    ((char**)entry.args)[0] = strdup(post_data);
    ((char**)entry.args)[1] = strdup(path);

    entry.result_size = result_size;
    return add_to_work_queue(entry);
}

size_t get_finished_queue_size() {
    if (!pthread_mutex_lock(&work_queue_mutex))
        return -1;
    return finished_queue.count;
    pthread_mutex_unlock(&work_queue_mutex);
}

optional$ipa_queue_entry_t$ get_from_work_queue() {
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
