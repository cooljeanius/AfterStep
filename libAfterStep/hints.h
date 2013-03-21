#ifndef HINTS_H_HEADER_INCLUDED
#define HINTS_H_HEADER_INCLUDED

#include <X11/Xmd.h>
#include "clientprops.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ASRawHints;
struct MwmHints;
struct ASDatabaseRecord;
struct ASVector;
struct ScreenInfo;
struct ASFeel;
struct ASImage;
/***********************************************************************/
/*       AfterStep structure to hold summary of all the hints :        */
/***********************************************************************/

/***********************************************************/
/* AS Startup flags :                                      */
#define AS_StartPosition        (1<<0)
#define AS_StartPositionUser    (1<<1)
#define AS_Position 		    (1<<1)
#define AS_StartSize			(1<<2)
#define AS_Size					(1<<2)
#define AS_StartSizeUser		(1<<3)
#define AS_StartBorderWidth     (1<<4)
#define AS_BorderWidth     		(1<<4)
/* Viewport cannot be changed after window is mapped : */
#define AS_StartViewportX       (1<<5)
#define AS_StartViewportY       (1<<6)
#define AS_StartDesktop         (1<<7)
#define AS_Desktop 		        (1<<7)
#define AS_StartLayer			(1<<8)
#define AS_Layer				(1<<8)
/* the following are flags identifying client's status : */
#define AS_StartsIconic         (1<<9)
#define AS_Iconic               (1<<9)
#define AS_StartsMaximizedX     (1<<10)
#define AS_MaximizedX           (1<<10)
#define AS_StartsMaximizedY     (1<<11)
#define AS_MaximizedY           (1<<11)
#define AS_StartsSticky         (1<<12)
#define AS_Sticky               (1<<12)
#define AS_StartsShaded         (1<<13)
#define AS_Shaded               (1<<13)
/* special state - client withdrawn itself */
#define AS_Withdrawn            (1<<14)
#define AS_Dead                 (1<<15) /* dead client - has been destroyd or about to be destroyed */
/* special state - client is mapped - there is a small gap between MapRequest+XMapWindow and MapNotify event */
#define AS_Mapped               (1<<16)
#define AS_IconMapped           (1<<17)
#define AS_Hidden               (1<<18)
#define AS_Shaped               (1<<19)        /* client itself is shaped */
#define AS_ShapedDecor          (1<<20)        /* frame decorations use shaped MyStyle */
#define AS_ShapedIcon           (1<<21)        /* icon decorations use shaped MyStyle */
#define AS_UnMapPending         (1<<22)
#define AS_MoveresizeInProgress (1<<23)
#define AS_Fullscreen			(1<<24)
#define AS_Urgent				(1<<25)


/***********************************************************/
/* General flags                                           */
#define AS_MinSize				(1<<0)
#define AS_MaxSize				(1<<1)
#define AS_SizeInc				(1<<2)
#define AS_Aspect				(1<<3)
#define AS_BaseSize				(1<<4)
#define AS_Gravity				(1<<5)
#define AS_PID					(1<<6)
#define AS_Transient 			(1<<7)
#define AS_AcceptsFocus			(1<<8)
#define AS_ClickToFocus			(1<<9)
#define AS_Titlebar				(1<<10)
#define AS_VerticalTitle		(1<<11)
#define AS_Border				(1<<12)
#define AS_Handles				(1<<13)
#define AS_Frame				(1<<14)
#define AS_SkipWinList          (1<<15)
#define AS_DontCirculate        (1<<16)
#define AS_AvoidCover           (1<<17)
#define AS_IconTitle            (1<<18)
#define AS_Icon                 (1<<19)
#define AS_Windowbox            (1<<20)
#define AS_FocusOnMap           (1<<21)
#define AS_ShortLived           (1<<22)
#define AS_Module				(1<<23)
#define AS_IgnoreConfigRequest  (1<<24)
#define AS_IgnoreRestackRequest (1<<25)
#define AS_WMDockApp			(1<<26)  /* res_class == "DockApp" and main 
										  * window is 1x1 (just don't ask why)
										  * usually that means that icon 
										  * window should be animated */ 
#define AS_UseCurrentViewport  	(1<<27)
#define AS_WindowOpacity	   	(1<<28)
#define AS_HitPager	   	(1<<29)

#define NOLOOK_HINT_FLAGS	(AS_IgnoreConfigRequest|AS_Module|AS_ShortLived| \
							 AS_AvoidCover|AS_AcceptsFocus|AS_ClickToFocus| \
							 AS_UseCurrentViewport)


