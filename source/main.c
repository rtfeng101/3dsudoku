// sudoku: a sudoku game for the 3DS/2DS.
//
// Layout demo drawn with citro2d: the full board on the top screen, a zoomed
// view of the cursor's 3x3 box (with pencil marks) and a number pad on the bottom.
//
// Controls: D-pad/Circle Pad move, tap a cell or a number, Y/L/R toggle
// pen/pencil, B erases, START exits.

#include <3ds.h>
#include <citro2d.h>
#include <stdio.h>
#include <stdbool.h>

// A well-known example puzzle, row by row. 0 = empty.
static const char* PUZZLE =
	"530070000" "600195000" "098000060"
	"800060003" "400803001" "700020006"
	"060000280" "000419005" "000080079";

static u8   value[81];      // 0 = empty
static bool given[81];      // starting digits the player can't change
static u16  marks[81];      // bit n set = pencil mark n
static int  cursor = 0;     // selected cell: row * 9 + col
static bool pencil = false;

//---------------------------------------------------------------------------------
// Game logic
//---------------------------------------------------------------------------------

// True if cell i's digit also appears in its row, column or box.
static bool conflicts(int i)
{
	int v = value[i], r = i / 9, c = i % 9;
	if (!v) return false;

	for (int j = 0; j < 81; j++)
	{
		if (j == i || value[j] != v) continue;
		int jr = j / 9, jc = j % 9;
		if (jr == r || jc == c || (jr / 3 == r / 3 && jc / 3 == c / 3))
			return true;
	}
	return false;
}

static bool solved(void)
{
	for (int i = 0; i < 81; i++)
		if (!value[i] || conflicts(i)) return false;
	return true;
}

static int placedCount(int n)
{
	int count = 0;
	for (int i = 0; i < 81; i++)
		if (value[i] == n) count++;
	return count;
}

static void enter(int n)
{
	if (given[cursor]) return;

	if (pencil)
	{
		if (!value[cursor]) marks[cursor] ^= 1 << n;
	}
	else
		value[cursor] = (value[cursor] == n) ? 0 : n;  // same digit again clears it
}

static void erase(void)
{
	if (given[cursor]) return;
	value[cursor] = 0;
	marks[cursor] = 0;
}

// First cell (top-left) of the box the cursor is in.
static int boxOrigin(void)
{
	return (cursor / 27) * 27 + (cursor % 9 / 3) * 3;
}

static bool sameBox(int a, int b)
{
	return a / 27 == b / 27 && a % 9 / 3 == b % 9 / 3;
}

//---------------------------------------------------------------------------------
// Drawing. citro2d redraws both screens from scratch every frame.
//---------------------------------------------------------------------------------

#define COL_BG        C2D_Color32(0xF4, 0xF1, 0xEA, 0xFF)  // cream page
#define COL_PAPER     C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF)
#define COL_THIN      C2D_Color32(0xC8, 0xC4, 0xBC, 0xFF)  // cell lines
#define COL_THICK     C2D_Color32(0x3A, 0x3A, 0x44, 0xFF)  // box lines
#define COL_RELATED   C2D_Color32(0xE8, 0xEE, 0xF8, 0xFF)  // same row/column/box as cursor
#define COL_SAME      C2D_Color32(0xD3, 0xE0, 0xF5, 0xFF)  // same digit as cursor
#define COL_SELECTED  C2D_Color32(0xB4, 0xCD, 0xF7, 0xFF)
#define COL_CLASH_BG  C2D_Color32(0xF8, 0xDA, 0xDA, 0xFF)
#define COL_GIVEN     C2D_Color32(0x2B, 0x2B, 0x33, 0xFF)
#define COL_USER      C2D_Color32(0x2F, 0x6F, 0xDB, 0xFF)
#define COL_CLASH     C2D_Color32(0xD6, 0x45, 0x45, 0xFF)
#define COL_MARK      C2D_Color32(0x6B, 0x6B, 0x78, 0xFF)
#define COL_PENCIL    C2D_Color32(0xD9, 0x8C, 0x1F, 0xFF)  // pencil-mode accent
#define COL_PENCIL_BG C2D_Color32(0xFB, 0xEB, 0xD0, 0xFF)
#define COL_DONE      C2D_Color32(0xC8, 0xC4, 0xBC, 0xFF)  // digit already placed 9 times
#define COL_SOLVED    C2D_Color32(0x2E, 0x9E, 0x5B, 0xFF)

static C2D_TextBuf textBuf;  // cleared every frame

// Draws str with its middle at y. align: 0 (left edge at x) or C2D_AlignCenter (centered on x).
static void drawText(const char* str, float x, float y, float scale, u32 color, u32 align)
{
	C2D_Text text;
	C2D_TextParse(&text, textBuf, str);
	// The system font is 30px tall with its baseline at 25px; digits sit roughly at 7..25.
	C2D_DrawText(&text, C2D_WithColor | align, x, y - 16 * scale, 0, scale, scale, color);
}

static void drawDigit(int n, float x, float y, float scale, u32 color)
{
	char str[2] = { '0' + n, 0 };
	drawText(str, x, y, scale, color, C2D_AlignCenter);
}

