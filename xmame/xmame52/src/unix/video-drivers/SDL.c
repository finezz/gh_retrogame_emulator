/***************************************************************************
                                          
 This is the SDL XMAME display driver.
 FIrst incarnation by Tadeusz Szczyrba <trevor@pik-nel.pl>,
 based on the Linux SVGALib adaptation by Phillip Ezolt.

 updated and patched by Ricardo Calixto Quesada (riq@core-sdi.com)

 patched by Patrice Mandin (pmandin@caramail.com)
  modified support for fullscreen modes using SDL and XFree 4
  added toggle fullscreen/windowed mode (Alt + Return)
  added title for the window
  hide mouse cursor in fullscreen mode
  added command line switch to start fullscreen or windowed
  modified the search for the best screen size (SDL modes are sorted by
    Y size)

 patched by Dan Scholnik (scholnik@ieee.org)
  added support for 32bpp XFree86 modes
  new update routines: 8->32bpp & 16->32bpp

 TODO: Test the 16bpp->24bpp update routine
       Test the 16bpp->32bpp update routine
       Improve performance.
       Test mouse buttons (which games use them?)

***************************************************************************/
#define __SDL_C

#undef SDL_DEBUG

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <SDL.h>
#include "xmame.h"
#include "devices.h"
#include "keyboard.h"
#include "driver.h"
#include "SDL-keytable.h"


#ifdef VISIBLE_AREA_WORKAROUND
static int display_already_created = 0;
#endif
static int Display_pitch;	/* Added for IPU_SCALER support; used by IPU and Software scalers */
static int Display_BytesPerPixel;
static int Display_width;
static int Display_height;
static int Display_centering_offset;
static int Display_centering_scaled_width;
static int Display_centering_scaled_height;
static int Vid_depth;
/* The following are used by the scale blitter */
unsigned int iAddX = 1, iModuloX = 1, iAddY = 1, iModuloY = 1;
#ifdef GCW0
#define DRAWSURF Surface
static int doublebuf=0;
#ifdef IPU_SCALER
static int ipu_scaler;
static char *ipu_scaler_file;
static char *ipu_aspect_ratio_file;
#endif
#else
static SDL_Surface* Offscreen_surface;
#endif
static SDL_Surface* Surface;
static int hardware=1;
static int mode_number=-1;
static int list_modes=0;
static int start_fullscreen=0;
SDL_Color *Colors=NULL;
static int cursor_state; /* previous mouse cursor state */

typedef void (*update_func_t)(struct osd_bitmap *bitmap);

update_func_t update_function;

static int sdl_mapkey(struct rc_option *option, const char *arg, int priority);

struct rc_option display_opts[] = {
    /* name, shortname, type, dest, deflt, min, max, func, help */
   { "SDL Related",  NULL,    rc_seperator,  NULL,
       NULL,         0,       0,             NULL,
       NULL },
   { "listmodes",    NULL,    rc_bool,       &list_modes,
      "0",           0,       0,             NULL,
      "List all posible full-screen modes" },
   { "fullscreen",   NULL,    rc_bool,       &start_fullscreen,
      "0",           0,       0,             NULL,
      "Start fullscreen" },
#ifdef GCW0
   { "doublebuf",   "triplebuf", rc_bool,    &doublebuf,
      "0",           0,       0,             NULL,
      "Use triple buffering to reduce flicker/tearing" },
#ifdef IPU_SCALER
   { "ipu_scaler",  "ipu",    rc_int,       &ipu_scaler,
     "2",            0,       2,             NULL,
     "Use the GCW-Zero IPU to perform hardware scaling." },
#ifdef IPU_SCALER
   { "ipu_scaler_file","isf", rc_string,     &ipu_scaler_file,
     "/sys/devices/platform/jz-lcd.0/allow_downscaling", 0, 0, NULL,
     "Filename to allow GCW-Zero IPU scaling" },
   { "ipu_aspect_ratio_file","iarf", rc_string,     &ipu_aspect_ratio_file,
     "/sys/devices/platform/jz-lcd.0/keep_aspect_ratio", 0, 0, NULL,
     "Filename to determine type of GCW-Zero IPU scaling" },
#endif
#endif
#endif
   { "modenumber",   NULL,    rc_int,        &mode_number,
      "-1",          0,       0,             NULL,
      "Try to use the 'n' possible full-screen mode" },
   { "sdlmapkey",	"sdlmk",	rc_use_function,	NULL,
     NULL,		0,			0,		sdl_mapkey,
     "Set a specific key mapping, see xmamerc.dist" },
   { NULL,           NULL,    rc_end,        NULL,
      NULL,          0,       0,             NULL,
      NULL }
};

void list_sdl_modes(void);
#ifdef GCW0
void sdl_update_8_to_16bpp(struct osd_bitmap *bitmap);
void sdl_update_8_to_16bpp_horzscale(struct osd_bitmap *bitmap);
void sdl_update_8_to_16bpp_scale(struct osd_bitmap *bitmap);
void sdl_update_8_to_16bpp_rot0_1_2(struct osd_bitmap *bitmap);
void sdl_update_8_to_16bpp_horzscale_1_2(struct osd_bitmap *bitmap);
void sdl_update_16_to_16bpp(struct osd_bitmap *bitmap);
void sdl_update_16_to_16bpp_horzscale(struct osd_bitmap *bitmap);
void sdl_update_16_to_16bpp_scale(struct osd_bitmap *bitmap);
void sdl_update_16_to_16bpp_rot0_1_2(struct osd_bitmap *bitmap);
void sdl_update_16_to_16bpp_horzscale_1_2(struct osd_bitmap *bitmap);
void sdl_update_32_to_32bpp_horzscale(struct osd_bitmap *bitmap);
void sdl_update_rgb_direct_16bpp(struct osd_bitmap *bitmap);
void sdl_update_rgb_direct_32bpp(struct osd_bitmap *bitmap);

