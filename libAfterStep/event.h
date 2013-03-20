#ifndef EVENT_H_HEADER_INCLUDED
#define EVENT_H_HEADER_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ScreenInfo;

/* couple additional event masks to compensate for X defaults :*/
#ifndef SelectionMask
#define SelectionMask	(1L<<28)
#endif
#ifndef ClientMask
#define ClientMask      (1L<<29)
#endif
#ifndef MappingMask
#define MappingMask     (1L<<30)
#endif

#define ButtonAnyMask	(Button1Mask|Button2Mask|Button3Mask|Button4Mask|Button5Mask)

#define AllButtonMask    ButtonAnyMask
#ifndef __CYGWIN__
#define AllModifierMask  (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask)
#else
#define AllModifierMask  (ShiftMask|ControlMask|Mod1Mask)
#endif


typedef enum
{
    ASE_NotSupported     = 0,
    ASE_MouseMotionEvent = (0x01<<0),
    ASE_MousePressEvent  = (0x01<<1),
    ASE_KeyboardEvent    = (0x01<<2),
    ASE_Expose           = (0x01<<3),
	ASE_Config           = (0x01<<4),
	ASE_Redirection      = (0x01<<5),
#define ASE_SUBWINDOW_EVENTS     (ASE_MouseMotionEvent|ASE_MousePressEvent|ASE_KeyboardEvent)
#define ASE_POINTER_EVENTS       (ASE_MouseMotionEvent|ASE_MousePressEvent|ASE_KeyboardEvent)
}ASEventClass;

typedef struct ASEventDescription
{
	char         *name;
	unsigned long mask;
	ASEventClass  event_class;
	/* private fields : */
	ptrdiff_t		  time_offset;
	ptrdiff_t		  window_offset;
	Time		  last_time;
}ASEventDescription;


/* Event Handling return codes: */
#define ASE_Usefull		(0x01<<0)      /* event was processed */
#define ASE_Consumed 	(0x01<<1)      /* event was consumed by
                                        * processing function, and
								   		* should not be handled by
								   		* anything else */

typedef struct ASEvent
{
	/* the following info is parsed using ASEventDescription */
	unsigned long 		mask ;
	ASEventClass		eclass;
	Time                last_time ;
	Time                typed_last_time ;
	Time                event_time ;

	Window 	 	 		w ;
	struct ScreenInfo  *scr;
    struct ASWindow    *client ;
    struct ASCanvas    *widget ;
    int                 context ;
	XEvent 		 		x ;
}ASEvent;

/**********************************************************************/

/* This is used for error handling : */
extern int           last_event_type ;
extern Window        last_event_window;

void event_setup( Bool local );
const char *event_type2name( int type );
const char *context2text(int ctx);

/* better use macros so we could override/trace calls later on */
#define ASCheckMaskEvent(m,e)       check_event_masked(m,e)
#define ASCheckTypedEvent(t,e)      check_event_typed(t,e)
#define ASCheckTypedWindowEvent(w,t,e) check_event_typed_windowed(w,t,e)
#define ASCheckWindowEvent(w,m,e)   check_event_windowed(w,m,e)

#define ASEventScreen(e)            (ASDefaultScr)
#define ASNextEvent(e,compress_motion)              next_event (e,compress_motion)
#define ASMaskEvent(m,e)            mask_event (m,e)
#define ASWindowEvent(w,m,e)        window_event(w,m,e)

#define ASPeekEvent(e)              peek_event (e)

#define ASFlushIfEmpty()            flush_event_queue(True)
#define ASFlush()                   flush_event_queue(False)
#define ASFlushAndSync()            sync_event_queue(False)
#define ASSync(f)                   sync_event_queue(f)

/* We should use this event handling procedures instead of directly calling Xlib : */
void flush_event_queue(Bool check_pending);
void sync_event_queue(Bool forget);

struct ScreenInfo *query_event_screen( register XEvent *event );

Window get_xevent_window( XEvent *xevt );

void setup_asevent_from_xevent( ASEvent *event );

Bool check_event_masked( register long mask, register XEvent * event_return);
Bool check_event_typed( register int event_type, register XEvent * event_return);
Bool check_event_typed_windowed( Window w, int event_type, register XEvent *event_return );
Bool check_event_windowed (Window w, long event_mask, register XEvent * event_return);

