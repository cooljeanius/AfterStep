#include "config.h"

#include <string.h>
#include <stdlib.h>

/****h* libAfterImage/tutorials/ASView
 * NAME
 * ASView
 * SYNOPSIS
 * Simple image viewer based on libAfterImage.
 * DESCRIPTION
 * All we want to do here is to get image filename from the command line,
 * then load this image, and display it in simple window.
 * After that we would want to wait, until user closes our window.
 * SOURCE
 */

#define DO_CLOCKING

#include "../afterbase.h"
#include "../afterimage.h"
#include "common.h"

#ifdef DO_CLOCKING
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#endif


void usage()
{
	printf( "Usage: asview [-h]|[image]\n");
	printf( "Where: image - filename of the image to display.\n");
}


int main(int argc, char* argv[])
{
	char *image_file = "rose512.jpg" ;
	ASImage *im ;
	ASVisual *asv ;
	int screen = 0, depth = 24;
	Display *dpy = NULL;

	/* see ASView.1 : */
	set_application_name( argv[0] );
#if (HAVE_AFTERBASE_FLAG==1)
	set_output_threshold(OUTPUT_LEVEL_DEBUG);
#ifdef DEBUG_ALLOCS
	fprintf( stderr, "have DEBUG_ALLOCS\n");
#endif
#ifdef AFTERBASE_DEBUG_ALLOCS
	fprintf( stderr, "have AFTERBASE_DEBUG_ALLOCS\n");
#endif
#endif

	if( argc > 1 )
	{
		if( strcmp( argv[1], "-h" ) == 0 )
		{
			usage();
			return 0;
		}
		image_file = argv[1] ;
	}else
	{
		show_warning( 	"Image filename was not specified. "
						"Using default: \"%s\"", image_file );
		usage();
	}
#ifndef X_DISPLAY_MISSING
	dpy = XOpenDisplay(NULL);
	XSynchronize (dpy, True);
	_XA_WM_DELETE_WINDOW = XInternAtom( dpy, "WM_DELETE_WINDOW", 
										False);
	screen = DefaultScreen(dpy);
	depth = DefaultDepth( dpy, screen );
#endif	
	/* see ASView.3 : */
	asv = create_asvisual( dpy, screen, depth, NULL );
	/* asv = create_asvisual_for_id( dpy, screen, depth, 0x28, None, NULL ); */

	/* see ASView.2 : */
	im = file2ASImage( image_file, 0xFFFFFFFF, SCREEN_GAMMA, 0, getenv("IMAGE_PATH"), NULL );

	/* The following could be used to dump JPEG version of the image into
	 * stdout : */
	/* 	ASImage2file( im, NULL, NULL, ASIT_Jpeg, NULL ); 
		ASImage2file( im, NULL, "asview.png", ASIT_Png, NULL );
		ASImage2file( im, NULL, "asview.gif", ASIT_Gif, NULL );
	*/

	if( im != NULL )
	{
#ifndef X_DISPLAY_MISSING
		Window w ;
#if 0
		/* test example for get_asimage_channel_rects() : */
		XRectangle *rects ;	unsigned int rects_count =0; int i ;
		rects = get_asimage_channel_rects( im, IC_ALPHA, 10, 
											&rects_count );
		fprintf( stderr, " %d rectangles generated : \n", rects_count );
		for( i = 0 ; i < rects_count ; ++i )
			fprintf( stderr, "\trect[%d]=%dx%d%+d%+d;\n", 
					 i, rects[i].width, rects[i].height, 
					 rects[i].x, rects[i].y );
#endif


#if 0		 
		/* test example for fill_asimage : */
		fill_asimage(asv, im, 0, 0, 50, 50, 0xFFFF0000);
		fill_asimage(asv, im, 50, 50, 100, 50, 0xFFFF0000);
		fill_asimage(asv, im, 0, 100, 200, 50, 0xFFFF0000);
		fill_asimage(asv, im, 150, 0, 50, 50, 0xFFFF0000);
#endif
#if 0
		/* test example for conversion to argb32 :*/
		{
			ASImage *tmp = tile_asimage( asv, im, 0, 0, im->width, im->height, TINT_NONE, ASA_ARGB32, 
										  0, ASIMAGE_QUALITY_DEFAULT );	 
			destroy_asimage( &im );
			set_flags( tmp->flags, ASIM_DATA_NOT_USEFUL|ASIM_XIMAGE_NOT_USEFUL );
			im = tmp ;
		}		   
#endif		   
		/* see ASView.4 : */
		w = create_top_level_window( asv, DefaultRootWindow(dpy), 32, 32,
			                         im->width, im->height, 1, 0, NULL,
									 "ASView", image_file );
		if( w != None )
		{
			Pixmap p ;
	  		
			XMapRaised   (dpy, w);
			XSync(dpy,False);
			/* see ASView.5 : */
	  		p = create_visual_pixmap( asv, DefaultRootWindow(dpy), im->width, im->height, 0 );
	
			{
				START_TIME(started);
				/* for( int i = 0 ; i < 100 ; ++i )  To test performance! */
				asimage2drawable( asv, p, im, NULL, 0, 0, 0, 0, im->width, im->height, False);
				SHOW_TIME("", started);
			}
			/* print_storage(NULL); */
			destroy_asimage( &im );
			/* see common.c:set_window_background_and_free(): */
			p = set_window_background_and_free( w, p );
		}
		/* see common.c: wait_closedown() : */
		wait_closedown(w);
		dpy = NULL;
		
		/* no longer need this - lets clean it up :*/
		destroy_asvisual( asv, False );
		asv = NULL ;

#else
		/* writing result into the file */
		ASImage2file( im, NULL, "asview.png", ASIT_Png, NULL );
#endif
	}

#ifdef DEBUG_ALLOCS
    /* different cleanups of static memory pools : */
    flush_ashash_memory_pool();
	asxml_var_cleanup();
	custom_color_cleanup();
    build_xpm_colormap( NULL );
	flush_default_asstorage();
	/* requires libAfterBase */
	print_unfreed_mem();
#endif

    return 0 ;
}
/**************/

