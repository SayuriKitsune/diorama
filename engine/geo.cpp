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
	int geo_point = 0; /* Current point */
	int geo_type = 0; /* Current thing to render */
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
		geo_point = 0;
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
	/* Transforms vector */
	void transform(Vector *v)
	{
		v->multiply(geo_transform[geo_stack]);
	}
	/* Begins new shape */
	void begin(int t)
	{
		geo_point = 0;
		geo_type = t;
	}
	/* Adds point */
	void point(float x,float y,float z)
	{
		if(geo_point >= GEO_MAX_POINTS)
			return;
		geo_points[geo_point]->set(x,y,z,1.0f);
		geo_point++;
	}
	/* Finish and render shape */
	void end()
	{
		int i;
		/* First we transform points */
		for(i = 0;i < geo_point;i++)
		{
			transform(geo_points[i]);
			screen(geo_points[i],&geo_vertex[i].x,&geo_vertex[i].y);
			geo_vertex[i].u = 0;
			geo_vertex[i].v = 0;
			geo_vertex[i].color = DRAW_WHITE;
		}
		/* Then we render */
		switch(geo_type)
		{
		case GEO_TRIANGLE:
			Draw::triangle(&geo_vertex[0],&geo_vertex[1],&geo_vertex[2],geo_texture,geo_mode);
			break;
		case GEO_QUAD:
			Draw::triangle(&geo_vertex[0],&geo_vertex[1],&geo_vertex[2],geo_texture,geo_mode);
			Draw::triangle(&geo_vertex[0],&geo_vertex[2],&geo_vertex[3],geo_texture,geo_mode);
			break;
		}
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