#else
void sdl_update_8_to_8bpp(struct osd_bitmap *bitmap);
void sdl_update_8_to_16bpp(struct osd_bitmap *bitmap);
void sdl_update_8_to_24bpp(struct osd_bitmap *bitmap);
void sdl_update_8_to_32bpp(struct osd_bitmap *bitmap);
void sdl_update_16_to_16bpp(struct osd_bitmap *bitmap);
void sdl_update_16_to_24bpp(struct osd_bitmap *bitmap);
void sdl_update_16_to_32bpp(struct osd_bitmap *bitmap);
void sdl_update_rgb_direct_32bpp(struct osd_bitmap *bitmap);
#endif

static inline void CalcCenterDisplay(void)
{
    int offset_x;
    int offset_y;
    offset_x = (Display_width - Display_centering_scaled_width*widthscale ) / 2;
    offset_y = (Display_height - Display_centering_scaled_height*heightscale ) / 2;

    if (offset_x < 0) offset_x = 0;
    if (offset_y < 0) offset_y = 0;

    Display_centering_offset = offset_x + (Display_width * offset_y);

    printf("Display_centering_offset = %d (x:%d y:%d)\n", Display_centering_offset, offset_x, offset_y);
    printf("Display_height %d Display_centering_scaled_height %d heightscale %d\n", Display_height, Display_centering_scaled_height, heightscale);
    printf("Display_width %d Display_centering_scaled_width %d widthscale %d\n", Display_width, Display_centering_scaled_width, widthscale);
}

void sysdep_ipu_aspect_ratio(int keep_aspect_ratio)
{
    FILE* aspect_ratio_file = fopen(ipu_aspect_ratio_file, "w");
    if (aspect_ratio_file)
    {
        fwrite(keep_aspect_ratio?"1":"0", 1, 1, aspect_ratio_file);
        fclose(aspect_ratio_file);
    }
}

void sysdep_ipu_scaler(int enable_ipu_scaler)
{
    FILE* scaler_file = fopen(ipu_scaler_file, "w");
    if (scaler_file)
    {
        fwrite(enable_ipu_scaler?"1":"0", 1, 1, scaler_file);
        fclose(scaler_file);
    }
}


int sysdep_init(void)
{
   /* This needs to be set before SDL_Init() is called */
   if (ipu_scaler)
   {
      sysdep_ipu_scaler(1);
      printf("Enable IPU hardware scaling\n");
      /* Setup the IPU scaler */
      if (ipu_scaler==2 /* fullscreen */)
         sysdep_ipu_aspect_ratio(0);
      else 
         sysdep_ipu_aspect_ratio(1);
   }

   if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      fprintf (stderr, "SDL: Error: %s\n",SDL_GetError());
      return OSD_NOT_OK;
   } 
   fprintf (stderr, "SDL: Info: SDL initialized\n");
   //atexit (SDL_Quit);
   return OSD_OK;
}

void sysdep_close(void)
{
#ifdef VISIBLE_AREA_WORKAROUND
   display_already_created = 0;
#endif
   /* Restore cursor state */
   SDL_ShowCursor(cursor_state);

   SDL_Quit();
}

