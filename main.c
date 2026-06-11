#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include <malloc.h>
#include <citro2d.h>

#include <curl/curl.h>
#include <jansson.h>

#include "utils.h"
#include "graphics/graphics.h"
#include "matrix/matrix-client.h"
#include "matrix/matrix-utils.h"
#include "matrix/login.h"

int main(int argc, char** argv) {
    int res = init_general();
    if (res) {
        return res;
    }
    res = init_graphics();
    if (res) {
        return res;
    }
    res = init_curl();
    if (res) {
        return res;
    }

    matrix_client_t client = make_client();
    // matrix_login_pass(&client.login, "server", "user", "pass", "IPA-testing");
    // matrix_client_sync_account_data(&client);
    // printf("Logged in and synced, good luck reading it\n");

    client.menu = MENU_CHAT;

    while (aptMainLoop()) {
        hidScanInput();

        u32 kDown = hidKeysDown();

        if (kDown & KEY_START)
            break;

        // Flush and swap framebuffers
        // gfxFlushBuffers();
        // gfxSwapBuffers();

        // Render the scene
        draw(&client);

        gspWaitForVBlank();
    }

    destroy_curl();
    gfxExit();
    return 0;
}
