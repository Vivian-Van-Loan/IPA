#include "matrix-event.h"

#include "../../utils.h"

matrix_event_t matrix_make_event(json_t* json) {
    matrix_event_t event = {0};

    json_t* type_json = json_object_get(json, "type");
    if (!type_json || json_is_string(type_json)) {
        return event;
    }
    matrix_event_type_t type = matrix_event_type_from_str(json_string_value(type_json));
    if (type == EVENT_UNKNOWN) {
        event.type = (pair$matrix_event_type_t$alloc_str$){EVENT_UNKNOWN, strdup(json_string_value(type_json))};
    } else {
        event.type = (pair$matrix_event_type_t$alloc_str$){type, nullptr};
    }

    if (event_is_room(type)) {
        event.room = matrix_make_room_event(type, json);
    } else if (event_is_encrypted(type)) {
        //TODO: BIG ONE
    } else if (event_is_message(type)) {
        event.message = matrix_make_message(json);
    } else if (event_is_ephemeral(type)) {
        //todo
    }

    json_t* id = json_object_get(json, "event_id");
    if (id && json_is_string(id)) {
        event.id = strdup(json_string_value(id));
    }
    json_t* time = json_object_get(json, "origin_server_ts");
    if (time && json_is_integer(time)) {
        event.origin_server_ts = json_integer_value(time);
    }
    json_t* room_id = json_object_get(json, "room_id");
    if (room_id && json_is_string(room_id)) {
        event.room_id = strdup(json_string_value(room_id));
    }
    json_t* sender = json_object_get(json, "sender");
    if (sender && json_is_string(sender)) {
        event.sender = strdup(json_string_value(sender));
    }
    json_t* state = json_object_get(json, "state_key");
    if (state && json_is_string(state)) {
        event.state_key = strdup(json_string_value(state));
    }

    json_t* unsigned_data = json_object_get(json, "unsigned");
    if (unsigned_data && json_is_object(unsigned_data)) {
        json_t* age = json_object_get(unsigned_data, "age");
        if (age && json_is_integer(age)) {
            event.unsigned_data.age = json_integer_value(age);
        }
        json_t* membership = json_object_get(unsigned_data, "membership");
        if (membership && json_is_string(membership)) {
            event.unsigned_data.membership = strdup(json_string_value(membership));
        }
        json_t* replaces = json_object_get(unsigned_data, "replaces_state");
        if (replaces && json_is_string(replaces)) {
            event.unsigned_data.replaces_state = strdup(json_string_value(replaces));
        }
        json_t* transaction_id = json_object_get(unsigned_data, "transaction_id");
        if (transaction_id && json_is_string(transaction_id)) {
            event.unsigned_data.transaction_id = strdup(json_string_value(transaction_id));
        }
    }

    return event;
}

void matrix_event_destroy(matrix_event_t* event) {
    matrix_event_type_t type = event->type.first;
    if (event_is_room(type)) {
        matrix_event_room_destroy(type, &event->room);
    } else if (event_is_ephemeral(type)) {
        // matrix_event_ephemeral_destroy(type, (matrix_event_ephemeral_t*)event);
        efuncprintf("matrix_event_destroy on ephemeral before ready!\n");
        exit(1);
    } else if (event->type.first == EVENT_MSG_MESSAGE) {
        matrix_message_destroy(&event->message);
    } else {
        efuncprintf("matrix_event_destroy on unknown/unready event type!\n");
        exit(1);
    }

    free(event->type.second);
    free(event->id);
    free(event->sender);
    free(event->room_id);
    free(event->state_key);
    free(event->unsigned_data.membership);
    free(event->unsigned_data.replaces_state);
    free(event->unsigned_data.transaction_id);
}
