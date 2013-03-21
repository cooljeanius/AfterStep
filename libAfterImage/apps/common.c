#include "config.h"

#include <string.h>

#include "../afterbase.h"
#include "../afterimage.h"

/****h* libAfterImage/tutorials/common.h
 * SYNOPSIS
 * Generic Xlib related functionality, common for all the tutorials.
 * SEE ALSO
 * libAfterImage,
 * ASView, ASScale, ASTile, ASMerge, ASGrad, ASFlip, ASText
 **************/

/****v* libAfterImage/tutorials/_XA_WM_DELETE_WINDOW
 * NAME
 * _XA_WM_DELETE_WINDOW
 * SYNOPSIS
 * _XA_WM_DELETE_WINDOW - stores value of X Atom "WM_DELETE_WINDOW".
 * DESCRIPTION
 * In X all client's top level windows are managed by window manager.
 * That includes moving, resizing, decorating, focusing and closing of
 * windows. Interactions between window manager and client window are
 * governed by ICCCM specification.
 * All the parts of this specification are completely optional, but it
 * is recommended that the following minimum set of hints be supplied
 * for any client window:
 * Window's title(WM_NAME), iconified window title(WM_ICON_NAME), class
 * hint (WM_CLASS) and supported protocols (WM_PROTOCOLS). It is
 * recommended also that WM_DELETE_WINDOW protocol be supported, as
 * otherwise there are no way to safely close client window, but to
 * kill it.
 * All of the above mentioned hints are identified by atoms that have
 * standard preset values, except for WM_DELETE_WINDOW. As the result we
 * need to obtain WM_DELETE_WINDOW atoms ID explicitly. We use
 * _XA_WM_DELETE_WINDOW to store the ID of that atom, so it is accessible
 * anywhere from our application.
 * SOURCE
 */
Atom _XA_WM_DELETE_WINDOW = None;
/**************/

/****f* libAfterImage/tutorials/create_top_level_window()
 * NAME
 * create_top_level_window()
 * SYNOPSIS
 * Window create_top_level_window( ASVisual *asv, Window root,
 *                                 int x, int y,
 *                                 unsigned int width,
 *                                 unsigned int height,
 *                                 unsigned int border_width,
 *                                 unsigned long attr_mask,
 *                                 XSetWindowAttributes *attr,
 *                                 char *app_class );
 * INPUTS
 * asv           - pointer to valid preinitialized ASVisual structure.
 * root          - root window of the screen on which to create window.
 * x, y          - desired position of the window
 * width, height - desired window size.
 * border_width  - desired initial border width of the window (may not
 *                 have any effect due to Window Manager intervention.
 * attr_mask     - mask of the attributes that has to be set on the
 *                 window
 * attr          - values of the attributes to be set.
 * app_class     - title of the application to be used as its window
 *                 Title and resources class.
 * RETURN VALUE
 * ID of the created window.
 * DESCRIPTION
 * create_top_level_window() is autyomating process of creating top
 * level application window. It creates window for specified ASVisual,
 * and then sets up basic ICCCM hints for that window, such as WM_NAME,
 * WM_ICON_NAME, WM_CLASS and WM_PROTOCOLS.
 * SOURCE
 */
Window
create_top_level_window( ASVisual *asv, Window root, int x, int y,
                         unsigned int width, unsigned int height,
						 unsigned int border_width, 
						 unsigned long attr_mask,
						 XSetWindowAttributes *attr, 
						 const char *app_class, const char *app_name )
{
 	Window w = None;
#ifndef X_DISPLAY_MISSING
	char *tmp ;
	XTextProperty name;
	XClassHint class1;

	w = create_visual_window(asv, root, x, y, width, height, border_width, 
							 InputOutput, attr_mask, attr );

	tmp = (app_name==NULL)?(char*)get_application_name():(char*)app_name;
    XStringListToTextProperty (&tmp, 1, &name);

    class1.res_name = tmp;	/* for future use */
    class1.res_class = (char*)app_class;
    XSetWMProtocols (asv->dpy, w, &_XA_WM_DELETE_WINDOW, 1);
    XSetWMProperties (asv->dpy, w, &name, &name, NULL, 0, NULL, NULL, &class1);
    /* final cleanup */
    XFree ((char *) name.value);

#endif /* X_DISPLAY_MISSING */
	return w;
}
/**************/
/****f* libAfterImage/tutorials/set_window_background_and_free()
 * NAME
 * set_window_background_and_free()
 * SYNOPSIS
 * Pixmap set_window_background_and_free( Window w, Pixmap p );
 * INPUTS
 * w - ID of the window background of which we need to set.
 * p - Pixmap to set background to.
 * RETURN VALUE
 * None on success. Pixmap ID of original Pixmap on failure.
 * DESCRIPTION
 * set_window_background_and_free() will set window's background to
 * specified Pixmap, Then refresh window contents so that background
 * is drawn by the server, flush all the requests to force it to be sent
 * to server to be processed as fast as possible.
 * NOTES
 * After Window's background has been set to Pixmap - X server makes
 * hidden copy of this Pixmap for later window refreshing. As the result
 * original Pixmap is no longer needed and can be freed to conserve
 * resources.
 * SOURCE
 */
Pixmap
set_window_background_and_free( Window w, Pixmap p )
{
#ifndef X_DISPLAY_MISSING
	if( p != None && w != None )
	{
		Display *dpy = get_default_asvisual()->dpy;

		if (dpy)
		{
			XSetWindowBackgroundPixmap( dpy, w, p );
			XClearWindow( dpy, w );
			XFlush( dpy );
			XFreePixmap( dpy, p );
			p = None ;
		}
	}
#endif /* X_DISPLAY_MISSING */
	return p ;
}
/**************/

/****f* libAfterImage/tutorials/wait_closedown()
 * NAME
 * wait_closedown()
 * SYNOPSIS
 * void wait_closedown( Window w );
 * INPUTS
 * w - ID of the window from which to wait for events.
 * DESCRIPTION
 * User action requesting window to be closed is generally received
 * first by Window Manager. Window Manager is then handles it down to
 * the window by sending it ClientMessage event with first 32 bit word
 * of data set to the value of WM_DELETE_WINDOW Atom.
 * Accordingly, all client has to do is wait for such event from X server
 * and, when received, it should destroy its window and generally exit.
 *
 * NOTES
 * It is recommended that XFlush() is issued right after XDestroyWindow()
 * as Window Manager itself may attempt to do something with the window
 * until it receives DestroyNotify event.
 * SEE ALSO
 * ICCCM, Window
 * SOURCE
 */
void
wait_closedown( Window w )
{
#ifndef X_DISPLAY_MISSING
	Display *dpy = get_default_asvisual()->dpy;

    if(dpy)
	{
		if (w)
		{
			XSelectInput (dpy, w, ( StructureNotifyMask | 
									ButtonPressMask|
									ButtonReleaseMask));

			while(w != None)
  			{
    			XEvent event ;

		    	XNextEvent (dpy, &event);
  				switch(event.type)
				{
	  				case ClientMessage:
			    		if ((event.xclient.format != 32) ||
	  			    		(event.xclient.data.l[0] != _XA_WM_DELETE_WINDOW))
							break ;
			  		case ButtonPress:
						XDestroyWindow( dpy, w );
						XFlush( dpy );
						w = None ;
						break ;
				}
	  		}
		}
    	XCloseDisplay (dpy);
	}
#endif
}
/**************/
