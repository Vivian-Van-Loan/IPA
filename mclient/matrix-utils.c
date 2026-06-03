#include "matrix-utils.h"

#include <curl/curl.h>
#include <jansson.h>
#include <string.h>

#include "../utils.h"

char* resolve_url(char const* base_url) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s/.well-known/matrix/client", base_url);
    char* json_txt = download_to_string(buf);
    if (!json_txt)
        return nullptr;

    char* resolved = nullptr;
    json_error_t error;
    json_t* root = json_loads(json_txt, 0, &error);
    free(json_txt);

    if (!root)
        return resolved;
    json_t* url = json_object_get(root, "m.homeserver");
    if (!url) {
        goto exit;
    }
    resolved = strdup(json_string_value(url));

    exit:
    json_decref(root);
    return resolved;
}
