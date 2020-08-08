/* A subset of the olcPixelGameEngine
    https://github.com/OneLoneCoder/olcPixelGameEngine/

   Code has been stripped down to the bare minimum required to
   support Linux and Windows pixel drawing.

   Information on the API can be found at:
    https://github.com/OneLoneCoder/olcPixelGameEngine/wiki/olc::PixelGameEngine

   Currently up to date with v1.19
*/


#ifdef _WIN32  // Visual Studio compiler

	//
	#pragma comment(lib, "user32.lib")		
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "opengl32.lib")

	// Include WinAPI
	#include <windows.h>

	// OpenGL Extension
	#include <GL/gl.h>

	// Other, JK
	#include <stdint.h>

#else

	#include <GL/gl.h>
	#include <GL/glx.h>
	#include <X11/X.h>
	#include <X11/Xlib.h>

#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>  // strdup

#include "olcPGE_min.h"


//================================================================================

static char* appTitle;

static Sprite* pDefaultDrawTarget = NULL;
static Sprite* pDrawTarget        = NULL;

static uint32_t nScreenWidth  = 256;
static uint32_t nScreenHeight = 240;
static uint32_t nPixelWidth   = 4;
static uint32_t nPixelHeight  = 4;

static int32_t nWindowWidth  = 0;
static int32_t nWindowHeight = 0;
static int32_t nViewX        = 0;
static int32_t nViewY        = 0;
static int32_t nViewW        = 0;
static int32_t nViewH        = 0;

static int32_t nMousePosX      = 0;
static int32_t nMousePosY      = 0;
static int32_t nMousePosXCache = 0;
static int32_t nMousePosYCache = 0;

static bool     pMouseNewState [ 5 ] = { 0 };
static bool     pMouseOldState [ 5 ] = { 0 };
static HWButton pMouseState    [ 5 ] = { { 0 } };

static bool     pKeyNewState   [ 256 ] = { 0 };
static bool     pKeyOldState   [ 256 ] = { 0 };
static HWButton pKeyboardState [ 256 ] = { { 0 } };

static bool bHasInputFocus = false;

static GLuint glBuffer;

static bool bAtomActive = false;  // JK, not yet implemented as atomic

#ifdef _WIN32

	static HDC   glDeviceContext = NULL;
	static HGLRC glRenderContext = NULL;

	static HWND   olc_hWnd = NULL;
	static LPVOID sge      = NULL;

	static LRESULT CALLBACK olc_WindowEvent  ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static HWND             PGE_windowCreate ( void );

#else

	static GLXContext glDeviceContext = NULL;

	static Display*             olc_Display = NULL;
	static Window               olc_WindowRoot;
	static Window               olc_Window;
	static XVisualInfo*         olc_VisualInfo;
	static Colormap             olc_ColourMap;
	static XSetWindowAttributes olc_SetWindowAttribs;

	static Display* PGE_windowCreate ( void );

#endif


static enum Key mapKey           ( unsigned int sym );
static bool     PGE_OpenGLCreate ( void );


//================================================================================

static void Pixel_setRGB ( Pixel* p, uint8_t r, uint8_t g, uint8_t b )
{
	p->r = r;
	p->g = g;
	p->b = b;
	p->a = 255;
}


//================================================================================

static Sprite* Sprite_new ( int w, int h )
{
	Sprite* sp;
	Pixel*  p;
	int32_t i;

	sp = ( Sprite* ) malloc( sizeof( Sprite ) );

	sp->width  = w;
	sp->height = h;

	sp->pColData = ( Pixel* ) malloc( w * h * sizeof( Pixel ) );

	for ( i = 0; i < w * h; i += 1 )
	{
		p = sp->pColData + i;

		// Pixel_setRGB( p, 0, 0, 0 );
		Pixel_setRGB( p, 0, 255, 0 );
	}

	return sp;
}

static void Sprite_free ( Sprite* sp )
{
	free( sp->pColData );

	free( sp );

	sp = NULL;
}

static bool Sprite_setPixelRGB ( Sprite* sp, int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b )
{
	Pixel* psp;

	if ( x >= 0 && x < sp->width &&
	     y >= 0 && y < sp->height )
	{
		// sp->pColData[ y * sp->width + x ] = p;

		psp = sp->pColData + ( y * sp->width + x );

		Pixel_setRGB( psp, r, g, b );

		return true;
	}
	else
	{
		return false;
	}
}

