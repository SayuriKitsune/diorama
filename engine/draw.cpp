/*
	Draw - Renders 2D primitives onto the framebuffer
*/

/* Includes */
#include <stdlib.h>
#include <memory.h>
#include "video.h"
#include "draw.h"

/* Get mask */
int Texture :: get_mask(int s)
{
	switch(s)
	{
	case 8:
		return 3;
	case 16:
		return 4;
	case 32:
		return 5;
	case 64:
		return 6;
	case 128:
		return 7;
	case 256:
		return 8;
	}
	return -1;
}

/* New texture */
Texture :: Texture(int w,int h)
{
	/* Check for valid size */
	width_mask = get_mask(w);
	height_mask = get_mask(h);
	if(width_mask < 0 || height_mask < 0)
	{
		/* Invalid texture */
		width = 32;
		height = 32;
		width_mask = 5;
		height_mask = 5;
		make_test_pattern();
	}
	else
	{
		/* Valid allocation */
		width = w;
		height = h;
		data = new int[width*height];
		memset(data,0,sizeof(int)*width*height);
	}
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
	ux = (ux&width_mask);
	uy = (uy&height_mask);
	return data[ux+(uy<<width_mask)];
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
				c = DRAW_GRAY;
			else
				c = DRAW_WHITE;
			/* Borders become cyan/yellow pattern */
			if(x == 0 || y == 0)
			{
				if(s)
					c = DRAW_DARK;
			}
			/* Set */
			set_pixel(x,y,c);
		}
	}
}

