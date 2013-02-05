############
MACHINE= $(shell uname -s)

ifeq ($(MACHINE),Darwin)
    OPENGL_INC= -FOpenGL
    OPENGL_LIB= -framework OpenGL
    SDL_INC= `sdl-config --cflags`
    SDL_LIB= `sdl-config --libs`
else
    OPENGL_INC= -I/usr/X11R6/include
    OPENGL_LIB= -I/usr/lib64 -lGL -lGLU
    SDL_INC= `sdl-config --cflags`
    SDL_LIB= `sdl-config --libs`
endif

# object files have corresponding source files
OBJS=sdlglsl.o
CXX=g++
COMPILER_FLAGS= -g
INCLUDE= $(SDL_INC) $(OPENGL_INC)
LIBS= $(SDL_LIB) $(OPENGL_LIB)

EXEC= sdlglsl

$(EXEC): $(OBJS)
    $(CXX) $(COMPILER_FLAGS) -o $(EXEC) $(OBJS) $(LIBS)

%.o:    %.cpp
    $(CXX) -c $(COMPILER_FLAGS) -o $@ $< $(INCLUDE)

#clean:
#    rm -f $(EXEC) $(OBJS)
