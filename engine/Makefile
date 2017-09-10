# The input source code files and compiled objects for the engine
CFILES = diorama.cpp video.cpp draw.cpp system.cpp vector.cpp geo.cpp
HFILES = video.h draw.h system.h vector.h geo.h
OFILES = diorama.o video.o draw.o system.o vector.o geo.o

# Optimizer flags
CFLAGS = -Wall -Werror -Wno-maybe-uninitialized -g
RFLAGS = 
OFLAGS = -O3 -Ofast -mfpmath=sse -msse3 -m64

# Libraries that we use
INCLUDES = -I"sdl"
LIBS = -lSDL2

# Compiling to test engine
test: $(CFILES) $(HFILES)
	-rm diorama
	g++ $(CFLAGS) $(RFLAGS) -c $(INCLUDES) $(CFILES)
	g++ $(OFILES) $(LIBS) $(RFLAGS) -o diorama
	./diorama