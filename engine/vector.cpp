/*
	Vector - Matrix and vector math library
*/

/* Include */
#include <math.h>
#include <memory.h>
#include "vector.h"

/* New vector */
Vector :: Vector(float x,float y,float z,float w)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	pos_w = w;
}

/* Delete vector */
Vector :: ~Vector()
{
}

/* Add vector */
void Vector :: add(Vector *v)
{
	pos_x += v->pos_x;
	pos_y += v->pos_y;
	pos_z += v->pos_z;
	pos_w += v->pos_w;
}

/* Subtract vector */
void Vector :: subtract(Vector *v)
{
	pos_x -= v->pos_x;
	pos_y -= v->pos_y;
	pos_z -= v->pos_z;
	pos_w -= v->pos_w;
}

/* Cross product */
void Vector :: cross3(Vector *v)
{
	float x,y,z;
	x = (pos_y*v->pos_z)-(pos_z*v->pos_y);
	y = (pos_z*v->pos_x)-(pos_x*v->pos_z);
	z = (pos_x*v->pos_y)-(pos_y*v->pos_x);
	pos_x = x;
	pos_y = y;
	pos_z = z;
}

/* Dot product */
float Vector :: dot4(Vector *v)
{
	return (pos_x*v->pos_x)+(pos_y*v->pos_y)+(pos_z*v->pos_z)+(pos_w*v->pos_w);
}

/* Dot product (3) */
float Vector :: dot3(Vector *v)
{
	return (pos_x*v->pos_x)+(pos_y*v->pos_y)+(pos_z*v->pos_z);
}

/* 3D Length */
float Vector :: length3()
{
	return (float)sqrt(pos_x*pos_x+pos_y*pos_y+pos_z*pos_z);
}

/* 2D length */
float Vector :: length2()
{
	return (float)sqrt(pos_x*pos_x+pos_y*pos_y);
}

/* Set vector */
void Vector :: set(float x,float y,float z,float w)
{
	pos_x = x;
	pos_y = y;
	pos_z = z;
	pos_w = w;
}

/* Set vector by vector */
void Vector :: set(Vector *v)
{
	set(v->pos_x,v->pos_y,v->pos_z,v->pos_w);
}

/* Matrix zero */
Matrix :: Matrix()
{
	memset(data,0,sizeof(data));
}

/* Delete matrix */
Matrix :: ~Matrix()
{
}

/* Identity */
void Matrix :: identity()
{
	memset(data,0,sizeof(data));
	data[0] = 1.0f;
	data[5] = 1.0f;
	data[10] = 1.0f;
	data[15] = 1.0f;
}

/* Get data */
float Matrix :: get(int r,int c)
{
	return data[c+r*4];
}

/* Multiply vector */
void Vector :: multiply(Matrix *m)
{
	float x,y,z,w;
	x = m->get(0,0)*pos_x+m->get(0,1)*pos_y+m->get(0,2)*pos_z+m->get(0,3)*pos_w;
	y = m->get(1,0)*pos_x+m->get(1,1)*pos_y+m->get(1,2)*pos_z+m->get(1,3)*pos_w;
	z = m->get(2,0)*pos_x+m->get(2,1)*pos_y+m->get(2,2)*pos_z+m->get(2,3)*pos_w;
	w = m->get(3,0)*pos_x+m->get(3,1)*pos_y+m->get(3,2)*pos_z+m->get(3,3)*pos_w;
	set(x,y,z,w);
}

