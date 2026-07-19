#ifndef MATRIX_CLIENT_MATRIX_EVENT_ROOM_H
#define MATRIX_CLIENT_MATRIX_EVENT_ROOM_H

#include "../../types/general_types.h"
#include "matrix-event-type.h"

typedef struct matrix_prev_room_t {
    char* event_id;
    char* room_id;
} matrix_prev_room_t;

typedef struct matrix_room_create_t {
    char* creator;
    vector$alloc_str$ additional_creators;
    bool m_federate;
    char* room_version;
    char* type;
} matrix_room_create_t;

typedef struct matrix_room_name_t {
    char* name;
} matrix_room_name_t;

typedef struct matrix_thumbnail_info_t {
    size_t h;
    size_t w;
    size_t size;
    char* mimetype;
} matrix_thumbnail_info_t;

typedef struct matrix_avatar_info_t {
    size_t h;
    size_t w;
    size_t size;
    char* mimetype;
    matrix_thumbnail_info_t* thumbnail_info;
    char* thumbnail_url;
} matrix_avatar_info_t;

typedef struct matrix_room_avatar_t {
    matrix_avatar_info_t* info; //will probably be discarded/null most of the time for our purposes
    char* url;
} matrix_room_avatar_t;

typedef struct matrix_room_topic_t {
    //todo: MAYBE bother with the topic content blocks? Idk very unlikely to be used by us
    char* topic;
} matrix_room_topic_t;

typedef enum matrix_join_rule_type_t {
    MATRIX_JOIN_PUBLIC,
    MATRIX_JOIN_INVITE,
    MATRIX_JOIN_KNOCK,
    MATRIX_JOIN_RESTRICTED,
    MATRIX_JOIN_KNOCK_RESTRICTED,
    MATRIX_JOIN_PRIVATE,
} matrix_join_rule_type_t;

typedef struct matrix_room_join_rules_t {
    matrix_join_rule_type_t type;
    char* allow_room_id; //given there's only one type of allow rule and it requires a room id, might as well just use that directly
} matrix_room_join_rules_t;

typedef struct matrix_room_canon_alias_t {
    char* alias;
    vector$alloc_str$ alt_aliases;
} matrix_room_canon_alias_t;

typedef struct matrix_room_encrypt {
    char* algorithm;
    int64_t rotation_period_ms;
    int64_t rotation_period_msgs;
} matrix_room_encrypt;

typedef struct matrix_room_member_t {
    char* avatar_url;
    char* display_name;
    bool is_direct;
    char* join_authorised_via_users_server;
    char* membership;
    char* reason;
    //todo: maybe figure out ThirdPartyInvite
} matrix_room_member_t;

typedef struct matrix_room_power_levels_t {
    int ban;
    int events_default;
    int invite;
    int kick;
    int redact;
    int state_default;
    int users_default;
    hash_map_entry$alloc_str$int$ events;
    hash_map_entry$alloc_str$int$ users;
} matrix_room_power_levels_t;

typedef union matrix_event_room_t {
    matrix_room_create_t create;
    matrix_room_name_t name;
    matrix_room_avatar_t avatar;
    matrix_room_topic_t topic;
    matrix_room_join_rules_t join_rules;
    matrix_room_canon_alias_t canon_alias;
    matrix_room_encrypt encrypt;
    matrix_room_member_t member;
    matrix_room_power_levels_t power_levels;
} matrix_event_room_t;

void matrix_event_room_destroy(matrix_event_type_t type, matrix_event_room_t* event);

#endif //MATRIX_CLIENT_MATRIX_EVENT_ROOM_H