int sysdep_create_display(int width, int height, int depth, int direct_mapped_15bpp)
{
   SDL_Rect** vid_modes;
   const SDL_VideoInfo* video_info;
   int vid_modes_i;
   int vid_mode_flag; /* Flag to set the video mode */
printf("sysdep_create_display(%dx%d %dbpp direct=%d\n", width, height, depth, direct_mapped_15bpp);

#ifdef VISIBLE_AREA_WORKAROUND
    if (display_already_created)
    {
       return OSD_OK;;
    }
    display_already_created = 1;
#endif

   if(list_modes){
      list_sdl_modes();
      exit (OSD_OK);
   }

   video_info = SDL_GetVideoInfo();

#ifdef SDL_DEBUG
   fprintf (stderr,"SDL: create_display(%d): \n",depth);
   fprintf (stderr,"SDL: Info: HW blits %d\n"
      "SDL: Info: SW blits %d\n"
      "SDL: Info: Vid mem %d\n"
      "SDL: Info: Best supported depth %d\n",
      video_info->blit_hw,
      video_info->blit_sw,
      video_info->video_mem,
      video_info->vfmt->BitsPerPixel);
#endif

#ifdef GCW0
   Vid_depth = 16;  // always 16-bit depth
#else
   Vid_depth = video_info->vfmt->BitsPerPixel;
#endif

#ifdef IPU_SCALER
   if (ipu_scaler)
   {
			
		Display_width = width;
		Display_height = height;

		/* Define the blitter */
		switch( depth ) 
		{
			case 8:
				printf("BLIT: sdl_update_8_to_16bpp\n");
				update_function = &sdl_update_8_to_16bpp;
				break;
			case 16:
				if (direct_mapped_15bpp)
				{
					printf("BLIT: sdl_update_rgb_direct_16bpp\n");
					update_function = &sdl_update_rgb_direct_16bpp;
				}
				else
				{
					printf("BLIT: sdl_update_16_to_16bpp\n");
					update_function = &sdl_update_16_to_16bpp;
				}
				break;
			case 32:
				printf("BLIT: sdl_update_rgb_direct_32bpp\n");
				update_function = &sdl_update_rgb_direct_32bpp;
				break;
		}
   }
   else
#endif
   {
#if 0
	   sysdep_ipu_scaler(0);
#endif
	   vid_modes = SDL_ListModes(NULL,SDL_FULLSCREEN);
	   vid_modes_i = 0;

	   hardware = video_info->hw_available;

	   if ( (! vid_modes) || ((long)vid_modes == -1)) {
#ifdef SDL_DEBUG
		  fprintf (stderr, "SDL: Info: Possible all video modes available\n");
#endif
		  Display_height = visual_height*heightscale;
		  Display_width = visual_width*widthscale;
	   } else {
		  int best_vid_mode; /* Best video mode found */
		  int best_width,best_height;
		  int i;

#ifdef SDL_DEBUG
		  fprintf (stderr, "SDL: visual w:%d visual h:%d\n", visual_width, visual_height);
#endif
		  best_vid_mode = 0;
		  best_width = vid_modes[best_vid_mode]->w;
		  best_height = vid_modes[best_vid_mode]->h;
		  for (i=0;vid_modes[i];++i)
		  {
			 int cur_width, cur_height;

			 cur_width = vid_modes[i]->w;
			 cur_height = vid_modes[i]->h;

#ifdef SDL_DEBUG
			 fprintf (stderr, "SDL: Info: Found mode %d x %d\n", cur_width, cur_height);
#endif /* SDL_DEBUG */

			 /* If width and height too small, skip to next mode */
			 if ((cur_width < visual_width*widthscale) || (cur_height < visual_height*heightscale)) {
				continue;
			 }

			 /* If width or height smaller than current best, keep it */
			 if ((cur_width < best_width) || (cur_height < best_height)) {
				best_vid_mode = i;
				best_width = cur_width;
				best_height = cur_height;
			 }
		  }

#ifdef SDL_DEBUG
		  fprintf (stderr, "SDL: Info: Best mode found : %d x %d\n",
			 vid_modes[best_vid_mode]->w,
			 vid_modes[best_vid_mode]->h);
#endif /* SDL_DEBUG */

		  vid_modes_i = best_vid_mode;

		  /* mode_number is a command line option */
		  if( mode_number != -1) {
			 if( mode_number >vid_modes_i)
				fprintf(stderr, "SDL: The mode number is invalid... ignoring\n");
			 else
				vid_modes_i = mode_number;
		  }
		  if( vid_modes_i<0 ) {
			 fprintf(stderr, "SDL: None of the modes match :-(\n");
			 Display_height = visual_height*heightscale;
			 Display_width = visual_width*widthscale;
		  } else {
			 if(*(vid_modes+vid_modes_i)==NULL) 
				vid_modes_i--;

			 Display_width = (*(vid_modes + vid_modes_i))->w;
			 Display_height = (*(vid_modes + vid_modes_i))->h;
		  }
	   }

#ifdef GCW0
		Display_centering_scaled_width = visual_width;
		Display_centering_scaled_height = visual_height;

		/* Check for 1:2 ratio games.
		 * ie 640x240 which are just double horizontal resolution
		 * Note: no 2:1 ratio games exist so not checking for VIDEO_PIXEL_ASPECT_RATIO_2_1
		 */
		if (Machine->drv->video_attributes & VIDEO_PIXEL_ASPECT_RATIO_1_2)
		{
		Display_centering_scaled_width = visual_width/2;
		Display_centering_scaled_height = visual_height;
		heightscale=1;// reset back to 1

		// If the game's width fits (when horizontal resolution is halved)
		// use a standard blitter (but skip every other SRC pixel)
		if (Display_width < (visual_width/2))
		{
			fprintf(stderr, "BLIT: sdl_update_horzscale_1_2\n");
			if( depth == 8 ) 
			update_function = &sdl_update_8_to_16bpp_horzscale_1_2;
			else
			update_function = &sdl_update_16_to_16bpp_horzscale_1_2;
		}
		else
		{
			fprintf(stderr, "BLIT: sdl_update_rot0_1_2\n");
			if( depth == 8 ) 
			update_function = &sdl_update_8_to_16bpp_rot0_1_2;
			else
			update_function = &sdl_update_16_to_16bpp_rot0_1_2;
		}
		}
		else
		{
		  if (visual_width > Display_width)
		  {
		 if (visual_width > Display_width && ((Display_width/(visual_width-Display_width))*(visual_width-Display_width)==Display_width) && (!(Machine->drv->video_attributes & VIDEO_DUAL_MONITOR)))
		 {
			 fprintf (stderr, "BLIT: Using horzscale blitter\n");
			 switch (depth)
			 {
			 case 8:
				 update_function = &sdl_update_8_to_16bpp_horzscale;
				 break;
			 case 16:
				 update_function = &sdl_update_16_to_16bpp_horzscale;
				 break;
			 case 32:
				 update_function = &sdl_update_32_to_32bpp_horzscale;
				 break;
			 }
		 }
		 else
		 {
			if (Display_width < visual_width)
			{
			iAddX = Display_width;
			iModuloX = visual_width;
			Display_centering_scaled_width = Display_width;
			}
			if (Display_height < visual_height)
			{
			iAddY = Display_height;
			iModuloY = visual_height;
			Display_centering_scaled_height = Display_height;
			}

			if (visual_height * Display_width < Display_height * visual_width)
			{
			iAddY = iAddX;
			iModuloY = iModuloX;
			Display_centering_scaled_height = (visual_height * Display_width + visual_width - 1) / visual_width;
			}
			else
			{
			iAddX = iAddY;
			iModuloX = iModuloY;
			Display_centering_scaled_width = (visual_width * Display_height + visual_height - 1) / visual_height;
			}

			 fprintf (stderr, "SDL: Using scale blitter\n");
			 if( depth == 8 ) 
			 update_function = &sdl_update_8_to_16bpp_scale;
			 else
			 update_function = &sdl_update_16_to_16bpp_scale;
		 }
		  }
		  else if (visual_height > Display_height)
		  {
			if (Display_width < visual_width)
			{
			iAddX = Display_width;
			iModuloX = visual_width;
			Display_centering_scaled_width = Display_width;
			}
			if (Display_height < visual_height)
			{
			iAddY = Display_height;
			iModuloY = visual_height;
			Display_centering_scaled_height = Display_height;
			}

			if (visual_height * Display_width < Display_height * visual_width)
			{
			iAddY = iAddX;
			iModuloY = iModuloX;
			Display_centering_scaled_height = (visual_height * Display_width + visual_width - 1) / visual_width;
			}
			else
			{
			iAddX = iAddY;
			iModuloX = iModuloY;
			Display_centering_scaled_width = (visual_width * Display_height + visual_height - 1) / visual_height;
			}

			 fprintf (stderr, "SDL: Using scale blitter\n");
			 if( depth == 8 ) 
				 update_function = &sdl_update_8_to_16bpp_scale;
			 else
				 update_function = &sdl_update_16_to_16bpp_scale;
		  }
		  else
		  {
		 fprintf (stderr, "SDL: Using standard blitter\n");
		 if( depth == 8 ) 
			 update_function = &sdl_update_8_to_16bpp;
		 else
			 update_function = &sdl_update_16_to_16bpp;
		  }
		}
	   

		/* Centering limits */
		if (Display_centering_scaled_width > Display_width)
		{
		Display_centering_scaled_width = Display_width;
		}
		if (Display_centering_scaled_height > Display_height)
		{
		Display_centering_scaled_height = Display_height;
		}

	#else
	   if( depth == 8 ) {
		  switch( Vid_depth ) {
		  case 32:
			 update_function = &sdl_update_8_to_32bpp;
			 break;
		  case 24:
			 update_function = &sdl_update_8_to_24bpp;
			 break;
		  case 16:
			 update_function = &sdl_update_8_to_16bpp;
			 break;
		  case 8:
			 update_function = &sdl_update_8_to_8bpp;
			 break;
		  default:
			 fprintf (stderr, "SDL: Unsupported Vid_depth=%d in depth=%d\n", Vid_depth,depth);
			 SDL_Quit();
			 exit (OSD_NOT_OK);
			 break;
		  }
	   }
	   else if( depth == 16 )
	   {
		  switch( Vid_depth ) {
		  case 32:
			 update_function = &sdl_update_16_to_32bpp;
			 break;
		  case 24:
			 update_function = &sdl_update_16_to_24bpp;
			 break;
		  case 16:
			 update_function = &sdl_update_16_to_16bpp;
			 break;
		  default:
			 fprintf (stderr, "SDL: Unsupported Vid_depth=%d in depth=%d\n", Vid_depth,depth);
			 SDL_Quit();
			 exit (OSD_NOT_OK);
			 break;
		  }
	   }
	   else if (depth == 32)
	   {
		  if (Vid_depth == 32 && Machine->drv->video_attributes & VIDEO_RGB_DIRECT)
		  {
			 update_function = &sdl_update_rgb_direct_32bpp; 
		  }
		  else
		  {
			 fprintf (stderr, "SDL: Unsupported Vid_depth=%d in depth=%d\n",
				Vid_depth, depth);
			 SDL_Quit();
			 exit (OSD_NOT_OK);
		  }
	   }
	   else
	   {
		  fprintf (stderr, "SDL: Unsupported depth=%d\n", depth);
		  SDL_Quit();
		  exit (OSD_NOT_OK);
	   }
#endif

   }


   /* Set video mode according to flags */
   vid_mode_flag = SDL_HWSURFACE;
   if (start_fullscreen) {
      vid_mode_flag |= SDL_FULLSCREEN;
   }
#ifdef GCW0
   if (doublebuf) {
      vid_mode_flag |= SDL_TRIPLEBUF;
   }
#endif

  printf("SDL_SetVideoMode(%d,%d,%d,%u)\n", Display_width, Display_height,Vid_depth, vid_mode_flag);
   if(! (Surface = SDL_SetVideoMode(Display_width, Display_height,Vid_depth, vid_mode_flag))) {
      fprintf (stderr, "SDL: Error: Setting video mode failed (%dx%d %dbpp)\n", Display_width, Display_height, Vid_depth);
      SDL_Quit();
      exit (OSD_NOT_OK);
   } else {
      fprintf (stderr, "SDL: Info: Video mode set as %d x %d, depth %d\n", Display_width, Display_height, Vid_depth);
   }

#ifdef GCW0
   /* Center the display */
	Display_pitch = Surface->pitch;
	Display_BytesPerPixel = Surface->format->BytesPerPixel;
#ifdef IPU_SCALER
    if (ipu_scaler)
	{
		printf("SDL_SetVideoMode() returned w:%d h:%d pitch:%d\n", Surface->w,Surface->h,Surface->pitch);
		Display_centering_offset = 0;
		if (Display_pitch/Display_BytesPerPixel > Display_width)
		{
			Display_centering_offset += ((Display_pitch/Display_BytesPerPixel - Display_width)/2);
		}
	}
	else
#endif
		CalcCenterDisplay();
#else
   Offscreen_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,Display_width,Display_height,Vid_depth,0,0,0,0); 
   if(Offscreen_surface==NULL) {
      SDL_Quit();
      exit (OSD_NOT_OK);
   }
