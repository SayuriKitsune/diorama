# The input source code files and compiled objects for the engine
CFILES = diorama.cpp video.cpp draw.cpp system.cpp
HFILES = video.h draw.h system.h
OFILES = diorama.o video.o draw.o system.o

# Libraries that we use
INCLUDES = -I"sdl"
LIBS = -lSDL2

# Compiling to test engine
test: $(CFILES) $(HFILES)
	-rm diorama
	g++ -Wall -Werror -g -c $(INCLUDES) $(CFILES)
	g++ $(OFILES) $(LIBS) -o diorama
	./diorama