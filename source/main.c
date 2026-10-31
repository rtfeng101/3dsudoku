// sudoku: a sudoku game for the 3DS/2DS.
//
// Skeleton only: the main loop and the function stubs are wired up.
// Fill in every TODO. Press START to quit.

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

// Screen sizes in pixels. (0, 0) is the top-left corner of each screen.
#define TOP_WIDTH     400
#define BOTTOM_WIDTH  320
#define SCREEN_HEIGHT 240	// both screens are the same height

// Bottom screen layout, shared by draw_bottom (drawing) and handle_input (touch hitboxes).
// 3x3 box
#define CELL_SIZE 48
#define BOX_SIZE  (3 * CELL_SIZE)	// 144
#define BOX_X     10	// 8px left margin + 2px border
#define BOX_Y     42	// leaves room above for the settings button and "Box N" label
// number pad
#define KEY_SIZE  44
#define KEY_GAP   8
#define PAD_SIZE  (3 * KEY_SIZE + 2 * KEY_GAP)	// 148, same as the box with its border
#define PAD_X     (BOX_X + BOX_SIZE + 10)	// 164: 8px gap after the border, ends at x=311 (8px right margin)
#define PAD_Y     (BOX_Y + (BOX_SIZE - PAD_SIZE) / 2)	// 40: centered on the box, so it lines up with the border
// Pen | Pencil switch and Erase button
#define TOOL_Y    (BOX_Y + BOX_SIZE + 10)	// 196, 8px gap after the bottom border
#define TOOL_H    36	// ends at y=231 (8px bottom margin)
#define SWITCH_X  (BOX_X - 2)	// 8, lines up with the box's outer border
#define SWITCH_W  (BOX_SIZE + 4)	// 148, the box plus its border
#define HALF_W    (SWITCH_W / 2)	// 74, Pen | Pencil
// settings button
#define SET_X     8
#define SET_Y     8
#define SET_SIZE  24	// ends at y=31, 8px above the box border

// Colors, shared by both screens. #define because C2D_Color32 isn't a compile-time constant in C.
#define WHITE               C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)	// screen background
#define CELL_COLOR          C2D_Color32(0xF4, 0xF1, 0xEA, 0xFF)	// cream, so the cells show on the white background
#define THIN_COLOR          C2D_Color32(0xC8, 0xC4, 0xBC, 0xFF)	// light gray, between cells
#define THICK_COLOR         C2D_Color32(0x3A, 0x3A, 0x44, 0xFF)	// dark, around the box
#define HIGHLIGHT_COLOR     C2D_Color32(0xAD, 0xD8, 0xE6, 0xFF)	// pastel blue, highlight
#define BOX_HIGHLIGHT_COLOR C2D_Color32(0xD6, 0xEB, 0xF2, 0xFF)	// HIGHLIGHT_COLOR halfway to white, the cursor's 3x3 box
#define PEN_COLOR           C2D_Color32(0x2B, 0x4C, 0x9E, 0xFF)	// dark blue, player numbers that aren't locked in yet

// 0 means an empty cell.
int board[9][9];
// true for cells that came with the puzzle or got locked in (the player can't change them).
bool fixed[9][9];

int cursorRow = 0, cursorCol = 0;
bool pencil = false;

// The answer to the puzzle in load_puzzle, used to lock in correct rows, columns and boxes.
// shortcut: hardcoded alongside the puzzle, replace with a solver when you add more puzzles
static const int solution[9][9] = {
	{5, 3, 4,  6, 7, 8,  9, 1, 2},
	{6, 7, 2,  1, 9, 5,  3, 4, 8},
	{1, 9, 8,  3, 4, 2,  5, 6, 7},

	{8, 5, 9,  7, 6, 1,  4, 2, 3},
	{4, 2, 6,  8, 5, 3,  7, 9, 1},
	{7, 1, 3,  9, 2, 4,  8, 5, 6},

	{9, 6, 1,  5, 3, 7,  2, 8, 4},
	{2, 8, 7,  4, 1, 9,  6, 3, 5},
	{3, 4, 5,  2, 8, 6,  1, 7, 9},
};

