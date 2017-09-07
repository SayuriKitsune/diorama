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
	pitch = get_mask(w);
	width_mask = w-1;
	height_mask = h-1;
	if(pitch < 0)
	{
		/* Invalid texture */
		width = 32;
		height = 32;
		width_mask = 31;
		height_mask = 31;
		pitch = 5;
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
	return data[ux+(uy<<pitch)];
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
				c = DRAW_DARK;
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
	int draw_y2my3; /* Components of the calculation we only need once per triangle */
	int draw_x1mx3;
	int draw_x3mx2;
	int draw_y1my3;
	int draw_y3my1;
	int draw_det;
	int barycentric(Vertex2D *a,Vertex2D *b,Vertex2D *c,int x,int y,float *af,float *bf,float *cf)
	{
		int xmx3;
		int ymy3;
		int tx,ty;
		/* Find components */
		draw_y2my3 = b->y-c->y;
		draw_x1mx3 = a->x-c->x;
		draw_x3mx2 = c->x-b->x;
		draw_y1my3 = a->y-c->y;
		draw_y3my1 = c->y-a->y;
		/* Find DET */
		draw_det = (draw_y2my3*draw_x1mx3)+(draw_x3mx2*draw_y1my3);
		if(draw_det == 0)
			return 0; /* Triangle is a degenerate */
		/* Find additional components */
		xmx3 = (x-c->x);
		ymy3 = (y-c->y);
		/* Find tx and ty */
		tx = (draw_y2my3*xmx3)+(draw_x3mx2*ymy3);
		ty = (draw_y3my1*xmx3)+(draw_x1mx3*ymy3);
		/* Output result */
		af[0] = ((float)tx)/((float)draw_det);
		bf[0] = ((float)ty)/((float)draw_det);
		cf[0] = 1.0f-af[0]-bf[0];
		return 1;
	}
	/* Look up barycentric coordinates (faster) having already found the more constant intermediate values */
	void barycentric_fast(Vertex2D *c,int x,int y,fint *af,fint *bf,fint *cf)
	{
		int xmx3;
		int ymy3;
		int tx,ty;
		float aa,bb,cc;
		/* Find additional components */
		xmx3 = (x-c->x);
		ymy3 = (y-c->y);
		/* Find tx and ty */
		tx = (draw_y2my3*xmx3)+(draw_x3mx2*ymy3);
		ty = (draw_y3my1*xmx3)+(draw_x1mx3*ymy3);
		/* Find result */
		aa = ((float)tx)/((float)draw_det);
		bb = ((float)ty)/((float)draw_det);
		cc = 1.0f-aa-bb;
		/* Convert */
		af[0] = FINT_FROM_FLOAT(aa);
		bf[0] = FINT_FROM_FLOAT(bb);
		cf[0] = FINT_FROM_FLOAT(cc);
	}
	/* Convert from pixel to fragment */
	void pixel_to_fragment(int c,Fragment *f)
	{
		/* Separate channels */
		f->red = (c&0x000000FF);
		f->green = ((c&0x0000FF00)>>8);
		f->blue = ((c&0x00FF0000)>>16);
		f->extra = ((c&0xFF000000)>>24);
	}
	/* Convert from fragment back to pixel */
	int fragment_to_pixel(Fragment *f)
	{
		unsigned int c;
		/* Clamp */
		if(f->red   < 0)      f->red = 0;
		if(f->green < 0)    f->green = 0;
		if(f->blue  < 0)     f->blue = 0;
		if(f->extra < 0)    f->extra = 0;
		if(f->red   > FINT_MASK)   f->red = FINT_MASK;
		if(f->green > FINT_MASK) f->green = FINT_MASK;
		if(f->blue  > FINT_MASK)  f->blue = FINT_MASK;
		if(f->extra > FINT_MASK) f->extra = FINT_MASK;
		/* Combine */
		c = FINT_TO_COLOR(f->red);
		c += (FINT_TO_COLOR(f->green)<<8);
		c += (FINT_TO_COLOR(f->blue)<<16);
		c += (FINT_TO_COLOR(f->extra)<<24);
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
	int blend(int x,int y,int c,int s)
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
	fint draw_a1,draw_b1,draw_c1; /* Barycentric coordinate (from) */
	fint draw_a2,draw_b2,draw_c2; /* Barycentric coordinate (to) */
	Fragment draw_f1,draw_f2,draw_f3; /* Fragment versions of each vertex color */
	void slice(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,int from,int to,int y,int *data)
	{
		int x,color,sample;
		fint af,bf,cf;
		fint d1,d2,d3;
		fint run;
		fint u1,v1,u2,v2,u3,v3;
		fint dred,dgreen,dblue,dextra;
		fint red,green,blue,extra;
		fint du,dv,uu,vv;
		int u,v,s;
		unsigned char *colorb;
		/* Init */
		red = 0;
		green = 0;
		blue = 0;
		extra = 0;
		/* Find the run length of the slice */
		run = FINT_FROM_INT(to-from);
		if(run <= 0)
			return;
		d1 = draw_a2-draw_a1;
		d2 = draw_b2-draw_b1;
		d3 = draw_c2-draw_c1;
		d1 = FINT_DIV(d1,run);
		d1 = FINT_DIV(d2,run);
		d1 = FINT_DIV(d3,run);
		af = draw_a1;
		bf = draw_b1;
		cf = draw_c1;
		/* Color pointer */
		colorb = (unsigned char*)&color;
		/* Initial color */
		red =    FINT_MUL(draw_f1.red,draw_a1)  +FINT_MUL(draw_f2.red,draw_b1)  +FINT_MUL(draw_f3.red,draw_c1);
		green =  FINT_MUL(draw_f1.green,draw_a1)+FINT_MUL(draw_f2.green,draw_b1)+FINT_MUL(draw_f3.green,draw_c1);
		blue =   FINT_MUL(draw_f1.blue,draw_a1) +FINT_MUL(draw_f2.blue,draw_b1) +FINT_MUL(draw_f3.blue,draw_c1);
		extra =  FINT_MUL(draw_f1.extra,draw_a1)+FINT_MUL(draw_f2.extra,draw_b1)+FINT_MUL(draw_f3.extra,draw_c1);
		/* Gourad coordinates */
		dred =   FINT_MUL(draw_f1.red,draw_a2)  +FINT_MUL(draw_f2.red,draw_b2)  +FINT_MUL(draw_f3.red,draw_c2);
		dgreen = FINT_MUL(draw_f1.green,draw_a2)+FINT_MUL(draw_f2.green,draw_b2)+FINT_MUL(draw_f3.green,draw_c2);
		dblue =  FINT_MUL(draw_f1.blue,draw_a2) +FINT_MUL(draw_f2.blue,draw_b2) +FINT_MUL(draw_f3.blue,draw_c2);
		dextra = FINT_MUL(draw_f1.extra,draw_a2)+FINT_MUL(draw_f2.extra,draw_b2)+FINT_MUL(draw_f3.extra,draw_c2);
		dred =   FINT_SUB(dred,red);
		dgreen = FINT_SUB(dgreen,green);
		dblue =  FINT_SUB(dblue,blue);
		dextra = FINT_SUB(dextra,extra);
		dred =   FINT_DIV(dred,run);
		dgreen = FINT_DIV(dgreen,run);
		dblue =  FINT_DIV(dblue,run);
		dextra = FINT_DIV(dextra,run);
		/* Texture coordinates */
		u1 = FINT_FROM_INT(a->u);
		u2 = FINT_FROM_INT(b->u);
		u3 = FINT_FROM_INT(c->u);
		v1 = FINT_FROM_INT(a->v);
		v2 = FINT_FROM_INT(b->v);
		v3 = FINT_FROM_INT(c->v);
		uu = FINT_MUL(u1,draw_a1)+FINT_MUL(u2,draw_b1)+FINT_MUL(u3,draw_c1);
		vv = FINT_MUL(v1,draw_a1)+FINT_MUL(v2,draw_b1)+FINT_MUL(v3,draw_c1);
		du = FINT_MUL(u1,draw_a2)+FINT_MUL(u2,draw_b2)+FINT_MUL(u3,draw_c2);
		dv = FINT_MUL(v1,draw_a2)+FINT_MUL(v2,draw_b2)+FINT_MUL(v3,draw_c2);
		du = FINT_SUB(du,uu);
		dv = FINT_SUB(dv,vv);
		du = FINT_DIV(du,run);
		dv = FINT_DIV(dv,run);
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			/* Find interpolated color */
			if(red > 0xFF)   colorb[0] = 0xFF; else colorb[0] = red;
			if(green > 0xFF) colorb[1] = 0xFF; else colorb[1] = green;
			if(blue > 0xFF)  colorb[2] = 0xFF; else colorb[2] = blue;
			if(extra > 0xFF) colorb[3] = 0xFF; else colorb[3] = extra;
			red += dred;
			green += dgreen;
			blue += dblue;
			extra += dextra;
			/* Find texture coordinate and sample */
			u = FINT_TO_INT(uu);
			v = FINT_TO_INT(vv);
			sample = tex->get_pixel(u,v);
			/* Alpha blend */
			color = multiply_color(color,sample);
			s = data[0];
			data[0] = blend(x,y,color,s);
			/* Advance */
			uu += du;
			vv += dv;
			af += d1;
			bf += d2;
			cf += d3;
			data++;
			pixels_filled++;
		}
	}
	/* Draws a single rise of a triangle */
	float rise(Vertex2D *top,Vertex2D *bottom,Vertex2D *side,int yfrom,int yto,float dlong,float dside,float xslong,float xsside,Texture *t,int mode)
	{
		int y; /* Current y coordinate */
		float xlong; /* Long side x location */
		float xside; /* Short side x location */
		int from; /* Left side x coordinate of slice */
		int to; /* Right side x coordinate of slice */
		int xfrom; /* Actual left x coordinate of slice */
		int xto; /* Actual right x coordinate of slice */
		int *data; /* Pointer to pixel data */
		/* Start x coordinates off */
		xside = xsside;
		xlong = xslong;
		/* Process all the slices */
		for(y = yfrom;y < yto;y++)
		{
			/* Perform a range check */
			if(y >= 0 && y < Video::get_height())
			{
				/* Find range */
				from = (int)xlong;
				to = (int)xside;
				if(from < to)
				{
					xfrom = from;
					xto = to;
				}
				else
				{
					xfrom = to;
					xto = from;
				}
				/* Limit range */
				if(xfrom < 0)
					xfrom = 0;
				if(xto >= Video::get_width())
					xto = Video::get_width();
				/* Find interpolants */
				barycentric_fast(side,xfrom,y,&draw_a1,&draw_b1,&draw_c1);
				barycentric_fast(side,xto,y,&draw_a2,&draw_b2,&draw_c2);
				/* Draw slice */
				data = Video::get_data(xfrom,y);
				slice(top,bottom,side,t,xfrom,xto,y,data);
				/* Adjust */
				xlong += dlong;
				xside += dside;
			}
		}
		return xlong;
	}
	/* Draw a 2D textured triangle */
	void triangle(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *t,int mode)
	{
		Vertex2D *top; /* The vertex assigned to be the top */
		Vertex2D *bottom; /* The vertex assigned to be the bottom */
		Vertex2D *side; /* The vertex assigned to be the side */
		int dy1,dy2,dy3; /* Differences in heights between the points */
		int dx1,dx2,dx3; /* Differences in x */
		float d1,d2,d3; /* Step sizes for x */
		float xcont; /* Where the x coordinate on the long side is to be resumed at */
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
		/* Assign fragments */
		if(top == a) pixel_to_fragment(a->color,&draw_f1);
		if(top == b) pixel_to_fragment(b->color,&draw_f1);
		if(top == c) pixel_to_fragment(c->color,&draw_f1);
		if(bottom == a) pixel_to_fragment(a->color,&draw_f2);
		if(bottom == b) pixel_to_fragment(b->color,&draw_f2);
		if(bottom == c) pixel_to_fragment(c->color,&draw_f2);
		if(side == a) pixel_to_fragment(a->color,&draw_f3);
		if(side == b) pixel_to_fragment(b->color,&draw_f3);
		if(side == c) pixel_to_fragment(c->color,&draw_f3);
		/* Call once to populate barycentric intermediates */
		barycentric(top,bottom,side,0,0,&d1,&d2,&d3);
		if(draw_det == 0)
			return;
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
		xcont = rise(top,bottom,side,top->y,top->y+dy1,d3,d1,(float)top->x,(float)top->x,t,mode);
		rise(top,bottom,side,top->y+dy1,top->y+dy3,d3,d2,xcont,(float)side->x,t,mode);
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