static Pixel* Sprite_getData ( Sprite* sp )
{
	return sp->pColData;
}


//================================================================================

void PGE_setDrawTarget ( Sprite* target )
{
	if ( target )
	{
		pDrawTarget = target;
	}
	else
	{
		pDrawTarget = pDefaultDrawTarget;
	}
}

Sprite* PGE_getDrawTarget ( void )
{
	return pDrawTarget;
}

int32_t PGE_getDrawTargetWidth ( void )
{
	if ( pDrawTarget )
	{
		return pDrawTarget->width;
	}
	else
	{
		return 0;
	}
}

int32_t PGE_getDrawTargetHeight ( void )
{
	if ( pDrawTarget )
	{
		return pDrawTarget->height;
	}
	else
	{
		return 0;
	}
}

bool PGE_drawRGB ( int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b )
{
	if ( ! pDrawTarget )
	{
		return false;
	}

	// Assume Pixel::NORMAL
	return Sprite_setPixelRGB( pDrawTarget, x, y, r, g, b );
}

void PGE_clearRGB ( uint8_t r, uint8_t g, uint8_t b )
{
	Sprite* sp;
	Pixel*  psp;
	int     nPixels;
	int     i;

	nPixels = PGE_getDrawTargetWidth() * PGE_getDrawTargetHeight();

	sp = PGE_getDrawTarget();

	// Per pixel...
	for ( i = 0; i < nPixels; i += 1 )
	{
		psp = sp->pColData + i;

		Pixel_setRGB( psp, r, g, b );
	}
}


//================================================================================

int32_t PGE_getScreenWidth ( void )
{
	return nScreenWidth;
}

int32_t PGE_getScreenHeight ( void )
{
	return nScreenHeight;
}

static void PGE_updateViewport ( void )
{
	int32_t ww;
	int32_t wh;
	float   wasp;

	ww   = nScreenWidth * nPixelWidth;
	wh   = nScreenHeight * nPixelHeight;
	wasp = ( float ) ww / ( float ) wh;

	nViewW = ( int32_t ) nWindowWidth;
	nViewH = ( int32_t ) ( ( float ) nViewW / wasp );

	if ( nViewH > nWindowHeight )
	{
		nViewH = nWindowHeight;
		nViewW = ( int32_t ) ( ( float ) nViewH * wasp );
	}

	nViewX = ( nWindowWidth - nViewW ) / 2;
	nViewY = ( nWindowHeight - nViewH ) / 2;
}

void PGE_updateWindowSize ( int32_t x, int32_t y )
{
	nWindowWidth = x;
	nWindowHeight = y;

	PGE_updateViewport();
}


//================================================================================

bool PGE_isFocused ( void )
{
	return bHasInputFocus;
}


//================================================================================

HWButton PGE_getMouse ( enum MouseButton b )
{
	return pMouseState[ b ];
}

int32_t PGE_getMouseX ( void )
{
	return nMousePosX;
}

int32_t PGE_getMouseY ( void )
{
	return nMousePosY;
}

static void PGE_updateMouse ( int32_t x, int32_t y )
{
	/* Mouse coords come in screen space,
	   but leave in pixel space.
	*/

	// Full Screen mode may have a weird viewport we must clamp to
	x -= nViewX;
	y -= nViewY;

	nMousePosXCache = ( int32_t ) ( ( float ) x /
	                                ( float ) ( nWindowWidth - ( nViewX * 2 ) ) *
	                                ( float ) nScreenWidth );

	nMousePosYCache = ( int32_t ) ( ( float ) y /
	                                ( float ) ( nWindowHeight - ( nViewY * 2 ) ) *
	                                ( float ) nScreenHeight );

	if ( nMousePosXCache >= ( int32_t ) nScreenWidth )
	{
		nMousePosXCache = nScreenWidth - 1;
	}
	if ( nMousePosYCache >= ( int32_t ) nScreenHeight )
	{
		nMousePosYCache = nScreenHeight - 1;
	}

	if ( nMousePosXCache < 0 )
	{
		nMousePosXCache = 0;
	}
	if ( nMousePosYCache < 0 )
	{
		nMousePosYCache = 0;
	}
}


//================================================================================

