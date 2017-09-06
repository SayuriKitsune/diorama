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
	/* Draws a slice of triangle (texture) */
	void slice_t(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y,int *data)
	{
		int x,color,sample;
		float af,bf,cf;
		float d1,d2,d3;
		float run;
		float u1,v1,u2,v2,u3,v3;
		float du,dv,uu,vv;
		int u,v;
		/* Init */
		color = a->color;
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
		/* Texture coordinates */
		u1 = (float)a->u;
		u2 = (float)b->u;
		u3 = (float)c->u;
		v1 = (float)a->v;
		v2 = (float)b->v;
		v3 = (float)c->v;
		uu = (u1*a1+u2*b1+u3*c1);
		vv = (v1*a1+v2*b1+v3*c1);
		du = (u1*a2+u2*b2+u3*c2)-uu;
		dv = (v1*a2+v2*b2+v3*c2)-vv;
		du /= run;
		dv /= run;
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			/* Find texture coordinate and sample */
			u = (int)(uu);
			v = (int)(vv);
			sample = tex->get_pixel(u,v);
			/* Alpha blend */
			if(sample&0xFF000000)
				data[0] = multiply_color(color,sample);
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
	/* Draws a slice of triangle (texture, gourad) */
	void slice_tg(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y,int *data)
	{
		int x,color,sample;
		float af,bf,cf;
		float d1,d2,d3;
		float run;
		float u1,v1,u2,v2,u3,v3;
		float dred,dgreen,dblue,dextra;
		float red,green,blue,extra;
		float du,dv,uu,vv;
		int u,v;
		unsigned char *colorb;
		/* Init */
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		extra = 0.0f;
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
		red =    (a->fragment.red*a1   +b->fragment.red*b1   +c->fragment.red*c1);
		green =  (a->fragment.green*a1 +b->fragment.green*b1 +c->fragment.green*c1);
		blue =   (a->fragment.blue*a1  +b->fragment.blue*b1  +c->fragment.blue*c1);
		extra =  (a->fragment.extra*a1 +b->fragment.extra*b1 +c->fragment.extra*c1);
		/* Gourad coordinates */
		dred =   (a->fragment.red*a2   +b->fragment.red*b2   +c->fragment.red*c2)   -red;
		dgreen = (a->fragment.green*a2 +b->fragment.green*b2 +c->fragment.green*c2) -green;
		dblue =  (a->fragment.blue*a2  +b->fragment.blue*b2  +c->fragment.blue*c2)  -blue;
		dextra = (a->fragment.extra*a2 +b->fragment.extra*b2 +c->fragment.extra*c2) -extra;
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
		uu = (u1*a1+u2*b1+u3*c1);
		vv = (v1*a1+v2*b1+v3*c1);
		du = (u1*a2+u2*b2+u3*c2)-uu;
		dv = (v1*a2+v2*b2+v3*c2)-vv;
		du /= run;
		dv /= run;
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			/* Find interpolated color */
			if(red > 1.0f) red = 1.0f;
			if(green > 1.0f) green = 1.0f;
			if(blue > 1.0f) blue = 1.0f;
			if(extra > 1.0f) extra = 1.0f;
			colorb[0] = (unsigned char)(red*255.0f);
			colorb[1] = (unsigned char)(green*255.0f);
			colorb[2] = (unsigned char)(blue*255.0f);
			colorb[3] = (unsigned char)(extra*255.0f);
			red += dred;
			green += dgreen;
			blue += dblue;
			extra += dextra;
			/* Find texture coordinate and sample */
			u = (int)(uu);
			v = (int)(vv);
			sample = tex->get_pixel(u,v);
			/* Alpha blend */
			if(sample&0xFF000000)
				data[0] = multiply_color(color,sample);
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
	/* Draws a slice of triangle (texture, blend) */
	void slice_tb(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y,int *data)
	{
		int x,color,sample;
		float af,bf,cf;
		float d1,d2,d3;
		float run;
		float u1,v1,u2,v2,u3,v3;
		float du,dv,uu,vv;
		int u,v,s;
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
		/* Texture coordinates */
		u1 = (float)a->u;
		u2 = (float)b->u;
		u3 = (float)c->u;
		v1 = (float)a->v;
		v2 = (float)b->v;
		v3 = (float)c->v;
		uu = (u1*a1+u2*b1+u3*c1);
		vv = (v1*a1+v2*b1+v3*c1);
		du = (u1*a2+u2*b2+u3*c2)-uu;
		dv = (v1*a2+v2*b2+v3*c2)-vv;
		du /= run;
		dv /= run;
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			/* Find texture coordinate and sample */
			u = (int)(uu);
			v = (int)(vv);
			sample = tex->get_pixel(u,v);
			/* Alpha blend */
			color = multiply_color(a->color,sample);
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
	/* Draws a slice of triangle (texture, gourad, blend) */
	void slice_tgb(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y,int *data)
	{
		int x,color,sample;
		float af,bf,cf;
		float d1,d2,d3;
		float run;
		float u1,v1,u2,v2,u3,v3;
		float dred,dgreen,dblue,dextra;
		float red,green,blue,extra;
		float du,dv,uu,vv;
		int u,v,s;
		unsigned char *colorb;
		/* Init */
		red = 0.0f;
		green = 0.0f;
		blue = 0.0f;
		extra = 0.0f;
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
		red =    (a->fragment.red*a1   +b->fragment.red*b1   +c->fragment.red*c1);
		green =  (a->fragment.green*a1 +b->fragment.green*b1 +c->fragment.green*c1);
		blue =   (a->fragment.blue*a1  +b->fragment.blue*b1  +c->fragment.blue*c1);
		extra =  (a->fragment.extra*a1 +b->fragment.extra*b1 +c->fragment.extra*c1);
		/* Gourad coordinates */
		dred =   (a->fragment.red*a2   +b->fragment.red*b2   +c->fragment.red*c2)   -red;
		dgreen = (a->fragment.green*a2 +b->fragment.green*b2 +c->fragment.green*c2) -green;
		dblue =  (a->fragment.blue*a2  +b->fragment.blue*b2  +c->fragment.blue*c2)  -blue;
		dextra = (a->fragment.extra*a2 +b->fragment.extra*b2 +c->fragment.extra*c2) -extra;
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
		uu = (u1*a1+u2*b1+u3*c1);
		vv = (v1*a1+v2*b1+v3*c1);
		du = (u1*a2+u2*b2+u3*c2)-uu;
		dv = (v1*a2+v2*b2+v3*c2)-vv;
		du /= run;
		dv /= run;
		/* Draw slice */
		for(x = from;x < to;x++)
		{
			/* Find interpolated color */
			if(red > 1.0f) red = 1.0f;
			if(green > 1.0f) green = 1.0f;
			if(blue > 1.0f) blue = 1.0f;
			if(extra > 1.0f) extra = 1.0f;
			colorb[0] = (unsigned char)(red*255.0f);
			colorb[1] = (unsigned char)(green*255.0f);
			colorb[2] = (unsigned char)(blue*255.0f);
			colorb[3] = (unsigned char)(extra*255.0f);
			red += dred;
			green += dgreen;
			blue += dblue;
			extra += dextra;
			/* Find texture coordinate and sample */
			u = (int)(uu);
			v = (int)(vv);
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
		float a1,b1,c1; /* Starting barycentric coordinate */
		float a2,b2,c2; /* Ending barycentric coordinate */
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
				barycentric(top,bottom,side,xfrom,y,&a1,&b1,&c1);
				barycentric(top,bottom,side,xto,y,&a2,&b2,&c2);
				data = Video::get_data(xfrom,y);
				switch(mode)
				{
				case 7:
					slice_tgb(top,bottom,side,t,a1,b1,c1,a2,b2,c2,xfrom,xto,y,data);
					break;
				case 5:
					slice_tg(top,bottom,side,t,a1,b1,c1,a2,b2,c2,xfrom,xto,y,data);
					break;
				case 6:
					slice_tb(top,bottom,side,t,a1,b1,c1,a2,b2,c2,xfrom,xto,y,data);
					break;
				case 4:
					slice_t(top,bottom,side,t,a1,b1,c1,a2,b2,c2,xfrom,xto,y,data);
					break;
				}
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
		/* Update the floating point component for triangle colors */
		pixel_to_fragment(a->color,&a->fragment);
		pixel_to_fragment(b->color,&b->fragment);
		pixel_to_fragment(c->color,&c->fragment);
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