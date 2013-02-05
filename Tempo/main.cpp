#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <string>
#include <sstream>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;

SDL_Surface *screen = NULL;
SDL_Surface *imageSurface = NULL;
SDL_Surface *debugInfoSurface = NULL;

//This is the font that's going to be used.
TTF_Font *font = NULL;
SDL_Color textColor = { 255, 255, 255 };

class Timer {
	private:
		int startTicks;
		int pausedTicks;
		bool paused;
		bool started;

	public:
		Timer();

		void start();
		void stop();
		void pause();
		void unpause();

		int get_ticks();

		bool is_started();
		bool is_paused();
};

bool init() {
	// Start SDL
	if (SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
		return false;
	}

	// SDL_SWSURFACE implies that the surface is set up in software memory.
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
	if (screen == NULL) {
		return false;
	}

	//Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
	if (TTF_Init() == -1) {
		return false;
	}

	// Set the caption on the window.
	SDL_WM_SetCaption("Tempo", NULL);
	return true;
}

void clean_up() {
	SDL_FreeSurface(imageSurface);
	SDL_FreeSurface(debugInfoSurface);

	TTF_CloseFont(font);
	TTF_Quit();
	
	// Quit SDL. Also handles cleanup of the screen object.
	SDL_Quit();
}

SDL_Surface *load_image(std::string filename) {
	SDL_Surface *loadedImage = NULL;
	SDL_Surface *optimizedImage = NULL;
	
	loadedImage = IMG_Load(filename.c_str());

	// if the image isn't null, stick the image onto another surface. doing this wil ensure that the loaded image is in the same format as the screen.
	if (loadedImage != NULL) {
		optimizedImage = SDL_DisplayFormat(loadedImage);
		SDL_FreeSurface(loadedImage);

		//If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
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

Timer::Timer()
{
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start()
{
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop()
{
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause()
{
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause()
{
    //If the timer is paused
    if( paused == true )
    {
        //Unpause the timer
        paused = false;

        //Reset the starting ticks
        startTicks = SDL_GetTicks() - pausedTicks;

        //Reset the paused ticks
        pausedTicks = 0;
    }
}

int Timer::get_ticks()
{
    //If the timer is running
    if( started == true )
    {
        //If the timer is paused
        if( paused == true )
        {
            //Return the number of ticks when the timer was paused
            return pausedTicks;
        }
        else
        {
            //Return the current time minus the start time
            return SDL_GetTicks() - startTicks;
        }
    }

    //If the timer isn't running
    return 0;
}

bool Timer::is_started()
{
    return started;
}

bool Timer::is_paused()
{
    return paused;
}

int main( int argc, char* args[] ) {
		
	SDL_Surface *background = NULL;
	SDL_Event event;
	bool eventTriggered = false;

	int frameCount = 0;
	// Timer to calculate the fps
	Timer fps;
	// Timer to update the fps caption
	Timer update;

	if (init() == false) {
		return 1;
	}

	imageSurface = load_image("../../res/images/wallpaper.jpg");
	font = TTF_OpenFont("../../res/fonts/EunjinNakseo.ttf", 28);
	debugInfoSurface = TTF_RenderText_Solid(font, "Test", textColor);
	apply_surface(0, 0, imageSurface, screen);
	apply_surface(0, 150, debugInfoSurface, screen);
	// Force update
	if (SDL_Flip(screen) == -1) {
		return 1;
	}

	update.start();
	fps.start();

	while (!eventTriggered) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				eventTriggered = true;
			}
		}
		
		frameCount++;

		if (update.get_ticks() > 1000) {
			std::stringstream caption;

			caption << "Average FPS: " << frameCount / (fps.get_ticks() / 1000.f);
			std::string temp = caption.str();
			debugInfoSurface = TTF_RenderText_Solid(font, temp.c_str(), textColor);

			update.start();
		}
		
		apply_surface(0, 0, imageSurface, screen);
		apply_surface(0, 150, debugInfoSurface, screen);
		// Force update
		if (SDL_Flip(screen) == -1) {
			return 1;
		}
	}
	
	clean_up();

	return 0;
}