HWButton PGE_getKey ( enum Key k )
{
	return pKeyboardState[ k ];
}

#ifdef _WIN32

	static enum Key mapKey ( unsigned int sym )
	{
		// Create Keyboard Mapping
		switch ( sym )
		{
			case 0x00 : return KEY_NONE;
			case 0x41 : return KEY_A;
			case 0x42 : return KEY_B;
			case 0x43 : return KEY_C;
			case 0x44 : return KEY_D;
			case 0x45 : return KEY_E;
			case 0x46 : return KEY_F;
			case 0x47 : return KEY_G;
			case 0x48 : return KEY_H;
			case 0x49 : return KEY_I;
			case 0x4A : return KEY_J;
			case 0x4B : return KEY_K;
			case 0x4C : return KEY_L;
			case 0x4D : return KEY_M;
			case 0x4E : return KEY_N;
			case 0x4F : return KEY_O;
			case 0x50 : return KEY_P;
			case 0x51 : return KEY_Q;
			case 0x52 : return KEY_R;
			case 0x53 : return KEY_S;
			case 0x54 : return KEY_T;
			case 0x55 : return KEY_U;
			case 0x56 : return KEY_V;
			case 0x57 : return KEY_W;
			case 0x58 : return KEY_X;
			case 0x59 : return KEY_Y;
			case 0x5A : return KEY_Z;

			case VK_F1  : return KEY_F1;
			case VK_F2  : return KEY_F2;
			case VK_F3  : return KEY_F3;
			case VK_F4  : return KEY_F4;
			case VK_F5  : return KEY_F5;
			case VK_F6  : return KEY_F6;
			case VK_F7  : return KEY_F7;
			case VK_F8  : return KEY_F8;
			case VK_F9  : return KEY_F9;
			case VK_F10 : return KEY_F10;
			case VK_F11 : return KEY_F11;
			case VK_F12 : return KEY_F12;

			case VK_DOWN   : return KEY_DOWN;
			case VK_LEFT   : return KEY_LEFT;
			case VK_RIGHT  : return KEY_RIGHT;
			case VK_UP     : return KEY_UP;
			case VK_RETURN : return KEY_ENTER;
			// case VK_RETURN : return KEY_RETURN;

			case VK_BACK    : return KEY_BACK;
			case VK_ESCAPE  : return KEY_ESCAPE;
			// case VK_RETURN  : return KEY_ENTER;
			case VK_PAUSE   : return KEY_PAUSE;
			case VK_SCROLL  : return KEY_SCROLL;
			case VK_TAB     : return KEY_TAB;
			case VK_DELETE  : return KEY_DEL;
			case VK_HOME    : return KEY_HOME;
			case VK_END     : return KEY_END;
			case VK_PRIOR   : return KEY_PGUP;
			case VK_NEXT    : return KEY_PGDN;
			case VK_INSERT  : return KEY_INS;
			case VK_SHIFT   : return KEY_SHIFT;
			case VK_CONTROL : return KEY_CTRL;
			case VK_SPACE   : return KEY_SPACE;

			case 0x30 : return KEY_0;
			case 0x31 : return KEY_1;
			case 0x32 : return KEY_2;
			case 0x33 : return KEY_3;
			case 0x34 : return KEY_4;
			case 0x35 : return KEY_5;
			case 0x36 : return KEY_6;
			case 0x37 : return KEY_7;
			case 0x38 : return KEY_8;
			case 0x39 : return KEY_9;

			case VK_NUMPAD0 : return KEY_NP0;
			case VK_NUMPAD1 : return KEY_NP1;
			case VK_NUMPAD2 : return KEY_NP2;
			case VK_NUMPAD3 : return KEY_NP3;
			case VK_NUMPAD4 : return KEY_NP4;
			case VK_NUMPAD5 : return KEY_NP5;
			case VK_NUMPAD6 : return KEY_NP6;
			case VK_NUMPAD7 : return KEY_NP7;
			case VK_NUMPAD8 : return KEY_NP8;
			case VK_NUMPAD9 : return KEY_NP9;

			case VK_MULTIPLY : return KEY_NP_MUL;
			case VK_ADD      : return KEY_NP_ADD;
			case VK_DIVIDE   : return KEY_NP_DIV;
			case VK_SUBTRACT : return KEY_NP_SUB;
			case VK_DECIMAL  : return KEY_NP_DECIMAL;
		}

		//
		return KEY_NONE;
	}

