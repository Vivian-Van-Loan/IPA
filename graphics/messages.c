#include "messages.h"

#include "menus/chat_menu.h"

bool event_can_be_message(matrix_event_t* event) {
    //todo: probably should convert that to a case or direct compare rather than range
    return event_is_message(event->type.first) || (event->type.first >= EVENT_ROOM_CREATE && event->type.first <= EVENT_ROOM_TOPIC) || event->type.first == EVENT_ROOM_MEMBER;
}

ipa_message_t make_ipa_message(matrix_event_t* event, int max_line_width, matrix_room_member_t* user) {
    ipa_message_t message = {0};
    if (!event_can_be_message(event)) {
        return message;
    }
    message.event_id = strdup(event->id);
    char const* text = "";
    bool text_allocated = false;
    bool use_nameplate = false;
    if (event_is_message(event->type.first)) {
        text = event->message.body; //todo: obviously this does only handle text right now, include the others at some point
        message.type = IPA_MSG_MSG;
        use_nameplate = true;
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
                //todo: somehow user events between being created and processed in update_events break somehow
                // and overwrite the membership values with JOIN
                // quite possibly memory corruption, hard to debug
                if (event->state_key) {
                    message.user = strdup(event->state_key);
                }
                if (event->room.member.membership == MATRIX_MEMBERSHIP_LEAVE) {
                    text_allocated = true;
                    text = alloc_sprintf("\"%s\" left", event->room.member.display_name);
                } else if (event->room.member.membership == MATRIX_MEMBERSHIP_BAN) {
                    text_allocated = true;
                    char* display = event->room.member.display_name;
                    if (!display) {
                        display = event->state_key;
                    }
                    text = alloc_sprintf("\"%s\" banned \"%s\"", user->display_name, display);
                } else if (event->room.member.membership == MATRIX_MEMBERSHIP_KNOCK) {
                    text_allocated = true;
                    text = alloc_sprintf("\"%s\" knocked", event->room.member.display_name);
                } else if (event->room.member.membership == MATRIX_MEMBERSHIP_INVITE) {
                    text_allocated = true;
                    char* display = event->room.member.display_name;
                    if (!display) {
                        display = event->state_key;
                    }
                    text = alloc_sprintf("\"%s\" invited \"%s\"", user->display_name, display);
                } else if (event->room.member.membership == MATRIX_MEMBERSHIP_JOIN) {
                    if (event->room.member.prev_membership != MATRIX_MEMBERSHIP_JOIN) {
                        text_allocated = true;
                        text = alloc_sprintf("\"%s\" joined", event->room.member.display_name);
                    } else if (strcmp_null(event->room.member.avatar_url, event->room.member.prev_avatar_url) != 0) {
                        text_allocated = true;
                        text = alloc_sprintf("\"%s\" changed their avatar", event->room.member.display_name);
                    } else if (strcmp_null(event->room.member.display_name, event->room.member.prev_display_name) != 0) {
                        text_allocated = true;
                        text = alloc_sprintf("\"%s\" changed their name (previously \"%s\")", event->room.member.display_name, event->room.member.prev_display_name);
                    } else {
                        text_allocated = true;
                        text = alloc_sprintf("\"%s\" did something", event->room.member.display_name);
                    }
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
    pair$int$int$ nameplate_widths = get_nameplate_width(user->display_name, user->avatar_url);
    int first_line_max_width;
    if (use_nameplate) {
        first_line_max_width = max_line_width - nameplate_widths.second;
    } else {
        first_line_max_width = max_line_width;
    }
    message.first_line_max_width = first_line_max_width;
    message.max_line_width = max_line_width;

    size_t len = strlen(text);
    size_t offset = 0;
    while (offset < len) {
        pair$size_t$ipa_string_t$ result = ipa_string_conv_max_width(text + offset, offset == 0 ? message.first_line_max_width : message.max_line_width, true);
        if (result.first == (size_t)-1) {
            goto error;
        }
        offset += result.first;
        vector$ipa_string_t$_push_back(&message.lines, result.second);
    }
    vector$ipa_string_t$_shrink(&message.lines);

    goto done;

    error:
    ipa_message_destroy(&message);

    done:
    if (text_allocated) {
        free((void*)text);
    }
    return message;
}

void ipa_message_destroy(ipa_message_t* message) {
    vector$ipa_string_t$_free_callback(&message->lines, ipa_string_destroy);
    free((void*)message->user);
    free((void*)message->event_id);
    memset(message, 0, sizeof(*message));
}
