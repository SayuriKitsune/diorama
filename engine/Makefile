# The input source code files and compiled objects for the engine
CFILES = diorama.cpp video.cpp
HFILES = video.h
OFILES = diorama.o video.o

# Libraries that we use
INCLUDES = -I"sdl"
LIBS = -lSDL2

# Compiling to test engine
test: $(CFILES) $(HFILES)
	g++ -Wall -Werror -g -c $(INCLUDES) $(CFILES)
	g++ $(LIBS) $(OFILES) -o diorama