/* Multiply matrix */
void Matrix :: multiply(Matrix *m)
{
	float x1,y1,z1,w1;
	float x2,y2,z2,w2;
	float x3,y3,z3,w3;
	float x4,y4,z4,w4;
	/* 0 */
	x1 = get(0,0)*m->get(0,0)+get(0,1)*m->get(1,0)+get(0,2)*m->get(2,0)+get(0,3)*m->get(3,0);
	y1 = get(1,0)*m->get(0,0)+get(1,1)*m->get(1,0)+get(1,2)*m->get(2,0)+get(1,3)*m->get(3,0);
	z1 = get(2,0)*m->get(0,0)+get(2,1)*m->get(1,0)+get(2,2)*m->get(2,0)+get(2,3)*m->get(3,0);
	w1 = get(3,0)*m->get(0,0)+get(3,1)*m->get(1,0)+get(3,2)*m->get(2,0)+get(3,3)*m->get(3,0);
	/* 1 */
	x2 = get(0,0)*m->get(0,1)+get(0,1)*m->get(1,1)+get(0,2)*m->get(2,1)+get(0,3)*m->get(3,1);
	y2 = get(1,0)*m->get(0,1)+get(1,1)*m->get(1,1)+get(1,2)*m->get(2,1)+get(1,3)*m->get(3,1);
	z2 = get(2,0)*m->get(0,1)+get(2,1)*m->get(1,1)+get(2,2)*m->get(2,1)+get(2,3)*m->get(3,1);
	w2 = get(3,0)*m->get(0,1)+get(3,1)*m->get(1,1)+get(3,2)*m->get(2,1)+get(3,3)*m->get(3,1);
	/* 2 */
	x3 = get(0,0)*m->get(0,2)+get(0,1)*m->get(1,2)+get(0,2)*m->get(2,2)+get(0,3)*m->get(3,2);
	y3 = get(1,0)*m->get(0,2)+get(1,1)*m->get(1,2)+get(1,2)*m->get(2,2)+get(1,3)*m->get(3,2);
	z3 = get(2,0)*m->get(0,2)+get(2,1)*m->get(1,2)+get(2,2)*m->get(2,2)+get(2,3)*m->get(3,2);
	w3 = get(3,0)*m->get(0,2)+get(3,1)*m->get(1,2)+get(3,2)*m->get(2,2)+get(3,3)*m->get(3,2);
	/* 3 */
	x4 = get(0,0)*m->get(0,3)+get(0,1)*m->get(1,3)+get(0,2)*m->get(2,3)+get(0,3)*m->get(3,3);
	y4 = get(1,0)*m->get(0,3)+get(1,1)*m->get(1,3)+get(1,2)*m->get(2,3)+get(1,3)*m->get(3,3);
	z4 = get(2,0)*m->get(0,3)+get(2,1)*m->get(1,3)+get(2,2)*m->get(2,3)+get(2,3)*m->get(3,3);
	w4 = get(3,0)*m->get(0,3)+get(3,1)*m->get(1,3)+get(3,2)*m->get(2,3)+get(3,3)*m->get(3,3);
	/* Update */
	data[0] = x1;
	data[1] = x2;
	data[2] = x3;
	data[3] = x4;
	data[4] = y1;
	data[5] = y2;
	data[6] = y3;
	data[7] = y4;
	data[8] = z1;
	data[9] = z2;
	data[10] = z3;
	data[11] = z4;
	data[12] = w1;
	data[13] = w2;
	data[14] = w3;
	data[15] = w4;
}

/* Translate matrix */
void Matrix :: translate(float x,float y,float z)
{
	data[3] = x;
	data[7] = y;
	data[11] = z;
}

/* Scale matrix */
void Matrix :: scale(float sx,float sy,float sz)
{
	data[0] = sx;
	data[5] = sy;
	data[10] = sz;
}

/* Clone vector */
Vector *Vector :: clone()
{
	Vector *v;
	v = new Vector(pos_x,pos_y,pos_z,pos_w);
	return v;
}

/* Gets x */
float Vector :: get_x()
{
	return pos_x;
}

/* Gets y */
float Vector :: get_y()
{
	return pos_y;
}

/* Gets z */
float Vector :: get_z()
{
	return pos_z;
}

/* Gets w */
float Vector :: get_w()
{
	return pos_w;
}

/* Sets by matrix */
void Matrix :: set(Matrix *m)
{
	memcpy(data,m->data,sizeof(data));
}