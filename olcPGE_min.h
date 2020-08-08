/* A subset of the olcPixelGameEngine
    https://github.com/OneLoneCoder/olcPixelGameEngine/

   Code has been stripped down to the bare minimum required to
   support Linux and Windows pixel drawing.

   Information on the API can be found at:
    https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/olc::PixelGameEngine

   Currently up to date with v1.19
*/


// -------------------------------------------

enum rcode
{
	FAIL    =   0,
	OK      =   1,
	NO_FILE = - 1
};


// -------------------------------------------

struct _HWButton
{
	bool bPressed;   // set once, when button transitions from released to pressed
	bool bReleased;  // set once, when button transitions from pressed to released
	bool bHeld;      // set for all frames between press (inclusive) and release (exclusive)
};

typedef struct _HWButton HWButton;


enum Key
{
	KEY_NONE,
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
	KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
	KEY_SPACE, KEY_TAB, KEY_SHIFT, KEY_CTRL, KEY_INS, KEY_DEL, KEY_HOME, KEY_END, KEY_PGUP, KEY_PGDN,
	KEY_BACK, KEY_ESCAPE, KEY_RETURN, KEY_ENTER, KEY_PAUSE, KEY_SCROLL,
	KEY_NP0, KEY_NP1, KEY_NP2, KEY_NP3, KEY_NP4, KEY_NP5, KEY_NP6, KEY_NP7, KEY_NP8, KEY_NP9,
	KEY_NP_MUL, KEY_NP_DIV, KEY_NP_ADD, KEY_NP_SUB, KEY_NP_DECIMAL,
};


// -------------------------------------------

enum MouseButton  // JK
{
	MOUSE_LEFT,   // 0
	MOUSE_RIGHT,  // 1
	MOUSE_MIDDLE  // 2
};


// -------------------------------------------

struct _Pixel
{
	uint8_t  r;
	uint8_t  g;
	uint8_t  b;
	uint8_t  a;
};

typedef struct _Pixel Pixel;


// -------------------------------------------

struct _Sprite
{
	int32_t width;
	int32_t height;
	Pixel*  pColData;
};

typedef struct _Sprite Sprite;



// -------------------------------------------

// User defined...

/* Called once on application startup.
   Use to load resources
*/
bool UI_onUserCreate ( void );

/* Called every frame, and provides user with a time per frame value
*/
bool UI_onUserUpdate ( void );

/* Called once on application termination
   Use for clean up
*/
bool UI_onUserDestroy ( void );



// -------------------------------------------

// Setup
enum rcode PGE_construct ( uint32_t screen_w, uint32_t screen_h,
                           uint32_t pixel_w, uint32_t pixel_h, char* app_title );
enum rcode PGE_start     ( void );
enum rcode PGE_destroy   ( void );  // JK, cleanup


// User input
HWButton PGE_getKey    ( enum Key k );
HWButton PGE_getMouse  ( enum MouseButton b );
int32_t  PGE_getMouseX ( void );
int32_t  PGE_getMouseY ( void );
// bool     PGE_isFocused ( void );


// Environment
int32_t PGE_getScreenWidth  ( void );
int32_t PGE_getScreenHeight ( void );

void    PGE_setDrawTarget       ( Sprite* target );
Sprite* PGE_getDrawTarget       ( void );
int32_t PGE_getDrawTargetWidth  ( void );
int32_t PGE_getDrawTargetHeight ( void );


// Drawing
void PGE_clearRGB ( uint8_t r, uint8_t g, uint8_t b );
bool PGE_drawRGB  ( int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b );
// bool PGE_draw    ( int32_t x, int32_t y, Pixel* p );
