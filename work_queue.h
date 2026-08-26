#ifndef MATRIX_CLIENT_WORK_QUEUE_H
#define MATRIX_CLIENT_WORK_QUEUE_H

#include <stddef.h>
#include <unistd.h>

typedef enum ipa_queue_entry_type_t {
    IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_STR,
    IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_DATA,
    IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_FILE,
    IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_STR,
    IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_DATA,
    IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_FILE,
    IPA_QUEUE_ENTRY_TYPE_FUNCTION,
} ipa_queue_entry_type_t;

typedef struct ipa_queue_entry_t {
    ipa_queue_entry_type_t type;
    void* (*func)(void*);
    char const* url;
    void* args;

    size_t result_size; //this is sometimes filled by work thread, filled by user if the type is function

    //these are filled by the work thread
    ssize_t id;
    bool success;
    void* result;
    long status;
} ipa_queue_entry_t;

#define TEMPLATE_TYPE ipa_queue_entry_t
#include "types/vector.h"

#define TEMPLATE_TYPE ipa_queue_entry_t
#include "types/optional.h"

void destroy_queue_entry(ipa_queue_entry_t* entry);

int init_work_queue();
int destroy_work_queue();

ssize_t add_to_work_queue(ipa_queue_entry_t entry); //these all return the entry ID
ssize_t add_download_str_to_work_queue(char const* url);
ssize_t add_download_data_to_work_queue(char const* url);
ssize_t add_download_file_to_work_queue(char const* url, char const* path);
ssize_t add_post_download_str_to_work_queue(char const* url, char const* post_data);
ssize_t add_post_download_data_to_work_queue(char const* url, char const* post_data);
ssize_t add_post_download_file_to_work_queue(char const* url, char const* post_data, char const* path);
ssize_t add_download_str_to_work_queue_callback(char const* url, void* (*callback)(void*), size_t result_size);
ssize_t add_download_data_to_work_queue_callback(char const* url, void* (*callback)(void*), size_t result_size);
ssize_t add_download_file_to_work_queue_callback(char const* url, char const* path, void* (*callback)(void*), size_t result_size);
ssize_t add_post_download_str_to_work_queue_callback(char const* url, char const* post_data, void* (*callback)(void*), size_t result_size);
ssize_t add_post_download_data_to_work_queue_callback(char const* url, char const* post_data, void* (*callback)(void*), size_t result_size);
ssize_t add_post_download_file_to_work_queue_callback(char const* url, char const* post_data, char const* path, void* (*callback)(void*), size_t result_size);

size_t get_finished_queue_size();
optional$ipa_queue_entry_t$ get_from_work_queue();

#endif //MATRIX_CLIENT_WORK_QUEUE_H
