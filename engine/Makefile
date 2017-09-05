# The input source code files and compiled objects for the engine
CFILES = diorama.cpp video.cpp draw.cpp
HFILES = video.h draw.h
OFILES = diorama.o video.o draw.o

# Libraries that we use
INCLUDES = -I"sdl"
LIBS = -lSDL2

# Compiling to test engine
test: $(CFILES) $(HFILES)
	g++ -Wall -Werror -g -c $(INCLUDES) $(CFILES)
	g++ $(OFILES) $(LIBS) -o diorama