int  next_event (register XEvent * event_return, Bool compress_motion);
int  mask_event (long event_mask, register XEvent * event_return);
int  window_event (Window w, long event_mask, register XEvent * event_return);

int  peek_event (register XEvent * event_return);

/* blocking UnmapNotify events since that may bring us into Withdrawn state */
void quietly_unmap_window( Window w, long event_mask );
void quietly_reparent_window( Window w, Window new_parent, int x, int y, long event_mask );
void safely_destroy_window (Window w);
/* convinience function : all the parameters are optional ! */
Bool query_pointer( Window w,
                    Window *root_return, Window *child_return,
                    int *root_x_return, int *root_y_return,
                    int *win_x_return, int *win_y_return,
                    unsigned int *mask_return );
#define ASQueryPointerXY(pchild,prx,pry,px,py)   query_pointer(None,NULL,(pchild),(prx),(pry),(px),(py),NULL)
#define ASQueryPointerWinXY(w,px,py)   query_pointer(w,NULL,NULL,NULL,NULL,(px),(py),NULL)
#define ASQueryPointerWinXYMask(w,prx,pry,px,py,pmask)   query_pointer(w,NULL,NULL,(prx),(pry),(px),(py),(pmask))
#define ASQueryPointerRootXY(prx,pry)   query_pointer(None,NULL,NULL,(prx),(pry),NULL,NULL,NULL)
#define ASQueryPointerRootXYMask(w,prx,pry,pmask) query_pointer(w,NULL,NULL,(prx),(pry),NULL,NULL,(pmask))
#define ASQueryPointerMask(pmask)       query_pointer(None,NULL,NULL,NULL,NULL,NULL,NULL,(pmask))
#define ASQueryPointerChild(w,pchild)   query_pointer((w),NULL,(pchild),NULL,NULL,NULL,NULL,NULL)
#define ASQueryPointerRoot(proot,pchild) query_pointer(None,(proot),(pchild),NULL,NULL,NULL,NULL,NULL)

void add_window_event_mask( Window w, long event_mask );

void handle_ShmCompletion(ASEvent *event);

#if !defined(EVENT_TRACE) || defined(NO_DEBUG_OUTPUT)
#define SHOW_EVENT_TRACE(event) \
    do{ if( get_output_threshold() >= OUTPUT_LEVEL_DEBUG ){ \
			if( event->x.type == PropertyNotify ) { \
				char *atom_name = XGetAtomName(dpy, event->x.xproperty.atom); \
				show_progress("%s:%s:%d><<EVENT type(%d(%s))->x.window(%lx)->property(%s)->client(%p)->send_event(%d)", __FILE__, __FUNCTION__, __LINE__, event->x.type, event_type2name((event)->x.type), (event)->x.xany.window, atom_name, (event)->client, (event)->x.xany.send_event); \
				XFree( atom_name ); \
			}else	show_progress("%s:%s:%d><<EVENT type(%d(%s))->x.window(%lx)->event.w(%lx)->client(%p)->context(%s)->send_event(%d)", __FILE__, __FUNCTION__, __LINE__, event->x.type, event_type2name((event)->x.type), (event)->x.xany.window, (event)->w, (event)->client, context2text((event)->context), (event)->x.xany.send_event); \
	}}while(0)
#else
#define SHOW_EVENT_TRACE(event) \
	do{ 	show_progress("****************************************************************"); \
        	show_progress("%s:%s:%d><<EVENT type(%d(%s))->x.window(%lx)->event.w(%lx)->client(%p)->context(%s)->send_event(%d)", __FILE__, __FUNCTION__, __LINE__, event->x.type, event_type2name((event)->x.type), (event)->x.xany.window, (event)->w, (event)->client, context2text((event)->context), (event)->x.xany.send_event); \
			if( event->x.type == PropertyNotify ) { \
				char *atom_name = XGetAtomName(dpy, event->x.xproperty.atom); \
				LOCAL_DEBUG_OUT( "\tproperty %s(%lX), _XROOTPMAP_ID = %lX, event->w = %lX, root = %lX", atom_name, event->x.xproperty.atom, _XROOTPMAP_ID, event->w, Scr.Root ); \
				XFree( atom_name ); \
	}}while(0)
#endif


#ifdef __cplusplus
}
#endif



#endif