/* Draw */
namespace Draw
{
	/* Globals */
	int pixels_filled = 0; /* Number of pixels filled (used to calculate fill rate) */
	unsigned char blend_multiply[256][256]; /* Multiply operation LUT */
	unsigned char blend_multiply_inv[256][256]; /* Multiply by one minus alpha LUT */
	/* Populates multiply blend LUTs */
	void calculate_multiply()
	{
		int x,a;
		float xf,af,mf,mfi;
		for(x = 0;x < 256;x++)
		{
			for(a = 0;a < 256;a++)
			{
				/* Convert to fragment range */
				xf = (float)x;
				xf /= 255.0f;
				af = (float)a;
				af /= 255.0f;
				/* Calculate */
				mf = xf*af;
				mfi = xf*(1.0f-af);
				/* Convert back to byte range */
				mf *= 255.0f;
				mfi *= 255.0f;
				blend_multiply[x][a] = (unsigned char)(mf);
				blend_multiply_inv[x][a] = (unsigned char)(mfi);
			}
		}
	}
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
		unsigned char *c1b;
		unsigned char *c2b;
		unsigned char *c3b;
		unsigned char ab,bb,cb;
		/* Convert alpha to bytes */
		ab = (unsigned char)(af*255.0f);
		bb = (unsigned char)(bf*255.0f);
		cb = (unsigned char)(cf*255.0f);
		/* Map to bytes */
		c1b = (unsigned char*)&c1;
		c2b = (unsigned char*)&c2;
		c3b = (unsigned char*)&c3;
		/* Perform */
		c1b[0] = blend_multiply[c1b[0]][ab]+blend_multiply[c2b[0]][bb]+blend_multiply[c3b[0]][cb];
		c1b[1] = blend_multiply[c1b[1]][ab]+blend_multiply[c2b[1]][bb]+blend_multiply[c3b[1]][cb];
		c1b[2] = blend_multiply[c1b[2]][ab]+blend_multiply[c2b[2]][bb]+blend_multiply[c3b[2]][cb];
		c1b[3] = blend_multiply[c1b[3]][ab]+blend_multiply[c2b[3]][bb]+blend_multiply[c3b[3]][cb];
		/* Result is in c1 */
		return c1;
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
		unsigned char *c1b;
		unsigned char *c2b;
		/* Map to bytes */
		c1b = (unsigned char*)&c1;
		c2b = (unsigned char*)&c2;
		/* Apply LUT */
		c1b[0] = blend_multiply[c1b[0]][c2b[0]];
		c1b[1] = blend_multiply[c1b[1]][c2b[1]];
		c1b[2] = blend_multiply[c1b[2]][c2b[2]];
		c1b[3] = blend_multiply[c1b[3]][c2b[3]];
		/* c1 is modified with the result */
		return c1;
	}
	/* Blend pixel */
	int blend(int x,int y,int c,int m,int s)
	{
		unsigned char *cb;
		unsigned char *sb;
		/* Map to bytes */
		cb = (unsigned char*)&c;
		sb = (unsigned char*)&s;
		sb[0] = blend_multiply[cb[0]][cb[3]]+blend_multiply_inv[sb[0]][cb[3]];
		sb[1] = blend_multiply[cb[1]][cb[3]]+blend_multiply_inv[sb[1]][cb[3]];
		sb[2] = blend_multiply[cb[2]][cb[3]]+blend_multiply_inv[sb[2]][cb[3]];
		/* s has been modified with the result */
		return s;
	}
	/* Draws a slice of triangle */
	void slice(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y,int *data)
	{
		int x,color,sample;
		float af,bf,cf;
		float d1,d2,d3;
		float run;
		float u1,v1,u2,v2,u3,v3;
		float dred,dgreen,dblue,dextra;
		float red,green,blue,extra;
		int u,v,s;
		Fragment f1,f2,f3;
		unsigned char *colorb;
		/* Convert colors to fragments */
		pixel_to_fragment(a->color,&f1);
		pixel_to_fragment(b->color,&f2);
		pixel_to_fragment(c->color,&f3);
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
		/* Color pointer */
		colorb = (unsigned char*)&color;
		/* Initial color */
		red = (f1.red*a1+f2.red*b1+f3.red*c1);
		green = (f1.green*a1+f2.green*b1+f3.green*c1);
		blue = (f1.blue*a1+f2.blue*b1+f3.blue*c1);
		extra = (f1.extra*a1+f2.extra*b1+f3.extra*c1);
		/* Gourad coordinates */
		dred = (f1.red*a2+f2.red*b2+f3.red*c2)-red;
		dgreen = (f1.green*a2+f2.green*b2+f3.green*c2)-green;
		dblue = (f1.blue*a2+f2.blue*b2+f3.blue*c2)-blue;
		dextra = (f1.extra*a2+f2.extra*b2+f3.extra*c2)-extra;
		dred /= run;
		dgreen /= run;
		dblue /= run;
		dextra /= run;
		/* Texture coordinates */
		u1 = (float)a->u;
		u2 = (float)b->u;
		u3 = (float)c->u;
		v1 = (float)a->v;
		v2 = (float)b->v;
		v3 = (float)c->v;
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			/* Find multiply source color */
			f1.red = red;
			f1.green = green;
			f1.blue = blue;
			f1.extra = extra;
			colorb[0] = (unsigned char)(f1.red*255.0f);
			colorb[1] = (unsigned char)(f1.green*255.0f);
			colorb[2] = (unsigned char)(f1.blue*255.0f);
			colorb[3] = (unsigned char)(f1.extra*255.0f);
			u = (int)(u1*af+u2*bf+u3*cf);
			v = (int)(v1*af+v2*bf+v3*cf);
			sample = tex->get_pixel(u,v);
			color = multiply_color(color,sample);
			/* Take pixel, blend it and put it back */
			s = data[0];
			data[0] = blend(x,y,color,1,s);
			/* Advance */
			af += d1;
			bf += d2;
			cf += d3;
			red += dred;
			green += dgreen;
			blue += dblue;
			extra += dextra;
			data++;
			pixels_filled++;
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
		int from,to,xf,xt,y;
		int *data;
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
			/* Perform a range check */
			if((y+top->y) >= 0 || (y+top->y) < Video::get_height())
			{
				/* Find range */
				from = (int)x3;
				to = (int)x12;
				if(from < to)
				{
					xf = from;
					xt = to;
				}
				else
				{
					xf = to;
					xt = from;
				}
				/* Limit range */
				if(from < 0)
					from = 0;
				if(to >= Video::get_width())
					to = Video::get_width();
				/* Find interpolants */
				barycentric(top,bottom,side,xf,y+top->y,&a1,&b1,&c1);
				barycentric(top,bottom,side,xt,y+top->y,&a2,&b2,&c2);
				data = Video::get_data(xf,y+top->y);
				slice(top,bottom,side,t,a1,b1,c1,a2,b2,c2,xf,xt,y+top->y,data);
				/* Adjust */
				x3 += d3;
				x12 += d1;
			}
		}
		/* Draw bottom slice of triangle */
		x12 = (float)side->x;
		for(y = dy1;y < dy3;y++)
		{
			/* Perform a range check */
			if((y+top->y) >= 0 || (y+top->y) < Video::get_height())
			{
				/* Find range */
				from = (int)x3;
				to = (int)x12;
				if(from < to)
				{
					xf = from;
					xt = to;
				}
				else
				{
					xf = to;
					xt = from;
				}
				/* Limit range */
				if(from < 0)
					from = 0;
				if(to >= Video::get_width())
					to = Video::get_width();
				/* Find interpolants */
				barycentric(top,bottom,side,xf,y+top->y,&a1,&b1,&c1);
				barycentric(top,bottom,side,xt,y+top->y,&a2,&b2,&c2);
				data = Video::get_data(xf,y+top->y);
				slice(top,bottom,side,t,a1,b1,c1,a2,b2,c2,xf,xt,y+top->y,data);
				/* Adjust */
				x3 += d3;
				x12 += d2;
			}
		}
	}
	/* Get current fill count */
	int get_pixels_filled()
	{
		return pixels_filled;
	}
	/* Reset fill count */
	void reset_pixels_filled()
	{
		pixels_filled = 0;
	}
}