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

void flush_files() {
    fflush(stdout);
    fflush(stderr);
}

int main(int argc, char** argv) {
    freopen(SAVE_DIR"stdout.txt", "w", stdout);
    freopen(SAVE_DIR"stderr.txt", "w", stderr);
    atexit(flush_files);

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
    matrix_login_from_save(&client.login, false);
    if (!client.login.logged_in) {
        eprintf("Refreshing login failed, attempting password login\n");
        matrix_login_pass(&client.login, "toasterwaffle.win", "alice", "STRIPPED_OUT", "IPA-testing");
    }
    matrix_client_sync_account_data(&client);
    matrix_client_sync_directs(&client);

    matrix_client_sync_dump(&client);
    printf("Logged in and synced, good luck reading it\n");

    matrix_client_set_and_sync_room(&client, &client.rooms.data[0]);
    client.menu = MENU_CHAT;

    while (aptMainLoop()) {
        // hidScanInput();
        //
        // u32 kDown = hidKeysDown();
        //
        // if (kDown & KEY_START)
        //     break;

        // Flush and swap framebuffers
        // gfxFlushBuffers();
        // gfxSwapBuffers();

        // Render the scene
        draw_and_input(&client);

        gspWaitForVBlank();
    }

    destroy_curl();
    gfxExit();
    return 0;
}