#else

	static enum Key mapKey ( unsigned int sym )
	{
		// Create Keyboard Mapping
		switch ( sym )
		{
			case 0x00 : return KEY_NONE;
			case 0x61 : return KEY_A;
			case 0x62 : return KEY_B;
			case 0x63 : return KEY_C;
			case 0x64 : return KEY_D;
			case 0x65 : return KEY_E;
			case 0x66 : return KEY_F;
			case 0x67 : return KEY_G;
			case 0x68 : return KEY_H;
			case 0x69 : return KEY_I;
			case 0x6A : return KEY_J;
			case 0x6B : return KEY_K;
			case 0x6C : return KEY_L;
			case 0x6D : return KEY_M;
			case 0x6E : return KEY_N;
			case 0x6F : return KEY_O;
			case 0x70 : return KEY_P;
			case 0x71 : return KEY_Q;
			case 0x72 : return KEY_R;
			case 0x73 : return KEY_S;
			case 0x74 : return KEY_T;
			case 0x75 : return KEY_U;
			case 0x76 : return KEY_V;
			case 0x77 : return KEY_W;
			case 0x78 : return KEY_X;
			case 0x79 : return KEY_Y;
			case 0x7A : return KEY_Z;

			case XK_F1  : return KEY_F1;
			case XK_F2  : return KEY_F2;
			case XK_F3  : return KEY_F3;
			case XK_F4  : return KEY_F4;
			case XK_F5  : return KEY_F5;
			case XK_F6  : return KEY_F6;
			case XK_F7  : return KEY_F7;
			case XK_F8  : return KEY_F8;
			case XK_F9  : return KEY_F9;
			case XK_F10 : return KEY_F10;
			case XK_F11 : return KEY_F11;
			case XK_F12 : return KEY_F12;

			case XK_Down     : return KEY_DOWN;
			case XK_Left     : return KEY_LEFT;
			case XK_Right    : return KEY_RIGHT;
			case XK_Up       : return KEY_UP;
			case XK_KP_Enter : return KEY_ENTER;
			case XK_Return   : return KEY_ENTER;

			case XK_BackSpace   : return KEY_BACK;
			case XK_Escape      : return KEY_ESCAPE;
			case XK_Linefeed    : return KEY_ENTER;
			case XK_Pause       : return KEY_PAUSE;
			case XK_Scroll_Lock : return KEY_SCROLL;
			case XK_Tab         : return KEY_TAB;
			case XK_Delete      : return KEY_DEL;
			case XK_Home        : return KEY_HOME;
			case XK_End         : return KEY_END;
			case XK_Page_Up     : return KEY_PGUP;
			case XK_Page_Down   : return KEY_PGDN;
			case XK_Insert      : return KEY_INS;
			case XK_Shift_L     : return KEY_SHIFT;
			case XK_Shift_R     : return KEY_SHIFT;
			case XK_Control_L   : return KEY_CTRL;
			case XK_Control_R   : return KEY_CTRL;
			case XK_space       : return KEY_SPACE;

			case XK_0 : return KEY_0;
			case XK_1 : return KEY_1;
			case XK_2 : return KEY_2;
			case XK_3 : return KEY_3;
			case XK_4 : return KEY_4;
			case XK_5 : return KEY_5;
			case XK_6 : return KEY_6;
			case XK_7 : return KEY_7;
			case XK_8 : return KEY_8;
			case XK_9 : return KEY_9;

			case XK_KP_0 : return KEY_NP0;
			case XK_KP_1 : return KEY_NP1;
			case XK_KP_2 : return KEY_NP2;
			case XK_KP_3 : return KEY_NP3;
			case XK_KP_4 : return KEY_NP4;
			case XK_KP_5 : return KEY_NP5;
			case XK_KP_6 : return KEY_NP6;
			case XK_KP_7 : return KEY_NP7;
			case XK_KP_8 : return KEY_NP8;
			case XK_KP_9 : return KEY_NP9;

			case XK_KP_Multiply : return KEY_NP_MUL;
			case XK_KP_Add      : return KEY_NP_ADD;
			case XK_KP_Divide   : return KEY_NP_DIV;
			case XK_KP_Subtract : return KEY_NP_SUB;
			case XK_KP_Decimal  : return KEY_NP_DECIMAL;
		}

		//
		return KEY_NONE;
	}