#endif


   /* Creating event mask */
   SDL_EventState(SDL_KEYUP, SDL_ENABLE);
   SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
   SDL_EnableUNICODE(1);
   
    /* fill the display_palette_info struct */
    memset(&display_palette_info, 0, sizeof(struct sysdep_palette_info));
    display_palette_info.depth = Vid_depth;
    if (Vid_depth == 8)
         display_palette_info.writable_colors = 256;
    else if (Vid_depth == 16) {
      display_palette_info.red_mask = 0xF800;
      display_palette_info.green_mask = 0x07E0;
      display_palette_info.blue_mask   = 0x001F;
    }
    else {
      display_palette_info.red_mask   = 0x00FF0000;
      display_palette_info.green_mask = 0x0000FF00;
      display_palette_info.blue_mask  = 0x000000FF;
    };

   /* Hide mouse cursor and save its previous status */
   cursor_state = SDL_ShowCursor(0);

   /* Set window title */
   SDL_WM_SetCaption(title, NULL);

   return OSD_OK;
}

/*
 *  keyboard remapping routine
 *  invoiced in startup code
 *  returns 0-> success 1-> invalid from or to
 */
static int sdl_mapkey(struct rc_option *option, const char *arg, int priority)
{
   unsigned int from, to;
   /* ultrix sscanf() requires explicit leading of 0x for hex numbers */
   if (sscanf(arg, "0x%x,0x%x", &from, &to) == 2)
   {
      /* perform tests */
      /* fprintf(stderr,"trying to map %x to %x\n", from, to); */
      if (from >= SDLK_FIRST && from < SDLK_LAST && to >= 0 && to <= 127)
      {
         klookup[from] = to;
	 return OSD_OK;
      }
      /* stderr_file isn't defined yet when we're called. */
      fprintf(stderr,"Invalid keymapping %s. Ignoring...\n", arg);
   }
   return OSD_NOT_OK;
}

