#include "shared.h"

void screen_draw(void);
void Get_resolution(void);
void Set_resolution(unsigned short w, unsigned short h);
void SetVideo(unsigned char mode);
void Set_DrawRegion(void);
void take_screenshot(void);

extern unsigned char FPS;
extern unsigned char HVMode;

struct wheretodraw
{
	unsigned short w;
	unsigned short h;
	unsigned short offset_x;
	unsigned short offset_y;
} screen_to_draw_region;

#if !defined(SCALING) && !defined(_TINSPIRE)
	#define flip_screen SDL_Flip
#elif defined(_TINSPIRE)
	void flip_screen(SDL_Surface* screen);
#else
	void flip_screen(SDL_Surface* screen);
	SDL_Surface *real_screen;
#endif

#if !defined(_TINSPIRE)
SDL_Rect screen_position;
#endif

struct scaling
{
	unsigned short w_display;
	unsigned short h_display;
	float w_scale;
	float h_scale;
	float w_scale_size;
	float h_scale_size;
} screen_scale;
