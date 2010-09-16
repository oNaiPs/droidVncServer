#define OUT_T CONCAT3E(uint,OUT,_t)
#define FUNCTION CONCAT2E(FUNCNAME,OUT)

static void rfbRotate(rfbScreenInfoPtr vncbuf,rfbScreenInfoPtr fbmmap)
{
	uint_32_t * buffer = (uint_32_t*)screen->frameBuffer;
	int i, j, w = screen->width, h = screen->height;
	uint_32_t * newBuffer = (uint_32_t*)malloc(w * h * sizeof(uint_32_t));

	for (j = 0; j < h; j++)
		for (i = 0; i < w; i++)
			newBuffer[(h - 1 - j + i * h)] = buffer[i + j * w];

	memcpy(buffer, newBuffer, w * h * sizeof(uint_32_t));
	free(newBuffer);

	rfbMarkRectAsModified(screen, 0, 0, screen->width, screen->height);
}

#if OUT == 32
void FUNCNAME(rfbScreenInfoPtr screen) {
	if (screen->serverFormat.bitsPerPixel == 32)
		CONCAT2E(FUNCNAME,32)(screen);
	else if (screen->serverFormat.bitsPerPixel == 16)
		CONCAT2E(FUNCNAME,16)(screen);
	else if (screen->serverFormat.bitsPerPixel == 8)
		CONCAT2E(FUNCNAME,8)(screen);
	else {
		rfbErr("Unsupported pixel depth: %d\n",
			screen->serverFormat.bitsPerPixel);
		return;
	}
}
#endif

#undef FUNCTION
#undef OUT