/****f* libAfterImage/tutorials/ASView.1 [1.1]
 * SYNOPSIS
 * Step 1. Initialization.
 * DESCRIPTION
 * libAfterImage requires only 2 global things to be setup, and both of
 * those are inherited from libAfterBase: dpy - pointer to open X display-
 * naturally that is something we cannot live without; application name -
 * used in all the text output, such as error and warning messages and
 * also debugging messages if such are enabled.
 * The following two line are about all that is required to setup both
 * of this global variables :
 * EXAMPLE
 *     set_application_name( argv[0] );
 *     dpy = XOpenDisplay(NULL);
 * NOTES
 * First line is setting up application name from command line's
 * program name. Second opens up X display specified in DISPLAY env.
 * variable. Naturally based on application purpose different parameters
 * can be passed to these functions, such as some custom display string.
 * SEE ALSO
 * libAfterBase, set_application_name(), XOpenDisplay(), Display,
 *******/
/****f* libAfterImage/tutorials/ASView.2 [1.2]
 * SYNOPSIS
 * Step 2. Loading image file.
 * DESCRIPTION
 * At this point we are ready to load image from file into memory. Since
 * libAfterImage does not use any X facilities to store image - we don't
 * have to create any window or anything else yet. Even dpy is optional
 * here - it will only be used to try and parse names of colors from
 * .XPM images.
 * EXAMPLE
 *     im = file2ASImage( image_file, 0xFFFFFFFF, SCREEN_GAMMA, 0, NULL);
 * NOTES
 * We used compression set to 0, as we do not intend to store
 * image in memory for any considerable amount of time, and we want to
 * avoid additional processing overhead related to image compression.
 * If image was loaded successfully, which is indicated by returned
 * pointer being not NULL, we can proceed to creation of the window and
 * displaying of the image.
 * SEE ALSO
 * file2ASImage()
 ********/
/****f* libAfterImage/tutorials/ASView.3 [1.3]
 * SYNOPSIS
 * Step 3. Preparation of the visual.
 * DESCRIPTION
 * At this point we have to obtain Visual information, as window
 * creation is highly dependant on Visual being used. In fact when X
 * creates a window it ties it to a particular Visual, and all its
 * attributes, such as colormap, pixel values, pixmaps, etc. must be
 * associated with the same Visual. Accordingly we need to acquire
 * ASVisual structure, which is our abstraction layer from them naughty
 * X Visuals. :
 * EXAMPLE
 *     asv = create_asvisual( dpy, screen, depth, NULL );
 * NOTES
 * If any Window or Pixmap is created based on particular ASVisual, then
 * this ASVisual structure must not be destroyed untill all such
 * Windows and Pixmaps are destroyed.
 * SEE ALSO
 * See create_asvisual() for details.
 ********/
/****f* libAfterImage/tutorials/ASView.4 [1.4]
 * SYNOPSIS
 * Step 4. Preparation of the window.
 * DESCRIPTION
 * Creation of top level window consists of several steps of its own:
 * a) create the window of desired size and placement
 * b) set ICCCM hints on the window
 * c) select appropriate events on the window
 * c) map the window.
 * First two steps has been moved out into create_top_level_window()
 * function.
 * EXAMPLE
 *     w = create_top_level_window( asv, DefaultRootWindow(dpy), 32, 32,
 *                                  im->width, im->height, 1, 0, NULL,
 *                                  "ASView" );
 *     if( w != None )
 *     {
 *         XSelectInput (dpy, w, (StructureNotifyMask | ButtonPress));
 *         XMapRaised   (dpy, w);
 *     }
 * NOTES
 * Map request should be made only for window that has all its hints set
 * up already, so that Window Manager can read them right away.
 * We want to map window as soon as possible so that User could see that
 * something really is going on, even before image is displayed.
 * SEE ALSO
 * ASImage, create_top_level_window()
 ********/
/****f* libAfterImage/tutorials/ASView.5 [1.5]
 * SYNOPSIS
 * Step 5. Displaying the image.
 * DESCRIPTION
 * The simplest way to display image in the window is to convert it
 * into Pixmap, then set Window's background to this Pixmap, and,
 * at last, clear the window, so that background shows up.
 * EXAMPLE
 *  p = asimage2pixmap( asv, DefaultRootWindow(dpy), im, NULL, False );
 *  destroy_asimage( &im );
 * NOTES
 * We no longer need ASImage after we transfered it onto the Pixmap, so
 * we better destroy it to conserve resources.
 * SEE ALSO
 * asimage2pixmap(), destroy_asimage(), set_window_background_and_free()
 ********/
