/*
	Video - Allows the creation and use of a basic framebuffer and window
*/

/* Includes */
#include <SDL.h>
#include "video.h"

/* Video */
namespace Video
{
	/* Globals */
	SDL_Window *window = 0; /* Window holding the game engine's framebuffer (the window you see stuff in) */
	SDL_Surface *surface = 0; /* Internal video surface used as framebuffer */
	SDL_Surface *screen = 0; /* Window's destination surface, you must get a new one if user ever changes video scale (internal surface doesn't change but this one does) */
	int internal_width = VIDEO_DEFAULT_WIDTH; /* Internal video width */
	int internal_height = VIDEO_DEFAULT_HEIGHT; /* Internal video height */
	int window_scale = VIDEO_DEFAULT_SCALE; /* Default screen scale of display (player gets to set this themselves typically) */
	int active = 0; /* If the video system is active */
	int drawing = 0; /* If the video system is now drawing a frame */
	int surface_pitch = 0; /* Width of a scanline on surface (in ints) */
	int *surface_pixels = 0; /* Pointer to actual surface pixels */
	/* Set internal resolution */
	void set_resolution(int w,int h)
	{
		/* Cannot be done while video is active */
		if(active)
			return;
		/* Set */
		internal_width = w;
		internal_height = h;
	}
	/* Start video */
	int start()
	{
		/* Already started? */	
		if(active)
			return VIDEO_ALREADY_STARTED;
		/* Try to start SDL */
		if(SDL_Init(SDL_INIT_VIDEO) == -1)
			return VIDEO_SDL_FAILURE;
		/* Create window, window title should be set later on by game */
		window = SDL_CreateWindow("...",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,internal_width*window_scale,internal_height*window_scale,0);
		if(!window)
			return VIDEO_WINDOW_FAILURE;
		/* Get destination surface */
		screen = SDL_GetWindowSurface(window);
		if(!screen)
			return VIDEO_SCREEN_FAILURE;
		/* Create surface */
		surface = SDL_CreateRGBSurface(0,internal_width,internal_height,32,VIDEO_MASK_RED,VIDEO_MASK_GREEN,VIDEO_MASK_BLUE,VIDEO_MASK_EXTRA);
		if(!surface)
			return VIDEO_SURFACE_FAILURE;
		SDL_SetSurfaceBlendMode(surface,SDL_BLENDMODE_NONE); /* We don't want SDL to blend the surface used as framebuffer */
		/* Ready */
		active = 1;
		return 0;
	}
	/* Stop video */
	void stop()
	{
		/* Already stopped? */
		if(!active)
			return;
		/* Remove surface */
		SDL_FreeSurface(surface);
		/* Remove window */
		SDL_DestroyWindow(window);
		/* Stop SDL */
		SDL_Quit();
		/* Ready */
		active = 0;
	}
	/* Handles video */
	int handle()
	{
		SDL_Event ev;
		int ret;
		/* Start out willing to continue */
		ret = 1;
		/* Process all events waiting */
		while(SDL_PollEvent(&ev))
		{
			/* User clicked the X on the window typically, or system posted a quit message */
			if(ev.type == SDL_QUIT)
				ret = 0;
		}
		return ret;
	}
	/* Begins a new frame */
	int begin()
	{
		/* Already drawing? */
		if(drawing)
			return VIDEO_ALREADY_STARTED;
		/* Blank out internal surface */
		if(SDL_FillRect(surface,0,0))
			return VIDEO_FILL_FAILURE;
		/* Lock surface */
		if(SDL_LockSurface(surface))
			return VIDEO_LOCK_FAILURE;
		surface_pitch = surface->pitch/4;
		surface_pixels = (int*)surface->pixels;
		/* Ready */
		drawing = 1;
		return 0;
	}
	/* Ends the frame and displays result */
	int end()
	{
		/* Not drawing */
		if(!drawing)
			return VIDEO_ALREADY_ENDED;
		/* Unlock */
		SDL_UnlockSurface(surface);
		/* Transfer to main window */
		if(SDL_BlitScaled(surface,0,screen,0))
			return VIDEO_FILL_FAILURE;
		/* Show */
		SDL_UpdateWindowSurface(window);
		/* Ready */
		drawing = 0;
		return 0;
	}
	/* Sets a pixel on the framebuffer */
	void set_pixel(int x,int y,int c)
	{
		/* Range check */
		if(x < 0 || x >= internal_width)
			return;
		if(y < 0 || y >= internal_height)
			return;
		/* Set at position */
		surface_pixels[x+y*surface_pitch] = c;
	}
	/* Gets a pixel on the framebuffer */
	int get_pixel(int x,int y)
	{
		/* Range check */
		if(x < 0 || x >= internal_width)
			return 0;
		if(y < 0 || y >= internal_height)
			return 0;
		/* Get */
		return surface_pixels[x+y*surface_pitch];
	}
	/* Gets internal width */
	int get_width()
	{
		return internal_width;
	}
	/* Gets internal height */
	int get_height()
	{
		return internal_height;
	}
}