#include <ft2build.h>
#include FT_FREETYPE_H
#include "LOpenGL.h"

class Text {
  public:
    // initializes FreeType for text
    static bool initText();

    // Constructor
    Text();

    // Deconstructor
    ~Text();

    // Loads bitmap font
    bool loadBitmap( std::string path );

    bool loadFreeType(std::string path, GLuint pixelSize);

    void freeFont();

    void renderText(GLfloat x, GLfloat y, std::string text);
  private:
    //Font TTF library
    static FT_Library mLibrary;

    //Spacing variables
    GLfloat mSpace;
    GLfloat mLineHeight;
    GLfloat mNewLine;
}
