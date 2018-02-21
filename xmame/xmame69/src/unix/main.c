/*
 * X-mame main-routine
 */

#define __MAIN_C_
#include "xmame.h"
#ifdef FRONTEND
#include <limits.h>
#include <libgen.h>
#endif

#ifdef FRONTEND
char *frontend = NULL;
char frontendname[PATH_MAX];
#endif

/* From video.c. */
void osd_video_initpre();

/* put here anything you need to do when the program is started. Return 0 if */
/* initialization was successful, nonzero otherwise. */
int osd_init(void)
{
	/* now invoice system-dependent initialization */
#ifdef MAME_NET
	if (osd_net_init()      !=OSD_OK) return OSD_NOT_OK;
#endif	
	if (osd_input_initpre() !=OSD_OK) return OSD_NOT_OK;

	return OSD_OK;
}

/*
 * Cleanup routines to be executed when the program is terminated.
 */
void osd_exit (void)
{
#ifdef MAME_NET
	osd_net_close();
#endif
	osd_input_close();
}


void set_xmameroot(void)
{
    if (getenv("XMAMEROOT")==(char*)0)
	{
		static char current_dir[128];
		static char newenv[128];
		getcwd(current_dir, PATH_MAX);
		snprintf(newenv, PATH_MAX, "XMAMEROOT=%s", current_dir);
		putenv(newenv);
	}
    printf ("Using XMAMEROOT=%s\n", getenv("XMAMEROOT"));
}


int main (int argc, char **argv)
{
	int res;

	set_xmameroot();

	/* to be absolutly safe force giving up root rights here in case
	   a display method doesn't */
	if(setuid(getuid()))
	{
		perror("setuid");
		sysdep_close();
		return OSD_NOT_OK;
	}
	
        /* Set the title, now auto build from defines from the makefile */
        sprintf(title,"%s (%s) version %s", NAME, DISPLAY_METHOD, build_version);

	/* parse configuration file and environment */
	if ((res = config_init(argc, argv)) != 1234) goto leave;
	
	/* some display methods need to do some stuff with root rights */
	/*
	 * sysdep_init() has been moved to here.
	 * Originally located before config_init() but we need
	 * to run config_init() first before initializing SDL (and video)
	 * otherwise the frontend loses it's screen
	 * when it's running xmame to query ROM lists, etc.
	 */
	if (sysdep_init()!= OSD_OK) exit(OSD_NOT_OK);
	
        /* Check the colordepth we're requesting */
        if (!options.color_depth && !sysdep_display_16bpp_capable())
           options.color_depth = 8;

	/* 
	 * Initialize whatever is needed before the display is actually 
	 * opened, e.g., artwork setup.
	 */
	osd_video_initpre();

	/* go for it */
	res = run_game (game_index);

leave:
	sysdep_close();
	/* should be done last since this also closes stdout and stderr */
	config_exit();

#ifdef FRONTEND
	if (strlen(frontendname))
	{
	    int ret;
	    char frontend_path[PATH_MAX];

	    /* Launch frontend instead of exit() */
	    printf("Restarting frontend\n");

	    strcpy(frontend_path, frontendname);
	    chdir(dirname(frontend_path));
	    ret = execl(frontendname, frontendname);

	    printf("Failed to execl %s, code = %d\n", frontendname, ret);

	    return 1; /* fail */

	}
	else
	{
	    return res;
	}
#else
	return res;
#endif
}
