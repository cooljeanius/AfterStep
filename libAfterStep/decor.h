#ifndef DECOR_H_HEADER
#define DECOR_H_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "canvas.h"

struct MyStyle ;
struct ASImage;
struct icon_t;
struct button_t;
struct ASImageManager;
struct ASBalloon;
struct ASBalloonState;

typedef struct ASTBtnData{
    struct ASImage *unpressed ;
    struct ASImage *pressed ;
    struct ASImage *current ;
    unsigned short width, height ;
    short x, y ;                               /* relative to the button block origin !*/
    unsigned long context ;
    struct ASBalloon *balloon ;
    /* 24 bytes */
}ASTBtnData;

#define TBTN_ORDER_L2R      0
#define TBTN_ORDER_T2B      1
#define TBTN_ORDER_R2L      2
#define TBTN_ORDER_B2T      3
#define TBTN_ORDER_VERTICAL (0x01<<0)
#define TBTN_ORDER_REVERSE  (0x01<<1)
#define TBTN_ORDER_MASK     (TBTN_ORDER_VERTICAL|TBTN_ORDER_REVERSE)

typedef struct ASBtnBlock {
    ASTBtnData      *buttons;                  /* array of [count] structures */
    unsigned int     buttons_num;
    /* 8 bytes */
}ASBtnBlock;

typedef struct ASLabel {
    char            *text ;
	unsigned long    encoding ;                /* one of those AS_Text_ values */
	struct ASImage  *rendered[2] ;
	short			 h_padding, v_padding ;
	/* 20 bytes */
}ASLabel;

typedef struct ASImageTile {
    ASImage *im ;
	unsigned short slice_x_start, slice_x_end ;
	unsigned short slice_y_start, slice_y_end ;
	/* 12 bytes */
}ASImageTile;


typedef struct ASTile {
#define AS_TileSpacer	 	0
#define AS_TileBtnBlock 	1
#define AS_TileIcon		 	2
#define AS_TileLabel	 	3
#define AS_TileFreed        7

#define AS_TileTypeMask     (0x07<<0)
#define AS_TileTypes        8
#define ASTileType(t)      ((t).flags&AS_TileTypeMask)
#define ASSetTileType(tl,ty)  ((tl)->flags=((tl)->flags&(~AS_TileTypeMask))|(ty))
#define ASGetTileType(tl)   (((tl)->flags)&AS_TileTypeMask)

#define AS_TileColOffset    4
#define AS_TileColumns      8
#define AS_TileColMask      (0x07<<AS_TileColOffset)
#define ASTileCol(t)        (((t).flags&AS_TileColMask)>>AS_TileColOffset)

#define AS_TileRowOffset    (AS_TileColOffset+3)  /* 7 */
#define AS_TileRows         8
#define AS_TileRowMask      (0x07<<AS_TileRowOffset)
#define ASTileRow(t)        (((t).flags&AS_TileRowMask)>>AS_TileRowOffset)

#define AS_TileFlipOffset   (AS_TileRowOffset+3)  /* 10 */
#define AS_TileVFlip        (0x01<<AS_TileFlipOffset)
#define AS_TileUFlip        (0x01<<(AS_TileFlipOffset+1))
#define AS_TileFlipMask     (AS_TileVFlip|AS_TileUFlip)
#define ASTileFlip(t)       (((t).flags&AS_TileFlipMask)>>AS_TileFlipOffset)

#define AS_TileSublayersOffset  (AS_TileFlipOffset+2)  /* 12 */
#define AS_TileSublayersMask    (0x00FF<<AS_TileSublayersOffset)  /* max 256 sublayers */
#define ASTileSublayers(t)      (((t).flags&AS_TileSublayersMask)>>AS_TileSublayersOffset)
#define ASSetTileSublayers(t,c)  ((t).flags = ((t).flags&(~AS_TileSublayersMask))|((c<<AS_TileSublayersOffset)&AS_TileSublayersMask))

/* PAD_ stuff begins with 0 offset (see afterstep.h) */
#define AS_TilePadOffset    (AS_TileSublayersOffset+8)  /* 20 */
#define AS_TilePadLeft      (PAD_LEFT  <<AS_TilePadOffset)
#define AS_TilePadRight     (PAD_RIGHT <<AS_TilePadOffset)
#define AS_TileHPadMask     (PAD_H_MASK<<AS_TilePadOffset)
#define AS_TilePadBottom    (PAD_BOTTOM<<AS_TilePadOffset)  /* rotated with Top on purpose */
#define AS_TilePadTop       (PAD_TOP   <<AS_TilePadOffset)
#define AS_TileVPadMask     (PAD_V_MASK<<AS_TilePadOffset)
#define AS_TilePadMask      (AS_TileVPadMask|AS_TileHPadMask)
/* RESIZE_ stuff is already offset as related to PAD_ (see afterstep.h) */
#define AS_TileHResize      (RESIZE_H       <<AS_TilePadOffset) /* either tile or scale in H direction*/
#define AS_TileHScale       (RESIZE_H_SCALE <<AS_TilePadOffset) /* otherwise tiled in H direction */
#define AS_TileHResizeMask  (RESIZE_H_MASK  <<AS_TilePadOffset)
#define AS_TileVResize      (RESIZE_V       <<AS_TilePadOffset) /* either tile or scale in V direction*/
#define AS_TileVScale       (RESIZE_V_SCALE <<AS_TilePadOffset) /* otherwise tiled in V direction */
#define AS_TileVResizeMask  (RESIZE_V_MASK  <<AS_TilePadOffset)
#define AS_TileIgnoreWidth  (FIT_LABEL_WIDTH<<AS_TilePadOffset)
#define AS_TileIgnoreHeight (FIT_LABEL_HEIGHT<<AS_TilePadOffset)
#define AS_TileIgnoreSize   (FIT_LABEL_SIZE <<AS_TilePadOffset)
#define AS_TileResizeMask   (AS_TileHResizeMask|AS_TileVResizeMask)
#define ASTileHResizeable(t)    ((t).flags&AS_TileHResize)
#define ASTileVResizeable(t)    ((t).flags&AS_TileVResize)
#define ASTileResizeable(t)     ((t).flags&(AS_TileHResize|AS_TileVResize))
#define ASTileScalable(t)       ((t).flags&(AS_TileHScale|AS_TileVScale))
#define ASTileIgnoreWidth(t)    ((t).flags&(AS_TileIgnoreWidth))
#define ASTileIgnoreHeight(t)   ((t).flags&(AS_TileIgnoreHeight))
#define ASTileIgnoreSize(t)     ((t).flags&(AS_TileIgnoreSize))

/* 28 */
#define AS_TileFloatingOffset   AS_TilePadOffset
/* 29 */
#define AS_TileHFloatingMask    (AS_TileHResize|AS_TileHPadMask)
#define AS_TileVFloatingMask    (AS_TileVResize|AS_TileVPadMask)
#define AS_TileFloatingMask     (AS_TileHFloatingMask|AS_TileVFloatingMask)
#define ASTileHFloating(t)      ((t).flags&(AS_TileHResize|AS_TileHPadMask))
#define ASTileHPad(t)           ((t).flags&(AS_TileHPadMask))
#define ASTileVFloating(t)      ((t).flags&(AS_TileVResize|AS_TileVPadMask))
#define ASTileVPad(t)           ((t).flags&(AS_TileVPadMask))

    ASFlagType flags;
    short x, y;
    short width, height;
	union {
		ASBtnBlock	 bblock;
        ASImageTile  image ;
		ASLabel      label ;
        unsigned long raw[4];
	}data;
    /* 28 bytes */
}ASTile;

