#include "SDL.h"

int main( int argc, char* args[] ) {

	// SDL_Surface is an image.
	SDL_Surface* hello = NULL;
	SDL_Surface* screen = NULL;

	// Start SDL
	SDL_Init( SDL_INIT_EVERYTHING );

	// SDL_SWSURFACE implies that the surface is set up in software memory.
	screen = SDL_SetVideoMode( 640, 480, 32, SDL_SWSURFACE );

	hello = SDL_LoadBMP( "hello.bmp" );
	SDL_BlitSurface( hello, NULL, screen, NULL );
	// Force update
	SDL_Flip(screen);
	SDL_Delay(2000);

	// Cleanup
	SDL_FreeSurface(hello);

	// Quit SDL
	SDL_Quit();

	return 0;
}