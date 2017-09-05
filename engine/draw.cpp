/*
	Draw - Renders 2D primitives onto the framebuffer
*/

/* Includes */
#include <memory.h>
#include "video.h"
#include "draw.h"

/* New texture */
Texture :: Texture(int w,int h)
{
	width = w;
	height = h;
	data = new int[width*height];
	memset(data,0,sizeof(int)*width*height);
}

/* Delete texture */
Texture :: ~Texture()
{
	delete data;
	data = 0;
}

/* Get width */
int Texture :: get_width()
{
	return width;
}

/* Get height */
int Texture :: get_height()
{
	return height;
}

/* Get pixel */
int Texture :: get_pixel(int x,int y)
{
	unsigned int ux,uy;
	ux = (unsigned int)x;
	uy = (unsigned int)y;
	ux = ux%width;
	uy = uy%height;
	return data[ux+uy*width];
}

/* Set pixel */
void Texture :: set_pixel(int x,int y,int c)
{
	/* Range check */
	if(x < 0 || x >= width)
		return;
	if(y < 0 || y >= height)
		return;
	/* Set */
	data[x+y*width] = c;
}

/* Set test pattern */
void Texture :: make_test_pattern()
{
	int x,y,s,c;
	for(y = 0;y < height;y++)
	{
		for(x = 0;x < width;x++)
		{
			/* Checkerboard pixel pattern */
			s = ((x+y)%2);
			if(s)
				c = DRAW_RED;
			else
				c = DRAW_BLUE;
			/* Borders become cyan/yellow pattern */
			if(x == 0 || x == (width-1))
				c = (c|DRAW_GREEN);
			if(y == 0 || y == (height-1))
				c = (c|DRAW_GREEN);
			/* Set */
			set_pixel(x,y,c);
		}
	}
}

/* Draw */
namespace Draw
{
	/* Draw a texture directly */
	void texture(int x,int y,Texture *t)
	{
		int px,py;
		for(py = 0;py < t->get_height();py++)
		{
			for(px = 0;px < t->get_width();px++)
			{
				Video::set_pixel(x+px,y+py,t->get_pixel(px,py));
			}
		}
	}
}