#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include <malloc.h>

#include <curl/curl.h>
#include <jansson.h>

#include "utils.h"
#include "matrix/matrix-client.h"
#include "matrix/matrix-utils.h"
#include "matrix/login.h"

int main(int argc, char** argv) {
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	int res = init_general();
	if (res) {
		return res;
	}

	res = init_curl();
	if (res) {
		return res;
	}

	matrix_login_t login = matrix_login_pass("server", "user", "pass", nullptr);
	printf("Logged in as: %s\n\tto server: %s\n\tto with device_id: %s\n\tand access token: %s", login.user_id, login.homeserver, login.device_id, login.access_token);

	while (aptMainLoop()) {
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		gspWaitForVBlank();
	}

	destroy_curl();
	gfxExit();
	return 0;
}
