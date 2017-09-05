/*
	Diorama Game Engine
	A retro-style game engine inspired by PSX/Saturn era 3rd person isometric games
*/

/* Includes */
#include "video.h"

/* Entry */
int main(int argn,char **argv)
{
	/* Start video */
	if(Video::start())
		return -1;
	/* Handle video */
	while(Video::handle())
	{
		if(Video::begin())
			return -1;
		Video::set_pixel(0,0,0xFFFFFFFF);
		Video::set_pixel(16,8,VIDEO_MASK_RED);
		Video::set_pixel(8,16,VIDEO_MASK_GREEN);
		Video::set_pixel(16,16,VIDEO_MASK_BLUE);
		if(Video::end())
			return -1;
	}
	/* Stop video */
	Video::stop();
	return 0;
}