/* Update routines */

#ifdef GCW0

#define G_R_B_MASK16  0x07E0F81F
INLINE unsigned short mix_color16 (unsigned short color1, unsigned short color2)
{
    unsigned int expanded_pixel1 = (color1 | (color1 << 16)) & G_R_B_MASK16;
    unsigned int expanded_pixel2 = (color2 | (color2 << 16)) & G_R_B_MASK16;
    unsigned int averaged_pixel = (expanded_pixel1+expanded_pixel2)>>1;
    averaged_pixel &= G_R_B_MASK16;
    return averaged_pixel | (averaged_pixel >>16);
}

#define R_G_B_MASK32  0x00FEFEFE
INLINE unsigned int mix_color32 (unsigned int color1, unsigned int color2)
{
    return (color1&R_G_B_MASK32)+(color2&R_G_B_MASK32)>>1;
}

/* Special fast scaler to downsize larger than 320x240 screens
 * Aspect ratio not handled so only works for landscape games
 */
#define SRC_PIXEL  unsigned int
#define DEST_PIXEL unsigned int
#define DEST ((DEST_PIXEL *)DRAWSURF->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define SRC_INC 1
#define SRC_SHIFT 2
#define MY_FUNCTION sdl_update_32_to_32bpp_horzscale
#define DESTMIX mix_color32
#define INDIRECT current_palette->lookup
#include "blit_horzscale.h"
#undef INDIRECT
#undef DESTMIX
#undef MY_FUNCTION
#undef SRC_SHIFT
#undef SRC_INC
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL


#define SRC_PIXEL  unsigned short
#define DEST_PIXEL unsigned short
#define DEST ((DEST_PIXEL *)DRAWSURF->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define SRC_INC 1
#define SRC_SHIFT 1
#define MY_FUNCTION sdl_update_16_to_16bpp_horzscale
#define DESTMIX mix_color16
#define INDIRECT current_palette->lookup
#include "blit_horzscale.h"
#undef INDIRECT
#undef DESTMIX
#undef MY_FUNCTION
#undef SRC_SHIFT
#undef SRC_INC
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL

#define SRC_PIXEL  unsigned char
#define DEST_PIXEL unsigned short
#define DEST ((DEST_PIXEL *)DRAWSURF->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define SRC_INC 1
#define SRC_SHIFT 0
#define MY_FUNCTION sdl_update_8_to_16bpp_horzscale
#define DESTMIX mix_color16
#define INDIRECT current_palette->lookup
#include "blit_horzscale.h"
#undef INDIRECT
#undef DESTMIX
#undef MY_FUNCTION
#undef SRC_SHIFT
#undef SRC_INC
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL

#define SRC_PIXEL  unsigned char
#define DEST_PIXEL unsigned short
#define DEST ((DEST_PIXEL *)Surface->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define SRC_INC 2
#define SRC_SHIFT 0
#define MY_FUNCTION sdl_update_8_to_16bpp_horzscale_1_2
#define DESTMIX mix_color16
#define INDIRECT current_palette->lookup
#include "blit_horzscale.h"
#undef INDIRECT
#undef MY_FUNCTION
#undef SRC_SHIFT
#undef DESTMIX
#undef SRC_INC
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL

#define SRC_PIXEL  unsigned short
#define DEST_PIXEL unsigned short
#define DEST ((DEST_PIXEL *)Surface->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define SRC_INC 2
#define SRC_SHIFT 1
#define MY_FUNCTION sdl_update_16_to_16bpp_horzscale_1_2
#define DESTMIX mix_color16
#define INDIRECT current_palette->lookup
#include "blit_horzscale.h"
#undef INDIRECT
#undef MY_FUNCTION
#undef SRC_SHIFT
#undef DESTMIX
#undef SRC_INC
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL

