#include "matrix-event.h"

#include "../../utils.h"

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
    free(event->unsigned_data.replaces);
    free(event->unsigned_data.transaction_id);
}
