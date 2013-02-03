#include "SDL.h"
#include "SDL_image.h"
#include <string>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

bool init(SDL_Surface *screen) {
	// Start SDL
	if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
		return false;
	}

	// SDL_SWSURFACE implies that the surface is set up in software memory.
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
	if (screen == NULL) {
		return false;
	}

	// Set the caption on the window.
	SDL_WM_SetCaption("Tempo", NULL);
	return true;
}

SDL_Surface *load_image(std::string filename) {
	SDL_Surface *loadedImage = NULL;
	SDL_Surface *optimizedImage = NULL;
	
	loadedImage = IMG_Load(filename.c_str());

	// if the image isn't null, stick the image onto another surface. doing this wil ensure that the loaded image is in the same format as the screen.
	if (loadedImage != NULL) {
		optimizedImage = SDL_DisplayFormat(loadedImage);
		SDL_FreeSurface(loadedImage);
	}
	return optimizedImage;
}

void apply_surface(int x, int y, SDL_Surface *source, SDL_Surface *dest) {
	// SDL_Rect represents a rectangle, has x, y, width, height.
	SDL_Rect offset;

	offset.x = x;
	offset.y = y;

	SDL_BlitSurface(source, NULL, dest, &offset);
}

int main( int argc, char* args[] ) {

	// SDL_Surface is an image.
	SDL_Surface *subjectItem = NULL;
	SDL_Surface *background = NULL;
	SDL_Surface *screen = NULL;
	SDL_Event event;
	bool eventTriggered = false;

	if (init(screen) == false) {
		return 1;
	}

	// SDL_SWSURFACE implies that the surface is set up in software memory.
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
	if (screen == NULL) {
		return 1;
	}

	subjectItem = SDL_LoadBMP( "hello.bmp" );
	background = load_image("../../res/images/wallpaper.jpg");
	apply_surface(0, 0, background, screen);
	apply_surface(160, 160, subjectItem, screen);
	// Force update
	if (SDL_Flip(screen) == -1) {
		return 1;
	}

	while (!eventTriggered) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				eventTriggered = true;
			}
		}
	}
	
	// Cleanup
	SDL_FreeSurface(subjectItem);
	SDL_FreeSurface(background);

	// Quit SDL. Also handles cleanup of the screen object.
	//SDL_Quit();

	return 0;
}
