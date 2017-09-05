/*
	Diorama Game Engine
	A retro-style game engine inspired by PSX/Saturn era 3rd person isometric games
*/

/* Includes */
#include "video.h"

#include <stdio.h>
#include "draw.h"

/* Entry */
int main(int argn,char **argv)
{
	int i;
	/* Start video */
	if(Video::start())
		return -1;
	/* Handle video */
	Texture *t = new Texture(32,32);
	t->make_test_pattern();
	Vertex2D a,b,c;
	a.x = 0;
	a.y = 0;
	a.u = 0;
	a.v = 0;
	a.color = DRAW_TRANSLUCENT;
	b.x = 0;
	b.y = 64;
	b.u = 32;
	b.v = 0;
	b.color = DRAW_TRANSLUCENT;
	c.x = 64;
	c.y = 64;
	c.u = 32;
	c.v = 32;
	c.color = DRAW_TRANSLUCENT;
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		for(i = 0;i < 4;i++)
		{
			Draw::make_random_vertex(&a,t);
			Draw::make_random_vertex(&b,t);
			Draw::make_random_vertex(&c,t);
			a.color = DRAW_TRANSLUCENT;
			b.color = DRAW_TRANSLUCENT;
			c.color = DRAW_TRANSLUCENT;
			Draw::triangle(&a,&b,&c,t);
		}
		if(Video::end())
			return -1;
		/* fgetc(stdin); */
	}
	/* Stop video */
	Video::stop();
	return 0;
}