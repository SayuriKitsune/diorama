/*
	System - Contains basic functions for the underlying system and time
*/

/* Includes */
#include <SDL.h>
#include "system.h"

/* System */
namespace System
{
	/* Get current millisecond tick */
	int get_tick()
	{
		Uint64 c,f;
		c = SDL_GetPerformanceCounter();
		f = SDL_GetPerformanceFrequency();
		f = f/1000; /* Frequency is counts per second, we convert to ms */
		return c/f; /* .. convert to current time in ms */
	}
}