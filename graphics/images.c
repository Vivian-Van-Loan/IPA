#include "images.h"

#include <stb_image.h>
#include <webp/decode.h>
#include <rg-etc1/rg_etc1_c.h>
#include <sys/unistd.h>

#include "graphics.h"
#include "stb_image_resize2.h"
#include "../utils.h"

size_t bits_per_pixel(enum image_format format) {
    switch (format) {
        case IMAGE_FORMAT_ETC1:
            return 4;
        case IMAGE_FORMAT_ETC1A4:
            return 8;
        case IMAGE_FORMAT_RGBA:
            return 32;
        default:
            return 0;
    }
}

size_t bytes_per_pixel(enum image_format format) {
    return bits_per_pixel(format) / 8;
}

void form_avatar_path(img_path dest, char const* image_id, size_t width, size_t height) {
    snprintf(dest, sizeof(img_path), "%s%zux%zu/%s", AVATAR_CACHE_DIR, width, height, image_id);
}

void form_image_path(img_path dest, char const* image_id) {
    snprintf(dest, sizeof(img_path), "%s%s", IMG_CACHE_DIR, image_id); //todo: quite lazy for a double string concat, change to memccpy
}

ipa_image_t* lookup_direct(hash_map$img_path$ipa_image_t$* map, char* path) {
    if (!map || !path) {
        return nullptr;
    }
    ipa_image_t* image = hash_map$img_path$ipa_image_t$_find(map, path);
    return image;
}

ipa_image_t* lookup_avatar(hash_map$img_path$ipa_image_t$* map, char const* mxc_url, size_t width, size_t height) {
    if (!mxc_url) {
        return nullptr;
    }
    char const* img_id = strrchr(mxc_url, '/');
    if (!img_id) {
        return nullptr;
    }
    char buf[sizeof(img_path)];
    form_avatar_path(buf, img_id + 1, width, height);
    return lookup_direct(map, buf);
}

ipa_image_t* lookup_image(hash_map$img_path$ipa_image_t$* map, char const* mxc_url) {
    if (!mxc_url) {
        return nullptr;
    }
    char const* img_id = strrchr(mxc_url, '/');
    if (!img_id) {
        return nullptr;
    }
    char buf[sizeof(img_path)];
    form_image_path(buf, img_id + 1);
    return lookup_direct(map, buf);
}

ipa_image_t load_image(unsigned char const* input, size_t len, enum image_format format) {
    ipa_image_t image = {.data = nullptr, .format = IMAGE_FORMAT_RGBA, .id = 0};

    int x, y, channels_in_file;
    int z = 1;
    switch (format) {
        case IMAGE_FORMAT_PNG:
        case IMAGE_FORMAT_BMP:
        case IMAGE_FORMAT_JPG:
        case IMAGE_FORMAT_GIF:
            if (format == IMAGE_FORMAT_GIF) {
                int* delays = malloc(sizeof(int));
                image.data = stbi_load_gif_from_memory(input, len, &delays, &x, &y, &z, &channels_in_file, STBI_rgb_alpha);
                if (!image.data) {
                    efuncprintf("Failed to decode stb image\n");
                    break;
                }
                image.delay = delays[0]; //not a good way of doing it, but it will work, can refine later if wanted
                free(delays);
            } else {
                image.data = stbi_load_from_memory(input, len, &x, &y, &channels_in_file, STBI_rgb_alpha);
                if (!image.data) {
                    break;
                }
            }
            image.width = x;
            image.height = y;
            image.frames = z;
            image.num_channels = STBI_rgb_alpha;
            // image.size = x * y * z * STBI_rgb_alpha;
            break;
        case IMAGE_FORMAT_WEBP:
            image.data = WebPDecodeRGBA(input, len, &x, &y);
            if (!image.data) {
                efuncprintf("Failed to decode webp\n");
                break;
            }
            image.width = x;
            image.height = y;
            image.frames = 1;
            image.num_channels = 4;
            // image.size = x * y * 4;
            image.delay = 0;
            break;
        case IMAGE_FORMAT_ETC1:
        case IMAGE_FORMAT_ETC1A4:
        case IMAGE_FORMAT_RGBA:
            //todo: figure out what to do with these, probably nothing
            break;
        case IMAGE_FORMAT_UNKNOWN:
            break;
    }
    return image;
}

