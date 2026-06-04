#ifndef MATRIX_CLIENT_UTILS_H
#define MATRIX_CLIENT_UTILS_H

#include <stdint.h>

#include <curl/curl.h>

#define DOWNLOAD_BUFFER_SIZE (64 * 1024) //64kB
#define CACERT_PATH "sdmc:/config/ssl/cacert.pem" //stolen from universal updater, which means they will probably usually keep it updated for me
#define SAVE_DIR "sdmc:/3ds/IPA"

extern CURL* curl_handle;
extern struct curl_slist* curl_headers;

#define eprintf(args...) fprintf(stderr, args)
#define efuncprintf(format, ...) fprintf(stderr, "%s: " format, __func__, ##__VA_ARGS__)

typedef struct curl_write_result_t {
    char* data;
    size_t pos;
    size_t max_size;
} curl_write_result_t;

int init_general();
int init_curl();
void destroy_curl();

int curl_add_header(char const* header);

int make_dirs(char const* path);

int download_file(char const* url, char const* path);
int update_root_ca();

size_t write_response(void* ptr, size_t size, size_t nmemb, void* stream); //used as a callback for curl easy function
char* http_get_string(char const* url);
char* post_json_string(char const* url, char const* json);

#endif //MATRIX_CLIENT_UTILS_H