#endif


//================================================================================

#ifdef _WIN32

	// Windows event handler...
	static LRESULT CALLBACK olc_WindowEvent ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		switch ( uMsg )
		{
			case WM_CREATE:

				sge = ( ( LPCREATESTRUCT ) lParam )->lpCreateParams;

				return 0;

			case WM_SIZE:

				PGE_updateWindowSize( lParam & 0xFFFF, ( lParam >> 16 ) & 0xFFFF );

				return 0;

			case WM_KEYDOWN:

				pKeyNewState[ mapKey( wParam ) ] = true;

				return 0;

			case WM_KEYUP:

				pKeyNewState[ mapKey( wParam ) ] = false;

				return 0;

			case WM_LBUTTONDOWN:

				pMouseNewState[ 0 ] = true;

				return 0;

			case WM_LBUTTONUP:

				pMouseNewState[ 0 ] = false;

				return 0;

			case WM_RBUTTONDOWN:

				pMouseNewState[ 1 ] = true;

				return 0;

			case WM_RBUTTONUP:

				pMouseNewState[ 1 ] = false;

				return 0;

			case WM_MBUTTONDOWN:

				pMouseNewState[ 2 ] = true;

				return 0;

			case WM_MBUTTONUP:

				pMouseNewState[ 2 ] = false;

				return 0;

			case WM_MOUSEMOVE:

				uint16_t  x = lParam & 0xFFFF;
				uint16_t  y = ( lParam >> 16 ) & 0xFFFF;
				int16_t  ix = *( ( int16_t* ) &x );
				int16_t  iy = *( ( int16_t* ) &y );

				PGE_updateMouse( ix, iy );

				return 0;

			case WM_SETFOCUS:

				bHasInputFocus = true;

				return 0;

			case WM_KILLFOCUS:

				bHasInputFocus = false;

				return 0;

			case WM_CLOSE:

				bAtomActive = false;

				return 0;

			case WM_DESTROY:

				PostQuitMessage( 0 );

				return 0;
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

#endif


static void PGE_engineThread ( void )
{
	int i;

	// Start OpenGL, the context is owned by the game thread
	PGE_OpenGLCreate();


	// Create Screen Texture - disable filtering
	glEnable( GL_TEXTURE_2D );
	glGenTextures( 1, &glBuffer );
	glBindTexture( GL_TEXTURE_2D, glBuffer );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

	glTexImage2D(

		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		nScreenWidth, nScreenHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		Sprite_getData( pDefaultDrawTarget )
	);


	// User setup
	if ( ! UI_onUserCreate() )
	{
		bAtomActive = false;
	}


	while ( bAtomActive )
	{
		// Run as fast as possible
		while ( bAtomActive )
		{
			// Xlib message loop -------------------------------------------------
			#ifndef _WIN32

				XEvent x_event;

				while ( XPending( olc_Display ) )
				{
					XNextEvent( olc_Display, &x_event );

					// ??
					if ( x_event.type == Expose )
					{
						XWindowAttributes gwa;

						XGetWindowAttributes( olc_Display, olc_Window, &gwa );

						nWindowWidth  = gwa.width;
						nWindowHeight = gwa.height;

						PGE_updateViewport();

						glClear( GL_COLOR_BUFFER_BIT );
					}

					// ??
					else if ( x_event.type == ConfigureNotify )
					{
						XConfigureEvent xce;

						xce = x_event.xconfigure;

						nWindowWidth  = xce.width;
						nWindowHeight = xce.height;
					}

					else if ( x_event.type == KeyPress )
					{
						XKeyEvent* xke;
						KeySym     sym;

						sym = XLookupKeysym( &x_event.xkey, 0 );

						pKeyNewState[ mapKey( sym ) ] = true;

						xke = ( XKeyEvent* ) &x_event;

						XLookupString( xke, NULL, 0, &sym, NULL );

						pKeyNewState[ mapKey( sym ) ] = true;
					}

					else if ( x_event.type == KeyRelease )
					{
						XKeyEvent* xke;
						KeySym     sym;

						sym = XLookupKeysym( &x_event.xkey, 0 );

						pKeyNewState[ mapKey( sym ) ] = false;

						xke = ( XKeyEvent* ) &x_event;

						XLookupString( xke, NULL, 0, &sym, NULL );

						pKeyNewState[ mapKey( sym ) ] = false;
					}

					else if ( x_event.type == ButtonPress )
					{
						switch ( x_event.xbutton.button )
						{
							case 1:

								pMouseNewState[ 0 ] = true;
								break;

							case 2:

								pMouseNewState[ 1 ] = true;
								break;

							case 3:

								pMouseNewState[ 2 ] = true;
								break;

							default:

								break;
						}
					}

					else if ( x_event.type == ButtonRelease )
					{
						switch ( x_event.xbutton.button )
						{
							case 1:

								pMouseNewState[ 0 ] = false;
								break;

							case 2:

								pMouseNewState[ 1 ] = false;
								break;

							case 3:

								pMouseNewState[ 2 ] = false;
								break;

							default:

								break;
						}
					}

					else if ( x_event.type == MotionNotify )
					{
						PGE_updateMouse( x_event.xmotion.x, x_event.xmotion.y );
					}

					else if ( x_event.type == FocusIn )
					{
						bHasInputFocus = true;
					}

					else if ( x_event.type == FocusOut )
					{
						bHasInputFocus = false;
					}

					else if ( x_event.type == ClientMessage )
					{
						bAtomActive = false;
					}
				}

			#endif


			// Handle user input - Keyboard --------------------------------------

			for ( i = 0; i < 256; i += 1 )
			{
				pKeyboardState[ i ].bPressed  = false;
				pKeyboardState[ i ].bReleased = false;

				// state has changed (press/release)
				if ( pKeyNewState[ i ] != pKeyOldState[ i ] )
				{
					// pressed
					if ( pKeyNewState[ i ] )
					{
						// bPressed is set once, the first time key is pressed
						pKeyboardState[ i ].bPressed = ! pKeyboardState[ i ].bHeld;

						// bHeld is set for all frames between press and release
						pKeyboardState[ i ].bHeld = true;
					}

					// released
					else
					{
						// bReleased is set once, the first time key is released
						pKeyboardState[ i ].bReleased = true;
						pKeyboardState[ i ].bHeld = false;
					}
				}

				pKeyOldState[ i ] = pKeyNewState[ i ];
			}


			// Handle user input - Mouse -----------------------------------------

			for ( i = 0; i < 5; i += 1 )
			{
				pMouseState[ i ].bPressed  = false;
				pMouseState[ i ].bReleased = false;

				if ( pMouseNewState[ i ] != pMouseOldState[ i ] )
				{
					if ( pMouseNewState[ i ] )
					{
						pMouseState[ i ].bPressed = ! pMouseState[ i ].bHeld;
						pMouseState[ i ].bHeld = true;
					}
					else
					{
						pMouseState[ i ].bReleased = true;
						pMouseState[ i ].bHeld = false;
					}
				}

				pMouseOldState[ i ] = pMouseNewState[ i ];
			}

			// Cache mouse coordinates so they remain consistent during a frame
			nMousePosX = nMousePosXCache;
			nMousePosY = nMousePosYCache;


			// Handle user frame update ------------------------------------------

			if ( ! UI_onUserUpdate() )
			{
				bAtomActive = false;
			}


			// Display graphics --------------------------------------------------

			glViewport( nViewX, nViewY, nViewW, nViewH );

			// Copy pixel array into texture
			glTexSubImage2D(

				GL_TEXTURE_2D,
				0, 0, 0,
				nScreenWidth, nScreenHeight,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				Sprite_getData( pDefaultDrawTarget )
			);

			// Display texture on screen
			glBegin( GL_QUADS );

				glTexCoord2f( 0.0, 1.0 );
				glVertex3f( - 1.0f, - 1.0f, 0.0f );

				glTexCoord2f( 0.0, 0.0 );
				glVertex3f( - 1.0f,   1.0f, 0.0f );

				glTexCoord2f( 1.0, 0.0 );
				glVertex3f(   1.0f,   1.0f, 0.0f );

				glTexCoord2f( 1.0, 1.0 );
				glVertex3f(   1.0f, - 1.0f, 0.0f );

			glEnd();

			// Present Graphics to screen
			#ifdef _WIN32

				SwapBuffers( glDeviceContext );

			#else
			
				glXSwapBuffers( olc_Display, olc_Window );

			#endif

		}


		// Allow user to free resources
		if ( UI_onUserDestroy() )
		{
			// User has permitted destroy, so exit and clean up
		}
		else
		{
			// User denied destroy for some reason, so continue running
			bAtomActive = true;
		}
	}


	// ?
	#ifdef _WIN32

		wglDeleteContext( glRenderContext );
		PostMessage( olc_hWnd, WM_DESTROY, 0, 0 );

	#else

		glXMakeCurrent( olc_Display, None, NULL );
		glXDestroyContext( olc_Display, glDeviceContext );
		XDestroyWindow( olc_Display, olc_Window );
		XCloseDisplay( olc_Display );

	#endif
}


//================================================================================

enum rcode PGE_construct (

	uint32_t screen_w, uint32_t screen_h,
	uint32_t pixel_w, uint32_t pixel_h,
	char* app_title
)
{
	nScreenWidth  = screen_w;
	nScreenHeight = screen_h;
	nPixelWidth   = pixel_w;
	nPixelHeight  = pixel_h;

	if ( nPixelWidth == 0 || nPixelHeight == 0 ||
	     nScreenWidth == 0 || nScreenHeight == 0 )
	{
		return FAIL;
	}

	// Create a sprite that represents the primary drawing target
	pDefaultDrawTarget = Sprite_new( nScreenWidth, nScreenHeight );

	PGE_setDrawTarget( NULL );


	// Set the title bar text
	if ( app_title != NULL )
	{
		appTitle = strdup( app_title );
	}
	else
	{
		appTitle = strdup( "Untitled" );
	}

	return OK;
}

#ifdef _WIN32

	// https://docs.microsoft.com/en-us/windows/win32/procthread/creating-threads
	// https://stackoverflow.com/a/1981467

	DWORD WINAPI myThreadFunction ( LPVOID lpParam )
	{
		PGE_engineThread();

		return 0;
	}

	enum rcode PGE_start ( void )
	{
		HANDLE engineThread;

		if ( ! PGE_windowCreate() )
		{
			return FAIL;
		}


		// Start the thread...
		bAtomActive = true;

		// std::thread t = std::thread(&PixelGameEngine::EngineThread, this);
		engineThread = CreateThread(

            NULL,              // default security attributes
            0,                 // use default stack size  
            myThreadFunction,  // thread function name
            NULL,              // argument to thread function 
            0,                 // use default creation flags 
            NULL               // returns the thread identifier		
		);


		// Handle Windows Message Loop
		MSG msg;

		while ( GetMessage( &msg, NULL, 0, 0 ) > 0 )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}


		// Wait for thread to be exited
		// t.join();
		WaitForSingleObject( engineThread, INFINITE );


		return OK;
	}

