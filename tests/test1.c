#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  // rand

#include "../olcPGE_min.h"

void drawACursor ( int32_t x, int32_t y )
{
	PGE_drawRGB( x + 0, y + 0, 0, 0, 0 );
	PGE_drawRGB( x + 2, y + 0, 0, 0, 0 );
	PGE_drawRGB( x + 1, y + 0, 0, 0, 0 );
	PGE_drawRGB( x - 1, y + 0, 0, 0, 0 );
	PGE_drawRGB( x - 2, y + 0, 0, 0, 0 );
	PGE_drawRGB( x + 0, y + 2, 0, 0, 0 );
	PGE_drawRGB( x + 0, y + 1, 0, 0, 0 );
	PGE_drawRGB( x + 0, y - 1, 0, 0, 0 );
	PGE_drawRGB( x + 0, y - 2, 0, 0, 0 );
}

void doAThing ( void )
{
	PGE_clearRGB( 255, 255, 0 );

	PGE_drawRGB( 10, 10, 255, 0, 0 );
	PGE_drawRGB( 11, 11, 255, 0, 0 );

	PGE_drawRGB( 0, 0, 0, 0, 255 );
	PGE_drawRGB( 0, 99, 0, 0, 255 );
	PGE_drawRGB( 99, 99, 0, 0, 255 );
	PGE_drawRGB( 99, 0, 0, 0, 255 );

	drawACursor( PGE_getMouseX(), PGE_getMouseY() );

	/*if ( PGE_isFocused() )
	{
		printf( "true\n" );
	}
	else
	{
		printf( "false\n" );
	}*/

	if ( PGE_getKey( KEY_ENTER ).bPressed )
	{
		printf( "!\n" );
	}
	if ( PGE_getKey( KEY_ENTER ).bHeld )
	{
		printf( "?\n" );
	}
	if ( PGE_getMouse( MOUSE_LEFT ).bPressed )
	{
		printf( "click\n" );
	}
}

void doAThing2 ( void )
{
	int x, y;

	for ( x = 0; x < PGE_getScreenWidth(); x += 1 )
	{
		for ( y = 0; y < PGE_getScreenHeight(); y += 1 )
		{
			PGE_drawRGB( x, y, rand() % 255, rand() % 255, rand() % 255 );
		}
	}
}


bool UI_onUserCreate ( void )
{
	return true;
}

bool UI_onUserUpdate ( void )
{
	// doAThing();
	doAThing2();

	return true;
}

bool UI_onUserDestroy ( void )
{
	return true;
}


int main ( void )
{
	if ( PGE_construct( 100, 100, 4, 4, "Hello" ) )
	{
		PGE_start();
	}

	PGE_destroy();

	return 0;
}


/*
	Linux:

		gcc olcPGE_min_x11_gdi.c test.c -lX11 -lGL -lpthread -o bin/test.e

		gcc -Werror -Wall -Wno-unused olcPGE_min_x11_gdi.c test.c -lX11 -lGL -lpthread -o bin/test.e

	Windows (VS command prompt):

		cd binWin
		cl ../olcPGE_min_x11_gdi.c ../test.c /Fe"test.exe"
*/
