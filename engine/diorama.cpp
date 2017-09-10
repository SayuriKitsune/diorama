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

/* Entry */
int main(int argn,char **argv)
{
	/* Start video */
	if(Video::start())
		return -1;
	/* Prepare a test quad */
	Texture *t = new Texture(32,32);
	t->make_test_pattern();
	Vertex2D a,b,c,d;
	a.u = 0;
	a.v = 0;
	a.color = DRAW_TRANSLUCENT;
	b.u = 32;
	b.v = 0;
	b.color = DRAW_TRANSLUCENT;
	c.u = 32;
	c.v = 32;
	c.color = DRAW_TRANSLUCENT;
	d.u = 0;
	d.v = 32;
	d.color = DRAW_TRANSLUCENT;
	/* Render quad */
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		/* Points */
		Vector *v1,*v2,*v3,*v4;
		v1 = new Vector(-1.0f,-1.0f,0.0f,1.0f);
		v2 = new Vector(1.0f,-1.0f,0.0f,1.0f);
		v3 = new Vector(1.0f,1.0f,0.0f,1.0f);
		v4 = new Vector(-1.0f,1.0f,0.0f,1.0f);
		/* Transform */
		Matrix *m;
		m = new Matrix();
		m->identity();
		m->scale(64.0f,64.0f,1.0f);
		Matrix *mt;
		mt = new Matrix();
		mt->identity();
		mt->translate(1.0f,1.0f,0.0f);
		m->multiply(mt);
		delete mt;
		v1->multiply(m);
		v2->multiply(m);
		v3->multiply(m);
		v4->multiply(m);
		/* Apply */
		a.x = (int)v1->get_x();
		a.y = (int)v1->get_y();
		b.x = (int)v2->get_x();
		b.y = (int)v2->get_y();
		c.x = (int)v3->get_x();
		c.y = (int)v3->get_y();
		d.x = (int)v4->get_x();
		d.y = (int)v4->get_y();
		/* Draw */
		Draw::triangle(&a,&b,&c,t,7);
		Draw::triangle(&a,&c,&d,t,7);
		/* Reset */
		delete m;
		delete v1;
		delete v2;
		delete v3;
		delete v4;
		if(Video::end())
			return -1;
	}
	/* Stop video */
	Video::stop();
	return 0;
}