// Puts a starting puzzle into board[][] and marks its cells in fixed[][].
void load_puzzle(void)
{
	// shortcut: one hardcoded puzzle, swap for a list or generator when you want variety
	static const int puzzle[9][9] = {
		{5, 3, 0,  0, 7, 0,  0, 0, 0},
		{6, 0, 0,  1, 9, 5,  0, 0, 0},
		{0, 9, 8,  0, 0, 0,  0, 6, 0},

		{8, 0, 0,  0, 6, 0,  0, 0, 3},
		{4, 0, 0,  8, 0, 3,  0, 0, 1},
		{7, 0, 0,  0, 2, 0,  0, 0, 6},

		{0, 6, 0,  0, 0, 0,  2, 8, 0},
		{0, 0, 0,  4, 1, 9,  0, 0, 5},
		{0, 0, 0,  0, 8, 0,  0, 7, 9},
	};

	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			board[row][col] = puzzle[row][col];
			fixed[row][col] = puzzle[row][col] != 0;	// given numbers can't be changed
		}
	}
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

// true when (px, py) is inside the rect (x, y, w, h), same pixels C2D_DrawRectSolid fills
bool inside(int px, int py, int x, int y, int w, int h)
{
	return px >= x && px < x + w && py >= y && py < y + h;
}

// After a pen entry at (row, col): if its row, column or box matches the solution, lock those 9 cells.
void lock_groups(int row, int col)
{
	int boxRow = row / 3 * 3, boxCol = col / 3 * 3;	// top-left cell of the box
	bool rowOk = true, colOk = true, boxOk = true;

	// check: any cell that differs from solution[][] makes its group not ok
	for (int i = 0; i < 9; i++) {
		// check row
		if (board[row][i] != solution[row][i]) rowOk = false;
		// check col
		if (board[i][col] != solution[i][col]) colOk = false;
		// box: board[boxRow + i / 3][boxCol + i % 3]
		if (board[boxRow + i / 3][boxCol + i % 3] != solution[boxRow + i / 3][boxCol + i % 3]) boxOk = false;
	}

	// lock: set fixed = true for every cell of each ok group
	for (int i = 0; i < 9; i++) {
		if (rowOk) fixed[row][i] = true;
		if (colOk) fixed[i][col] = true;
		if (boxOk) fixed[boxRow + i/3][boxCol + i % 3] = true;
	}
}

