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

/* Entry */
int main(int argn,char **argv)
{
	int n,ts,dts,pxl;
	float sec,tps,pps;
	/* Start video */
	if(Video::start())
		return -1;
	/* Handle video */
	Texture *t = new Texture(32,32);
	t->make_test_pattern();
	Vertex2D a,b,c;
	/* Benchmark parameters */
	n = 0;
	ts = System::get_tick();
	pxl = 0;
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		/* Draw np random triangles in one frame */
		for(;;)
		{
			Draw::make_random_vertex(&a,t);
			Draw::make_random_vertex(&b,t);
			Draw::make_random_vertex(&c,t);
			a.color = DRAW_CYAN;
			b.color = DRAW_YELLOW;
			c.color = DRAW_MAGENTA;
			Draw::triangle(&a,&b,&c,t,7);
			n++;
			dts = System::get_tick()-ts;
			if(dts >= 5000)
				break;
		}
		if(Video::end())
			return -1;
		dts = System::get_tick()-ts;
		if(dts >= 5000)
		{
			/* fgetc(stdin); */
			break;
		}
	}
	/* Counts */
	pxl = Draw::get_pixels_filled();
	dts = System::get_tick()-ts;
	printf("Duration: %dms\n",dts);
	printf("Triangles: %d\n",n);
	printf("Pixels: %d\n",pxl);
	/* Power */
	sec = (float)dts;
	sec /= 1000.0f;
	tps = (float)(n);
	tps /= sec;
	pps = (float)(pxl);
	pps /= sec;
	printf("Triangles/sec.: %0.1f\n",tps);
	printf("Fill rate: %0.1f\n",pps);
	/* 803  / 1777 */
	/* 1034 / 1939 */
	/* Goal: 120,000/sec. */
	/* Stop video */
	Video::stop();
	return 0;
}