/*
	Diorama Game Engine
	A retro-style game engine inspired by PSX/Saturn era 3rd person isometric games
*/

/* Includes */
#include "video.h"

#include <stdio.h>
#include <stdlib.h>
#include "draw.h"
#include "system.h"
#include "vector.h"
#include "geo.h"

/* Entry */
float points[] = {-1.0f,-1.0f,0.0f,
                  1.0f,-1.0f,0.0f,
                  1.0f,1.0f,0.0f,
                  -1.0f,1.0f,0.0f};
int coords[] = {0,0,32,0,32,32,0,32};
int colors[] = {DRAW_CYAN,DRAW_MAGENTA,DRAW_YELLOW,DRAW_TRANSLUCENT};
int triangles[] = {0,1,2,0,2,3};
int main(int argn,char **argv)
{
	/* Start video */
	if(Video::start())
		return -1;
	/* Prepare a test quad */
	Texture *t = new Texture(32,32);
	t->make_test_pattern();
	/* Render quad */
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		Geo::identity();
		Geo::scale(0.5f,0.5f,0.5f);
		Geo::texture(t);
		Geo::draw(4,points,coords,colors,2,triangles);
		if(Video::end())
			return -1;
	}
	/* Stop video */
	Video::stop();
	return 0;
}