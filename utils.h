#ifndef MATRIX_CLIENT_UTILS_H
#define MATRIX_CLIENT_UTILS_H

#include <curl/curl.h>

#define CACERT_PATH "sdmc:/config/ssl/cacert.pem" //stolen from universal updater, which means they will probably usually keep it updated for me

extern CURL* curlhandle;

#define eprintf(args...) fprintf(stderr, args)
#define efuncprintf(format, ...) fprintf(stderr, "%s: " format, __func__, ##__VA_ARGS__)

int init_curl();
void destroy_curl();

int make_dirs(char const* path);

int download_file(char const* url, char const* path);
int update_root_ca();

#endif //MATRIX_CLIENT_UTILS_H