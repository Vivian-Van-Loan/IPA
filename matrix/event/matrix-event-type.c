#include "matrix-event-type.h"

bool event_is_room(matrix_event_type_t type) {
    return type >= EVENT_ROOM_CREATE && type <= EVENT_ROOM_POWER_LEVELS;
}

bool event_is_ephemeral(matrix_event_type_t type) {
    return type >= EVENT_EPH_TYPING && type <= EVENT_EPH_PRESENCE;
}
