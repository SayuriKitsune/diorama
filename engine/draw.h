#ifndef DRAW_H
#define DRAW_H

/* Basic colors */
#define DRAW_WHITE   0xFFFFFFFF
#define DRAW_BLACK   0xFF000000
#define DRAW_RED     0xFF0000FF
#define DRAW_GREEN   0xFF00FF00
#define DRAW_BLUE    0xFFFF0000
#define DRAW_YELLOW  0xFF00FFFF
#define DRAW_CYAN    0xFFFFFF00
#define DRAW_MAGENTA 0xFFFF00FF

/* Texture */
class Texture
{
private:
	int width; /* Width of texture */
	int height; /* Height of texture */
	int *data; /* Image pixel data */
public:
	/*
		Allocates a new blank texture
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
}

#endif