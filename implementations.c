#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

#define TEMPLATE_IMPLEMENTATION
#include "utils.h"
#include "work_queue.h"
#include "types/general_types.h"
#include "matrix/matrix_room_t.h"
#include "matrix/matrix-utils.h"
#include "matrix/matrix-client.h"
#include "matrix/event/matrix-event-room.h"
#include "matrix/event/matrix-event.h"
#include "graphics/images.h"
#include "graphics/graphics.h"
#include "graphics/messages.h"
#include "graphics/font.h"