// Pencil marks for cell i in a 3x3 keypad layout (1 top-left, 9 bottom-right).
static void drawMarks(int i, float x, float y, float cell, float scale)
{
	for (int n = 1; n <= 9; n++)
		if (marks[i] & (1 << n))
			drawDigit(n, x + ((n - 1) % 3 + 0.5f) * cell / 3,
			             y + ((n - 1) / 3 + 0.5f) * cell / 3, scale, COL_MARK);
}

static u32 digitColor(int i)
{
	if (conflicts(i)) return COL_CLASH;
	return given[i] ? COL_GIVEN : COL_USER;
}

static u32 cellFill(int i)
{
	int sv = value[cursor];
	if (i == cursor)                        return COL_SELECTED;
	if (conflicts(i))                       return COL_CLASH_BG;
	if (sv && value[i] == sv)               return COL_SAME;
	if (i / 9 == cursor / 9 || i % 9 == cursor % 9 || sameBox(i, cursor))
		return COL_RELATED;
	return COL_PAPER;
}

// Grid lines for an n x n grid of square cells; every `thickEvery` lines is thick.
static void drawGridLines(float x, float y, float cell, int n, int thickEvery)
{
	float size = cell * n;
	for (int pass = 0; pass < 2; pass++)  // thin lines first so thick ones cover the crossings
		for (int k = 0; k <= n; k++)
		{
			bool thick = k % thickEvery == 0;
			if (thick != pass) continue;
			float w = thick ? 2 : 1;
			C2D_DrawRectSolid(x + k * cell - w / 2, y - 1, 0, w, size + 2, thick ? COL_THICK : COL_THIN);
			C2D_DrawRectSolid(x - 1, y + k * cell - w / 2, 0, size + 2, w, thick ? COL_THICK : COL_THIN);
		}
}

// Top screen: 400x240
#define CELL  26
#define TOP_X 83   // (400 - 9 * CELL) / 2
#define TOP_Y 3    // (240 - 9 * CELL) / 2

static void drawTop(void)
{
	for (int i = 0; i < 81; i++)
	{
		float x = TOP_X + (i % 9) * CELL, y = TOP_Y + (i / 9) * CELL;
		C2D_DrawRectSolid(x, y, 0, CELL, CELL, cellFill(i));
		if (value[i]) drawDigit(value[i], x + CELL / 2, y + CELL / 2, 0.75f, digitColor(i));
		else          drawMarks(i, x, y, CELL, 0.3f);
	}
	drawGridLines(TOP_X, TOP_Y, CELL, 9, 3);

	// Left panel
	drawText("Sudoku", TOP_X / 2, 30, 0.75f, COL_GIVEN, C2D_AlignCenter);
	drawText(pencil ? "PENCIL" : "PEN", TOP_X / 2, 60, 0.55f, pencil ? COL_PENCIL : COL_USER, C2D_AlignCenter);
	drawText("Y/L/R mode", 6, 180, 0.42f, COL_MARK, 0);
	drawText("B  erase",   6, 198, 0.42f, COL_MARK, 0);
	drawText("START quit", 6, 216, 0.42f, COL_MARK, 0);

	// Right panel
	if (solved()) drawText("Solved!", (TOP_X + 9 * CELL + 400) / 2, 120, 0.7f, COL_SOLVED, C2D_AlignCenter);
}

// Bottom screen: 320x240
#define BCELL 52
#define BOX_X 8
#define BOX_Y 30
#define PAD_X 170
#define PAD_Y 30
#define KEY   44
#define GAP   6

enum { ERASE = 10, MODE = 11 };

// Touch buttons, in pixels. action: 1-9 = digit, or ERASE / MODE.
typedef struct { int x, y, w, h, action; } Button;

// The number pad uses the same layout as the pencil marks: 1 top-left, 9 bottom-right.
#define PAD(n) { PAD_X + ((n) - 1) % 3 * (KEY + GAP), PAD_Y + ((n) - 1) / 3 * (KEY + GAP), KEY, KEY, n }
static const Button buttons[] = {
	PAD(1), PAD(2), PAD(3), PAD(4), PAD(5), PAD(6), PAD(7), PAD(8), PAD(9),
	{ BOX_X, 196, 3 * BCELL, 36, MODE },
	{ PAD_X, 196, 3 * KEY + 2 * GAP, 36, ERASE },
};
#define BUTTON_COUNT (sizeof(buttons) / sizeof(buttons[0]))

static void drawPanel(float x, float y, float w, float h, u32 fill)
{
	C2D_DrawRectSolid(x, y, 0, w, h, COL_THIN);
	C2D_DrawRectSolid(x + 1, y + 1, 0, w - 2, h - 2, fill);
}