/***********************************************************/
/* AS supported protocols :                                */
#define AS_DoesWmTakeFocus 		(1<<0)
#define AS_DoesWmDeleteWindow 	(1<<1)
#define AS_DoesWmPing           (1<<2)
#define AS_NeedsVisibleName		(1<<3)  /* only if window has _NET_WM_NAME hint */
#define AS_DoesKIPC				(1<<4)

/***********************************************************/
/* AS function masks :                                     */
#define AS_FuncPopup            (1<<0)
#define AS_FuncMinimize         (1<<1)
#define AS_FuncMaximize         (1<<2)
#define AS_FuncResize           (1<<3)
#define AS_FuncMove             (1<<4)
#define AS_FuncClose            (1<<7)
#define AS_FuncKill             (1<<8)
#define AS_FuncPinMenu          (1<<9)
/***********************************************************/
/* AS layers :                                             */
#define AS_LayerLowest          AS_LayerDesktop
#define AS_LayerDesktop         -10000  /* our desktop - just for the heck of it */
#define AS_LayerOtherDesktop    -2      /* for all those other file managers, KDE, GNOME, etc. */
#define AS_LayerBack            -1      /* normal windows below */
#define AS_LayerNormal           0      /* normal windows */
#define AS_LayerTop              1      /* normal windows above */
#define AS_LayerService          2      /* primarily for Wharfs, etc. */
#define AS_LayerUrgent           3      /* for modal dialogs that needs urgent answer (System Modal)*/
#define AS_LayerOtherMenu        4      /* for all those other menus - KDE, GNOME, etc. */
#define AS_LayerMenu             10000  /* our menu  - can't go wrong with that */
#define AS_LayerHighest          AS_LayerMenu

#define ASHINTS_STATIC_DATA 	 28     /* number of elements below that are not */
                                        /* dynamic arrays */

typedef struct ASHints
{
  /* NULL terminated list of names/aliases */
#define MAX_WINDOW_NAMES        8
  char *names[MAX_WINDOW_NAMES+1] ;
  /* these are merely shortcuts to the above list DON'T FREE THEM !!! */
  char *res_name, *res_class, *icon_name ;
  int res_name_idx, res_class_idx, icon_name_idx ;

  unsigned char names_encoding[MAX_WINDOW_NAMES+1];  /* one of the AS_Text_ values */

  /* these are copy of above done, when ASDatabase was last matched */
  char *matched_name0 ;
  unsigned char matched_name0_encoding;  

  ASFlagType flags ;
  ASFlagType protocols ;
  ASFlagType function_mask ;

#define AS_ClientIcon           (1<<0)
#define AS_ClientIconPixmap     (1<<1)
#define AS_ClientIconPosition   (1<<2)
#define AS_ClientIconARGB		(1<<3)

  ASFlagType client_icon_flags ;
  union { Window window; Pixmap pixmap; } icon ;
  Pixmap icon_mask ;
  CARD32 *icon_argb ; 
  int icon_x, icon_y ;
  char *icon_file ;

  int min_width, min_height ;
  int max_width, max_height ;
  int width_inc, height_inc ;
  struct { int x; int y ; } min_aspect, max_aspect ;
  int base_width, base_height ;
  int gravity ;
  unsigned int border_width ; /* this is border width that will be used
                               * in the frame decoration  - not to confuse with border width
							   * of the initial withdrawn-to-normal transition  */
  unsigned int handle_width ;
  Window group_lead ;
  Window transient_for;

  CARD32 *cmap_windows ; /* terminated with None value (zero)*/

  int pid ;

  char *frame_name ;
  char *windowbox_name ;
  char *mystyle_names[BACK_STYLES];

  ASFlagType disabled_buttons ;

  ASFlagType hints_types_raw ;
  ASFlagType hints_types_clean ;

  char *client_host ;    /* hostname of the computer on which client was executed */
  char *client_cmd  ;    /* preparsed command line of the client */
  CARD32 window_opacity ;
}
ASHints;