ipa_image_t load_etc_image(unsigned char const* input, size_t width, size_t height, enum image_format format) {
    ipa_image_t image = {.data = nullptr, .format = IMAGE_FORMAT_ETC1A4};
    if (format != IMAGE_FORMAT_ETC1A4 && format != IMAGE_FORMAT_ETC1) {
        return image;
    }

    image.data = malloc(width * height * bytes_per_pixel(format));
    memcpy(image.data, input, width * height * bytes_per_pixel(format));
    image.width = width;
    image.height = height;
    image.frames = 1;
    image.num_channels = format == IMAGE_FORMAT_ETC1A4 ? 4 : 3;
    return image;
}

void destroy_image(ipa_image_t* image) {
    if (image->data) {
        free(image->data);
    }
    if (image->in_vram) {
        image_unload_vram(image);
        free(image->sprite);
    }
    memset(image, 0, sizeof(ipa_image_t));
}

//https://github.com/Cruel/3dstex/blob/5cdd9a149239a54242368e604810ed0de6ae040c/src/Encoder.cpp#L203
void encode_etc1_block(size_t block_x, size_t block_y, size_t width, size_t height, size_t width_og, size_t height_og, unsigned char const* rgba, unsigned char** block_out) {
    enum image_format target_format = IMAGE_FORMAT_ETC1A4;
    unsigned int const* rgba32 = (unsigned int*) rgba;
    uint64_t block = 0; //god do I HATE this, I should really reorganize this into a standard 8 unsigned char array
    unsigned int pixels[4 * 4] = {0};
    unsigned char* pixels_b = (unsigned char*)pixels;
    unsigned char alpha = 0;
    size_t alpha_count = 0;

    // for (size_t i = 0; i < 4 * 4; i++) {
    //     pixels_b[i * 4 + 3] = 0xFF;
    // }

    for (size_t y = 0; y < 4; y++) {
        for (size_t x = 0; x < 4; x++) {
            size_t x_pos = block_x + x;
            // size_t y_pos = height - 1 - (block_y + y);
            size_t y_pos = (block_y + y);

            if (x_pos < width_og && y_pos < height_og) {
                pixels[y * 4 + x] = rgba32[y_pos * width_og + x_pos];
                pixels_b[(y * 4 + x) * 4 + 3] = 0xFF; //etc encode freaks out with non solid alpha
            }

            //ETC1A4 stuff
            if (target_format == IMAGE_FORMAT_ETC1A4) {
                x_pos = block_x + y;
                // y_pos = height - 1 - (block_y + x);
                y_pos = (block_y + x);
                alpha = 0;
                if (x_pos < width_og && y_pos < height_og) {
                    alpha = rgba[(y_pos * width_og + x_pos) * 4 + 3];
                }
                alpha >>= 4;
                block |= ((uint64_t)alpha) << (alpha_count * 4);
                alpha_count++;
            }
        }
    }
    //also ETC1A4 stff
    if (target_format == IMAGE_FORMAT_ETC1A4) {
        memcpy(*block_out, &block, sizeof(uint64_t));
        *block_out += sizeof(uint64_t);
    }

    pack_etc1_block(&block, pixels, ETC_MEDIUM_QUALITY, false);
    block = __bswap64(block);
    memcpy(*block_out, &block, sizeof(uint64_t));
    *block_out += sizeof(uint64_t);
}

int convert_to_etc(ipa_image_t* image) {
    size_t width = next_power_of_two(image->width);
    size_t height = next_power_of_two(image->height);

    if (image->format != IMAGE_FORMAT_RGBA) {
        efuncprintf("Image is not in RGBA format\n");
        return -1;
    }
    unsigned char* data = malloc(width * height * bytes_per_pixel(IMAGE_FORMAT_ETC1A4));

    pack_etc1_block_init();

    unsigned char* conv = data;
    for (size_t y = 0; y < image->height; y += 8) {
        for (size_t x = 0; x < image->width; x += 8) {
            for (size_t i = 0; i < 8; i += 4) { //4x4 blocks
                for (size_t j = 0; j < 8; j += 4) {
                    encode_etc1_block(x + j, y + i, width, height, image->width, image->height, image->data, &conv);
                }
            }
        }
    }
    free(image->data);
    image->data = data;
    image->format = IMAGE_FORMAT_ETC1A4;
    image->num_channels = image->format == IMAGE_FORMAT_ETC1A4 ? 4 : 3;
    image->width = width;
    image->height = height;

    return 0;
}

