#include "matrix-utils.h"

#include <curl/curl.h>
#include <jansson.h>
#include <string.h>

#include "../utils.h"

char* matrix_resolve_homeserver(char const* homeserver_base) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s/.well-known/matrix/client", homeserver_base);
    char* json_str = http_get_string(buf);
    if (!json_str)
        return nullptr;

    char* resolved = nullptr;
    json_error_t error;
    json_t* root = json_loads(json_str, 0, &error);
    free(json_str);

    if (!root)
        return resolved;
    json_t* container = json_object_get(root, "m.homeserver");
    if (!container) {
        efuncprintf("Failed to get m.homeserver");
        goto exit;
    }
    json_t* url = json_object_get(container, "base_url");
    if (!url || !json_is_string(url)) {
        efuncprintf("Failed to get base_url from m.homeserver");
        goto exit;
    }
    resolved = strdup(json_string_value(url));
    size_t len = strlen(resolved);
    if (resolved[len - 1] == '/') {
        resolved[len - 1] = '\0';
    }

    exit:
    json_decref(root);
    return resolved;
}
