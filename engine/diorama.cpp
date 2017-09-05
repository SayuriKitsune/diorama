/*
	Diorama Game Engine
	A retro-style game engine inspired by PSX/Saturn era 3rd person isometric games
*/

/* Includes */
#include "video.h"

#include "draw.h"

/* Entry */
int main(int argn,char **argv)
{
	/* Start video */
	if(Video::start())
		return -1;
	/* Handle video */
	Texture *t = new Texture(32,32);
	t->make_test_pattern();
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		Draw::texture(8,16,t);
		if(Video::end())
			return -1;
	}
	/* Stop video */
	Video::stop();
	return 0;
}