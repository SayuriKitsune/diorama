#ifndef VIDEO_H
#define VIDEO_H

/* Default internal video size */
#define VIDEO_DEFAULT_WIDTH 320
#define VIDEO_DEFAULT_HEIGHT 240
#define VIDEO_DEFAULT_SCALE 2

/* Color channel masks */
#define VIDEO_MASK_RED   0x000000FF
#define VIDEO_MASK_GREEN 0x0000FF00
#define VIDEO_MASK_BLUE  0x00FF0000
#define VIDEO_MASK_EXTRA 0xFF000000

/* Error codes */
#define VIDEO_ALREADY_STARTED -1
#define VIDEO_SDL_FAILURE -2
#define VIDEO_WINDOW_FAILURE -3
#define VIDEO_SURFACE_FAILURE -4
#define VIDEO_FILL_FAILURE -5
#define VIDEO_LOCK_FAILURE -6
#define VIDEO_ALREADY_ENDED -7
#define VIDEO_SCREEN_FAILURE -8

/* Video */
namespace Video
{
	/*
		Changes the internal resolution of video (only when video system is not active)
		w,h - the new resolution
	*/
	extern void set_resolution(int w,int h);
	/*
		Starts the video system which also displays the game window
		Returns result code
	*/
	extern int start();
	/*
		Stops the video system
	*/
	extern void stop();
	/*
		Handles one frame of video and window events
		Returns result code
	*/
	extern int handle();
	/*
		Starts drawing a new frame
		Returns result code
	*/
	extern int begin();
	/*
		Stops drawing the frame and displays the result to screen
		Returns result code
	*/
	extern int end();
	/*
		Sets a pixel on framebuffer while drawing a frame
		x,y - pixel coordinate (internal)
		c - pixel
	*/
	extern void set_pixel(int x,int y,int c);
}

#endif