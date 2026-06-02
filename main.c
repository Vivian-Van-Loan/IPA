#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include <malloc.h>

#include <curl/curl.h>
#include <jansson.h>

#include "utils.h"
#include "mclient/matrix-client.h"

int main(int argc, char** argv) {
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	int res = init_curl();
	if (res) {
		return res;
	}

	curl_easy_setopt(curlhandle, CURLOPT_URL, "https://toasterwaffle.win/");

	/* Perform the request, 'res' holds the return code */
	res = curl_easy_perform(curlhandle);
	/* Check for errors */
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}

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
