#ifndef VECTOR_H
#define VECTOR_H

/* Prototype */
class Matrix;

/* Vector */
class Vector
{
private:
	float pos_x; /* Vector components */
	float pos_y;
	float pos_z;
	float pos_w;
public:
	/*
		Creates a new vector with the given components
	*/
	Vector(float x,float y,float z,float w);
	~Vector();
	/*
		Add vector
	*/
	void add(Vector *v);
	/*
		Subtract vector
	*/
	void subtract(Vector *v);
	/*
		Cross product of three components
	*/
	void cross3(Vector *v);
	/*
		Dot product
	*/
	float dot4(Vector *v);
	float dot3(Vector *v); /* .. of only three components */
	/*
		Finds the 3-dimensional length of the vector
	*/
	float length3();
	float length2(); /* .. of only two components */
	/*
		Replaces the contents of this vector with what is given
		x,y,z,w - the new vector
	*/
	void set(float x,float y,float z,float w);
	void set(Vector *v); /* .. to that of other vector */
	/*
		Multiplies this vector by the given matrix, transforming it
	*/
	void multiply(Matrix *m);
	/*
		Creates a clone of this vector
	*/
	Vector *clone();
	/*
		Gets component
	*/
	float get_x();
	float get_y();
	float get_z();
	float get_w();
};

/* Matrix */
class Matrix
{
private:
	float data[16]; /* 4x4 matrix */
public:
	/*
		Creates a new zero matrix
	*/
	Matrix();
	~Matrix();
	/*
		Sets matrix to the identity matrix
	*/
	void identity();
	/*
		Gets a value via row and column
		r - row
		c - column
	*/
	float get(int r,int c);
	/*
		Multiplies this matrix with the given one, transforming it
	*/
	void multiply(Matrix *m);
	/*
		Creates a translation matrix
		x,y,z - amount to translate to
	*/
	void translate(float x,float y,float z);
	/*
		Creates a scaling matrix
		sx,sy,sz - factor to scale to in each dimension
	*/
	void scale(float sx,float sy,float sz);
};

#endif