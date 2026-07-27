#include "matrix-event-room.h"

#include "matrix-event.h"
#include "../../utils.h"

matrix_room_create_t matrix_make_room_create(json_t const* create_json, json_t const* event_json) {
    matrix_room_create_t create = {0};

    json_t* creator = json_object_get(create_json, "creator");
    if (!creator) {
        creator = json_object_get(event_json, "sender");
    }
    if (json_is_string(creator)) {
        create.creator = strdup(json_string_value(creator));
    }
    json_t* additional_creators = json_object_get(create_json, "additional_creators");
    if (additional_creators && json_is_array(additional_creators)) {
        create.additional_creators = vector$alloc_str$_new();
        size_t index;
        json_array_foreach(additional_creators, index, creator) {
            if (json_is_string(creator)) {
                vector$alloc_str$_push(&create.additional_creators, strdup(json_string_value(creator)));
            }
        }
    }

    json_t* federate = json_object_get(create_json, "m.federate");
    if (federate && json_is_boolean(federate)) {
        create.m_federate = json_boolean_value(federate);
    } else {
        create.m_federate = true;
    }
    json_t* room_version = json_object_get(create_json, "room_version");
    if (room_version && json_is_string(room_version)) {
        create.room_version = strdup(json_string_value(room_version));
    }
    json_t* type = json_object_get(create_json, "type");
    if (type && json_is_string(type)) {
        create.type = strdup(json_string_value(type));
    }
    json_t* predecessor = json_object_get(create_json, "predecessor");
    if (predecessor && json_is_object(predecessor)) {
        create.predecessor = calloc(1, sizeof(*create.predecessor));
        json_t* room_id = json_object_get(predecessor, "room_id");
        json_t* event_id = json_object_get(predecessor, "event_id");
        if (room_id && json_is_string(room_id)) {
            create.predecessor->room_id = strdup(json_string_value(room_id));
        }
        if (event_id && json_is_string(event_id)) {
            create.predecessor->event_id = strdup(json_string_value(event_id));
        }
    }

    return create;
}

matrix_room_name_t matrix_make_room_name(json_t const* name_json) {
    matrix_room_name_t name = {0};

    json_t* name_value = json_object_get(name_json, "name");
    if (name_value && json_is_string(name_value)) {
        name.name = strdup(json_string_value(name_value));
    }
    return name;
}

matrix_room_avatar_t matrix_make_room_avatar(json_t const* avatar_json) {
    matrix_room_avatar_t avatar = {0};

    json_t* url = json_object_get(avatar_json, "url");
    if (url && json_is_string(url)) {
        avatar.url = strdup(json_string_value(url));
    }
    //todo maybe : avatar info and thumbnail and such

    // json_t* info = json_object_get(avatar_json, "info");
    // if (info && json_is_object(info)) {
    //     avatar.info = calloc(1, sizeof(*avatar.info));
    //     // json_t* mimetype = json_object_get(info, "mimetype");
    // }
    return avatar;
}

matrix_room_topic_t matrix_make_room_topic(json_t const* topic_json) {
    matrix_room_topic_t topic = {0};

    json_t* topic_value = json_object_get(topic_json, "topic");
    if (topic_value && json_is_string(topic_value)) {
        topic.topic = strdup(json_string_value(topic_value));
    }
    return topic;
}