#define SRC_PIXEL  unsigned char
#define DEST_PIXEL unsigned short
#define SRC (SRC_PIXEL *)bitmap->line[visual.min_y] + visual.min_x;
#define DEST ((DEST_PIXEL *)Surface->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define MY_FUNCTION sdl_update_8_to_16bpp_scale
#define DESTMIX mix_color16
#define INDIRECT current_palette->lookup
#define SRC_INC 1
#define SRC_INC_LINE (bitmap->line[1] - bitmap->line[0])
#define MODX iModuloX
#define MODY iModuloY
#define ADDX iAddX
#define ADDY iAddY
#include "blit_scale.h"
#undef ADDY
#undef ADDX
#undef MODY
#undef MODX
#undef SRC_INC_LINE
#undef SRC_INC
#undef INDIRECT
#undef MY_FUNCTION
#undef DESTMIX
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef SRC
#undef DEST_PIXEL
#undef SRC_PIXEL

#define SRC_PIXEL  unsigned short
#define DEST_PIXEL unsigned short
#define SRC (SRC_PIXEL *)bitmap->line[visual.min_y] + visual.min_x;
#define DEST ((DEST_PIXEL *)Surface->pixels + Display_centering_offset)
#define DEST_WIDTH Display_width
#define DEST_HEIGHT Display_height
#define SRC_WIDTH visual_width
#define SRC_HEIGHT visual_height
#define MY_FUNCTION sdl_update_16_to_16bpp_scale
#define DESTMIX mix_color16
#define INDIRECT current_palette->lookup
#define SRC_INC 1
#define SRC_INC_LINE (bitmap->line[1] - bitmap->line[0])>>1
#define MODX iModuloX
#define MODY iModuloY
#define ADDX iAddX
#define ADDY iAddY
#include "blit_scale.h"
#undef ADDY
#undef ADDX
#undef MODY
#undef MODX
#undef SRC_INC_LINE
#undef SRC_INC
#undef INDIRECT
#undef MY_FUNCTION
#undef DESTMIX
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef DEST_HEIGHT
#undef DEST_WIDTH
#undef DEST
#undef SRC
#undef DEST_PIXEL
#undef SRC_PIXEL

#define MY_FUNCTION sdl_update_8_to_16bpp_rot0_1_2
#define SRC_PIXEL unsigned char
#define DEST_PIXEL unsigned short
#define SRC (SRC_PIXEL *)bitmap->line[visual.min_y] + visual.min_x;
#define DEST ((DEST_PIXEL *)Surface->pixels + Display_centering_offset)
#define INDIRECT current_palette->lookup
#define SRC_WIDTH (visual_width>>1)
#define SRC_HEIGHT visual_height
#define DEST_INC_LINE (Surface->pitch>>1)
#define SRC_INC_LINE (bitmap->line[1] - bitmap->line[0])
#define SRC_INC 2
#include "blit_fast.h"
#undef SRC_INC
#undef SRC_INC_LINE
#undef DEST_INC_LINE
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef INDIRECT
#undef DEST
#undef SRC
#undef DEST_PIXEL
#undef SRC_PIXEL
#undef MY_FUNCTION

#define MY_FUNCTION sdl_update_16_to_16bpp_rot0_1_2
#define SRC_PIXEL unsigned short
#define DEST_PIXEL unsigned short
#define SRC (SRC_PIXEL *)bitmap->line[visual.min_y] + visual.min_x;
#define DEST ((DEST_PIXEL *)Surface->pixels + Display_centering_offset)
#define INDIRECT current_palette->lookup
#define SRC_WIDTH (visual_width>>1)
#define SRC_HEIGHT visual_height
#define DEST_INC_LINE (Surface->pitch>>1)
#define SRC_INC_LINE ((bitmap->line[1] - bitmap->line[0])>>1)
#define SRC_INC 2
#include "blit_fast.h"
#undef SRC_INC
#undef SRC_INC_LINE
#undef DEST_INC_LINE
#undef SRC_HEIGHT
#undef SRC_WIDTH
#undef INDIRECT
#undef DEST
#undef SRC
#undef DEST_PIXEL
#undef SRC_PIXEL
#undef MY_FUNCTION



#endif

void sdl_update_8_to_16bpp(struct osd_bitmap *bitmap)
{
#define INDIRECT current_palette->lookup
#define COPY_LINE(SRC, END, DST) clut8to16_16bpp_hack(DST, SRC, INDIRECT, ((END)-(SRC)));
//#define COPY_LINE(SRC, END, DST) clut8to16(DST, SRC, INDIRECT, ((END)-(SRC)));	/* TODO: Test which is faster */
#define MEMCPY(SRC, END, DST) memcpy(DST, SRC, ((END)-(SRC)));
#define SRC_PIXEL  unsigned char
#define DEST_PIXEL unsigned short
#define DEST ((DEST_PIXEL *)DRAWSURF->pixels + Display_centering_offset)
#define DEST_WIDTH (Display_pitch/Display_BytesPerPixel)

#include "blit_normal.h"

#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL
#undef INDIRECT
#undef COPY_LINE
#undef MEMCPY
}

void sdl_update_16_to_16bpp (struct osd_bitmap *bitmap)
{
#define SRC_PIXEL  unsigned short
#define DEST_PIXEL unsigned short
#define COPY_LINE(SRC, END, DST) clut16to16(DST, SRC, INDIRECT, Display_width/*is in number of "pixels"*/);
#define MEMCPY(SRC, END, DST) memcpy(DST, SRC, visual_width<<1/*number of bytes*/);
//#define MEMCPY(SRC, END, DST) memcpy32(DST, SRC, visual_width>>1/*number of 32-bit words*/);
#define DEST ((DEST_PIXEL *)DRAWSURF->pixels + Display_centering_offset)
#define DEST_WIDTH (Display_pitch/Display_BytesPerPixel)

   if(current_palette->lookup)
   {
#define INDIRECT current_palette->lookup
#include "blit_normal.h"
#undef INDIRECT
   }
   else
   {
#include "blit_normal.h"
   }


#undef DEST
#undef DEST_WIDTH
#undef SRC_PIXEL
#undef DEST_PIXEL
#undef INDIRECT
#undef COPY_LINE
#undef MEMCPY
}

