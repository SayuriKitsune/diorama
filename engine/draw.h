#ifndef DRAW_H
#define DRAW_H

/* Basic colors */
#define DRAW_WHITE   0xFFFFFFFF
#define DRAW_GRAY    0xFF808080
#define DRAW_DARK    0xFF404040
#define DRAW_BLACK   0xFF000000
#define DRAW_RED     0xFF0000FF
#define DRAW_GREEN   0xFF00FF00
#define DRAW_BLUE    0xFFFF0000
#define DRAW_YELLOW  0xFF00FFFF
#define DRAW_CYAN    0xFFFFFF00
#define DRAW_MAGENTA 0xFFFF00FF

/* Half translucent */
#define DRAW_TRANSLUCENT 0x80FFFFFF

/* Render modes */
#define DRAW_RAW 0
#define DRAW_GOURAD 1
#define DRAW_BLEND 2
#define DRAW_TEXTURE 4

/* REMARKS: */
/*
	DRAW_RAW (Mode 0) is the fastest and probably most common mode,
	drawing a texture mapped triangle directly without any shading or blending against the scene.
	Textures can still have invisible pixels if the color has 0 in the alpha channel.

	DRAW_TEXTURE (Mode 4) is good for flat shading.

	Modes 5 through 7 shouldn't be used too often,
	static shading and lighting should be done instead where possible.

	DRAW_GOURAD (Mode 1) is good for drawing untextured triangles.
	It is also the second fastest mode which is great since you still get realtime shading.
*/
/* Mode 0: ~4840 ~22552 */
/* Mode 1: ~6829 ~8064  */
/* Mode 3: ~1773 ~2693  */
/* Mode 4: ~2052 ~6446  */
/* Mode 5: ~1805 ~2925  */
/* Mode 6: ~1277 ~2885  */
/* Mode 7: ~1097 ~1921  */

/* Includes */
#include "system.h"

/* Texture */
class Texture
{
private:
	int width; /* Width of texture */
	int height; /* Height of texture */
	int width_mask; /* Width wrap mask */
	int height_mask; /* Height wrap mask */
	int pitch; /* Image pitch (in bit shifts not pixels) */
	int *data; /* Image pixel data */
	/*
		Gets mask value for size, or -1 if invalid
		s - the size
	*/
	int get_mask(int s);
public:
	/*
		Allocates a new blank texture
		Sizes must be in powers of two
		If an invalid texture is defined, this becomes the test pattern texture instead
		w,h - size of texture (in pixels)
	*/
	Texture(int w,int h);
	~Texture();
	/*
		Gets the size of this texture (in pixels)
	*/
	int get_width();
	int get_height();
	/*
		Gets a pixel on texture
		This will wrap if you go out of bounds, so no worries
		x,y - coordinate of pixel to get
	*/
	int get_pixel(int x,int y);
	/*
		Sets a pixel on texture
		Setting a pixel out of bounds does nothing
		x,y - coordinate of pixel to set
		c - pixel
	*/
	void set_pixel(int x,int y,int c);
	/*
		Fills the texture with a test pattern
	*/
	void make_test_pattern();
};

/* Color fragment */
typedef struct
{
	fint red; /* Channels */
	fint green;
	fint blue;
	fint extra;
}Fragment;

/* 2D Vertex */
typedef struct
{
	int x; /* Location of vertex on screen */
	int y;
	int u; /* Texture coordinate */
	int v;
	int color; /* Vertex color */
}Vertex2D;

/* Draw */
namespace Draw
{
	/*
		Draws a texture directly to the framebuffer
		Used mostly for testing purposes
		x,y - coordinate to draw to
		t - texture to draw
	*/
	extern void texture(int x,int y,Texture *t);
	/*
		Sets the given vertex to a random state
		A texture is required as it sets a random texture coordinate too
		The texture is not modified
		v - the vertex
		t - reference texture
	*/
	extern void make_random_vertex(Vertex2D *v,Texture *t);
	/*
		Calculate the barycentric coordinates between three points
		Returns zero if the given triangle is invalid (degenerate)
		a,b,c - the three 2D vertices
		x,y - coordinate to convert
		af,bf,cf - barycentric coordinates relative to given vertices
	*/
	extern int barycentric(Vertex2D *a,Vertex2D *b,Vertex2D *c,int x,int y,float *af,float *bf,float *cf);
	/*
		Converts a pixel to fragment
		c - the pixel
		f - output fragment
	*/
	extern void pixel_to_fragment(int c,Fragment *f);
	/*
		Converts from fragment back to pixel and returns it
		f - the fragment
	*/
	extern int fragment_to_pixel(Fragment *f);
	/*
		Given three vertices and a texture, draws a 2D textured triangle
		a,b,c - the points of the triangle
		t - the texture to use
		mode - render mode
	*/
	extern void triangle(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *t,int mode);
	/*
		Gets the current pixel fill count
	*/
	extern int get_pixels_filled();
	/*
		Resets current pixel fill count
	*/
	extern void reset_pixels_filled();
	/*
		Populates blending LUT tables
	*/
	extern void calculate_multiply();
}

#endif