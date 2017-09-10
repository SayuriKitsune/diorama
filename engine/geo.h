#ifndef GEO_H
#define GEO_H

/* Defines */
#define GEO_MATRIX_STACK 16
#define GEO_MAX_POINTS 4

/* Types */
#define GEO_TRIANGLE 0
#define GEO_QUAD 1

/* Includes */
#include "vector.h"
#include "draw.h"

/* Geo */
namespace Geo
{
	/*
		Initializes geometry rendering library
	*/
	extern void init();
	/*
		Closes geometry rendering library
	*/
	extern void exit();
	/*
		Sets the current transform to identity
	*/
	extern void identity();
	/*
		Raises the current transform stack by 1
	*/
	extern void push();
	/*
		Returns to the previous transform
	*/
	extern void pop();
	/*
		Applies a translation
	*/
	extern void translate(float x,float y,float z);
	/*
		Applies a scale
	*/
	extern void scale(float sx,float sy,float sz);
	/*
		Converts from a point in world space to screen space
		Only works on finally transformed vectors
	*/
	extern void screen(Vector *v,int *px,int *py);
	/*
		Transforms vector according to current transform
	*/
	extern void transform(Vector *v);
	/*
		Begins a new shape
	*/
	extern void begin(int t);
	/*
		Adds a point
	*/
	extern void point(float x,float y,float z);
	/*
		End shape and render
	*/
	extern void end();
	/*
		Sets current texture
	*/
	extern void texture(Texture *t);
	/*
		Sets current mode
	*/
	extern void mode(int m);
}

#endif