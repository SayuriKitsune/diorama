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

/* Texture */
class Texture
{
private:
	int width; /* Width of texture */
	int height; /* Height of texture */
	int width_mask; /* Width wrap mask */
	int height_mask; /* Height wrap mask */
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

/* 2D Vertex */
typedef struct
{
	int x; /* Location of vertex on screen */
	int y;
	int u; /* Texture coordinate */
	int v;
	int color; /* Vertex color */
}Vertex2D;

/* Color fragment */
typedef struct
{
	float red; /* Channels */
	float green;
	float blue;
	float extra;
}Fragment;

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
		Given three vertices, returns the top one
		a,b,c - the three vertices
	*/
	extern Vertex2D *find_top(Vertex2D *a,Vertex2D *b,Vertex2D *c);
	/*
		Given three vertices, returns the bottom one
		a,b,c - the three vertices
	*/
	extern Vertex2D *find_bottom(Vertex2D *a,Vertex2D *b,Vertex2D *c);
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
		a,b,c - the three 2D vertices
		x,y - coordinate to convert
		af,bf,cf - barycentric coordinates relative to given vertices
	*/
	extern void barycentric(Vertex2D *a,Vertex2D *b,Vertex2D *c,int x,int y,float *af,float *bf,float *cf);
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
		Interpolate between three colors using three factors
		These three factors are often barycentric coordinates
		c1,c2,c3 - the three colors
		af,bf,cf - the three factors
	*/
	extern int interpolate_color(int c1,int c2,int c3,float af,float bf,float cf);
	/*
		Multiplies two colors together and returns result
		c1,c2 - the two colors
	*/
	extern int multiply_color(int c1,int c2);
	/*
		Interpolate between three values using three factors
		x1,x2,x3 - the three values
		af,bf,cf - the three factors
	*/
	extern int interpolate(int x1,int x2,int x3,float af,float bf,float cf);
	/*
		Draws a single slice of a textured 2D triangle
		a,b,c - the three points
		tex - texture
		a1,b1,c1 - first barycentric coordinate (left)
		a2,b2,c2 - second barycentric coordinate (right)
		from,to - x coordinates of left and right end of slice
		y - y coordinate of slice
	*/
	extern void slice(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *tex,float a1,float b1,float c1,float a2,float b2,float c2,int from,int to,int y,int *data);
	/*
		Given three vertices and a texture, draws a 2D textured triangle
		a,b,c - the points of the triangle
		t - the texture to use
	*/
	extern void triangle(Vertex2D *a,Vertex2D *b,Vertex2D *c,Texture *t);
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