// D-pad moves the cursor, the circle pad jumps a box, the touch screen picks numbers and tools,
// B erases, L/R flip Pen | Pencil. kDown = buttons pressed this frame (from hidKeysDown()).
void handle_input(u32 kDown)
{
	// set by steps 2-3: 1-9 from the pad, -1 for erase, 0 if nothing was pressed
	int num = 0;

	// step 1: D-pad moves the cursor around the 9x9 board
	if (kDown & KEY_DUP) {
		cursorRow = (cursorRow + 8) % 9;
	}
	if (kDown & KEY_DDOWN) {
		cursorRow = (cursorRow + 1) % 9;
	}
	if (kDown & KEY_DLEFT) {
		cursorCol = (cursorCol + 8) % 9;
	}
	if (kDown & KEY_DRIGHT) {
		cursorCol = (cursorCol + 1) % 9;
	}

	// circle pad jumps a whole box, keeping the spot inside the box
	if (kDown & KEY_CPAD_UP) {
		cursorRow = (cursorRow + 6) % 9;
	}
	if (kDown & KEY_CPAD_DOWN) {
		cursorRow = (cursorRow + 3) % 9;
	}
	if (kDown & KEY_CPAD_LEFT) {
		cursorCol = (cursorCol + 6) % 9;
	}
	if (kDown & KEY_CPAD_RIGHT) {
		cursorCol = (cursorCol + 3) % 9;
	}

	// step 2: buttons for the tools
	if (kDown & (KEY_L | KEY_R)) {
		pencil = !pencil;
	}
	if (kDown & KEY_B) {
		num = -1;
	}

	// step 3: touch, only on the frame the stylus lands. Each hitbox is the same rect draw_bottom draws.
	if (kDown & KEY_TOUCH) {
		touchPosition touch;
		hidTouchRead(&touch);	// touch.px, touch.py are bottom-screen pixels
		int px = touch.px;
		int py = touch.py;

		// cells of the 3x3 box: select that cell
		int boxRow = cursorRow / 3;
		int boxCol = cursorCol / 3;	// the box on screen, read before the cursor moves
		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 3; col++) {
				if (inside(px, py, BOX_X + col * CELL_SIZE, BOX_Y + row * CELL_SIZE, CELL_SIZE, CELL_SIZE)) {
					cursorRow = boxRow * 3 + row;
					cursorCol = boxCol * 3 + col;
				}
			}
		}

		// pad keys: enter that number
		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 3; col++) {
				if (inside(px, py, PAD_X + col * (KEY_SIZE + KEY_GAP), PAD_Y + row * (KEY_SIZE + KEY_GAP), KEY_SIZE, KEY_SIZE)) {
					num = col + 1 + row * 3;
				}
			}
		}

		// Pen half of the switch
		if (inside(px, py, SWITCH_X, TOOL_Y, HALF_W, TOOL_H)) {
			pencil = false;
		}
		// Pencil half of the switch
		if (inside(px, py, SWITCH_X + HALF_W, TOOL_Y, HALF_W, TOOL_H)) {
			pencil = true;
		}

		// Erase button
		if (inside(px, py, PAD_X, TOOL_Y, PAD_SIZE, TOOL_H)) {
			num = -1;
		}

		// settings button (nothing to open yet)
		if (inside(px, py, SET_X, SET_Y, SET_SIZE, SET_SIZE)) {
			// TODO: open settings
		}
	}

	// step 4: put num in the cursor's cell
	// no action or fixed cell
	if (num == 0 || fixed[cursorRow][cursorCol])
		return;

	// erase (num == -1) clears the cell in either mode
	if (num == -1) {
		board[cursorRow][cursorCol] = 0;
		return;
	}

	// num is 1-9 from here on
	if (!pencil) {
		board[cursorRow][cursorCol] = num;
		lock_groups(cursorRow, cursorCol);
	} else {
		// pencil: notes come later
	}
}

// Draws `str` centered inside the rectangle (x, y, w, h).
void draw_centered(C2D_TextBuf textBuf, const char* str, float x, float y, float w, float h, float scale, u32 color)
{
	C2D_Text text;
	C2D_TextParse(&text, textBuf, str);
	C2D_TextOptimize(&text);

	float textW, textH;
	C2D_TextGetDimensions(&text, scale, scale, &textW, &textH);
	// center by hand and round down, so the text starts on a whole pixel (fractional starts blur the glyphs)
	C2D_DrawText(&text, C2D_WithColor, floorf(x + (w - textW) / 2), floorf(y + (h - textH) / 2), 0, scale, scale, color);
}

