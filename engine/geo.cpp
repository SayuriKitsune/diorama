/*
	Geo - Geometry rendering
*/

/* Includes */
#include "geo.h"
#include "video.h"

/* Geo */
namespace Geo
{
	/* Globals */
	Matrix *geo_transform[GEO_MATRIX_STACK]; /* Current transformation matrix stack */
	Matrix *geo_adjust; /* Adjust transform matrix */
	int geo_stack = 0; /* Current transform stack pointer */
	float geo_screen_scale_y = 1.0f; /* Screen scale values (aspect correction) */
	float geo_screen_scale_x = 0.0f;
	int geo_active = 0; /* Geo render ready? */
	Vector *geo_points[GEO_MAX_POINTS]; /* Points to define object to be drawn */
	Vertex2D geo_vertex[GEO_MAX_POINTS]; /* Vertex matching points */
	Texture *geo_texture; /* Current texture */
	int geo_mode; /* Current render mode */
	/* Init geo library */
	void init()
	{
		int i;
		/* Already started? */
		if(geo_active)
			return;
		/* Find aspect correction */
		geo_screen_scale_y = 1.0f;
		geo_screen_scale_x = ((float)Video::get_height());
		geo_screen_scale_x /= ((float)Video::get_width());
		/* Transform matrix */
		for(i = 0;i < GEO_MATRIX_STACK;i++)
			geo_transform[i] = new Matrix();
		geo_adjust = new Matrix();
		geo_stack = 0;
		/* Points */
		for(i = 0;i < GEO_MAX_POINTS;i++)
			geo_points[i] = new Vector(0.0f,0.0f,0.0f,0.0f);
		/* Texture */
		geo_texture = 0;
		/* Mode */
		geo_mode = DRAW_GOURAD|DRAW_TEXTURE|DRAW_BLEND;
		/* Ready */
		geo_active = 1;
	}
	/* Exit geo library */
	void exit()
	{
		int i;
		/* Already stopped? */
		if(!geo_active)
			return;
		/* Points */
		for(i = 0;i < GEO_MAX_POINTS;i++)
			delete geo_points[i];
		/* Transform matrix */
		for(i = 0;i < GEO_MATRIX_STACK;i++)
			delete geo_transform[i];
		delete geo_adjust;
		/* Done */
		geo_active = 0;
	}
	/* Sets current transform to identity */
	void identity()
	{
		geo_transform[geo_stack]->identity();
	}
	/* Raises the transformation stack by 1 */
	void push()
	{
		Matrix *m;
		/* Max stack? */
		if(geo_stack >= GEO_MATRIX_STACK)
			return;
		/* Push */
		m = geo_transform[geo_stack];
		geo_stack++;
		geo_transform[geo_stack]->set(m);
	}
	/* Lowers the transformation stack by 1 */
	void pop()
	{
		/* Already at bottom? */
		if(geo_stack <= 0)
			return;
		/* Pop */
		geo_stack--;
	}
	/* Applies translation */
	void translate(float x,float y,float z)
	{
		geo_adjust->identity();
		geo_adjust->translate(x,y,z);
		geo_transform[geo_stack]->multiply(geo_adjust);
	}
	/* Applies scale */
	void scale(float sx,float sy,float sz)
	{
		geo_adjust->identity();
		geo_adjust->scale(sx,sy,sz);
		geo_transform[geo_stack]->multiply(geo_adjust);
	}
	/* Converts to screen integer */
	void screen(Vector *v,int *px,int *py)
	{
		float x,y;
		/* Get point (relative to 0,0) */
		x = v->get_x();
		y = v->get_y();
		/* If relative to (0,0) then moving it this much shall work */
		x *= geo_screen_scale_x;
		x *= 0.5f;
		y *= 0.5f;
		x += 0.5f;
		y += 0.5f;
		/* Now we may convert to coords */
		x *= ((float)Video::get_width());
		y *= ((float)Video::get_height());
		px[0] = (int)x;
		py[0] = (int)y;
	}
	/* Draw arrays */
	void draw(int pc,float *ps,int *txs,int *cs,int tc,int *ts)
	{
		int i,ix,ixx;
		Vertex2D *va,*vb,*vc;
		/* Exceeded maximum points */
		if(pc >= GEO_MAX_POINTS)
			return;
		/* Transform points */
		ix = 0;
		ixx = 0;
		for(i = 0;i < pc;i++)
		{
			/* Copy */
			geo_points[i]->set(ps[ix],ps[ix+1],ps[ix+2],1.0f);
			/* Transform */
			transform(geo_points[i]);
			/* To screen */
			screen(geo_points[i],&geo_vertex[i].x,&geo_vertex[i].y);
			/* Place results */
			geo_vertex[i].u = txs[ixx];
			geo_vertex[i].v = txs[ixx+1];
			geo_vertex[i].color = cs[i];
			/* Next */
			ix += 3;
			ixx += 2;
		}
		/* Render triangles */
		ix = 0;
		for(i = 0;i < tc;i++)
		{
			/* Assign vertices */
			va = &geo_vertex[ts[ix]];
			vb = &geo_vertex[ts[ix+1]];
			vc = &geo_vertex[ts[ix+2]];
			/* Draw */
			Draw::triangle(va,vb,vc,geo_texture,geo_mode);
			/* Next */
			ix += 3;
		}
	}
	/* Transforms vector */
	void transform(Vector *v)
	{
		v->multiply(geo_transform[geo_stack]);
	}
	/* Specify texture */
	void texture(Texture *t)
	{
		geo_texture = t;
	}
	/* Specify mode */
	void mode(int m)
	{
		geo_mode = m;
	}
}