int resize_compress(ipa_image_t* image, size_t width, size_t height) {
    if (!image->data || image->format != IMAGE_FORMAT_RGBA) {
        efuncprintf("Image is not in RGBA format\n");
        return -1;
    }
    void* resized_pixels = stbir_resize_uint8_srgb(image->data, image->width, image->height, 0, nullptr, width, height, 0, STBIR_RGBA);
    if (!resized_pixels) {
        efuncprintf("Failed to resize image\n");
        return -1;
    }
    // destroy_image(image);
    image->data = resized_pixels;
    image->width = width;
    image->height = height;
    image->frames = 1;
    image->format = IMAGE_FORMAT_RGBA;
    image->num_channels = 4;

    convert_to_etc(image);
    if (!image->data) {
        efuncprintf("Failed to convert image\n");
    }
    return 0;
}

ipa_image_t load_resize_compress_save(unsigned char const* input, size_t len, enum image_format format, size_t width, size_t height, img_path const path) {
    ipa_image_t image = load_image(input, len, format);
    if (!image.data) {
        efuncprintf("Failed to load image\n");
        return image;
    }
    resize_compress(&image, width, height);
    if (!image.data) {
        efuncprintf("Failed to resize and compress image\n");
        return image;
    }

    // form_image_path(image.path, image_id, image.width, image.height);
    // strncpy(image.path, path, sizeof(image.path));
    FILE* f = fopen(path, "wb");
    if (!f) {
        efuncprintf("Failed to open file: %s", path);
        // memset(image.path, 0, sizeof(image.path));
        return image;
    }
    size_t const to_write = image.width * image.height * image.num_channels;
    size_t written = fwrite(image.data, 1, to_write, f);
    fclose(f);
    if (written < to_write) {
        efuncprintf("Failed to write image to file: %s", path);
        unlink(path);
        // memset(image.path, 0, sizeof(image.path));
    }

    return image;
}

void image_load_vram(ipa_image_t* image) {
    if (!image || image->in_vram || !image->data || (image->format != IMAGE_FORMAT_ETC1 && image->format != IMAGE_FORMAT_ETC1A4)) {
        return;
    }
    if (!image->sprite) { //unloading from vram won't actually free the sprite pointer in the event reuse is needed
        image->sprite = malloc(sizeof(C2D_Sprite));
    }
    size_t width_2 = next_power_of_two(image->width);
    size_t height_2 = next_power_of_two(image->height);
    C2D_Image image_c2d;
    image_c2d.tex = malloc(sizeof(C3D_Tex));
    Tex3DS_SubTexture* subtex = malloc(sizeof(Tex3DS_SubTexture));
    image_c2d.subtex = subtex;
    subtex->width = image->width;
    subtex->height = image->height;
    subtex->left   = 0.0f;
    subtex->right  = (float)image->width  / width_2;
    subtex->top    = (float)image->height / height_2;
    subtex->bottom = 0.0f;

    C3D_TexInit(image_c2d.tex, width_2, height_2, image->format == IMAGE_FORMAT_ETC1A4 ? GPU_ETC1A4 : GPU_ETC1);
    C3D_TexSetFilter(image_c2d.tex, GPU_NEAREST, GPU_NEAREST);
    image_c2d.tex->border = C2D_Color32c(0xFF, 0xFF, 0xFF, 0xFF);
    C3D_TexSetWrap(image_c2d.tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);

    C3D_TexUpload(image_c2d.tex, image->data);

    C2D_SpriteFromImage(image->sprite, image_c2d);
    image->in_vram = true;
}

void image_unload_vram(ipa_image_t* image) {
    if (!image->in_vram) {
        return;
    }
    free(image->sprite->image.tex);
    free((void*)image->sprite->image.subtex); //Silence const cast warning
    memset(image->sprite, 0, sizeof(C2D_Sprite));
    image->in_vram = false;
}
