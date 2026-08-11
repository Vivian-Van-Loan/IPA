#include "matrix-event-type.h"

#include <string.h>

bool event_is_room(matrix_event_type_t type) {
    return type >= EVENT_ROOM_CREATE && type <= EVENT_ROOM_REDACTION;
}

bool event_is_redaction(matrix_event_type_t type) {
    return type == EVENT_ROOM_REDACTION;
}

bool event_is_encrypted(matrix_event_type_t type) {
    return type == EVENT_ENCRYPTED;
}

bool event_is_message(matrix_event_type_t type) {
    return type >= EVENT_MSG_MESSAGE && type <= EVENT_MSG_STICKER;
}

bool event_is_ephemeral(matrix_event_type_t type) {
    return type >= EVENT_EPH_TYPING && type <= EVENT_EPH_PRESENCE;
}

matrix_event_type_t matrix_event_type_from_str(const char* str) {
    if (strcmp(str, "m.room.create") == 0) {
        return EVENT_ROOM_CREATE;
    } else if (strcmp(str, "m.room.name") == 0) {
        return EVENT_ROOM_NAME;
    } else if (strcmp(str, "m.room.avatar") == 0) {
        return EVENT_ROOM_AVATAR;
    } else if (strcmp(str, "m.room.topic") == 0) {
        return EVENT_ROOM_TOPIC;
    } else if (strcmp(str, "m.room.join_rules") == 0) {
        return EVENT_ROOM_JOIN_RULES;
    } else if (strcmp(str, "m.room.canonical_alias") == 0) {
        return EVENT_ROOM_CANON_ALIAS;
    } else if (strcmp(str, "m.room.encryption") == 0) {
        return EVENT_ROOM_ENCRYPT;
    } else if (strcmp(str, "m.room.member") == 0) {
        return EVENT_ROOM_MEMBER;
    } else if (strcmp(str, "m.room.power_levels") == 0) {
        return EVENT_ROOM_POWER_LEVELS;
    }

    if (strcmp(str, "m.room.redaction") == 0) {
        return EVENT_ROOM_REDACTION;
    }

    if (strcmp(str, "m.room.encrypted") == 0) {
        return EVENT_ENCRYPTED;
    }

    if (strcmp(str, "m.room.message") == 0) {
        return EVENT_MSG_MESSAGE;
    } else if (strcmp(str, "m.sticker") == 0) {
        return EVENT_MSG_STICKER;
    }

    if (strcmp(str, "m.typing") == 0) {
        return EVENT_EPH_TYPING;
    } else if (strcmp(str, "m.receipt") == 0) {
        return EVENT_EPH_RECEIPT;
    } else if (strcmp(str, "m.presence") == 0) {
        return EVENT_EPH_PRESENCE;
    }

    return EVENT_UNKNOWN;
}

char const* str_from_matrix_event_type(matrix_event_type_t type) {
    switch (type) {
        default:
        case EVENT_UNKNOWN:
            return "com.ipa.unknown";
            break;
        case EVENT_ROOM_CREATE:
            return "m.room.create";
            break;
        case EVENT_ROOM_NAME:
            return "m.room.name";
            break;
        case EVENT_ROOM_AVATAR:
            return "m.room.avatar";
            break;
        case EVENT_ROOM_TOPIC:
            return "m.room.topic";
            break;
        case EVENT_ROOM_JOIN_RULES:
            return "m.room.join_rules";
            break;
        case EVENT_ROOM_CANON_ALIAS:
            return "m.room.canonical_alias";
            break;
        case EVENT_ROOM_ENCRYPT:
            return "m.room.encryption";
            break;
        case EVENT_ROOM_MEMBER:
            return "m.room.member";
            break;
        case EVENT_ROOM_POWER_LEVELS:
            return "m.room.power_levels";
            break;
        case EVENT_ROOM_REDACTION:
            return "m.room.redaction";
            break;
        case EVENT_ENCRYPTED:
            return "m.room.encrypted";
            break;
        case EVENT_MSG_MESSAGE:
            return "m.room.message";
            break;
        case EVENT_MSG_STICKER:
            return "m.sticker";
            break;
        case EVENT_EPH_TYPING:
            return "m.typing";
            break;
        case EVENT_EPH_RECEIPT:
            return "m.receipt";
            break;
        case EVENT_EPH_PRESENCE:
            return "m.presence";
            break;
    }
}