static void drawBottom(void)
{
	char title[16];
	snprintf(title, sizeof(title), "Box %d", cursor / 27 * 3 + cursor % 9 / 3 + 1);
	drawText(title, BOX_X, 14, 0.55f, COL_GIVEN, 0);

	// Zoomed box. Each cell shows a big digit or its pencil marks.
	int origin = boxOrigin();
	for (int k = 0; k < 9; k++)
	{
		int i = origin + (k / 3) * 9 + k % 3;
		float x = BOX_X + (k % 3) * BCELL, y = BOX_Y + (k / 3) * BCELL;
		C2D_DrawRectSolid(x, y, 0, BCELL, BCELL, i == cursor ? COL_SELECTED : cellFill(i));

		if (value[i])
			drawDigit(value[i], x + BCELL / 2, y + BCELL / 2, 1.4f, digitColor(i));
		else
			drawMarks(i, x, y, BCELL, 0.5f);
	}
	drawGridLines(BOX_X, BOX_Y, BCELL, 3, 3);

	for (int b = 0; b < BUTTON_COUNT; b++)
	{
		const Button* btn = &buttons[b];
		float cx = btn->x + btn->w / 2.0f, cy = btn->y + btn->h / 2.0f;

		if (btn->action == MODE)
		{
			// Two-part switch; the active half is filled.
			float half = btn->w / 2.0f;
			drawPanel(btn->x, btn->y, btn->w, btn->h, COL_PAPER);
			C2D_DrawRectSolid(btn->x + (pencil ? half : 1), btn->y + 1, 0, half - 1, btn->h - 2,
			                  pencil ? COL_PENCIL : COL_USER);
			drawText("Pen",    btn->x + half / 2,        cy, 0.6f, pencil ? COL_MARK : COL_PAPER, C2D_AlignCenter);
			drawText("Pencil", btn->x + half + half / 2, cy, 0.6f, pencil ? COL_PAPER : COL_MARK, C2D_AlignCenter);
		}
		else if (btn->action == ERASE)
		{
			drawPanel(btn->x, btn->y, btn->w, btn->h, COL_PAPER);
			drawText("Erase", cx, cy, 0.6f, COL_GIVEN, C2D_AlignCenter);
		}
		else
		{
			int n = btn->action;
			bool marked = pencil && !value[cursor] && (marks[cursor] & (1 << n));
			u32 color = placedCount(n) >= 9 ? COL_DONE : pencil ? COL_PENCIL : COL_USER;
			drawPanel(btn->x, btn->y, btn->w, btn->h, marked ? COL_PENCIL_BG : COL_PAPER);
			drawDigit(n, cx, cy, pencil ? 0.8f : 1.0f, color);
		}
	}
}

//---------------------------------------------------------------------------------
// Input
//---------------------------------------------------------------------------------

// px/py are bottom-screen pixels.
static void touch(int px, int py)
{
	int bx = px - BOX_X, by = py - BOX_Y;
	if (bx >= 0 && bx < 3 * BCELL && by >= 0 && by < 3 * BCELL)
	{
		cursor = boxOrigin() + (by / BCELL) * 9 + bx / BCELL;
		return;
	}

	for (int b = 0; b < BUTTON_COUNT; b++)
	{
		const Button* btn = &buttons[b];
		if (px < btn->x || px >= btn->x + btn->w || py < btn->y || py >= btn->y + btn->h)
			continue;
		if (btn->action == ERASE)     erase();
		else if (btn->action == MODE) pencil = !pencil;
		else                          enter(btn->action);
	}
}

int main(int argc, char* argv[])
{
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	C3D_RenderTarget* top    = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	textBuf = C2D_TextBufNew(1024);  // max glyphs drawn per frame

	for (int i = 0; i < 81; i++)
	{
		value[i] = PUZZLE[i] - '0';
		given[i] = value[i] != 0;
	}

	// Holding a direction repeats after 15 frames, then every 5 (60 frames = 1 second).
	hidSetRepeatParameters(15, 5);

	while (aptMainLoop())
	{
		hidScanInput();
		u32 kDown = hidKeysDown();
		u32 kRepeat = hidKeysDownRepeat();  // includes kDown

		if (kDown & KEY_START)
			break;

		// KEY_UP etc. cover both the D-pad and the Circle Pad. Edges wrap around.
		int r = cursor / 9, c = cursor % 9;
		if (kRepeat & KEY_UP)    r = (r + 8) % 9;
		if (kRepeat & KEY_DOWN)  r = (r + 1) % 9;
		if (kRepeat & KEY_LEFT)  c = (c + 8) % 9;
		if (kRepeat & KEY_RIGHT) c = (c + 1) % 9;
		cursor = r * 9 + c;

		if (kDown & (KEY_Y | KEY_L | KEY_R)) pencil = !pencil;
		if (kDown & KEY_B) erase();

		if (kDown & KEY_TOUCH)
		{
			touchPosition t;
			hidTouchRead(&t);
			touch(t.px, t.py);
		}

		// C3D_FrameBegin/End replace gfxFlushBuffers/gfxSwapBuffers/gspWaitForVBlank.
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TextBufClear(textBuf);

		C2D_TargetClear(top, COL_BG);
		C2D_SceneBegin(top);
		drawTop();

		C2D_TargetClear(bottom, COL_BG);
		C2D_SceneBegin(bottom);
		drawBottom();

		C3D_FrameEnd(0);
	}

	C2D_TextBufDelete(textBuf);
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	return 0;
}
