/*
	Diorama Game Engine
	A retro-style game engine inspired by PSX/Saturn era 3rd person isometric games
*/

/* Includes */
#include "video.h"

#include <stdio.h>
#include "draw.h"
#include "system.h"

/* Entry */
int main(int argn,char **argv)
{
	int i,n,ts,dts,np,pxl;
	float sec,tps,pps;
	/* Start video */
	if(Video::start())
		return -1;
	/* Handle video */
	Texture *t = new Texture(32,32);
	t->make_test_pattern();
	Vertex2D a,b,c;
	/* Benchmark parameters */
	np = 4;
	n = 0;
	ts = System::get_tick();
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		/* Draw np random triangles in one frame */
		for(i = 0;i < np;i++)
		{
			Draw::make_random_vertex(&a,t);
			Draw::make_random_vertex(&b,t);
			Draw::make_random_vertex(&c,t);
			a.color = DRAW_CYAN;
			b.color = DRAW_YELLOW;
			c.color = DRAW_MAGENTA;
			Draw::triangle(&a,&b,&c,t);
		}
		if(Video::end())
			return -1;
		/* Do 5 seconds worth of data */
		pxl += Draw::get_pixels_filled();
		n++;
		dts = System::get_tick()-ts;
		if(dts >= 5000)
			break;
	}
	/* Counts */
	dts = System::get_tick()-ts;
	printf("Duration: %dms\n",dts);
	printf("Triangles: %d\n",n*np);
	printf("Pixels: %d\n",pxl);
	/* Power */
	sec = (float)dts;
	sec /= 1000.0f;
	tps = (float)(n*np);
	tps /= sec;
	pps = (float)(pxl);
	pps /= sec;
	printf("Triangles/sec.: %0.1f\n",tps);
	printf("Fill rate: %0.1f\n",pps);
	/* 150 -> 170 -> 215 */
	/* Goal: 120,000/sec. */
	/* Stop video */
	Video::stop();
	return 0;
}