typedef struct ASTBarData {
#define BAR_STATE_UNFOCUSED		0
#define BAR_STATE_FOCUSED		(0x01<<0)
#define IsASTBarFocused(pb) 	((pb) && get_flags ((pb)->state, BAR_STATE_FOCUSED))

#define BAR_STATE_NUM			2
#define BAR_STATE_FOCUS_MASK	(0x01<<0)
#define BAR_STATE_PRESSED		(0x01<<1)
#define BAR_STATE_PRESSED_MASK	(0x01<<1)
#define IsASTBarPressed(pb) 	((pb) && get_flags ((pb)->state, BAR_STATE_PRESSED))

#define BAR_FLAGS_REND_PENDING  (0x01<<16)     /* has been moved, resized or otherwise changed and needs rerendering */
#define DoesBarNeedsRendering(pb) ((pb) && get_flags((pb)->state, BAR_FLAGS_REND_PENDING))
#define SetBarNeedsRendering(pb)  ((pb) && set_flags((pb)->state, BAR_FLAGS_REND_PENDING))

#define BAR_FLAGS_VERTICAL      (0x01<<17)     /* vertical label */
#define BAR_FLAGS_IMAGE_BACK    (0x01<<18)     /* back represents an icon instead of  */
#define BAR_FLAGS_CROP_UNFOCUSED_BACK   (0x01<<19)     /* crop background image in relation to canvas origin  */
#define BAR_FLAGS_CROP_FOCUSED_BACK     (0x01<<20)     /* crop background image in relation to canvas origin  */
#define BAR_FLAGS_CROP_BACK  (BAR_FLAGS_CROP_FOCUSED_BACK|BAR_FLAGS_CROP_UNFOCUSED_BACK)

    ASFlagType  state ;
    unsigned long context ;
    short win_x, win_y ;
    short root_x, root_y;
    /* 16 bytes */
    short rendered_root_x, rendered_root_y;
    unsigned short width, height ;
    unsigned char left_bevel, top_bevel, right_bevel, bottom_bevel ;
    /* 28 bytes */
    /* this is what we make our background from :*/
    struct MyStyle      *style[2] ;
    /* this is the actuall generated background : */
    struct ASImage      *back [2] ;
    /* 44 bytes */
    unsigned char h_border, v_border;
	unsigned char h_spacing, v_spacing;
    /* 48 bytes */
	ASTile *tiles;
    struct ASBalloon *balloon;
    /* 56 bytes */
	unsigned short tiles_num ;
    /* 58 bytes */
    unsigned char composition_method[2] ;         /* focused/unfocused may have different composition methods */
    unsigned char hilite[2] ;
    /* 62 bytes */
	short hue[2], sat[2] ;
    /* 70 bytes */
}ASTBarData ;

