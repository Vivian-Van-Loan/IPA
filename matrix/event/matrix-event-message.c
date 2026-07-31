#include "matrix-event-message.h"

matrix_message_type_t matrix_message_type_from_str(const char* str) {
    if (strcmp(str, "m.text") == 0) {
        return MATRIX_MSG_TEXT;
    } else if (strcmp(str, "m.emote") == 0) {
        return MATRIX_MSG_EMOTE;
    } else if (strcmp(str, "m.notice") == 0) {
        return MATRIX_MSG_NOTICE;
    } else if (strcmp(str, "m.image") == 0) {
        return MATRIX_MSG_IMAGE;
    } else if (strcmp(str, "m.file") == 0) {
        return MATRIX_MSG_FILE;
    } else if (strcmp(str, "m.audio") == 0) {
        return MATRIX_MSG_AUDIO;
    } else if (strcmp(str, "m.location") == 0) {
        return MATRIX_MSG_LOCATION;
    } else if (strcmp(str, "m.video") == 0) {
        return MATRIX_MSG_VIDEO;
    }
    // else if (strcmp(str, "m.key.verification.request") == 0) {
    //     return MATRIX_MSG_KEY_VER_REQ;
    // }
    else if (strcmp(str, "m.sticker") == 0) {
        return MATRIX_MSG_STICKER;
    }
    return MATRIX_MSG_UNKNOWN;
}

char const* matrix_message_type_to_str(matrix_message_type_t type) {
    switch (type) {
        default:
        case MATRIX_MSG_UNKNOWN:
            return "";
        case MATRIX_MSG_TEXT:
            return "m.text";
        case MATRIX_MSG_EMOTE:
            return "m.emote";
        case MATRIX_MSG_NOTICE:
            return "m.notice";
        case MATRIX_MSG_IMAGE:
            return "m.image";
        case MATRIX_MSG_FILE:
            return "m.file";
        case MATRIX_MSG_AUDIO:
            return "m.audio";
        case MATRIX_MSG_LOCATION:
            return "m.location";
        case MATRIX_MSG_VIDEO:
            return "m.video";
        // case MATRIX_MSG_KEY_VER_REQ:
        //     return "m.key.verification.request";
        case MATRIX_MSG_STICKER:
            return "m.sticker";
    }
}

matrix_message_t matrix_make_message(json_t* event_json) {
    matrix_message_t message = {0};

    json_t* content_json = json_object_get(event_json, "content");
    if (!content_json || !json_is_object(content_json)) {
        return message;
    }
    json_t* msg_type = json_object_get(content_json, "type");
    if (!msg_type) {
        msg_type = json_object_get(event_json, "type");
        if (!msg_type || !json_is_string(msg_type)) {
            return message;
        }
        if (matrix_message_type_from_str(json_string_value(msg_type)) != MATRIX_MSG_STICKER) {
            return message;
        }
    }
    if (!msg_type || !json_is_string(msg_type)) {
        return message;
    }
    matrix_message_type_t type = matrix_message_type_from_str(json_string_value(msg_type));
    if (type == MATRIX_MSG_UNKNOWN) {
        message.type = (pair$matrix_message_type_t$alloc_str$){MATRIX_MSG_UNKNOWN, strdup(json_string_value(msg_type))};
    } else {
        message.type = (pair$matrix_message_type_t$alloc_str$){type, nullptr};
    }
    json_t* body = json_object_get(content_json, "body");
    if (body && json_is_string(body)) {
        message.body = strdup(json_string_value(body));
    }
    json_t* url = json_object_get(content_json, "url");
    if (url && json_is_string(url)) {
        message.url = strdup(json_string_value(url));
    }
    json_t* filename = json_object_get(content_json, "filename");
    if (filename && json_is_string(filename)) {
        message.filename = strdup(json_string_value(filename));
    }
    json_t* info = json_object_get(content_json, "info");
    if (info && json_is_object(info)) {
        json_t* mimetype = json_object_get(info, "mimetype");
        if (mimetype && json_is_string(mimetype)) {
            message.mimetype = strdup(json_string_value(mimetype));
        }
    }

    return message;
}

void matrix_message_destroy(matrix_message_t* message) {
    free(message->type.second);
    free(message->body);
    free(message->url);
    free(message->filename);
    memset(message, 0, sizeof(*message));
}