// bottom screen: a zoomed view of the cursor's 3x3 box, a number pad,
// a Pen | Pencil switch and an Erase button.
void draw_bottom(C2D_TextBuf textBuf)
{
	// step 1: draw background for the 3x3 box
	C2D_DrawRectSolid(BOX_X, BOX_Y, 0, BOX_SIZE, BOX_SIZE, CELL_COLOR);

	// highlight the current selected cell
	C2D_DrawRectSolid(BOX_X + (cursorCol % 3) * CELL_SIZE, BOX_Y + (cursorRow % 3) * CELL_SIZE, 0, CELL_SIZE, CELL_SIZE, HIGHLIGHT_COLOR);

	// step 2: grid lines, drawn as thin rectangles
	C2D_DrawRectSolid(BOX_X + CELL_SIZE, BOX_Y, 0, 1, BOX_SIZE, THIN_COLOR);	// vertical
	C2D_DrawRectSolid(BOX_X + 2 * CELL_SIZE, BOX_Y, 0, 1, BOX_SIZE, THIN_COLOR);	// vertical
	C2D_DrawRectSolid(BOX_X, BOX_Y + CELL_SIZE, 0, BOX_SIZE, 1, THIN_COLOR);	// horizontal
	C2D_DrawRectSolid(BOX_X, BOX_Y + 2 * CELL_SIZE, 0, BOX_SIZE, 1, THIN_COLOR);	// horizontal

	// border last
	C2D_DrawRectSolid(BOX_X - 2, BOX_Y - 2, 0, 2, BOX_SIZE + 4, THICK_COLOR); // left
	C2D_DrawRectSolid(BOX_X + BOX_SIZE, BOX_Y - 2, 0, 2, BOX_SIZE + 4, THICK_COLOR); // right
	C2D_DrawRectSolid(BOX_X - 2, BOX_Y - 2, 0, BOX_SIZE + 4, 2, THICK_COLOR); // top
	C2D_DrawRectSolid(BOX_X - 2, BOX_Y + BOX_SIZE, 0, BOX_SIZE + 4, 2, THICK_COLOR); // bottom
	
	
	// step 3: 3x3 number pad to the right of the box, with gaps between keys
	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			C2D_DrawRectSolid(PAD_X + col * (KEY_SIZE + KEY_GAP), PAD_Y + row * (KEY_SIZE + KEY_GAP), 0, KEY_SIZE, KEY_SIZE, CELL_COLOR);
		}
	}

	// step 4: Pen | Pencil switch under the box, Erase button under the pad
	// switch
	C2D_DrawRectSolid(SWITCH_X, TOOL_Y, 0, SWITCH_W, TOOL_H, CELL_COLOR);
	C2D_DrawRectSolid(SWITCH_X + pencil * HALF_W, TOOL_Y, 0, HALF_W, TOOL_H, THICK_COLOR); // select switch

	// erase button
	C2D_DrawRectSolid(PAD_X, TOOL_Y, 0, PAD_SIZE, TOOL_H, CELL_COLOR);

	// settings button in the top left, with a ☰ icon
	C2D_DrawRectSolid(SET_X, SET_Y, 0, SET_SIZE, SET_SIZE, CELL_COLOR);
	for (int i = 0; i < 3; i++) {
		C2D_DrawRectSolid(SET_X + 5, SET_Y + 6 + i * 5, 0, 14, 2, THICK_COLOR);	// 3 bars, 14x2, 5px apart
	}

	// step 5: text for pad digits, button labels, "Box N" label
	// label
	int boxRow = cursorRow / 3;
	int boxCol = cursorCol / 3;
	
	char label[6];
	snprintf(label, sizeof label, "Box %d", (boxCol + 1 + boxRow * 3));
	draw_centered(textBuf, label, 0, SET_Y, BOTTOM_WIDTH, SET_SIZE, 1.0f, THICK_COLOR);
	
	// keypad
	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			char buf[4];
			snprintf(buf, sizeof buf, "%d", row * 3 + col + 1);
			draw_centered(textBuf, buf, PAD_X + col * (KEY_SIZE + KEY_GAP), PAD_Y + row * (KEY_SIZE + KEY_GAP), KEY_SIZE, KEY_SIZE, 1.0f, THICK_COLOR);
		}
	}

	// tools
	draw_centered(textBuf, "Pen", SWITCH_X, TOOL_Y, HALF_W, TOOL_H, 0.8f, pencil ? THICK_COLOR : THIN_COLOR);
	draw_centered(textBuf, "Pencil", SWITCH_X + HALF_W, TOOL_Y, HALF_W, TOOL_H, 0.8f, pencil ? THIN_COLOR : THICK_COLOR);
	draw_centered(textBuf, "Erase", PAD_X, TOOL_Y, PAD_SIZE, TOOL_H, 0.8f, THICK_COLOR);

	// step 6: draw the digits from board[][] for the cursor's box

	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			int digit = board[boxRow * 3 + row][boxCol * 3 + col];
			if (digit == 0)
				continue;
			char buf[4];
			snprintf(buf, sizeof buf, "%d", digit); // if the digit is 0, print nothing
			draw_centered(textBuf, buf, BOX_X + col * CELL_SIZE, BOX_Y + row * CELL_SIZE, CELL_SIZE, CELL_SIZE, 1.0f, fixed[boxRow * 3 + row][boxCol * 3 + col] ? THICK_COLOR : PEN_COLOR);
		}
	}
}