#else

	enum rcode PGE_start ( void )
	{
		if ( ! PGE_windowCreate() )
		{
			return FAIL;
		}


		// Start the thread...
		bAtomActive = true;

		// std::thread t = std::thread(&PixelGameEngine::EngineThread, this);
		PGE_engineThread();


		// Wait for thread to be exited
		// t.join();


		return OK;
	}

#endif


//================================================================================

#ifdef _WIN32

	static HWND PGE_windowCreate ( void )
	{
		WNDCLASS wc;
		wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
		wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
		wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.hInstance     = GetModuleHandle( NULL );
		wc.lpfnWndProc   = olc_WindowEvent;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.lpszMenuName  = NULL;
		wc.hbrBackground = NULL;
		wc.lpszClassName = "OLC_PIXEL_GAME_ENGINE";

		RegisterClass( &wc );


		nWindowWidth  = ( LONG ) nScreenWidth  * ( LONG ) nPixelWidth;
		nWindowHeight = ( LONG ) nScreenHeight * ( LONG ) nPixelHeight;

		nViewW = nWindowWidth;
		nViewH = nWindowHeight;


		// Define window furniture
		DWORD dwExStyle;
		DWORD dwStyle;
		int   nCosmeticOffset;

		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle   = WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_THICKFRAME;

		nCosmeticOffset = 30;


		//
		// if ( bFullScreen ) {}


		// PGE_updateViewport();


		// Keep client size as requested
		int width;
		int height;

		RECT rWndRect = { 0, 0, nWindowWidth, nWindowHeight };

		AdjustWindowRectEx( &rWndRect, dwStyle, FALSE, dwExStyle );

		width  = rWndRect.right - rWndRect.left;
		height = rWndRect.bottom - rWndRect.top;


		olc_hWnd = CreateWindowEx(

			dwExStyle,
			"OLC_PIXEL_GAME_ENGINE",
			appTitle,
			dwStyle,
			nCosmeticOffset,
			nCosmeticOffset,
			width,
			height,
			NULL,
			NULL,
			GetModuleHandle( NULL ),
			sge
		);

		return olc_hWnd;
	}

	static bool PGE_OpenGLCreate ( void )
	{
		// Create Device Context
		glDeviceContext = GetDC( olc_hWnd );

		PIXELFORMATDESCRIPTOR pfd = {

			sizeof( PIXELFORMATDESCRIPTOR ),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			PFD_MAIN_PLANE, 0, 0, 0, 0
		};

		int pf = ChoosePixelFormat( glDeviceContext, &pfd );

		if ( ! pf )
		{
			return false;
		}

		SetPixelFormat( glDeviceContext, pf, &pfd );

		glRenderContext = wglCreateContext( glDeviceContext );

		if ( ! glRenderContext )
		{
			return false;
		}

		wglMakeCurrent( glDeviceContext, glRenderContext );

		glViewport( nViewX, nViewY, nViewW, nViewH );

		return true;
	}

