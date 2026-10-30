// sudoku: a sudoku game for the 3DS/2DS.
//
// Starting point: sets up both screens and runs the main loop.
// Press START to quit.

#include <3ds.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
	gfxInitDefault();

	// Top screen is 50x30 characters, bottom screen is 40x30.
	PrintConsole topScreen, bottomScreen;
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);

	consoleSelect(&topScreen);
	printf("\x1b[2;2H\x1b[36mSudoku\x1b[0m");
	printf("\x1b[28;2HPress \x1b[32mSTART\x1b[0m to exit.");

	consoleSelect(&bottomScreen);
	printf("\x1b[2;2HThe board will go here.");

	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	}

	gfxExit();
	return 0;
}