/* the following flags will let us specify what exactly has
 * changed when hints are updated : */
#define AS_HintChangeName       (1<<0)
#define AS_HintChangeClass      (1<<1)
#define AS_HintChangeResName    (1<<2)
#define AS_HintChangeIconName   (1<<3)

#define AS_HintChangeEverything ASFLAGS_EVERYTHING

#define ASSTATUSHINTS_STATIC_DATA 11    /* number of elements below that are not */
                                        /* dynamic arrays */

typedef struct ASStatusHints
{
    ASFlagType   flags ;

    int             x, y;
    unsigned int    width, height;
    unsigned int    border_width ; /* this border width is needed only to calculate
								    * reference point when we are starting up */
    int    			viewport_x, viewport_y;
    int             desktop ;
	int 			layer ;

    /* get's set by window manager - not read from hints: */
    Window          icon_window;
	unsigned int    frame_size[FRAME_SIDES];   /* size of the frame decoration */

	unsigned int 	frame_border_width ;
}ASStatusHints;

/*
 * This structure will hold pointers to merge function according to
 * priorities set by user :
 */
typedef void (*hints_merge_func)(ASHints* clean, struct ASRawHints *raw,
								 struct ASDatabaseRecord *db_rec,
								 ASStatusHints *status,
                                 ASFlagType what);  /* see HINT_ flags above */

typedef struct ASSupportedHints
{
    ASFlagType hints_flags ;  /* 0x1<<type */
	HintsTypes 	  hints_types[HINTS_Supported];
    hints_merge_func merge_funcs[HINTS_Supported];
    int 		  hints_num ;
}ASSupportedHints;



/*************************************************************************/
/*                           Interface                                   */
/*************************************************************************/
/* this calculates offset from the anchor position to the left/top corner of the frame : */
#define APPLY_GRAVITY(grav,pos,size,bw1,bw2) \
{ switch( (grav) )					         \
{case  0 : (pos) -= ((int)(size)>>1)+(int)(bw1)    ; break ; /* Center/Forget */ \
 case  1 : (pos) -= (int)(bw1)+(int)(bw2)+(int)(size)   ; break ; /* South/East */    \
 case  2 : (pos) -= (int)(bw1)                 ; break ; /* Static */        \
 default :                                 break ; /* North/West */    \
}}

unsigned char get_hint_name_encoding( ASHints *hints, int name_idx );
ASHints *merge_hints( struct ASRawHints *raw, struct ASDatabase *db, ASStatusHints *status,
                      ASSupportedHints *list, ASFlagType what, ASHints* reusable_memory, Window client );
void merge_asdb_hints (ASHints * clean, struct ASRawHints * raw, struct ASDatabaseRecord * db_rec, ASStatusHints * status, ASFlagType what);

void check_motif_hints_sanity (struct MwmHints * motif_hints);
ASFlagType extwm_state2as_state_flags( ASFlagType extwm_flags );

/*
 * few function - shortcuts to implement update of selected hints :
 */
/* returns True if protocol/function hints actually changed :*/
Bool update_protocols( struct ScreenInfo *scr, Window w, ASSupportedHints *list, ASFlagType *pprots, ASFlagType *pfuncs );
Bool update_colormaps( struct ScreenInfo *scr, Window w, ASSupportedHints *list, CARD32 **pcmap_windows );
Bool update_property_hints( Window w, Atom property, ASHints *hints, ASStatusHints *status );
Bool update_property_hints_manager( Window w, Atom property, ASSupportedHints *list,
                                    struct ASDatabase * db, ASHints *hints, ASStatusHints *status );
void update_cmd_line_hints (Window w, Atom property, 
					   ASHints * hints, ASStatusHints * status);