#else

	static Display* PGE_windowCreate ( void )
	{
		//XInitThreads();


		// Grab the default display and window
		olc_Display    = XOpenDisplay( NULL );
		olc_WindowRoot = DefaultRootWindow( olc_Display );


		// Based on the display capabilities, configure the appearance of the window
		GLint olc_GLAttribs [] = {

			GLX_RGBA,
			GLX_DEPTH_SIZE,
			24,
			GLX_DOUBLEBUFFER,
			None
		};

		olc_VisualInfo = glXChooseVisual( olc_Display, 0, olc_GLAttribs );


		olc_ColourMap = XCreateColormap(

			olc_Display,
			olc_WindowRoot,
			olc_VisualInfo->visual,
			AllocNone
		);

		olc_SetWindowAttribs.colormap = olc_ColourMap;


		// Register which events we are interested in receiving
		olc_SetWindowAttribs.event_mask = (

			ExposureMask      |
			KeyPressMask      |
			KeyReleaseMask    |
			ButtonPressMask   |
			ButtonReleaseMask |
			PointerMotionMask |
			FocusChangeMask   |
			StructureNotifyMask
		);


		// Create the window
		olc_Window = XCreateWindow(

			olc_Display,                   // display
			olc_WindowRoot,                // parent
			30, 30,                        // x, y
			nScreenWidth * nPixelWidth,    // w
			nScreenHeight * nPixelHeight,  // h
			0,                             // border width
			olc_VisualInfo->depth,         // depth
			InputOutput,                   // class?
			olc_VisualInfo->visual,        // ?
			CWColormap | CWEventMask,      // ?
			&olc_SetWindowAttribs
		);

		Atom wmDelete = XInternAtom( olc_Display, "WM_DELETE_WINDOW", true );
		XSetWMProtocols( olc_Display, olc_Window, &wmDelete, 1 );

		XMapWindow( olc_Display, olc_Window);

		XStoreName( olc_Display, olc_Window, appTitle );


		//
		// if ( bFullScreen ) {}


		//
		return olc_Display;
	}

	static bool PGE_OpenGLCreate ( void )
	{
		XWindowAttributes gwa;

		glDeviceContext = glXCreateContext( olc_Display, olc_VisualInfo, NULL, GL_TRUE );
		glXMakeCurrent( olc_Display, olc_Window, glDeviceContext );


		XGetWindowAttributes( olc_Display, olc_Window, &gwa );
		glViewport( 0, 0, gwa.width, gwa.height );

		return true;
	}

#endif


//================================================================================

/* JK, attempt to free memory on exit.
   TODO: all the x11 & GDI stuff
*/
enum rcode PGE_destroy ( void )
{
	#ifdef _WIN32

		//

	#else

		XFree( olc_VisualInfo );

	#endif

	Sprite_free( pDefaultDrawTarget );

	free( appTitle );

	return OK;
}