matrix_room_join_rules_t matrix_make_room_join_rules(json_t const* join_json) {
    matrix_room_join_rules_t join_rules = {0};

    json_t* join_rule = json_object_get(join_json, "join_rule");
    if (join_rule && json_is_string(join_rule)) {
        char const* rule = json_string_value(join_rule);
        if (strcmp(rule, "public") == 0) {
            join_rules.type = MATRIX_JOIN_PUBLIC;
        } else if (strcmp(rule, "invite") == 0) {
            join_rules.type = MATRIX_JOIN_INVITE;
        } else if (strcmp(rule, "knock") == 0) {
            join_rules.type = MATRIX_JOIN_KNOCK;
        } else if (strcmp(rule, "private") == 0) {
            join_rules.type = MATRIX_JOIN_PRIVATE;
        } else if (strcmp(rule, "restricted") == 0) {
            join_rules.type = MATRIX_JOIN_RESTRICTED;
        } else if (strcmp(rule, "MATRIX_JOIN_KNOCK_RESTRICTED") == 0) {
            join_rules.type = MATRIX_JOIN_KNOCK_RESTRICTED;
        } else {
            efuncprintf("Unknown join rule %s\n", rule);
            join_rules.type = MATRIX_JOIN_NONE;
        }
    }
    json_t* allow = json_object_get(join_json, "allow");
    if (allow && json_is_object(allow)) {
        json_t* room_id = json_object_get(allow, "room_id");
        if (room_id && json_is_string(room_id)) {
            join_rules.allow_room_id = strdup(json_string_value(room_id));
        }
    }
    return join_rules;
}

matrix_room_canon_alias_t matrix_make_room_canon_alias(json_t const* alias_json) {
    matrix_room_canon_alias_t alias = {0};

    json_t* alias_value = json_object_get(alias_json, "alias");
    if (alias_value && json_is_string(alias_value)) {
        alias.alias = strdup(json_string_value(alias_value));
    }
    json_t* alt_aliases = json_object_get(alias_json, "alt_aliases");
    if (alt_aliases && json_is_array(alt_aliases)) {
        alias.alt_aliases = vector$alloc_str$_new();
        size_t index;
        json_array_foreach(alt_aliases, index, alias_value) {
            if (json_is_string(alias_value)) {
                vector$alloc_str$_push(&alias.alt_aliases, strdup(json_string_value(alias_value)));
            }
        }
    }
    return alias;
}

matrix_room_encrypt_t matrix_make_room_encrypt(json_t const* encrypt_json) {
    matrix_room_encrypt_t encrypt = {0};

    json_t* algorithm = json_object_get(encrypt_json, "algorithm");
    if (algorithm && json_is_string(algorithm)) {
        encrypt.algorithm = strdup(json_string_value(algorithm));
    }
    json_t* rotation_period_ms = json_object_get(encrypt_json, "rotation_period_ms");
    if (rotation_period_ms && json_is_integer(rotation_period_ms)) {
        encrypt.rotation_period_ms = json_integer_value(rotation_period_ms);
    }
    json_t* rotation_period_msgs = json_object_get(encrypt_json, "rotation_period_msgs");
    if (rotation_period_msgs && json_is_integer(rotation_period_msgs)) {
        encrypt.rotation_period_msgs = json_integer_value(rotation_period_msgs);
    }
    return encrypt;
}

matrix_room_member_t matrix_make_room_member(json_t const* member_json) {
    matrix_room_member_t member = {0};

    json_t* avatar_url = json_object_get(member_json, "avatar_url");
    if (avatar_url && json_is_string(avatar_url)) {
        member.avatar_url = strdup(json_string_value(avatar_url));
    }
    json_t* display_name = json_object_get(member_json, "displayname");
    if (display_name && json_is_string(display_name)) {
        member.display_name = strdup(json_string_value(display_name));
    }
    json_t* is_direct = json_object_get(member_json, "is_direct");
    if (is_direct && json_is_boolean(is_direct)) {
        member.is_direct = json_boolean_value(is_direct);
    }
    json_t* auth = json_object_get(member_json, "join_authorised_via_users_server");
    if (auth && json_is_string(auth)) {
        member.join_authorised_via_users_server = strdup(json_string_value(auth));
    }
    json_t* membership = json_object_get(member_json, "membership");
    if (membership && json_is_string(membership)) {
        char const* memb = json_string_value(membership);
        if (strcmp(memb, "invite") == 0) {
            member.membership = MATRIX_MEMBERSHIP_INVITE;
        } else if (strcmp(memb, "join") == 0) {
            member.membership = MATRIX_MEMBERSHIP_JOIN;
        } else if (strcmp(memb, "knock") == 0) {
            member.membership = MATRIX_MEMBERSHIP_KNOCK;
        } else if (strcmp(memb, "leave") == 0) {
            member.membership = MATRIX_MEMBERSHIP_LEAVE;
        } else if (strcmp(memb, "ban") == 0) {
            member.membership = MATRIX_MEMBERSHIP_BAN;
        } else {
            efuncprintf("Unknown membership %s\n", memb);
            member.membership = MATRIX_MEMBERSHIP_NONE;
        }
    }
    json_t* reason = json_object_get(member_json, "reason");
    if (reason && json_is_string(reason)) {
        member.reason = strdup(json_string_value(reason));
    }
    return member;
}

