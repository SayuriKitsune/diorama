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
		Geo::begin(GEO_QUAD);
		Geo::point(-1.0f,-1.0f,0.0f);
		Geo::point(1.0f,-1.0f,0.0f);
		Geo::point(1.0f,1.0f,0.0f);
		Geo::point(-1.0f,1.0f,0.0f);
		Geo::end();
		if(Video::end())
			return -1;
	}
	/* Stop video */
	Video::stop();
	return 0;
}