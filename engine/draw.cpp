/*
	Draw - Renders 2D primitives onto the framebuffer
*/

/* Includes */
#include <stdlib.h>
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
	/* Finds the top vertex */
	Vertex2D *find_top(Vertex2D *a,Vertex2D *b,Vertex2D *c)
	{
		Vertex2D *t;
		t = a;
		if(b->y < t->y)
			t = b;
		if(c->y < t->y)
			t = c;
		return t;
	}
	/* Finds the bottom vertex */
	Vertex2D *find_bottom(Vertex2D *a,Vertex2D *b,Vertex2D *c)
	{
		Vertex2D *t;
		t = a;
		if(b->y > t->y)
			t = b;
		if(c->y > t->y)
			t = c;
		return t;
	}
	/* Creates a randomized vertex */
	void make_random_vertex(Vertex2D *v,Texture *t)
	{
		v->x = rand()%Video::get_width();
		v->y = rand()%Video::get_height();
		v->u = rand()%t->get_width();
		v->v = rand()%t->get_height();
		v->color = DRAW_WHITE;
	}
	/* Look up barycentric coordinates */
	void barycentric(Vertex2D *a,Vertex2D *b,Vertex2D *c,int x,int y,float *af,float *bf,float *cf)
	{
		int y2my3;
		int x1mx3;
		int x3mx2;
		int y1my3;
		int y3my1;
		int xmx3;
		int ymy3;
		int det,tx,ty;
		/* Find components */
		y2my3 = b->y-c->y;
		x1mx3 = a->x-c->x;
		x3mx2 = c->x-b->x;
		y1my3 = a->y-c->y;
		y3my1 = c->y-a->y;
		/* Find DET */
		det = (y2my3*x1mx3)+(x3mx2*y1my3);
		/* Find additional components */
		xmx3 = (x-c->x);
		ymy3 = (y-c->y);
		/* Find TX */
		tx = (y2my3*xmx3)+(x3mx2*ymy3);
		ty = (y3my1*xmx3)+(x1mx3*ymy3);
		/* Output as float */
		af[0] = ((float)tx)/((float)det);
		bf[0] = ((float)ty)/((float)det);
		cf[0] = 1.0f-af[0]-bf[0];
	}
	/* Convert from pixel to fragment */
	void pixel_to_fragment(int c,Fragment *f)
	{
		unsigned int red,green,blue,extra;
		/* Separate channels */
		red = (c&0x000000FF);
		green = ((c&0x0000FF00)>>8);
		blue = ((c&0x00FF0000)>>16);
		extra = ((c&0xFF000000)>>24);
		/* Convert to float */
		f->red = (float)red;
		f->green = (float)green;
		f->blue = (float)blue;
		f->extra = (float)extra;
		f->red /= 255.0f;
		f->green /= 255.0f;
		f->blue /= 255.0f;
		f->extra /= 255.0f;
	}
	/* Convert from fragment back to pixel */
	int fragment_to_pixel(Fragment *f)
	{
		unsigned int red,green,blue,extra,c;
		/* Clamp */
		if(f->red < 0.0f)
			f->red = 0.0f;
		if(f->green < 0.0f)
			f->green = 0.0f;
		if(f->blue < 0.0f)
			f->blue = 0.0f;
		if(f->red > 1.0f)
			f->red = 1.0f;
		if(f->green > 1.0f)
			f->green = 1.0f;
		if(f->blue > 1.0f)
			f->blue = 1.0f;
		/* Expand */
		red = (unsigned int)(f->red*255.0f);
		green = (unsigned int)(f->green*255.0f);
		blue = (unsigned int)(f->blue*255.0f);
		extra = (unsigned int)(f->extra*255.0f);
		/* Combine */
		c = red;
		c += (green<<8);
		c += (blue<<16);
		c += (extra<<24);
		return c;
	}
	/* Interpolate three colors */
	int interpolate_color(int c1,int c2,int c3,float af,float bf,float cf)
	{
		Fragment f1,f2,f3,fc;
		/* Convert to fragments */
		pixel_to_fragment(c1,&f1);
		pixel_to_fragment(c2,&f2);
		pixel_to_fragment(c3,&f3);
		/* Interpolate to find center fragment */
		fc.red = f1.red*af+f2.red*bf+f3.red*cf;
		fc.green = f1.green*af+f2.green*bf+f3.green*cf;
		fc.blue = f1.blue*af+f2.blue*bf+f3.blue*cf;
		fc.extra = f1.extra*af+f2.extra*bf+f3.extra*cf;
		/* Return new pixel */
		return fragment_to_pixel(&fc);
	}
	/* Interpolate these values */
	int interpolate(int x1,int x2,int x3,float af,float bf,float cf)
	{
		float f1,f2,f3,f;
		f1 = (float)x1;
		f2 = (float)x2;
		f3 = (float)x3;
		f = f1*af+f2*bf+f3*cf;
		return (int)f;
	}
	/* Multiplies two colors */
	int multiply_color(int c1,int c2)
	{
		Fragment f1,f2,f;
		/* Convert */
		pixel_to_fragment(c1,&f1);
		pixel_to_fragment(c2,&f2);
		/* Multiply */
		f.red = f1.red*f2.red;
		f.green = f1.green*f2.green;
		f.blue = f1.blue*f2.blue;
		f.extra = f1.extra*f2.extra;
		/* Done */
		return fragment_to_pixel(&f);
	}
	/* Blend pixel */
	void blend(int x,int y,int c,int m)
	{
		int s;
		Fragment a,b;
		/* Get source pixel and convert both */
		s = Video::get_pixel(x,y);
		pixel_to_fragment(c,&a);
		pixel_to_fragment(s,&b);
		/* Alpha vs. 1-Alpha blend */
		a.red = a.red*a.extra+b.red*(1.0f-a.extra);
		a.green = a.green*a.extra+b.green*(1.0f-a.extra);
		a.blue = a.blue*a.extra+b.blue*(1.0f-a.extra);
		/* Back to color and set */
		s = fragment_to_pixel(&a);
		Video::set_pixel(x,y,s);
	}
	/* Draws a slice of triangle */
	void slice(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y)
	{
		int x,color,sample;
		float af,bf,cf;
		float d1,d2,d3;
		float run;
		int u,v;
		/* Find the run length of the slice */
		run = (float)(to-from);
		d1 = a2-a1;
		d2 = b2-b1;
		d3 = c2-c1;
		d1 /= run;
		d2 /= run;
		d3 /= run;
		af = a1;
		bf = b1;
		cf = c1;
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			color = interpolate_color(a->color,b->color,c->color,af,bf,cf);
			u = interpolate(a->u,b->u,c->u,af,bf,cf);
			v = interpolate(a->v,b->v,c->v,af,bf,cf);
			sample = tex->get_pixel(u,v);
			color = multiply_color(color,sample);
			blend(x,y,color,1);
			af += d1;
			bf += d2;
			cf += d3;
		}
	}
	/* Draw a 2D textured triangle */
	void triangle(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *t)
	{
		Vertex2D *top;
		Vertex2D *bottom;
		Vertex2D *side;
		int dy1,dy2,dy3;
		int dx1,dx2,dx3;
		float d1,d2,d3;
		float x12,x3;
		int from,to,y;
		float a1,b1,c1,a2,b2,c2;
		/* Find top and bottom */
		top = find_top(a,b,c);
		bottom = find_bottom(a,b,c);
		/* If these are the same, then the triangle is a degenerate and can be discarded from rendering */
		if(top == bottom)
			return;
		/* Find side */
		side = a;
		if(side == top || side == bottom)
			side = b;
		if(side == top || side == bottom)
			side = c;
		/* Find distances */
		dy1 = side->y-top->y; /* From top to side */
		dy2 = bottom->y-side->y; /* From side to bottom */
		dy3 = bottom->y-top->y; /* From whole height of triangle */
		dx1 = side->x-top->x; /* To side */
		dx2 = bottom->x-side->x; /* To bottom */
		dx3 = bottom->x-top->x; /* Whole length */
		/* Find adjustments per slice */
		d1 = ((float)dx1)/((float)dy1); /* Top side */
		d2 = ((float)dx2)/((float)dy2); /* Bottom side */
		d3 = ((float)dx3)/((float)dy3); /* Whole length */
		/* Draw top slice of triangle */
		x12 = (float)top->x;
		x3 = (float)top->x;
		for(y = 0;y < dy1;y++)
		{
			/* Draw dot */
			from = (int)x3;
			to = (int)x12;
			barycentric(top,bottom,side,from,y+top->y,&a1,&b1,&c1);
			barycentric(top,bottom,side,to,y+top->y,&a2,&b2,&c2);
			if(from < to)
				slice(top,bottom,side,t,a1,b1,c1,a2,b2,c2,from,to,y+top->y);
			else
				slice(top,bottom,side,t,a2,b2,c2,a1,b1,c1,to,from,y+top->y);
			/* Adjust */
			x3 += d3;
			x12 += d1;
		}
		/* Draw bottom slice of triangle */
		x12 = (float)side->x;
		for(y = dy1;y < dy3;y++)
		{
			/* Draw dot */
			from = (int)x3;
			to = (int)x12;
			barycentric(top,bottom,side,from,y+top->y,&a1,&b1,&c1);
			barycentric(top,bottom,side,to,y+top->y,&a2,&b2,&c2);
			if(from < to)
				slice(top,bottom,side,t,a1,b1,c1,a2,b2,c2,from,to,y+top->y);
			else
				slice(top,bottom,side,t,a2,b2,c2,a1,b1,c1,to,from,y+top->y);
			/* Adjust */
			x3 += d3;
			x12 += d2;
		}
	}
}