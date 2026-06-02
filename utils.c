#include "utils.h"

#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <3ds.h>
#include <malloc.h>

CURL* curlhandle = NULL;
void* socubuf;
bool verify_tls = true;

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

    curlhandle = curl_easy_init();

    curl_easy_setopt(curlhandle, CURLOPT_FOLLOWLOCATION, 1L); //todo: we should check return values on these
    curl_easy_setopt(curlhandle, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curlhandle, CURLOPT_ACCEPT_ENCODING, "gzip");
    curl_easy_setopt(curlhandle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curlhandle, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curlhandle, CURLOPT_STDERR, stdout);

    if (access(CACERT_PATH, F_OK) == 0 || update_root_ca() == 0) {
        verify_tls = true;
        res = curl_easy_setopt(curlhandle, CURLOPT_CAINFO, CACERT_PATH);
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
    curl_easy_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, verify_tls);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, nullptr);
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, stdout);

    return 0;
}

int make_dirs(char const* path) {
    char buf[PATH_MAX];
    strncpy(buf, path, sizeof(buf));
    buf[PATH_MAX - 1] = '\0';

    char* ptr = strchr(buf, '/');
    while (ptr) {
        *ptr = '\0';
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

void destroy_curl() {
    curl_easy_cleanup(curlhandle);
    socExit();
    free(socubuf);
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

    curl_easy_setopt(curlhandle, CURLOPT_WRITEFUNCTION, nullptr); //reset to default (ie. write to provided file below)
    curl_easy_setopt(curlhandle, CURLOPT_WRITEDATA, file_ptr); //these two are literally unable to fail

    res = curl_easy_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, verify_tls);
    if (res) {
        efuncprintf("Failed to change curl SSL verify state for reason: %s\n", curl_easy_strerror(res));
        goto exit;
    }

    res = curl_easy_setopt(curlhandle, CURLOPT_URL, url);
    if (res) {
        efuncprintf("Failed to change curl download URL to: %s for reason: %s\n", url, curl_easy_strerror(res));
        goto exit;
    }

    res = curl_easy_perform(curlhandle);
    if (res) {
        efuncprintf("Failed to fully download file for reason: %s\n", curl_easy_strerror(res));
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
