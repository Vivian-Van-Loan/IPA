#ifndef MATRIX_CLIENT_GRAPHICS_H
#define MATRIX_CLIENT_GRAPHICS_H

#include <citro2d.h>

struct matrix_client_t; //can't include the client due to circular refs

#define TOP_HEIGHT 240
#define TOP_WIDTH 400
#define BOTTOM_HEIGHT 240
#define BOTTOM_WIDTH 320

// struct graphics_state_t;

#define C2D_Color32c(r, g, b, a) ((u32)r | ((u32)g << (u32)8) | ((u32)b << (u32)16) | ((u32)a << (u32)24))

int init_graphics();

void draw(struct matrix_client_t* client);

#endif //MATRIX_CLIENT_GRAPHICS_H
