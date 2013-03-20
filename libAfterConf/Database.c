/*
 * Copyright (c) 1998 Sasha Vasko <sasha at aftercode.net>
 * some of the code used was written by either of:
 *  Copyright (c) 1999 Ethan Fischer <allanon@crystaltokyo.com>
 *  Copyright (C) 1995 Bo Yang
 *  Copyright (C) 1993 Robert Nation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#define LOCAL_DEBUG

#include "../configure.h"
#include "../libAfterStep/asapp.h"
#include "../libAfterStep/afterstep.h"
#include "../libAfterStep/asdatabase.h"
#include "../libAfterStep/parser.h"
#include "../libAfterStep/hints.h"
#include "../libAfterStep/session.h"

#include "afterconf.h"

/*****************************************************************************
 *
 * This routine is responsible for reading and parsing the base.<bpp> config
 * file
 *
 ****************************************************************************/

TermDef       GravityTerms[] = {
	{TF_NO_MYNAME_PREPENDING, "NorthWest", 9, TT_FLAG, GRAVITY_NorthWest_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "North", 5, TT_FLAG, GRAVITY_North_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NorthEast", 9, TT_FLAG, GRAVITY_NorthEast_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "West", 4, TT_FLAG, GRAVITY_West_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Center", 6, TT_FLAG, GRAVITY_Center_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "East", 4, TT_FLAG, GRAVITY_East_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "SouthWest", 9, TT_FLAG, GRAVITY_SouthWest_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "South", 5, TT_FLAG, GRAVITY_South_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "SouthEast", 9, TT_FLAG, GRAVITY_SouthEast_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Static", 6, TT_FLAG, GRAVITY_Static_ID, NULL},
	{0, NULL, 0, 0, 0}
};

SyntaxDef     GravitySyntax = {
	' ',
	',',
	GravityTerms,
	7,										   /* hash size */
    ' ',
	" ",
	"\t",
    "Database Gravity type specification",
	"Gravity",
	"",
	NULL,
	0
};


TermDef       StyleTerms[] = {
	{TF_NO_MYNAME_PREPENDING, "Icon", 4, TT_FILENAME, DATABASE_Icon_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoIcon", 6, TT_FLAG, DATABASE_NoIcon_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "FocusStyle", 10, TT_TEXT, DATABASE_FocusStyle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "UnfocusStyle", 12, TT_TEXT, DATABASE_UnfocusStyle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StickyStyle", 11, TT_TEXT, DATABASE_StickyStyle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoIconTitle", 11, TT_FLAG, DATABASE_NoIconTitle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "IconTitle", 9, TT_FLAG, DATABASE_IconTitle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Focus", 5, TT_FLAG, DATABASE_Focus_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoFocus", 7, TT_FLAG, DATABASE_NoFocus_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoTitle", 7, TT_FLAG, DATABASE_NoTitle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Title", 5, TT_FLAG, DATABASE_Title_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoHandles", 9, TT_FLAG, DATABASE_NoHandles_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Handles", 7, TT_FLAG, DATABASE_Handles_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoButton", 8, TT_INTEGER, DATABASE_NoButton_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Button", 6, TT_INTEGER, DATABASE_Button_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "WindowListSkip", 14, TT_FLAG, DATABASE_WindowListSkip_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "WindowListHit", 13, TT_FLAG, DATABASE_WindowListHit_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "CirculateSkip", 13, TT_FLAG, DATABASE_CirculateSkip_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "CirculateHit", 12, TT_FLAG, DATABASE_CirculateHit_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StartIconic", 11, TT_FLAG, DATABASE_StartIconic_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StartNormal", 11, TT_FLAG, DATABASE_StartNormal_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Layer", 5, TT_INTEGER, DATABASE_Layer_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StaysOnTop", 10, TT_FLAG, DATABASE_StaysOnTop_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StaysPut", 8, TT_FLAG, DATABASE_StaysPut_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StaysOnBack", 11, TT_FLAG, DATABASE_StaysOnBack_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "AvoidCover", 10, TT_FLAG, DATABASE_AvoidCover_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "AllowCover", 10, TT_FLAG, DATABASE_AllowCover_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "VerticalTitle", 13, TT_FLAG, DATABASE_VerticalTitle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HorizontalTitle", 15, TT_FLAG, DATABASE_HorizontalTitle_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Sticky", 6, TT_FLAG, DATABASE_Sticky_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "Slippery", 8, TT_FLAG, DATABASE_Slippery_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "BorderWidth", 11, TT_INTEGER, DATABASE_BorderWidth_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HandleWidth", 11, TT_INTEGER, DATABASE_HandleWidth_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StartsOnDesk", 12, TT_INTEGER, DATABASE_StartsOnDesk_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "ViewportX", 9, TT_INTEGER, DATABASE_ViewportX_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "ViewportY", 9, TT_INTEGER, DATABASE_ViewportY_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "StartsAnywhere", 14, TT_FLAG, DATABASE_StartsAnywhere_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoFrame", 7, TT_FLAG, DATABASE_NoFrame_ID, NULL},
    {TF_NO_MYNAME_PREPENDING, "Frame", 5, TT_TEXT, DATABASE_Frame_ID, NULL},
    {TF_NO_MYNAME_PREPENDING, "WindowBox", 9, TT_TEXT, DATABASE_Windowbox_ID, NULL},
    {TF_NO_MYNAME_PREPENDING, "DefaultGeometry", 15, TT_GEOMETRY, DATABASE_DefaultGeometry_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "OverrideGravity", 15, TT_FLAG, DATABASE_OverrideGravity_ID, &GravitySyntax},
	{TF_NO_MYNAME_PREPENDING, "HonorPPosition", 14, TT_FLAG, DATABASE_HonorPPosition_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoPPosition", 11, TT_FLAG, DATABASE_NoPPosition_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorGroupHints", 15, TT_FLAG, DATABASE_HonorGroupHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoGroupHints", 12, TT_FLAG, DATABASE_NoGroupHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorTransientHints", 19, TT_FLAG, DATABASE_HonorTransientHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoTransientHints", 16, TT_FLAG, DATABASE_NoTransientHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorMotifHints", 15, TT_FLAG, DATABASE_HonorMotifHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoMotifHints", 12, TT_FLAG, DATABASE_NoMotifHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorGnomeHints", 15, TT_FLAG, DATABASE_HonorGnomeHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoGnomeHints", 12, TT_FLAG, DATABASE_NoGnomeHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorKDEHints", 13, TT_FLAG, DATABASE_HonorKDEHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoKDEHints", 10, TT_FLAG, DATABASE_NoKDEHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorExtWMHints", 15, TT_FLAG, DATABASE_HonorExtWMHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoExtWMHints", 12, TT_FLAG, DATABASE_NoExtWMHints_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorXResources", 15, TT_FLAG, DATABASE_HonorXResources_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoXResources", 12, TT_FLAG, DATABASE_NoXResources_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "FocusOnMap", 10, TT_FLAG, DATABASE_FocusOnMap_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "NoFocusOnMap", 12, TT_FLAG, DATABASE_NoFocusOnMap_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "LongLiving", 10, TT_FLAG, DATABASE_LongLiving_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "ShortLiving", 11, TT_FLAG, DATABASE_ShortLiving_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "IgnoreConfig", 12, TT_FLAG, DATABASE_IgnoreConfig_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorConfig", 11, TT_FLAG, DATABASE_HonorConfig_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "UseCurrentViewport", 18, TT_FLAG, DATABASE_UseCurrentViewport_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "UseAnyViewport", 14, TT_FLAG, DATABASE_UseAnyViewport_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "IgnoreRestack", 13, TT_FLAG, DATABASE_IgnoreRestack_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "HonorRestack", 12, TT_FLAG, DATABASE_HonorRestack_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "PagerHit", 10, TT_FLAG, DATABASE_PagerHit_ID, NULL},
	{TF_NO_MYNAME_PREPENDING, "PagerSkip", 12, TT_FLAG, DATABASE_PagerSkip_ID, NULL},
	ASCF_DEFINE_KEYWORD(DATABASE,TF_NO_MYNAME_PREPENDING,Fullscreen,TT_FLAG,NULL),
	ASCF_DEFINE_KEYWORD(DATABASE,TF_NO_MYNAME_PREPENDING,NoFullscreen,TT_FLAG,NULL),
	ASCF_DEFINE_KEYWORD(DATABASE,TF_NO_MYNAME_PREPENDING,WindowOpacity,TT_INTEGER,NULL),
	{0, NULL, 0, 0, 0}
};

SyntaxDef     StyleSyntax = {
	',',
	'\n',
	StyleTerms,
	0,										   /* use default hash size */
    ' ',
	" ",
	"\t",
	"Database Style definition",
	"ASDatabaseEntry",
	"to be used in entry of AfterStep database",
	NULL,
	0
};

flag_options_xref StyleFlags[] = {
	{STYLE_FULLSCREEN,			DATABASE_Fullscreen_ID, 		DATABASE_NoFullscreen_ID},
	{STYLE_STICKY, 				DATABASE_Sticky_ID, 			DATABASE_Slippery_ID},
	{STYLE_TITLE, 				DATABASE_Title_ID, 				DATABASE_NoTitle_ID},
	{STYLE_CIRCULATE, 			DATABASE_CirculateHit_ID, 		DATABASE_CirculateSkip_ID},
	{STYLE_WINLIST, 			DATABASE_WindowListHit_ID, 		DATABASE_WindowListSkip_ID},
	{STYLE_START_ICONIC, 		DATABASE_StartIconic_ID, 		DATABASE_StartNormal_ID},
	{STYLE_ICON_TITLE, 			DATABASE_IconTitle_ID, 			DATABASE_NoIconTitle_ID},
	{STYLE_FOCUS, 				DATABASE_Focus_ID, 				DATABASE_NoFocus_ID},
	{STYLE_AVOID_COVER, 		DATABASE_AvoidCover_ID, 		DATABASE_AllowCover_ID},
	{STYLE_VERTICAL_TITLE, 		DATABASE_VerticalTitle_ID, 		DATABASE_HorizontalTitle_ID},
	{STYLE_HANDLES, 			DATABASE_Handles_ID, 			DATABASE_NoHandles_ID},
	{STYLE_PPOSITION, 			DATABASE_HonorPPosition_ID, 	DATABASE_NoPPosition_ID},
	{STYLE_GROUP_HINTS, 		DATABASE_HonorGroupHints_ID, 	DATABASE_NoGroupHints_ID},
	{STYLE_TRANSIENT_HINTS, 	DATABASE_HonorTransientHints_ID, DATABASE_NoTransientHints_ID},
	{STYLE_MOTIF_HINTS, 		DATABASE_HonorMotifHints_ID, 	DATABASE_NoMotifHints_ID},
	{STYLE_GNOME_HINTS, 		DATABASE_HonorGnomeHints_ID, 	DATABASE_NoGnomeHints_ID},
	{STYLE_KDE_HINTS, 			DATABASE_HonorKDEHints_ID, 		DATABASE_NoKDEHints_ID},
	{STYLE_EXTWM_HINTS, 		DATABASE_HonorExtWMHints_ID, 	DATABASE_NoExtWMHints_ID},
	{STYLE_XRESOURCES_HINTS, 	DATABASE_HonorXResources_ID, 	DATABASE_NoXResources_ID},
	{STYLE_FOCUS_ON_MAP, 		DATABASE_FocusOnMap_ID, 		DATABASE_NoFocusOnMap_ID},
	{STYLE_LONG_LIVING,			DATABASE_LongLiving_ID,			DATABASE_ShortLiving_ID},
	{STYLE_IGNORE_CONFIG, 	    DATABASE_IgnoreConfig_ID, 	    DATABASE_HonorConfig_ID},
	{STYLE_IGNORE_RESTACK, 	    DATABASE_IgnoreRestack_ID, 	    DATABASE_HonorRestack_ID},
	{STYLE_CURRENT_VIEWPORT,    DATABASE_UseCurrentViewport_ID, 	DATABASE_UseAnyViewport_ID},
	{STYLE_PAGER,    DATABASE_PagerHit_ID, 	DATABASE_PagerSkip_ID},
	{STYLE_ICON, 				0 /* special case */ , 			DATABASE_NoIcon_ID},
	{STYLE_STARTUP_DESK, 		0 /*special case */ , 			DATABASE_StartsAnywhere_ID},
	{STYLE_FRAME, 				0 /*special case */ , 			DATABASE_NoFrame_ID},
	{0, 0, 0}
};

flag_options_xref StyleDataFlags[] = {
	{STYLE_ICON, 				0 /* special case */ , 			DATABASE_NoIcon_ID},
	{STYLE_STARTUP_DESK, 		0 /*special case */ , 			DATABASE_StartsAnywhere_ID},
	{STYLE_FRAME, 				0 /*special case */ , 			DATABASE_NoFrame_ID},
	{STYLE_VIEWPORTY,			0 /*special case */ , 			DATABASE_UseAnyViewport_ID},
	{STYLE_VIEWPORTX,			0 /*special case */ , 			DATABASE_UseAnyViewport_ID},
	{0, 0, 0}
};

TermDef       DatabaseTerms[] = {
	{TF_NAMED | TF_NO_MYNAME_PREPENDING, "Style", 5, TT_QUOTED_TEXT, DATABASE_STYLE_ID, &StyleSyntax}
	,
	{0, NULL, 0, 0, 0}
};

SyntaxDef     DatabaseSyntax = {
	'\n',
	'\0',
	DatabaseTerms,
	0,										   /* use default hash size */
    '\t',
	"",
	"\t\t",
	"Database",
	"ASDatabase",
	"defines how windows should be treated by AfterStep depending on its name",
	NULL,
	0
};




void
style_init (name_list * nl)
{
	register int  i;

	nl->next = NULL;

	nl->name = NULL;
	nl->set_flags = 0;
	nl->set_data_flags = 0;
	nl->flags = 0 ; /*STYLE_DEFAULTS;*/
	init_asgeometry (&(nl->default_geometry));
	nl->icon_file = NULL;
	nl->Desk = 0;
	nl->layer = 0;
	nl->border_width = 0;
	nl->resize_width = 0;
	nl->ViewportX = -1;
	nl->ViewportY = -1;
	nl->gravity = NorthWestGravity;
	nl->window_opacity = 100 ;

	nl->off_buttons = 0;
	nl->on_buttons = 0;

	for (i = 0; i < BACK_STYLES; i++)
		nl->window_styles[i] = NULL;
	nl->frame_name = NULL;
    nl->windowbox_name = NULL ;
}

name_list    *
style_new (name_list ** tail)
{
	name_list    *nl;

	nl = (name_list *) safecalloc (1, sizeof (name_list));

	style_init (nl);

	/* add the style to the end of the list of styles */
	if (tail)
	{
		nl->next = *tail;
		*tail = nl;
	}

	return nl;
}

/* don't you dare style_delete the resulting style !!! */
void
style_copy (name_list * to, name_list * from)
{
	if (from)
	{
		register int  i;

		if (from->icon_file != NULL)
			to->icon_file = from->icon_file;
		if (from->name != NULL)
			to->name = from->name;
		if (get_flags (from->set_data_flags, STYLE_DEFAULT_GEOMETRY))
			to->default_geometry = from->default_geometry;

		if (get_flags (from->set_data_flags, STYLE_STARTUP_DESK))
			to->Desk = from->Desk;
		if (get_flags (from->set_data_flags, STYLE_VIEWPORTX))
		{
			LOCAL_DEBUG_OUT( "from->ViewportX = %d", from->ViewportX );	  
			to->ViewportX = from->ViewportX;
		}
		if (get_flags (from->set_data_flags, STYLE_VIEWPORTY))
			to->ViewportY = from->ViewportY;
		if (get_flags (from->set_data_flags, STYLE_BORDER_WIDTH))
			to->border_width = from->border_width;
		if (get_flags (from->set_data_flags, STYLE_HANDLE_WIDTH))
			to->resize_width = from->resize_width;
		if (get_flags (from->set_data_flags, STYLE_LAYER))
			to->layer = from->layer;
		if (get_flags (from->set_data_flags, STYLE_GRAVITY))
			to->gravity = from->gravity;
		if (get_flags (from->set_data_flags, STYLE_WINDOW_OPACITY))
			to->window_opacity = from->window_opacity;

		to->set_flags |= from->set_flags;
		to->flags |= (from->set_flags & from->flags);
		to->flags &= ~(from->set_flags) | from->flags;

		to->set_data_flags |= from->set_data_flags;

		to->off_buttons |= from->off_buttons;
		to->off_buttons &= ~from->on_buttons;
		for (i = 0; i < BACK_STYLES; i++)
			to->window_styles[i] = from->window_styles[i];
		to->frame_name = from->frame_name;
        to->windowbox_name = from->windowbox_name;
    }
}

/* you must style_delete the resulting style !!! */
name_list    *
style_dup (name_list * from)
{
	name_list    *to = style_new (NULL);

	if (from)
	{
		register int  i;

		/* that will copy most of the stuff */
		style_copy (to, from);
		/* for strings we have to allocate another memory block */
		if (from->icon_file != NULL)
			to->icon_file = mystrdup (from->icon_file);
		if (from->name != NULL)
			to->name = mystrdup (from->name);
		for (i = 0; i < BACK_STYLES; i++)
			to->window_styles[i] = mystrdup (from->window_styles[i]);
		if (from->frame_name != NULL)
			to->frame_name = mystrdup (from->frame_name);
        if (from->windowbox_name != NULL)
            to->windowbox_name = mystrdup (from->windowbox_name);
    }
	return to;
}

void
style_delete (name_list * style, name_list ** phead)
{
	name_list    *s;
	register int  i;

	if (style == NULL)
		return;
	if (phead)
	{
		/* find ourself in the list */
		for (s = *phead; s != NULL; s = s->next)
			if (s->next == style)
				break;

		/* remove ourself from the list */
		if (s != NULL)
			s->next = style->next;
		else if (*phead == style)
			*phead = style->next;
	}
	/* delete members */
	if (style->name != NULL)
		free (style->name);
	if (style->icon_file != NULL)
		free (style->icon_file);

	for (i = 0; i < BACK_STYLES; i++)
		if (style->window_styles[i] != NULL)
			free (style->window_styles[i]);

	if (style->frame_name != NULL)
		free (style->frame_name);
    if (style->windowbox_name != NULL)
        free (style->windowbox_name);

	/* free our own mem */
	free (style);
}

void
delete_name_list (name_list ** head)
{
	while (*head)
		style_delete (*head, head);
}

static unsigned long window_style_cross[][2] =
{ 	{DATABASE_FocusStyle_ID, BACK_FOCUSED},
	{DATABASE_UnfocusStyle_ID, BACK_UNFOCUSED},
	{DATABASE_StickyStyle_ID, BACK_STICKY},
	{0, BACK_DEFAULT}
};


void
ParseSingleStyle (FreeStorageElem * storage, name_list * style)
{
	ConfigItem    item;
	register int  i;

	item.memory = NULL;

	if (style == NULL)
		return;
	for (; storage; storage = storage->next)
	{
		Bool is_flag = False ; 
		if (storage->term == NULL)
			continue;
		
		if (ReadFlagItem (&(style->set_flags), &(style->flags), storage, StyleFlags))
			is_flag = True;
		if (ReadFlagItem (&(style->set_data_flags), NULL, storage, StyleDataFlags))
			is_flag = True;
		
		if( is_flag )
			continue;
		if (!ReadConfigItem (&item, storage))
			continue;

		switch (storage->term->id)
		{
		 case DATABASE_DefaultGeometry_ID:
			 LOCAL_DEBUG_OUT( "flags = %x", item.data.geometry.flags );
			 set_flags (style->set_data_flags, STYLE_DEFAULT_GEOMETRY);
			 style->default_geometry = item.data.geometry;
			 break;
		 case DATABASE_Icon_ID:
			 set_string_value (&(style->icon_file), item.data.string, &(style->set_data_flags), STYLE_ICON);
			 if (strlen (style->icon_file) <= 0)
				 clear_flags (style->set_data_flags, STYLE_ICON);
             /* regardles of weather the file was valid - indicate that app should have an icon !!! */
			 set_flags (style->set_flags, STYLE_ICON);
			 set_flags (style->flags, STYLE_ICON);
			 break;
		 case DATABASE_FocusStyle_ID:
		 case DATABASE_UnfocusStyle_ID:
		 case DATABASE_StickyStyle_ID:
			 for (i = 0; i < BACK_STYLES; i++)
				 if (window_style_cross[i][0] == storage->term->id)
				 {
					 char        **s = &(style->window_styles[window_style_cross[i][1]]);

					 if (*s)
						 free (*s);
					 *s = item.data.string;
				 }
			 break;
		 case DATABASE_NoButton_ID:
             style->off_buttons |= (C_TButton0<< item.data.integer );
			 break;
		 case DATABASE_Button_ID:
             style->on_buttons |= (C_TButton0 << item.data.integer );
			 break;
		 case DATABASE_Layer_ID:
			 style->layer = item.data.integer;
			 /* we want to limit user specifyed layers to AS_LayerDesktop < layer < AS_LayerMenu
			  * which is reasonable, since you cannot be lower then Desktop and higher then Menu
			  */
			 if (style->layer <= AS_LayerDesktop)
				 style->layer = AS_LayerDesktop + 1;
			 else if (style->layer >= AS_LayerMenu)
				 style->layer = AS_LayerMenu - 1;

			 set_flags (style->set_data_flags, STYLE_LAYER);
			 break;
		 case DATABASE_StaysOnTop_ID:
			 style->layer = 1;
			 set_flags (style->set_data_flags, STYLE_LAYER);
			 break;
		 case DATABASE_StaysPut_ID:
			 style->layer = 0;
			 set_flags (style->set_data_flags, STYLE_LAYER);
			 break;
		 case DATABASE_StaysOnBack_ID:
			 style->layer = -1;
			 set_flags (style->set_data_flags, STYLE_LAYER);
			 break;
		 case DATABASE_BorderWidth_ID:
			 set_flags (style->set_data_flags, STYLE_BORDER_WIDTH);
			 style->border_width = item.data.integer;
			 break;
		 case DATABASE_HandleWidth_ID:
			 set_flags (style->set_data_flags, STYLE_HANDLE_WIDTH);
			 style->resize_width = item.data.integer;
			 break;
		 case DATABASE_StartsOnDesk_ID:
			 set_flags (style->set_data_flags, STYLE_STARTUP_DESK);
			 style->Desk = item.data.integer;
			 break;
		 case DATABASE_ViewportX_ID:
			 set_flags (style->set_data_flags, STYLE_VIEWPORTX);
			 style->ViewportX = item.data.integer;
			 LOCAL_DEBUG_OUT( "style->ViewportX = %d", style->ViewportX );
			 break;
		 case DATABASE_ViewportY_ID:
			 set_flags (style->set_data_flags, STYLE_VIEWPORTY);
			 style->ViewportY = item.data.integer;
			 LOCAL_DEBUG_OUT( "style->ViewportY = %d", style->ViewportY );
			 break;
		 case DATABASE_WindowOpacity_ID:
			 set_flags (style->set_data_flags, STYLE_WINDOW_OPACITY);
			 style->window_opacity = item.data.integer;
			 LOCAL_DEBUG_OUT( "style->window_opacity = %d", style->window_opacity );
			 break;
		 case DATABASE_Frame_ID:
			 set_string_value (&(style->frame_name), item.data.string, &(style->set_data_flags), STYLE_FRAME);
             if (strlen (style->frame_name) <= 0)
				 clear_flags (style->set_data_flags, STYLE_FRAME);
			 break;
         case DATABASE_Windowbox_ID:
             set_string_value (&(style->windowbox_name), item.data.string, &(style->set_data_flags), STYLE_FRAME);
             if (strlen (style->windowbox_name) <= 0)
                 clear_flags (style->set_data_flags, STYLE_WINDOWBOX);
			 break;
         case DATABASE_OverrideGravity_ID:
			 if (storage->sub)
				 if (storage->sub->term)
				 {
					 style->gravity = storage->sub->term->id - GRAVITY_ID_START;
					 set_flags (style->set_data_flags, STYLE_GRAVITY);
				 }
			 break;
		 default:
			 item.ok_to_free = 1;
		}
	}
	ReadConfigItem (&item, NULL);
}

name_list    *
ParseDatabaseOptions (const char *filename, char *myname)
{
	ConfigData    cd ;
	ConfigDef    *ConfigReader;
	name_list    *config = NULL, **tail = &config, *curr_style;
	FreeStorageElem *Storage = NULL, *pCurr, *more_stuff;
	ConfigItem    item;

	cd.filename = filename ;
	ConfigReader = InitConfigReader (myname, &DatabaseSyntax, CDT_Filename, cd, NULL);
	if (!ConfigReader)
		return config;

	item.memory = NULL;
	PrintConfigReader (ConfigReader);
	ParseConfig (ConfigReader, &Storage);

	/* getting rid of all the crap first */
	StorageCleanUp (&Storage, &more_stuff, CF_DISABLED_OPTION);

	for (pCurr = Storage; pCurr; pCurr = pCurr->next)
	{
		if (pCurr->term == NULL)
			continue;
		if (pCurr->term->id != DATABASE_STYLE_ID)
			continue;
		if (!ReadConfigItem (&item, pCurr))
			continue;

		if (!(curr_style = style_new (tail)))
			continue;
		curr_style->name = item.data.string;
		ParseSingleStyle (pCurr->sub, curr_style);
		tail = &(curr_style->next);
	}
	ReadConfigItem (&item, NULL);

	DestroyConfig (ConfigReader);
	DestroyFreeStorage (&Storage);
	return config;
}

name_list    *
string2DatabaseStyle (char *style_txt)
{
	ConfigData    cd ;
	ConfigDef    *ConfigReader;
	name_list    *style = NULL;
	FreeStorageElem *Storage = NULL;

	cd.data = style_txt ;
	ConfigReader = InitConfigReader ("afterstep", &StyleSyntax, CDT_Data, cd, NULL);
	if (!ConfigReader)
		return NULL;
	
	PrintConfigReader (ConfigReader);
	
	ParseConfig (ConfigReader, &Storage);

	if ((style = style_new (NULL)) != NULL )
		ParseSingleStyle (Storage, style);

	DestroyFreeStorage (&Storage);
	DestroyConfig (ConfigReader);
	return style;
}


Bool ReloadASDatabase()
{
	char *configfile  = make_session_file(Session, DATABASE_FILE, False ); 
	destroy_asdatabase();
    if( configfile != NULL )
    {
	    struct name_list *list = NULL ;
	
		/* memory management for parsing buffer */
	    list = ParseDatabaseOptions (configfile, "afterstep");
    	if( list )
	    {
    	    Database = build_asdb( list );
        	if( is_output_level_under_threshold( OUTPUT_LEVEL_DATABASE ) )
	            print_asdb( NULL, NULL, Database );
    	    while (list != NULL)
        	    delete_name_list (&(list));
	        show_progress("DATABASE configuration loaded from \"%s\" ...", configfile);
	    }else
    	    show_progress( "no database records loaded." );
	    /* XResources : */
    	load_user_database();
        free( configfile );
    }else
    {
        show_warning("DATABASE configuration file cannot be found!");
        return False ;
    }
	return True ;
}


/*****************************************************************************/
/*                      Writer stuff                                         */
/*****************************************************************************/
#if 0										   /* don't work in as-stable */
FreeStorageElem **
WriteSingleStyle (name_list * style, FreeStorageElem ** tail)
{
	FreeStorageElem **d_tail;
	register int  i;

	/* building free storage here */
	d_tail = tail;
	/* our name : */
	tail = QuotedString2FreeStorage (&DatabaseSyntax, tail, NULL, style->name, DATABASE_STYLE_ID);

	/* now connect everything else as a subconfig : */
	if (d_tail != tail)						   /* checking if storage for style was actually created */
	{
		d_tail = &((*d_tail)->sub);

		if (style->set_flags != 0)			   /* that should take care of all the flags : */
			d_tail = Flags2FreeStorage (&StyleSyntax, d_tail, StyleFlags, style->set_flags, style->flags);

		/* Icon */
		if (style->icon_file)
			if (strlen (style->icon_file) > 0)
				d_tail = String2FreeStorage (&StyleSyntax, d_tail, style->icon_file, DATABASE_Icon_ID);

		for (i = 0; i < BACK_STYLES; i++)
		{
			if (window_style_cross[i][0] >= DATABASE_ID_START)
			{
				char         *s = style->window_styles[window_style_cross[i][1]];

				if (s)
				{
					if (strlen (s));
					d_tail = String2FreeStorage (&StyleSyntax, d_tail, s, window_style_cross[i][0]);
				}
			}
		}

		for (i = 0; i < MAX_BUTTONS; i++)
            if (style->off_buttons & (C_TButton0 << i))
                d_tail = Integer2FreeStorage (&StyleSyntax, d_tail, NULL, i, DATABASE_NoButton_ID);
            else if (style->on_buttons & (C_TButton0 << i))
                d_tail = Integer2FreeStorage (&StyleSyntax, d_tail, NULL, i, DATABASE_Button_ID);

		if (get_flags (style->set_flags, STYLE_DEFAULT_GEOMETRY))
			d_tail =
				Geometry2FreeStorage (&StyleSyntax, d_tail, &(style->default_geometry), DATABASE_DefaultGeometry_ID);

		if (get_flags (style->set_flags, STYLE_LAYER))
		{
			if (style->layer == AS_LayerBack)
				d_tail = Flag2FreeStorage (&StyleSyntax, d_tail, DATABASE_StaysOnBack_ID);
			else if (style->layer == AS_LayerNormal)
				d_tail = Flag2FreeStorage (&StyleSyntax, d_tail, DATABASE_StaysPut_ID);
			else if (style->layer == AS_LayerTop)
				d_tail = Flag2FreeStorage (&StyleSyntax, d_tail, DATABASE_StaysOnTop_ID);
			else
				d_tail = Integer2FreeStorage (&StyleSyntax, d_tail, NULL, style->layer, DATABASE_Layer_ID);
		}

		if (get_flags (style->set_flags, STYLE_BORDER_WIDTH))
			d_tail = Integer2FreeStorage (&StyleSyntax, d_tail, NULL, style->border_width, DATABASE_BorderWidth_ID);
		if (get_flags (style->set_flags, STYLE_HANDLE_WIDTH))
			d_tail = Integer2FreeStorage (&StyleSyntax, d_tail, NULL, style->resize_width, DATABASE_HandleWidth_ID);

		if (get_flags (style->flags, STYLE_STARTUP_DESK) && get_flags (style->set_flags, STYLE_STARTUP_DESK))
			d_tail = Integer2FreeStorage (&StyleSyntax, d_tail, NULL, style->Desk, DATABASE_StartsOnDesk_ID);
		/* ViewportXY -1 means that we use current viewport */
		if (get_flags (style->set_flags, STYLE_VIEWPORTX))
			d_tail =
				Integer2FreeStorage (&StyleSyntax, d_tail, NULL,
									 (get_flags (style->flags, STYLE_VIEWPORTX)) ? style->ViewportX : -1,
									 DATABASE_ViewportX_ID);
		if (get_flags (style->set_flags, STYLE_VIEWPORTY))
			d_tail =
				Integer2FreeStorage (&StyleSyntax, d_tail, NULL,
									 (get_flags (style->flags, STYLE_VIEWPORTY)) ? style->ViewportY : -1,
									 DATABASE_ViewportY_ID);

		if (get_flags (style->set_flags, STYLE_FRAME) && style->frame_name)
			if (get_flags (style->flags, STYLE_FRAME) && strlen (style->frame_name) > 0)
				d_tail = String2FreeStorage (&StyleSyntax, d_tail, style->frame_name, DATABASE_Frame_ID);

        if (get_flags (style->set_flags, STYLE_WINDOWBOX) && style->windowbox_name)
            if (get_flags (style->flags, STYLE_WINDOWBOX) && strlen (style->windowbox_name) > 0)
                d_tail = String2FreeStorage (&StyleSyntax, d_tail, style->windowbox_name, DATABASE_Windowbox_ID);

        if (get_flags (style->set_flags, STYLE_GRAVITY))
		{
			FreeStorageElem **grav_tail = d_tail;

			d_tail = Flag2FreeStorage (&StyleSyntax, d_tail, DATABASE_OverrideGravity_ID);
			if (*grav_tail)
				Flag2FreeStorage (&StyleSyntax, &((*grav_tail)->sub), GRAVITY_ID_START + style->gravity);
		}
	}

	return tail;
}

/* returns:
 *            0 on success
 *              1 if data is empty
 *              2 if ConfigWriter cannot be initialized
 *
 */
int
WriteDatabaseOptions (const char *filename, char *myname, struct name_list *config, unsigned long flags)
{
	ConfigDef    *DatabaseConfigWriter = NULL;
	FreeStorageElem *Storage = NULL, **tail = &Storage;

	if (config == NULL)
		return 1;
	if ((DatabaseConfigWriter = InitConfigWriter (myname, &DatabaseSyntax, CDT_Filename, (void *)filename)) == NULL)
		return 2;

	/*CopyFreeStorage (&Storage, config->more_stuff); */

	for (; config; config = config->next)
		tail = WriteSingleStyle (config, tail);

	/* writing config into the file */
	WriteConfig (DatabaseConfigWriter, Storage, CDT_Filename, (void **)&filename, flags);
	DestroyFreeStorage (&Storage);
	DestroyConfig (DatabaseConfigWriter);

	if (Storage)
	{
		fprintf (stderr,
				 "\n%s:Database Config Writing warning: Not all Free Storage discarded! Trying again...", myname);
		DestroyFreeStorage (&Storage);
		fprintf (stderr, (Storage != NULL) ? " failed." : " success.");
	}
	return 0;
}

#endif /* disabled writer stuff in as-stable */