void sdl_update_rgb_direct_16bpp (struct osd_bitmap *bitmap)
{
#define SRC_PIXEL  unsigned short
#define DEST_PIXEL unsigned short
#define COPY_LINE(SRC, END, DST) clut16to16(DST, SRC, INDIRECT, Display_width/*is in number of "pixels"*/);
#define MEMCPY(SRC, END, DST) memcpy(DST, SRC, visual_width<<1/*number of bytes*/);
#define DEST ((DEST_PIXEL *)DRAWSURF->pixels + Display_centering_offset)
#define DEST_WIDTH (Display_pitch/Display_BytesPerPixel)
#include "blit_normal.h"
#undef DEST
#undef DEST_WIDTH
#undef SRC_PIXEL
#undef DEST_PIXEL
#undef INDIRECT
#undef COPY_LINE
#undef MEMCPY
}

void sdl_update_rgb_direct_32bpp(struct osd_bitmap *bitmap)
{
#define SRC_PIXEL unsigned int
#define DEST_PIXEL unsigned int
#define DEST DRAWSURF->pixels
#define DEST_WIDTH Display_width
//#define MEMCPY(SRC, END, DST) memcpy(DST, SRC, ((END)-(SRC)));
#define MEMCPY(SRC, END, DST) memcpy(DST, SRC, Display_width<<2);
#include "blit_normal.h"
#undef MEMCPY
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL
}

#if 0
void sdl_update_8_to_8bpp (struct osd_bitmap *bitmap)
{
#define DEST_WIDTH Display_width
#define DEST DRAWSURF->pixels
#define SRC_PIXEL unsigned char
#define DEST_PIXEL unsigned char

#include "blit.h"

#undef DEST_PIXEL
#undef SRC_PIXEL
#undef DEST
#undef DEST_WIDTH
}

void sdl_update_8_to_24bpp (struct osd_bitmap *bitmap)
{
#define SRC_PIXEL  unsigned char
#define DEST_PIXEL unsigned int
#define PACK_BITS
#define DEST DRAWSURF->pixels
#define DEST_WIDTH Display_width
   if(current_palette->lookup)
   {
#define INDIRECT current_palette->lookup
#include "blit.h"
#undef INDIRECT
   }
   else
   {
#include "blit.h"
   }
#undef DEST_WIDTH
#undef DEST
#undef PACK_BITS
#undef DEST_PIXEL
#undef SRC_PIXEL
}

void sdl_update_8_to_32bpp (struct osd_bitmap *bitmap)
{
#define INDIRECT current_palette->lookup
#define SRC_PIXEL  unsigned char
#define DEST_PIXEL unsigned int
#define DEST DRAWSURF->pixels
#define DEST_WIDTH Display_width
#include "blit.h"
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL
#undef INDIRECT
}

void sdl_update_16_to_24bpp (struct osd_bitmap *bitmap)
{
#define SRC_PIXEL  unsigned short
#define DEST_PIXEL unsigned int
#define PACK_BITS
#define DEST DRAWSURF->pixels
#define DEST_WIDTH Display_width
   if(current_palette->lookup)
   {
#define INDIRECT current_palette->lookup
#include "blit.h"
#undef INDIRECT
   }
   else
   {
#include "blit.h"
   }
#undef DEST_WIDTH
#undef DEST
#undef PACK_BITS
#undef DEST_PIXEL
#undef SRC_PIXEL
}

void sdl_update_16_to_32bpp (struct osd_bitmap *bitmap)
{
#define INDIRECT current_palette->lookup
#define SRC_PIXEL unsigned short
#define DEST_PIXEL unsigned int
#define DEST DRAWSURF->pixels
#define DEST_WIDTH Display_width
#include "blit.h"
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL
#undef INDIRECT
}

void sdl_update_rgb_direct_32bpp(struct osd_bitmap *bitmap)
{
#define SRC_PIXEL unsigned int
#define DEST_PIXEL unsigned int
#define DEST DRAWSURF->pixels
#define DEST_WIDTH Display_width
#include "blit.h"
#undef DEST_WIDTH
#undef DEST
#undef DEST_PIXEL
#undef SRC_PIXEL
}
#endif

void sysdep_update_display(struct osd_bitmap *bitmap)
{
#if GCW0
   int old_use_dirty = use_dirty;

   if (current_palette->lookup_dirty)
      use_dirty = 0;
   
   (*update_function)(bitmap);

   //if (doublebuf == 1) // Always SDL_Flip
      SDL_Flip(Surface);

   use_dirty = old_use_dirty;
#else
   int old_use_dirty = use_dirty;

   SDL_Rect srect = { 0,0,0,0 };
   SDL_Rect drect = { 0,0,0,0 };
   srect.w = Display_width;
   srect.h = Display_height;

   /* Center the display */
   drect.x = (Display_width - visual_width*widthscale ) >> 1;
   drect.y = (Display_height - visual_height*heightscale ) >> 1;

   drect.w = Display_width;
   drect.h = Display_height;

   if (current_palette->lookup_dirty)
      use_dirty = 0;
   
   (*update_function)(bitmap);

   
   if(SDL_BlitSurface (Offscreen_surface, &srect, Surface, &drect)<0) 
      fprintf (stderr,"SDL: Warn: Unsuccessful blitting\n");

   if(hardware==0)
      SDL_UpdateRects(Surface,1, &drect);
   use_dirty = old_use_dirty;
#endif
}