void check_hints_sanity (struct ScreenInfo * scr, ASHints * clean, ASStatusHints * status, Window client);
void check_status_sanity (struct ScreenInfo * scr, ASStatusHints * status);


void destroy_hints( ASHints *clean, Bool reusable );
Bool compare_names( ASHints *old, ASHints *hints );
ASFlagType compare_hints( ASHints *old, ASHints *hints );
ASFlagType function2mask( int function );
void constrain_size ( ASHints *hints, ASStatusHints *status,
                 	  int max_width, int max_height );
void get_gravity_offsets (ASHints *hints, int *xp, int *yp);
int translate_asgeometry( struct ScreenInfo *scr, ASGeometry *asg, int *px, int *py, unsigned int *pwidth, unsigned int *pheight );
void real2virtual (ASStatusHints *status, int *x, int *y, int vx, int vy );
void virtual2real (ASStatusHints *status, int *x, int *y, int vx, int vy);
int make_anchor_pos (ASStatusHints * status, int pos, int size, int vpos, int grav, int max_pos);
void make_detach_pos (ASHints * hints, ASStatusHints * status, XRectangle *anchor, int *detach_x, int *detach_y);
ASFlagType change_placement( struct ScreenInfo *scr, ASHints *hints, ASStatusHints *status, XPoint *anchor, ASStatusHints *new_status, int vx, int vy, ASFlagType what );

void status2anchor( XRectangle *anchor, struct ASHints *hints, struct ASStatusHints *status, int vwidth, int vheight );
void anchor2status( struct ASStatusHints *status, struct ASHints *hints, XRectangle *anchor );

int calculate_viewport( int *pos, int size, int scr_vpos, int scr_size, int max_viewport );
char *make_client_geometry_string (struct ScreenInfo * scr, ASHints *hints, ASStatusHints *status, XRectangle *anchor, int vx, int vy, char **pure_geometry);
char *make_client_command( struct ScreenInfo *scr, ASHints *hints, ASStatusHints *status, XRectangle *anchor, int vx, int vy);

Bool set_all_client_hints( Window w, ASHints *hints, ASStatusHints *status, Bool set_command );
struct ASImage* get_client_icon_image( struct ScreenInfo * scr, ASHints *hints );
CARD32 set_hints_window_opacity_percent( ASHints *clean, int opaque_percent );


/* printing functions :
 * if func and stream are not specified - fprintf(stderr) is used ! */
void print_clean_hints( stream_func func, void* stream, ASHints *clean );
void print_status_hints( stream_func func, void* stream, ASStatusHints *status );

/* serialization so that we can send modules some data :                         */
void serialize_string( char * string, struct ASVector *buf );
void serialize_CARD32_zarray( CARD32 *array, struct ASVector *buf );
Bool serialize_clean_hints( ASHints *clean, struct ASVector *buf );
Bool serialize_names( ASHints *clean, struct ASVector *buf );
Bool serialize_status_hints( ASStatusHints *status, struct ASVector *buf );

/*  deserialization so that module can read out communications:                  */
char *deserialize_string( CARD32 **pbuf, size_t *buf_size );
CARD32 *deserialize_CARD32_zarray( CARD32 **pbuf, size_t *buf_size );
ASHints *deserialize_clean_hints( CARD32 **pbuf, size_t *buf_size, ASHints *reusable_memory );
Bool deserialize_names( ASHints *clean, CARD32 **pbuf, size_t *buf_size );
ASStatusHints *deserialize_status_hints( CARD32 **pbuf, size_t *buf_size, ASStatusHints *reusable_memory );


/* This will let us to change what hints are actually applied and in what order */
ASSupportedHints *create_hints_list();
void destroy_hints_list( ASSupportedHints **plist );
Bool enable_hints_support( ASSupportedHints *list, HintsTypes type );
Bool disable_hints_support( ASSupportedHints *list, HintsTypes type );
HintsTypes *supported_hints_types( ASSupportedHints *list, int *num_return );


/*************************************************************************/
/********************************THE END**********************************/
/*************************************************************************/
#ifdef __cplusplus
}
#endif


#endif /* HINTS_H_HEADER_INCLUDED */
