#ifndef MATRIX_CLIENT_MATRIX_EVENT_TYPE_H
#define MATRIX_CLIENT_MATRIX_EVENT_TYPE_H

typedef enum matrix_event_type_t {
    EVENT_UNKNOWN,

    EVENT_ROOM_CREATE,
    EVENT_ROOM_NAME,
    EVENT_ROOM_AVATAR,
    EVENT_ROOM_TOPIC,
    EVENT_ROOM_JOIN_RULES,
    EVENT_ROOM_CANON_ALIAS,
    EVENT_ROOM_ENCRYPT,
    EVENT_ROOM_MEMBER,
    EVENT_ROOM_POWER_LEVELS,
    EVENT_ROOM_REDACTION,

    EVENT_ENCRYPTED,

    EVENT_MSG_MESSAGE,
    EVENT_MSG_STICKER,

    EVENT_EPH_TYPING,
    EVENT_EPH_RECEIPT,
    EVENT_EPH_PRESENCE,
} matrix_event_type_t;

bool event_is_room(matrix_event_type_t type);
bool event_is_redaction(matrix_event_type_t type);
bool event_is_encrypted(matrix_event_type_t type);
bool event_is_message(matrix_event_type_t type);
bool event_is_ephemeral(matrix_event_type_t type);

matrix_event_type_t matrix_event_type_from_str(const char* str);
char const* str_from_matrix_event_type(matrix_event_type_t type);

#endif //MATRIX_CLIENT_MATRIX_EVENT_TYPE_H
