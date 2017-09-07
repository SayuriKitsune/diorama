#ifndef SYSTEM_H
#define SYSTEM_H

/* Fixed point numbers */
typedef int fint;
#define FINT_ONE 1024
#define FINT_MASK 1023
#define FINT_TO_COLOR(a) ((a)>>2)
#define FINT_FROM_COLOR(a) ((a)<<2)
#define FINT_ADD(a,b) ((a)+(b))
#define FINT_SUB(a,b) ((a)-(b))
#define FINT_MUL(a,b) (((a)*(b))>>10)
#define FINT_DIV(a,b) (((a)<<10)/(b))
#define FINT_TO_INT(a) ((a)>>10)
#define FINT_TO_FLOAT(a) (((float)a)/1024.0f)
#define FINT_FROM_INT(a) ((a)<<10)
#define FINT_FROM_FLOAT(a) ((int)((a)*1024.0f))

/* System */
namespace System
{
	/*
		Gets the current system hardware counter time, in milliseconds
		Only useful for basic benchmarking and not frame timing
	*/
	extern int get_tick();
}

#endif