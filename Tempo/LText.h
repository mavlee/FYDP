#include "LOpenGL.h"
#include "surfaceHelper.h"

class Text {
	public:
		// initializes FreeType for text
		bool initText();

		// Constructor
		Text(int width, int hewight);

		// Deconstructor
		~Text();

		void renderText(int width, int height, GLfloat x, GLfloat y, std::string text);

		SDL_Color color;

	private:
		TTF_Font *font;
		SDL_Surface *surface;
		GLuint texture;		
};