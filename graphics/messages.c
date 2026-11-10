#include "messages.h"

bool event_can_be_message(matrix_event_t* event) {
    //todo: probably should convert that to a case or direct compare rather than range
    return event_is_message(event->type.first) || (event->type.first >= EVENT_ROOM_CREATE && event->type.first <= EVENT_ROOM_TOPIC) || event->type.first == EVENT_ROOM_MEMBER;
}

ipa_message_t make_ipa_message(matrix_event_t* event, int first_line_max_width, int max_line_width) {
    ipa_message_t message = {0};
    if (!event_can_be_message(event)) {
        return message;
    }
    message.event_id = strdup(event->id);
    char const* text;
    if (event_is_message(event->type.first)) {
        text = event->message.body; //todo: obviously this does only handle text right now, include the others at some point
        message.type = IPA_MSG_MSG;
    } else {
        switch (event->type.first) {
            case EVENT_ROOM_CREATE:
                text = "Room created";
                break;
            case EVENT_ROOM_NAME:
                text = "Room name changed";
                break;
            case EVENT_ROOM_AVATAR:
                text = "Room avatar changed";
                break;
            case EVENT_ROOM_TOPIC:
                text = "Room topic changed";
                break;
            case EVENT_ROOM_MEMBER:
                text = "User joined";
                if (event->state_key) {
                    message.user = strdup(event->state_key);
                }
                break;
            default:
                return message;
        }
        message.type = IPA_MSG_ANNOUNCEMENT;
    }
    if (!message.user && event->sender) {
        message.user = strdup(event->sender);
    }
    message.first_line_max_width = first_line_max_width;
    message.max_line_width = max_line_width;

    size_t len = strlen(text);
    size_t offset = 0;
    while (offset < len) {
        pair$size_t$ipa_string_t$ result = ipa_string_conv_max_width(text + offset, offset == 0 ? first_line_max_width : max_line_width, true);
        if (result.first == (size_t)-1) {
            goto error;
        }
        offset += result.first;
        vector$ipa_string_t$_push_back(&message.lines, result.second);
    }
    vector$ipa_string_t$_shrink(&message.lines);

    return message;

    error:
    ipa_message_destroy(&message);
    return message;
}

void ipa_message_destroy(ipa_message_t* message) {
    vector$ipa_string_t$_free_callback(&message->lines, ipa_string_destroy);
    free((void*)message->user);
    free((void*)message->event_id);
    memset(message, 0, sizeof(*message));
}
