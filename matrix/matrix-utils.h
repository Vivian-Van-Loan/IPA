#ifndef MATRIX_CLIENT_MATRIX_UTILS_H
#define MATRIX_CLIENT_MATRIX_UTILS_H
#include "matrix-client.h"

char* matrix_resolve_homeserver(char const* homeserver_base);
char* matrix_download_mxc(matrix_client_t const* client, char const* mxc_url);

#endif //MATRIX_CLIENT_MATRIX_UTILS_H