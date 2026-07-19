#include "matrix-event-message.h"

void matrix_message_destroy(matrix_message_t* message) {
    free(message->type.second);
    free(message->body);
    free(message->url);
    free(message->filename);
    memset(message, 0, sizeof(*message));
}
