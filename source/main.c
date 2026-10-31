// sudoku: a sudoku game for the 3DS/2DS.
//
// Skeleton only: the main loop and the function stubs are wired up.
// Fill in every TODO. Press START to quit.

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdbool.h>

// Screen sizes in pixels. (0, 0) is the top-left corner of each screen.
#define TOP_WIDTH     400
#define BOTTOM_WIDTH  320
#define SCREEN_HEIGHT 240	// both screens are the same height

// 0 means an empty cell.
int board[9][9];
// true for cells that came with the puzzle (the player can't change them).
bool fixed[9][9];

int cursorRow = 0, cursorCol = 0;
bool pencil = false;

// TODO: put a starting puzzle into board[][] and mark its cells in fixed[][].
void load_puzzle(void)
{
}

// TODO: can `num` go at (row, col)? Check the row, the column and the 3x3 box.
bool is_valid(int row, int col, int num)
{
	return true;
}

// TODO: true when every cell is filled (and valid).
bool is_solved(void)
{
	return false;
}

// TODO: D-pad moves the cursor, A/B/X/Y (or the touch screen) change the
// number in the selected cell. hidKeysDown() gives you the buttons pressed this frame.
void handle_input(u32 kDown)
{
}

// TODO: draw the 9x9 grid, the numbers and the cursor on the bottom screen.
// Shapes: C2D_DrawRectSolid(x, y, depth, w, h, color), C2D_DrawLine(...).
// Numbers: C2D_TextParse(&text, textBuf, "5"), C2D_TextOptimize(&text),
// then C2D_DrawText(&text, C2D_WithColor, x, y, depth, scaleX, scaleY, color).
void draw_bottom(C2D_TextBuf textBuf)
{
}

// TODO: draw the top screen: the title, "Press START to exit",
// and a "Solved!" message once is_solved() is true. Same text calls as draw_bottom.
void draw_top(C2D_TextBuf textBuf)
{
}

int main(int argc, char* argv[])
{
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	C2D_TextBuf textBuf = C2D_TextBufNew(256);	// room for 256 glyphs per frame
	u32 white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

	load_puzzle();

	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;

		handle_input(kDown);

		C2D_TextBufClear(textBuf);
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		C2D_TargetClear(top, white);
		C2D_SceneBegin(top);
		draw_top(textBuf);

		C2D_TargetClear(bottom, white);
		C2D_SceneBegin(bottom);
		draw_bottom(textBuf);

		C3D_FrameEnd(0);	// also swaps buffers and waits for vblank
	}

	C2D_TextBufDelete(textBuf);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
