#ifndef MATRIX_CLIENT_IMAGES_H
#define MATRIX_CLIENT_IMAGES_H
#include <stddef.h>
#include <citro2d.h>
#include <citro3d.h>

#include "../types/general_types.h"
#include "../utils.h"

enum image_format {
    IMAGE_FORMAT_UNKNOWN,
    IMAGE_FORMAT_PNG,
    IMAGE_FORMAT_BMP,
    IMAGE_FORMAT_JPG,
    IMAGE_FORMAT_GIF,
    IMAGE_FORMAT_WEBP,
    IMAGE_FORMAT_RGBA,
    IMAGE_FORMAT_ETC1,
    IMAGE_FORMAT_ETC1A4,
};

typedef char img_path[sizeof(AVATAR_CACHE_DIR) + sizeof("NNNxNNN/") - 1 + NAME_MAX]; //Cache dir, image size, filename, - 1 to get rid of one NUL term
typedef struct ipa_image_t {
    unsigned char* data;
    bool in_vram;
    C2D_Sprite* sprite;
    // size_t size;
    size_t width;
    size_t height;
    size_t frames;
    size_t num_channels;
    int delay;
    enum image_format format;
    img_path path;
} ipa_image_t;

#define TEMPLATE_TYPE_K img_path //todo: figure out how this is going to fully work
#define TEMPLATE_TYPE_K_IS_ARRAY
#define TEMPLATE_TYPE_V ipa_image_t
// #define TEMPLATE_HASH_FUNC auto_hash
#define TEMPLATE_EQUAL_FUNC(lhs, rhs) (strncmp(lhs, rhs, sizeof(img_path)) == 0)
#include "../types/hash_map.h"

size_t bits_per_pixel(enum image_format format);

void form_avatar_path(img_path dest, char const* image_id, size_t width, size_t height);
void form_image_path(img_path dest, char const* image_id); //using img path is a bit overkill as it's longer than needed, but whatever it's minor

ipa_image_t* lookup_avatar(hash_map$img_path$ipa_image_t$* map, char const* mxc_url, size_t width, size_t height);
ipa_image_t* lookup_image(hash_map$img_path$ipa_image_t$* map, char const* mxc_url);

ipa_image_t load_image(unsigned char const* input, size_t len, enum image_format format);
void destroy_image(ipa_image_t* image);
ipa_image_t convert_to_etc(ipa_image_t* image); //image WILL be destroyed after (on successful conversion)
ipa_image_t resize_compress(ipa_image_t* image, size_t width, size_t height); //image WILL be destroyed after (on successful conversion)
ipa_image_t load_resize_compress_save(unsigned char const* input, size_t len, enum image_format format, size_t width, size_t height, img_path const path);

void image_load_vram(ipa_image_t* image);
void image_unload_vram(ipa_image_t* image);

#endif //MATRIX_CLIENT_IMAGES_H
