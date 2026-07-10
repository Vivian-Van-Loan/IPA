#ifndef MATRIX_CLIENT_MATRIX_UTILS_H
#define MATRIX_CLIENT_MATRIX_UTILS_H

#include "matrix-client.h"
#include "../utils.h"

#define TEMPLATE_TYPE_K alloc_void
#define TEMPLATE_TYPE_V size_t
#include "../types/pair.h"

char* matrix_resolve_homeserver(char const* homeserver_base);

char* matrix_get_string(matrix_login_t* login, char const* url);
pair$alloc_void$size_t$ matrix_get_data(matrix_login_t* login, char const* url);
int matrix_get_file(matrix_login_t* login, char const* url, char const* path);
char* matrix_post_json_string(matrix_login_t* login, char const* url, char const* json);
int matrix_post_json_file(matrix_login_t* login, char const* url, char const* json, char const* path);

pair$alloc_void$size_t$ matrix_download_mxc(matrix_client_t* client, char const* mxc_url);

#endif //MATRIX_CLIENT_MATRIX_UTILS_H