#ifndef MATRIX_CLIENT_UTILS_H
#define MATRIX_CLIENT_UTILS_H

#include <stdint.h>

#include <curl/curl.h>

#define DOWNLOAD_BUFFER_SIZE (2048 * 1024) //2048kB
#define URL_BUFFER_SIZE (2048)
#define CACERT_PATH "sdmc:/config/ssl/cacert.pem" //stolen from universal updater, which means they will probably usually keep it updated for me
#define SAVE_DIR "sdmc:/3ds/IPA/"
#define CACHE_DIR SAVE_DIR"cache/"
#define AVATAR_CACHE_DIR CACHE_DIR"ava/"
#define IMG_CACHE_DIR CACHE_DIR"img/"

extern CURL* curl_handle;
extern struct curl_slist* curl_headers;

#define eprintf(args...) fprintf(stderr, args)
#define efuncprintf(format, ...) fprintf(stderr, "%s: " format, __func__, ##__VA_ARGS__)
#define lengthof(arr) (sizeof(arr) / sizeof(arr[0]))
#define max(a,b) \
    ({ typeof (a) _a = (a); \
    typeof (b) _b = (b); \
    _a > _b ? _a : _b; })
#define min(a,b) \
    ({ typeof (a) _a = (a); \
    typeof (b) _b = (b); \
    _a < _b ? _a : _b; })
#define struct_var_size(st, mem) sizeof(((st*)nullptr)->mem)


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

size_t next_power_of_two(size_t x);

int download_file(char const* url, char const* path);
int update_root_ca();

size_t write_response(void* ptr, size_t size, size_t nmemb, void* stream); //used as a callback for curl easy function
char* http_get_string(char const* url);
void* http_get_data(char const* url);
int http_get_file(char const* url, char const* path);
char* post_json_string(char const* url, char const* json);
int post_json_file(char const* url, char const* json, char const* path);

#endif //MATRIX_CLIENT_UTILS_H