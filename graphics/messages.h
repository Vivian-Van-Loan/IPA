#ifndef MATRIX_CLIENT_MESSAGES_H
#define MATRIX_CLIENT_MESSAGES_H

#include "font.h"
#include "images.h"
#include "../matrix/event/matrix-event.h"

#define TEMPLATE_TYPE ipa_string_t
#include "../types/vector.h"

typedef enum ipa_message_type_t {
    IPA_MSG_MSG,
    IPA_MSG_ANNOUNCEMENT,
    //that's really it for now, normal messages get standard colouring depending on user name
    // announcement/system/whatever you want to call them messages get the black background silver text colouring
} ipa_message_type_t;

typedef struct ipa_message_t {
    ipa_message_type_t type;
    char const* event_id;
    char const* user;
    int first_line_max_width;
    int max_line_width;
    vector$ipa_string_t$ lines;
} ipa_message_t;

bool event_can_be_message(matrix_event_t* event);
ipa_message_t make_ipa_message(matrix_event_t* event, int first_line_max_width, int max_line_width);
void ipa_message_destroy(ipa_message_t* message);

#endif //MATRIX_CLIENT_MESSAGES_H
