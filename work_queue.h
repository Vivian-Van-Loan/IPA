#ifndef MATRIX_CLIENT_WORK_QUEUE_H
#define MATRIX_CLIENT_WORK_QUEUE_H

#include <stddef.h>
#include <unistd.h>

#include "matrix/login.h"
#include "types/general_types.h"

typedef enum ipa_queue_entry_type_t {
    IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_STR,
    IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_DATA,
    IPA_QUEUE_ENTRY_TYPE_DOWNLOAD_FILE,
    IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_STR,
    IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_DATA,
    IPA_QUEUE_ENTRY_TYPE_POST_DOWNLOAD_FILE,
    IPA_QUEUE_ENTRY_TYPE_FUNCTION,
} ipa_queue_entry_type_t;

struct ipa_queue_entry_t;

//return false to signify that the entry should not be moved to the finished queue, useful for requeuing events
//the results will be freed automatically if so, or passed if needed
typedef triple$bool$alloc_void$size_t$ (*ipa_callback_t)(struct ipa_queue_entry_t entry);

typedef struct ipa_queue_entry_t {
    ipa_queue_entry_type_t type;
    ipa_callback_t func;
    char const* url; //this struct owns the pointer types it has (excepting func) and it will free them on destruction
    void* args;
    size_t args_size;

    //these are filled by the work thread
    ssize_t id;
    void* download_args;
    long status;

    //these will be filled by the work thread UNLESS you are setting up a function call, in which case they are free to use
    void* result;
    size_t result_size;
} ipa_queue_entry_t;

struct ipa_process_args_t {
    matrix_login_t* login;
    bool retry;
    ipa_callback_t next_callback;
    void* next_args;
    size_t next_args_size;
};

#define TEMPLATE_TYPE ipa_queue_entry_t
#include "types/vector.h"

#define TEMPLATE_TYPE ipa_queue_entry_t
#include "types/optional.h"

struct ipa_process_args_t* ipa_process_args_new(matrix_login_t* login, bool retry, ipa_callback_t next_callback, void* next_args, size_t next_args_size);

void destroy_queue_entry(ipa_queue_entry_t* entry);

int init_work_queue();
int destroy_work_queue();

pair$str_const$str_const$ get_str_pair_args(void* args); //does not duplicate the returned strings
void* set_str_pair_args(str_const str_1, str_const str_2); //duplicates the passed strings, owning them

ipa_queue_entry_t make_function_queue_entry(ipa_callback_t func, void* data, size_t data_size, void* args, size_t arg_size);
ipa_queue_entry_t make_download_str_queue_entry_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size);
ipa_queue_entry_t make_download_data_queue_entry_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size);
ipa_queue_entry_t make_download_file_queue_entry_callback(char const* url, char const* path, ipa_callback_t callback, void* args, size_t args_size);
ipa_queue_entry_t make_post_download_str_queue_entry_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size);
ipa_queue_entry_t make_post_download_data_queue_entry_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size);
ipa_queue_entry_t make_post_download_file_queue_entry_callback(char const* url, char const* post_data, char const* path, ipa_callback_t callback, void* args, size_t args_size);

ssize_t prepend_to_work_queue(ipa_queue_entry_t entry, ssize_t forced_id);
ssize_t add_to_work_queue(ipa_queue_entry_t entry); //these all return the entry ID
ssize_t add_to_work_queue_with_id(ipa_queue_entry_t entry, ssize_t forced_id);
ssize_t add_function_to_work_queue(ipa_callback_t func, void* data, size_t data_size, void* args, size_t arg_size);
ssize_t add_download_str_to_work_queue(char const* url);
ssize_t add_download_data_to_work_queue(char const* url);
ssize_t add_download_file_to_work_queue(char const* url, char const* path);
ssize_t add_post_download_str_to_work_queue(char const* url, char const* post_data);
ssize_t add_post_download_data_to_work_queue(char const* url, char const* post_data);
ssize_t add_post_download_file_to_work_queue(char const* url, char const* post_data, char const* path);
ssize_t add_download_str_to_work_queue_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size);
ssize_t add_download_data_to_work_queue_callback(char const* url, ipa_callback_t callback, void* args, size_t args_size);
ssize_t add_download_file_to_work_queue_callback(char const* url, char const* path, ipa_callback_t callback, void* args, size_t args_size);
ssize_t add_post_download_str_to_work_queue_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size);
ssize_t add_post_download_data_to_work_queue_callback(char const* url, char const* post_data, ipa_callback_t callback, void* args, size_t args_size);
ssize_t add_post_download_file_to_work_queue_callback(char const* url, char const* post_data, char const* path, ipa_callback_t callback, void* args, size_t args_size);

size_t get_work_queue_size();
size_t get_finished_queue_size();
optional$ipa_queue_entry_t$ get_from_finished_queue();

#endif //MATRIX_CLIENT_WORK_QUEUE_H