matrix_room_power_levels_t matrix_make_room_power_levels(json_t const* power_levels_json) {
    matrix_room_power_levels_t power_levels = {0};

    json_t* ban = json_object_get(power_levels_json, "ban");
    if (ban && json_is_integer(ban)) {
        power_levels.ban = json_integer_value(ban);
    } else {
        power_levels.ban = 50;
    }
    json_t* events_default = json_object_get(power_levels_json, "events_default");
    if (events_default && json_is_integer(events_default)) {
        power_levels.events_default = json_integer_value(events_default);
    } else {
        power_levels.events_default = 0;
    }
    json_t* invite = json_object_get(power_levels_json, "invite");
    if (invite && json_is_integer(invite)) {
        power_levels.invite = json_integer_value(invite);
    } else {
        power_levels.invite = 0;
    }
    json_t* kick = json_object_get(power_levels_json, "kick");
    if (kick && json_is_integer(kick)) {
        power_levels.kick = json_integer_value(kick);
    } else {
        power_levels.kick = 50;
    }
    json_t* redact = json_object_get(power_levels_json, "redact");
    if (redact && json_is_integer(redact)) {
        power_levels.redact = json_integer_value(redact);
    } else {
        power_levels.redact = 50;
    }
    json_t* state_default = json_object_get(power_levels_json, "state_default");
    if (state_default && json_is_integer(state_default)) {
        power_levels.state_default = json_integer_value(state_default);
    } else {
        power_levels.state_default = 0;
    }

    json_t* events = json_object_get(power_levels_json, "events");
    if (events && json_is_object(events)) {
        power_levels.events = hash_map$alloc_str$int64_t$_new();
        char const* key;
        json_t* value;
        json_object_foreach(events, key, value) {
            if (json_is_integer(value)) {
                hash_map$alloc_str$int64_t$_add(&power_levels.events, strdup(key), json_integer_value(value));
            }
        }
    }
    //todo: what is going on with the m.room.tombstone rules? what?
    json_t* users = json_object_get(power_levels_json, "users");
    if (users && json_is_object(users)) {
        power_levels.users = hash_map$alloc_str$int64_t$_new();
        char const* key;
        json_t* value;
        json_object_foreach(users, key, value) {
            if (json_is_integer(value)) {
                hash_map$alloc_str$int64_t$_add(&power_levels.users, strdup(key), json_integer_value(value));
            }
        }
    }

    return power_levels;
}

