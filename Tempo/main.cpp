#include "spriteClipper.h"
#include "LUtil.h"

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
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_OPENGL );
	if (screen == NULL) {
		return false;
	}

	//Initialize SDL ttf. Must be called prior to all SDL_ttf functions.
	if (TTF_Init() == -1) {
		return false;
	}

	if (!initClipper()) {
		return false;
	}

	if (!initGL()) {
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

	cleanUpClipper();

	// Quit SDL. Also handles cleanup of the screen object.
	SDL_Quit();
}

Timer::Timer() {
    //Initialize the variables
    startTicks = 0;
    pausedTicks = 0;
    paused = false;
    started = false;
}

void Timer::start() {
    //Start the timer
    started = true;

    //Unpause the timer
    paused = false;

    //Get the current clock time
    startTicks = SDL_GetTicks();
}

void Timer::stop() {
    //Stop the timer
    started = false;

    //Unpause the timer
    paused = false;
}

void Timer::pause() {
    //If the timer is running and isn't already paused
    if( ( started == true ) && ( paused == false ) )
    {
        //Pause the timer
        paused = true;

        //Calculate the paused ticks
        pausedTicks = SDL_GetTicks() - startTicks;
    }
}

void Timer::unpause() {
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

int Timer::get_ticks() {
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

bool Timer::is_started() {
    return started;
}

bool Timer::is_paused() {
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
	Timer updateTimer;

	if (init() == false) {
		return 1;
	}

	imageSurface = loadImage("../res/images/wallpaper.jpg");
	font = TTF_OpenFont("../res/fonts/EunjinNakseo.ttf", 28);
	debugInfoSurface = TTF_RenderText_Solid(font, "Test", textColor);
	applySurface(0, 0, imageSurface, screen);
	applySurface(0, 150, debugInfoSurface, screen);

	SDL_Surface *dots = getSpriteMap();
	SDL_Rect *clip = getClipBounds();

	glClear(GL_COLOR_BUFFER_BIT);

	updateTimer.start();
	fps.start();

	while (!eventTriggered) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				eventTriggered = true;
			} else if (event.type = SDL_KEYDOWN) {
				handleKeys(event.key.keysym.sym);
			}
		}

		frameCount++;

		if (updateTimer.get_ticks() > 1000) {
			std::stringstream caption;

			caption << "Average FPS: " << frameCount / (fps.get_ticks() / 1000.f);
			std::string temp = caption.str();
			debugInfoSurface = TTF_RenderText_Solid(font, temp.c_str(), textColor);

			updateTimer.start();
		}

		update();
		render();
	}

	clean_up();

	return 0;
}
