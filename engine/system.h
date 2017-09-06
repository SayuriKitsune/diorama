#ifndef SYSTEM_H
#define SYSTEM_H

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