/* shut up the display */
void sysdep_display_close(void)
{
#ifdef GCW0
#else
   SDL_FreeSurface(Offscreen_surface);
#endif
}

/*
 * In 8 bpp we should alloc pallete - some ancient people  
 * are still using 8bpp displays
 */
int sysdep_display_alloc_palette(int totalcolors)
{
#ifdef GCW0
#else
   int ncolors;
   int i;
   ncolors = totalcolors;

   fprintf (stderr, "SDL: sysdep_display_alloc_palette(%d);\n",totalcolors);
   if (Vid_depth != 8)
      return 0;

   Colors = (SDL_Color*) malloc (totalcolors * sizeof(SDL_Color));
   if( !Colors )
      return 1;
   for (i=0;i<totalcolors;i++) {
      (Colors + i)->r = 0xFF;
      (Colors + i)->g = 0x00;
      (Colors + i)->b = 0x00;
   }
   SDL_SetColors (Offscreen_surface,Colors,0,totalcolors-1);

   fprintf (stderr, "SDL: Info: Palette with %d colors allocated\n", totalcolors);
#endif
   return 0;
}

int sysdep_display_set_pen(int pen,unsigned char red, unsigned char green, unsigned char blue)
{
#ifdef GCW0
#else
   static int warned = 0;
#ifdef SDL_DEBUG
   fprintf(stderr,"sysdep_display_set_pen(%d,%d,%d,%d)\n",pen,red,green,blue);
#endif

   if( Colors ) {
      (Colors + pen)->r = red;
      (Colors + pen)->g = green;
      (Colors + pen)->b = blue;
      if ( (! SDL_SetColors(Offscreen_surface, Colors + pen, pen,1)) && (! warned)) {
         printf ("Color allocation failed, or > 8 bit display\n");
         warned = 0;
      }
   }

#ifdef SDL_DEBUG
   fprintf(stderr, "STD: Debug: Pen %d modification: r %d, g %d, b, %d\n", pen, red,green,blue);
#endif /* SDL_DEBUG */
#endif
   return 0;
}

void sysdep_mouse_poll (void)
{
   int i;
   int x,y;
   Uint8 buttons;

   buttons = SDL_GetRelativeMouseState( &x, &y);
   mouse_data[0].deltas[0] = x;
   mouse_data[0].deltas[1] = y;
   for(i=0;i<MOUSE_BUTTONS;i++) {
      mouse_data[0].buttons[i] = buttons & (0x01 << i);
   }
}

/* Keyboard procs */
/* Lighting keyboard leds */
void sysdep_set_leds(int leds) 
{
}

void sysdep_update_keyboard() 
{
   struct keyboard_event kevent;
   SDL_Event event;
   
   if (Surface) {
      while(SDL_PollEvent(&event)) {
         kevent.press = 0;
         
         switch (event.type)
         {
            case SDL_KEYDOWN:
               kevent.press = 1;

               /* ALT-Enter: toggle fullscreen */
               if ( event.key.keysym.sym == SDLK_RETURN )
               {
                  if(event.key.keysym.mod & KMOD_ALT)
                     SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
               }

            case SDL_KEYUP:
               kevent.scancode = klookup[event.key.keysym.sym];
               kevent.unicode = event.key.keysym.unicode;
               keyboard_register_event(&kevent);
               if(!kevent.scancode)
                  fprintf (stderr, "Unknown symbol 0x%x\n",
                     event.key.keysym.sym);
#ifdef SDL_DEBUG
               fprintf (stderr, "Key %s %ssed\n",
                  SDL_GetKeyName(event.key.keysym.sym),
                  kevent.press? "pres":"relea");
#endif
               break;
            case SDL_QUIT:
               /* Shoult leave this to application */
               exit(OSD_OK);
               break;

    	    case SDL_JOYAXISMOTION:   
               if (event.jaxis.which < JOY_AXIS)
                  joy_data[event.jaxis.which].axis[event.jaxis.axis].val = event.jaxis.value;
#ifdef SDL_DEBUG
               fprintf (stderr,"Axis=%d,value=%d\n",event.jaxis.axis ,event.jaxis.value);
#endif
		break;
	    case SDL_JOYBUTTONDOWN:

	    case SDL_JOYBUTTONUP:
               if (event.jbutton.which < JOY_BUTTONS)
                  joy_data[event.jbutton.which].buttons[event.jbutton.button] = event.jbutton.state;
#ifdef SDL_DEBUG
               fprintf (stderr, "Button=%d,value=%d\n",event.jbutton.button ,event.jbutton.state);
#endif
		break;


            default:
#ifdef SDL_DEBUG
               fprintf(stderr, "SDL: Debug: Other event\n");
#endif /* SDL_DEBUG */
               break;
         }
    joy_evaluate_moves ();
      }
   }
}

/* added funcions */
int sysdep_display_16bpp_capable(void)
{
   const SDL_VideoInfo* video_info;
   video_info = SDL_GetVideoInfo();
   return ( video_info->vfmt->BitsPerPixel >=16);
}

void list_sdl_modes(void)
{
   SDL_Rect** vid_modes;
   int vid_modes_i;

   vid_modes = SDL_ListModes(NULL,SDL_FULLSCREEN);
   vid_modes_i = 0;

   if ( (! vid_modes) || ((long)vid_modes == -1)) {
      printf("This option only works in a full-screen mode (eg: linux's framebuffer)\n");
      return;
   }

   printf("Modes availables:\n");

   while( *(vid_modes+vid_modes_i) ) {
      printf("\t%d) Mode %d x %d\n",
         vid_modes_i,
         (*(vid_modes+vid_modes_i))->w,
         (*(vid_modes+vid_modes_i))->h
         );
   
      vid_modes_i++;
   }
}