// TODO: top screen: the whole 9x9 board, the title on the left,
// "Solved!" on the right.
void draw_top(C2D_TextBuf textBuf)
{
	// layout: the 9x9 board centered, a panel on each side. Lines get their own pixels, so every cell is the same size.
	const int cellSize = 23;
	const int boardSize = 9 * cellSize + 6 * 1 + 4 * 2;	// 221: 9 cells, 6 thin lines, 4 thick lines (border included)
	const int boardX = (TOP_WIDTH - boardSize) / 2;	// 89, the width of each side panel
	const int boardY = (SCREEN_HEIGHT - boardSize) / 2;	// 9, top and bottom margin

	// pos[i]: where cell i starts, from the board's edge. pos[9] = boardSize, the far edge.
	int pos[10];
	for (int i = 0; i < 10; i++) {
		pos[i] = 2 + i * (cellSize + 1) + i / 3;	// border, cells + 1px lines, +1 more per thick line passed
	}

	// step 1: background for the whole board (CELL_COLOR)
	C2D_DrawRectSolid(boardX, boardY, 0, boardSize, boardSize, CELL_COLOR);

	// step 2: highlight the cursor's block and cell
	int boxCol = cursorCol / 3 * 3;
	int boxRow = cursorRow / 3 * 3;	
	C2D_DrawRectSolid(boardX + pos[boxCol], boardY + pos[boxRow], 0, 3 * cellSize + 2, 3 * cellSize + 2, BOX_HIGHLIGHT_COLOR);	// 3 cells + 2 thin lines
	C2D_DrawRectSolid(boardX + pos[cursorCol], boardY + pos[cursorRow], 0, cellSize, cellSize, HIGHLIGHT_COLOR);

	// step 3: thin lines between cells, 1px, just before each cell
	// left right
	for (int row = 1; row < 9; row++) {
		C2D_DrawRectSolid(boardX, boardY + pos[row] - 1, 0, boardSize, 1, THIN_COLOR);
	}
	// top bottom
	for (int col = 1; col < 9; col++) {
		C2D_DrawRectSolid(boardX + pos[col] - 1, boardY, 0, 1, boardSize, THIN_COLOR);
	}

	// step 4: thick lines every 3 cells + the outer border, 2px, just before cells 0, 3, 6 and the far edge
	// left right
	for (int row = 0; row < 4; row++) {
		C2D_DrawRectSolid(boardX, boardY + pos[row * 3] - 2, 0, boardSize, 2, THICK_COLOR);
	}
	// top bottom
	for (int col = 0; col < 4; col++) {
		C2D_DrawRectSolid(boardX + pos[col * 3] - 2, boardY, 0, 2, boardSize, THICK_COLOR);
	}

	// step 5: digits from board[][], skip 0s (fixed[row][col] ? THICK_COLOR : PEN_COLOR)
	// cell (row, col) is at boardX + pos[col], boardY + pos[row], cellSize x cellSize
	for (int row = 0; row < 9; row++) {
		for (int col = 0; col < 9; col++) {
			int digit = board[row][col];
			if (digit == 0) continue;

			char buf[4];
			snprintf(buf, sizeof buf, "%d", digit);
			draw_centered(textBuf, buf, boardX + pos[col], boardY + pos[row], cellSize, cellSize, 0.6f, fixed[row][col] ? THICK_COLOR : PEN_COLOR);
		}
	}

	// step 6: left panel, "Sudoku" and Pen / Pencil, centered in the space left of the board
	draw_centered(textBuf, "Sudoku", 0, boardY, boardX, 24, 0.8f, THICK_COLOR);

	// step 7: right panel, "Solved!" when is_solved(), on the same line as the title
	if (is_solved()) {
		draw_centered(textBuf, "Solved!", boardX + boardSize, boardY, TOP_WIDTH - boardX - boardSize, 24, 0.8f, THICK_COLOR);
	}
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

		C2D_TargetClear(top, WHITE);
		C2D_SceneBegin(top);
		draw_top(textBuf);

		C2D_TargetClear(bottom, WHITE);
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
