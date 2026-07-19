#ifndef MATRIX_CLIENT_UTILS_H
#define MATRIX_CLIENT_UTILS_H

#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <curl/curl.h>

#include "types/general_types.h"

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

#define TEMPLATE_TYPE_K alloc_str
#define TEMPLATE_TYPE_V long
#include "types/pair.h"

#define TEMPLATE_TYPE_K alloc_void
#define TEMPLATE_TYPE_V long
#include "types/pair.h"

int init_general();
int init_curl();
void destroy_curl();

int curl_destroy_headers();
int curl_add_header(char const* header);

int make_dirs(char const* path);

size_t next_power_of_two(size_t x);

int download_file(char const* url, char const* path);
int update_root_ca();

size_t write_response(void* ptr, size_t size, size_t nmemb, void* stream); //used as a callback for curl easy function
pair$alloc_str$long$ http_get_string(char const* url);
pair$alloc_void$long$ http_get_data(char const* url);
long http_get_file(char const* url, char const* path);
pair$alloc_str$long$ post_json_string(char const* url, char const* json);
long post_json_file(char const* url, char const* json, char const* path);

off_t get_file_size(char const* path);
off_t get_file_size_fd(int fd);

#endif //MATRIX_CLIENT_UTILS_H