ASTBtnData *create_astbtn();
void        set_tbtn_images( ASTBtnData* btn, struct button_t *from );
ASTBtnData *make_tbtn( struct button_t *from );
void        destroy_astbtn(ASTBtnData **ptbtn );

ASTBarData* create_astbar();
void destroy_astbar( ASTBarData **ptbar );
unsigned int get_astbar_label_width( ASTBarData *tbar );
unsigned int get_astbar_label_height( ASTBarData *tbar );
unsigned int calculate_astbar_height( ASTBarData *tbar );
unsigned int calculate_astbar_width( ASTBarData *tbar );


Bool set_astbar_size( ASTBarData *tbar, unsigned int width, unsigned int height );
Bool set_astbar_hilite( ASTBarData *tbar, unsigned int state, ASFlagType hilite );
Bool set_astbar_composition_method( ASTBarData *tbar, unsigned int state, unsigned char method );
Bool set_astbar_huesat( ASTBarData *tbar, unsigned int state, int hue, int sat );
Bool set_astbar_style_ptr (ASTBarData * tbar, int state, struct MyStyle *style);
Bool set_astbar_style( ASTBarData *tbar, unsigned int state, const char *style_name );
Bool set_astbar_flip( ASTBarData * tbar, int flip );
Bool invalidate_astbar_style (ASTBarData * tbar, int state);


int make_tile_pad( Bool pad_before, Bool pad_after, int cell_size, int tile_size );

int add_astbar_spacer( ASTBarData *tbar, unsigned char col, unsigned char row, int flip, int align, unsigned short width, unsigned short height);
int add_astbar_btnblock( ASTBarData * tbar, unsigned char col, unsigned char row, int flip, int align,
                         struct button_t **from_list, ASFlagType context_mask, unsigned int count,
                         int left_margin, int top_margin, int spacing, int order);
int add_astbar_icon( ASTBarData * tbar, unsigned char col, unsigned char row, int flip, int align, struct ASImage *icon);
int add_astbar_image( ASTBarData * tbar, unsigned char col, unsigned char row, int flip, int align, ASImage *im, 
				  unsigned short slice_x_start, unsigned short slice_x_end,
				  unsigned short slice_y_start, unsigned short slice_y_end);

int add_astbar_label( ASTBarData * tbar, unsigned char col, unsigned char row, int flip, int align, short h_padding, short v_padding, const char *text, unsigned long encoding);
Bool delete_astbar_tile( ASTBarData *tbar, int idx );

Bool change_astbar_label (ASTBarData * tbar, int index, const char *label, unsigned long encoding);
Bool change_astbar_first_label (ASTBarData * tbar, const char *label, unsigned long encoding);


Bool move_astbar( ASTBarData *tbar, ASCanvas *pc, int win_x, int win_y );
Bool set_astbar_focused( ASTBarData *tbar, ASCanvas *pc, Bool focused );
Bool set_astbar_pressed( ASTBarData *tbar, ASCanvas *pc, Bool pressed );
Bool set_astbar_btn_pressed( ASTBarData * tbar, int context );

Bool update_astbar_transparency( ASTBarData *tbar, ASCanvas *pc, Bool force );
Bool is_astbar_shaped( ASTBarData *tbar, int state );
int  check_astbar_point( ASTBarData *tbar, int root_x, int root_y );

#ifdef TRACE_render_astbar
Bool  trace_render_astbar (ASTBarData * tbar, ASCanvas * pc, const char *file, int line);
#define render_astbar(t,p)  trace_render_astbar ((t),(p),__FILE__,__LINE__)
#else
Bool render_astbar( ASTBarData *tbar, ASCanvas *pc );
#endif
Bool render_astbar_cached_back (ASTBarData * tbar, ASCanvas * pc, ASImage **cache, ASCanvas *origin_canvas);

void on_astbar_pointer_action( ASTBarData *tbar, int context, Bool leave, Bool pointer_moved );
void set_astbar_balloon( ASTBarData *tbar, int context, const char *text, unsigned long encoding );
void set_astbar_balloon2( ASTBarData *tbar, struct ASBalloonState *balloon_state, int context, const char *text, unsigned long encoding );


#ifdef __cplusplus
}
#endif


#endif /* DECOR_H_HEADER */
