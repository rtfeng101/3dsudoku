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

// TODO: bottom screen: a zoomed view of the cursor's 3x3 box, a number pad,
// a Pen | Pencil switch and an Erase button.
void draw_bottom(C2D_TextBuf textBuf)
{
	// box label, 3x3 grid + numpad, tools
	int cellSize = 48;
	int boxSize = 3 * cellSize;	// 144
	int boxX = 10;	// 8px left margin + 2px border
	int boxY = 42;	// leaves room above for the settings button and "Box N" label

	u32 cellColor  = C2D_Color32(0xF4, 0xF1, 0xEA, 0xFF);	// cream, so the cells show on the white background
	u32 thinColor  = C2D_Color32(0xC8, 0xC4, 0xBC, 0xFF);	// light gray, between cells
	u32 thickColor = C2D_Color32(0x3A, 0x3A, 0x44, 0xFF);	// dark, around the box

	// step 1: draw background for the 3x3 box
	C2D_DrawRectSolid(boxX, boxY, 0, boxSize, boxSize, cellColor);

	// step 2: grid lines, drawn as thin rectangles
	C2D_DrawRectSolid(boxX + cellSize, boxY, 0, 1, boxSize, thinColor);	// vertical
	C2D_DrawRectSolid(boxX + 2 * cellSize, boxY, 0, 1, boxSize, thinColor);	// vertical
	C2D_DrawRectSolid(boxX, boxY + cellSize, 0, boxSize, 1, thinColor);	// horizontal
	C2D_DrawRectSolid(boxX, boxY + 2 * cellSize, 0, boxSize, 1, thinColor);	// horizontal

	// border last
	C2D_DrawRectSolid(boxX - 2, boxY - 2, 0, 2, boxSize + 4, thickColor); // left
	C2D_DrawRectSolid(boxX + boxSize, boxY - 2, 0, 2, boxSize + 4, thickColor); // right
	C2D_DrawRectSolid(boxX - 2, boxY - 2, 0, boxSize + 4, 2, thickColor); // top
	C2D_DrawRectSolid(boxX - 2, boxY + boxSize, 0, boxSize + 4, 2, thickColor); // bottom
	
	
	// step 3: 3x3 number pad to the right of the box, with gaps between keys
	int keySize = 44;
	int keyGap = 8;
	int padSize = 3 * keySize + 2 * keyGap;	// 148, same as the box with its border
	int padX = boxX + boxSize + 10;	// 164: 8px gap after the border, ends at x=311 (8px right margin)
	int padY = boxY + (boxSize - padSize) / 2;	// 40: centered on the box, so it lines up with the border

	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			C2D_DrawRectSolid(padX + col * (keySize + keyGap), padY + row * (keySize + keyGap), 0, keySize, keySize, cellColor);
		}
	}

	// step 4: Pen | Pencil switch under the box, Erase button under the pad
	int toolY = boxY + boxSize + 10;	// 196px, 8px gap after the bottom border
	int toolH = 36;	// 36px, ends at y=231 (8px bottom margin)
	int switchX = boxX - 2;	// 8px,	 lines up with the box's outer border
	int switchW = boxSize + 4;	// 148px, the box plus its border
	int halfW = switchW / 2;	// 74px, Pen | Pencil

	// switch
	C2D_DrawRectSolid(switchX, toolY, 0, switchW, toolH, cellColor);
	C2D_DrawRectSolid(switchX + pencil * halfW, toolY, 0, halfW, toolH, thickColor); // select switch

	// erase button
	C2D_DrawRectSolid(padX, toolY, 0, padSize, toolH, cellColor);

	// settings button in the top left, with a ☰ icon
	int setX = 8, setY = 8, setSize = 24;	// ends at y=31, 8px above the box border
	C2D_DrawRectSolid(setX, setY, 0, setSize, setSize, cellColor);
	for (int i = 0; i < 3; i++) {
		C2D_DrawRectSolid(setX + 5, setY + 6 + i * 5, 0, 14, 2, thickColor);	// 3 bars, 14x2, 5px apart
	}

	// step 5: text (C2D_TextParse + C2D_DrawText): pad digits, button labels, "Box N" label.

	// step 6: draw the digits from board[][] for the cursor's box.

	// step 7: highlight the cursor's cell (in step 1, before the lines).
}

// TODO: top screen: the whole 9x9 board, the title and mode on the left,
// "Solved!" on the right.
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
