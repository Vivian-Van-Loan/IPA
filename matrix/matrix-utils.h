#ifndef MATRIX_CLIENT_MATRIX_UTILS_H
#define MATRIX_CLIENT_MATRIX_UTILS_H

#include "matrix-client.h"
#include "../utils.h"
#include "../work_queue.h"

#define TEMPLATE_TYPE_K alloc_void
#define TEMPLATE_TYPE_V size_t
#include "../types/pair.h"

typedef matrix_login_t* matrix_login_t_p;
#define TEMPLATE_TYPE_K matrix_login_t_p
#define TEMPLATE_TYPE_V bool
#include "../types/pair.h"

//do NOT call any of these outside the work queue thread, it uses the curl thread
char* matrix_resolve_homeserver(char const* homeserver_base);

// char* matrix_get_string(matrix_login_t* login, char const* url);
// pair$alloc_void$size_t$ matrix_get_data(matrix_login_t* login, char const* url);
// int matrix_get_file(matrix_login_t* login, char const* url, char const* path);
// char* matrix_post_json_string(matrix_login_t* login, char const* url, char const* json);
// int matrix_post_json_file(matrix_login_t* login, char const* url, char const* json, char const* path);

// triple$bool$alloc_void$size_t$ matrix_process_string(ipa_queue_entry_t entry);
triple$bool$alloc_void$size_t$ matrix_process_data(ipa_queue_entry_t entry);
triple$bool$alloc_void$size_t$ matrix_process_file(ipa_queue_entry_t entry);
triple$bool$alloc_void$size_t$ matrix_process_post_data(ipa_queue_entry_t entry);
triple$bool$alloc_void$size_t$ matrix_process_post_file(ipa_queue_entry_t entry);

int matrix_get_mxc_url(matrix_client_t* client, char const* mxc_url, char out_url[URL_BUFFER_SIZE]);
// pair$alloc_void$size_t$ matrix_download_mxc(matrix_client_t* client, char const* mxc_url);

#endif //MATRIX_CLIENT_MATRIX_UTILS_H