matrix_event_room_t matrix_make_room_event(matrix_event_type_t type, json_t* event_json) {
    matrix_event_room_t event = {0};

    switch (type) {
        case EVENT_ROOM_CREATE:
            event.create = matrix_make_room_create(event_json, event_json);
            break;
        case EVENT_ROOM_NAME:
            event.name = matrix_make_room_name(event_json);
            break;
        case EVENT_ROOM_AVATAR:
            event.avatar = matrix_make_room_avatar(event_json);
            break;
        case EVENT_ROOM_TOPIC:
            event.topic = matrix_make_room_topic(event_json);
            break;
        case EVENT_ROOM_JOIN_RULES:
            event.join_rules = matrix_make_room_join_rules(event_json);
            break;
        case EVENT_ROOM_CANON_ALIAS:
            event.canon_alias = matrix_make_room_canon_alias(event_json);
            break;
        case EVENT_ROOM_ENCRYPT:
            event.encrypt = matrix_make_room_encrypt(event_json);
            break;
        case EVENT_ROOM_MEMBER:
            event.member = matrix_make_room_member(event_json);
            break;
        case EVENT_ROOM_POWER_LEVELS:
            break;
        case EVENT_REDACTED:
            break;
        default:
            efuncprintf("Called matrix_make_room_event with a non event type\n");
            exit(1);
    }

    return event;
}

void matrix_room_create_destroy(matrix_room_create_t* create) {
    free(create->creator);
    vector$alloc_str$_free_callback(&create->additional_creators, alloc_str_free);
    free(create->room_version);
    free(create->type);
}

void matrix_room_name_destroy(matrix_room_name_t* name) {
    free(name->name);
}

void matrix_room_avatar_destroy(matrix_room_avatar_t* avatar) {
    if (avatar->info) {
        free(avatar->info->mimetype);
        free(avatar->info->thumbnail_url);
        if (avatar->info->thumbnail_info) {
            free(avatar->info->thumbnail_info->mimetype);
            free(avatar->info->thumbnail_info);
        }
        free(avatar->info);
    }
    free(avatar->url);
}

void matrix_room_topic_destroy(matrix_room_topic_t* topic) {
    free(topic->topic);
}

void matrix_room_join_rules_destroy(matrix_room_join_rules_t* join_rules) {
    free(join_rules->allow_room_id);
}

void matrix_room_canon_alias_destroy(matrix_room_canon_alias_t* canon_alias) {
    free(canon_alias->alias);
    vector$alloc_str$_free_callback(&canon_alias->alt_aliases, alloc_str_free);
}

void matrix_room_encrypt_destroy(matrix_room_encrypt_t* encrypt) {
    free(encrypt->algorithm);
}

void matrix_room_member_destroy(matrix_room_member_t* member) {
    free(member->avatar_url);
    free(member->display_name);
    free(member->join_authorised_via_users_server);
    free(member->reason);
}

void matrix_room_power_levels_destroy(matrix_room_power_levels_t* power_levels) {
    hash_map$alloc_str$int64_t$_free_callback(&power_levels->events, alloc_str_free, nullptr);
    hash_map$alloc_str$int64_t$_free_callback(&power_levels->users, alloc_str_free, nullptr);
}

void matrix_event_room_destroy(matrix_event_type_t type, matrix_event_room_t* event) {
    switch (type) {
        case EVENT_ROOM_CREATE:
            matrix_room_create_destroy(&event->create);
            break;
        case EVENT_ROOM_NAME:
            matrix_room_name_destroy(&event->name);
            break;
        case EVENT_ROOM_AVATAR:
            matrix_room_avatar_destroy(&event->avatar);
            break;
        case EVENT_ROOM_TOPIC:
            matrix_room_topic_destroy(&event->topic);
            break;
        case EVENT_ROOM_JOIN_RULES:
            matrix_room_join_rules_destroy(&event->join_rules);
            break;
        case EVENT_ROOM_CANON_ALIAS:
            matrix_room_canon_alias_destroy(&event->canon_alias);
            break;
        case EVENT_ROOM_ENCRYPT:
            matrix_room_encrypt_destroy(&event->encrypt);
            break;
        case EVENT_ROOM_MEMBER:
            matrix_room_member_destroy(&event->member);
            break;
        case EVENT_ROOM_POWER_LEVELS:
            matrix_room_power_levels_destroy(&event->power_levels);
            break;
        default:
            efuncprintf("Called matrix_event_room_destroy with a non event type\n");
            exit(1);
    }
}
