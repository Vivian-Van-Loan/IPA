#ifndef MATRIX_CLIENT_MATRIX_EVENT_ROOM_H
#define MATRIX_CLIENT_MATRIX_EVENT_ROOM_H

#include <stdint.h>
#include <jansson.h>

#include "../../types/general_types.h"
#include "matrix-event-type.h"

#define TEMPLATE_TYPE_K alloc_str
#define TEMPLATE_TYPE_V int64_t
#define TEMPLATE_EQUAL_FUNC(lhs, rhs) (strcmp((lhs), (rhs)) == 0)
#include "../../types/hash_map.h"

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
    matrix_prev_room_t* predecessor;
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
    MATRIX_JOIN_NONE = 0,
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

typedef struct matrix_room_encrypt_t {
    char* algorithm;
    int64_t rotation_period_ms;
    int64_t rotation_period_msgs;
} matrix_room_encrypt_t;

typedef enum matrix_membership_type_t {
    MATRIX_MEMBERSHIP_NONE = 0,
    MATRIX_MEMBERSHIP_INVITE,
    MATRIX_MEMBERSHIP_JOIN,
    MATRIX_MEMBERSHIP_KNOCK,
    MATRIX_MEMBERSHIP_LEAVE,
    MATRIX_MEMBERSHIP_BAN,
} matrix_membership_type_t;

typedef struct matrix_room_member_t {
    char* avatar_url;
    char* display_name;
    bool is_direct;
    char* join_authorised_via_users_server;
    matrix_membership_type_t membership;
    char* reason;
    //todo: maybe figure out ThirdPartyInvite
} matrix_room_member_t;

typedef struct matrix_room_power_levels_t {
    int64_t ban;
    int64_t events_default;
    int64_t invite;
    int64_t kick;
    int64_t redact;
    int64_t state_default;
    int64_t users_default;
    hash_map$alloc_str$int64_t$ events;
    hash_map$alloc_str$int64_t$ users;
} matrix_room_power_levels_t;

typedef struct matrix_room_redaction_t {
    char* reason;
    char* redacts;
} matrix_room_redaction_t;

typedef union matrix_event_room_t {
    matrix_room_create_t create;
    matrix_room_name_t name;
    matrix_room_avatar_t avatar;
    matrix_room_topic_t topic;
    matrix_room_join_rules_t join_rules;
    matrix_room_canon_alias_t canon_alias;
    matrix_room_encrypt_t encrypt;
    matrix_room_member_t member;
    matrix_room_power_levels_t power_levels;
    matrix_room_redaction_t redaction;
} matrix_event_room_t;

matrix_event_room_t matrix_make_room_event(matrix_event_type_t type, json_t* json);
void matrix_event_room_destroy(matrix_event_type_t type, matrix_event_room_t* event);

#endif //MATRIX_CLIENT_MATRIX_EVENT_ROOM_H
