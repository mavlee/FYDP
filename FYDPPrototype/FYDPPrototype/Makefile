MACHINE= $(shell uname -s)
OBJFILES := stdafx.o FYDPPrototype.o

ifeq ($(MACHINE),Darwin)
	#INCLUDES = -I/Library/Frameworks/SDL.framework/Headers -I/Library/Frameworks/SDL_image.framework/Headers -I/Library/Frameworks/SDL_mixer.framework/Headers -I/Library/Frameworks/SDL_ttf.framework/Headers -I/System/Library/Frameworks/OpenGL.framework/Headers
	INCLUDES = ~/Downloads/bass24-osx/libbass.dylib -I/Library/Frameworks/SDL.framework/Headers -I/Library/Frameworks/SDL_ttf.framework/Headers -I/Library/Frameworks/SDL_image.framework/Headers -I/System/Library/Frameworks/OpenGL.framework/Headers
	LIBS = -m32 -framework SDL -framework SDL_ttf -framework SDL_image -framework Cocoa -framework OpenGL

	# SDL isn't in a great state on 64-bit Macs, so force 32-bit for now
	CXXFLAGS = -g -O0 -c -m32 -Wno-deprecated -DUSE_MAC_INCLUDES

	# to compile on OS X you need to include this Objective C file
	OSXCOMPAT = 
else
	INCLUDES = `sdl-config --cflags` -I/usr/X11R6/include
	LIBS = `sdl-config --libs`
	LIBS += -lEGL -lGLES_CM -lSDL_mixer -lSDL_ttf -lSDL_image
	CXXFLAGS = -O2 -c -Wno-deprecated
	OSXCOMPAT = 
endif

# object files have corresponding source files
CXX = g++

all: tempo

tempo: $(OBJFILES)
	$(CXX) -o tempo $(INCLUDES) $(OSXCOMPAT) $(OBJFILES) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f tempo *.o
