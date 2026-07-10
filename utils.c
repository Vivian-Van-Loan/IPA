#include "utils.h"

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <3ds.h>
#include <malloc.h>

CURL* curl_handle = NULL;
struct curl_slist* curl_headers = nullptr;
void* socubuf;
bool verify_tls = true;

int init_general() {
    romfsInit();
    return make_dirs(SAVE_DIR);
}

int init_curl() {
    socubuf = memalign(0x1000, 0x100000);
    if (!socubuf) {
        efuncprintf("memalign() failed\n");
        return -1;
    }

    int socRes = socInit(socubuf, 0x100000);
    if (R_FAILED(socRes)) {
        efuncprintf("socInit() failed\n");
        return socRes;
    }

    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK) {
        efuncprintf("curl_global_init() failed: %s\n", curl_easy_strerror(res));
        return res;
    }

    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L); //todo: we should check return values on these
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl_handle, CURLOPT_STDERR, stdout);

    if (access(CACERT_PATH, F_OK) == 0 || update_root_ca() == 0) {
        verify_tls = true;
        res = curl_easy_setopt(curl_handle, CURLOPT_CAINFO, CACERT_PATH);
        if (res) {
            efuncprintf("Failed to set root ca file for reason: %s\n", curl_easy_strerror(res));
            destroy_curl();
            return res;
        }
    } else {
        // verify_tls = false; //just a really really bad idea
        destroy_curl();
        return -2;
    }
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, verify_tls);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, stdout);

    return 0;
}

void destroy_curl() {
    curl_slist_free_all(curl_headers);
    curl_easy_cleanup(curl_handle);
    socExit();
    free(socubuf);
}

int curl_add_header(char const* header) {
    if (!header) {
        return -1;
    }
    struct curl_slist* new = curl_slist_append(curl_headers, header);
    curl_headers = new;
    CURLcode res = curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, curl_headers);
    if (res) {
        efuncprintf("Failed to add header: %s for reason: %s\n", header, curl_easy_strerror(res));
        return res;
    }
    return 0;
}

int make_dirs(char const* path) { //WILL NOT CREATE THE LAST PART AS DIR UNLESS IT ENDS IN /
    char buf[PATH_MAX];
    strncpy(buf, path, sizeof(buf));
    buf[PATH_MAX - 1] = '\0';

    char* ptr = strchr(buf, '/');
    while (ptr) {
        *ptr = '\0';
        // printf("%s\n", buf);
        int res = mkdir(buf, S_IRWXU | S_IRWXG | S_IRWXO);
        if (res && errno != EEXIST) {
            res = errno;
            efuncprintf("Failed to make dir: %s for reason: %s\n", buf, strerror(res));
            return res;
        }
        *ptr = '/';
        ptr++;
        ptr = strchr(ptr, '/');
    }
    return 0;
}

size_t next_power_of_two(size_t x) {
    size_t y = 1;
    while (y < x) {
        y <<= 1;
    }
    return y;
}

int download_file(char const* url, char const* path) {
    int res = make_dirs(path);
    if (res)
        return res;

    bool remove = true;
    FILE* file_ptr = fopen(path, "wb");
    if (!file_ptr) {
        res = errno;
        efuncprintf("Failed to open: %s for reason: %s\n", path, strerror(res));
        return res;
    }

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, nullptr); //reset to default (ie. write to provided file below)
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, file_ptr); //these two are literally unable to fail

    res = curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, verify_tls);
    if (res) {
        efuncprintf("Failed to change curl SSL verify state for reason: %s\n", curl_easy_strerror(res));
        goto exit;
    }

    res = curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    if (res) {
        efuncprintf("Failed to change curl download URL to: %s for reason: %s\n", url, curl_easy_strerror(res));
        goto exit;
    }

    res = curl_easy_perform(curl_handle);
    if (res) {
        efuncprintf("Failed to fully download file for reason: %s\n", curl_easy_strerror(res));
        goto exit;
    }
    long response_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200) {
        efuncprintf("Failed to download file, response code: %ld\n", response_code);
        goto exit;
    }

    remove = false;

exit:
    fclose(file_ptr);
    if (remove) {
        unlink(path);
    }
    return res;
}

int update_root_ca() {
    verify_tls = false;
    int res = download_file("https://curl.se/ca/cacert.pem", CACERT_PATH);
    if (res) {
        //todo: idk warn the user and ask if they want to try again or something
    }
    return res;
}

size_t write_response(void* ptr, size_t size, size_t nmemb, void* stream) {
    curl_write_result_t* result = stream;

    if (result->pos + size * nmemb >= result->max_size - 1) {
        efuncprintf("error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

long download(char const* url) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    CURLcode status = curl_easy_perform(curl_handle);
    long response_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (status != CURLE_OK) {
        efuncprintf("curl_easy_perform() failed, response code: %ld, reason: %s\n", response_code, curl_easy_strerror(status));
        return -((long) status);
    }
    if (response_code == 400) {
        efuncprintf("Failed to download page [%s], response code of 400\n", url);
    }
    // if (response_code != 200) {
    //     efuncprintf("Failed to download page, response code: %ld\n", response_code);
    //     return -1;
    // }
    return response_code;
}

pair$alloc_void$long$ download_to_data(char const* url) {
    pair$alloc_void$long$ ret = {nullptr, -1};

    // printf("URL: %s\n", url);
    curl_write_result_t result = {0};
    result.data = malloc(DOWNLOAD_BUFFER_SIZE);
    result.max_size = DOWNLOAD_BUFFER_SIZE;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &result);
    long status = download(url);
    if (status < 0)
        goto error;

    ret.first = result.data;
    ret.second = status;
    return ret;

    error:
        free(result.data);
    return ret;
}

pair$alloc_str$long$ download_to_string(char const* url) {
    pair$alloc_str$long$ ret = {nullptr, -1};

    // printf("URL: %s\n", url);
    curl_write_result_t result = {0};
    result.data = malloc(DOWNLOAD_BUFFER_SIZE);
    result.max_size = DOWNLOAD_BUFFER_SIZE;

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &result);
    long status = download(url);
    if (status < 0)
        goto error;

    result.data[result.pos] = '\0';
    size_t len = strlen(result.data);
    char* new = realloc(result.data, len + 1);
    if (!new) {
        efuncprintf("Failed to allocate memory for string\n");
        goto error;
    }
    ret.first = result.data;
    ret.second = status;
    return ret;

error:
    free(result.data);
    return ret;
}

pair$alloc_str$long$ http_get_string(char const* url) {
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
    return download_to_string(url);
}

pair$alloc_void$long$ http_get_data(char const* url) {
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
    return download_to_data(url);
}

long get_file(char const* url, char const* path) {
    make_dirs(path);
    FILE* f = fopen(path, "wb");
    if (!f) {
        efuncprintf("Failed to open file: %s", path);
        return -1;
    }
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, f);
    long status = download(url);
    fclose(f);
    if (status < 0) {
        unlink(path);
        return status;
    }
    return status;
}

long http_get_file(char const* url, char const* path) {
    curl_easy_setopt(curl_handle, CURLOPT_HTTPGET, 1);
    return get_file(url, path);
}

pair$alloc_str$long$ post_json_string(char const* url, char const* json) {
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json);
    return download_to_string(url);
}

long post_json_file(char const* url, char const* json, char const* path) {
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json);
    return get_file(url, path);
}
