/*
 * Copyright (c) 2000,2003 Sasha Vasko <sasha at aftercode.net>
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
#include "../libAfterStep/parser.h"
#include "../libAfterStep/hints.h"
#include "../libAfterStep/asfeel.h"

#include "afterconf.h"


flag_options_xref WindowBoxFlagsXref[] = {
    { ASA_Virtual			, WINDOWBOX_Virtual_ID, 0},
    { ASA_VerticalPriority	, WINDOWBOX_VerticalPriority_ID, 0},
    { ASA_ReverseOrderH		, WINDOWBOX_ReverseOrderHorizontal_ID, 0},
    { ASA_ReverseOrderV		, WINDOWBOX_ReverseOrderVertical_ID, 0},
    { ASA_ReverseOrder		, WINDOWBOX_ReverseOrder_ID, 0},
    {0, 0, 0}
};


/*****************************************************************************
 *
 * This routine is responsible for reading and parsing the base.<bpp> config
 * file
 *
 ****************************************************************************/
void input_context_destroy(ASHashableValue value, void *data);

flag_options_xref FeelFlagsXref[] = {
     /* focus : */
    {ClickToFocus          , FEEL_ClickToFocus_ID          , 0},
    {SloppyFocus           , FEEL_SloppyFocus_ID           , 0},
/*    {AutoFocusNewWindows   , FEEL_AutoFocus_ID             , 0}, */
     /* decorations : */
    {DecorateTransients    , FEEL_DecorateTransients_ID    , 0},
     /* placement : */
    {DontMoveOff           , FEEL_DontMoveOff_ID           , 0},
    {NoPPosition           , FEEL_NoPPosition_ID           , 0},
//    {SMART_PLACEMENT        , FEEL_SmartPlacement_ID        , 0},
//    {RandomPlacement       , FEEL_RandomPlacement_ID       , 0},
    {StubbornPlacement     , FEEL_StubbornPlacement_ID     , 0},
//    {MenusHigh             , FEEL_MenusHigh_ID             , 0},
    {CenterOnCirculate     , FEEL_CenterOnCirculate_ID     , 0},
     /* icons : */
    {SuppressIcons         , FEEL_SuppressIcons_ID         , 0},
    {IconTitle             , FEEL_IconTitle_ID             , 0},
    {KeepIconWindows       , FEEL_KeepIconWindows_ID       , 0},
    {StickyIcons           , FEEL_StickyIcons_ID           , 0},
    {StubbornIcons         , FEEL_StubbornIcons_ID         , 0},
    {StubbornIconPlacement , FEEL_StubbornIconPlacement_ID , 0},
    {CirculateSkipIcons    , FEEL_CirculateSkipIcons_ID    , 0},
     /* backing store/save unders : */
    {BackingStore          , FEEL_BackingStore_ID          , 0},
    {AppsBackingStore      , FEEL_AppsBackingStore_ID      , 0},
    {SaveUnders            , FEEL_SaveUnders_ID            , 0},
     /* pageing */
    {DoHandlePageing       , FEEL_PagingDefault_ID         , 0},
    {AutoTabThroughDesks   , FEEL_AutoTabThroughDesks_ID   , 0},
    {WarpPointer   	   , FEEL_WarpPointer_ID   	   , 0},

	{0, 0, 0}
};


/**************************************************************************************
 * WindowBox code :
 **************************************************************************************/

ASWindowBox *
ProcessWindowBoxOptions (FreeStorageElem * options)
{
	ConfigItem    item;
    ASWindowBox  *aswbox = NULL ;

	item.memory = NULL;

	for (; options; options = options->next)
	{
		if (options->term == NULL)
			continue;
        LOCAL_DEBUG_OUT( "WindowBox(%p)->options(%p)->keyword(\"%s\")", aswbox, options, options->term->keyword );
		if (options->term->id < WINDOWBOX_ID_START || options->term->id > WINDOWBOX_ID_END)
			continue;

		if (options->term->type == TT_FLAG)
		{
            if( aswbox != NULL  )
            {
                if( options->term->id == WINDOWBOX_DONE_ID )
                    break;                     /* for loop */
		        if (ReadFlagItem (&(aswbox->set_flags), &(aswbox->flags), options, WindowBoxFlagsXref))
        		    continue;
            }
            continue;
        }

		if (!ReadConfigItem (&item, options))
        {
            LOCAL_DEBUG_OUT( "ReadConfigItem has failed%s","");
			continue;
        }
		if( options->term->id == WINDOWBOX_START_ID )
		{
			if( aswbox != NULL )
			{
				show_error( "Unterminated WindowBox found - will be ignored" );
                destroy_aswindow_box( aswbox, False );
                aswbox = NULL ;
			}
			aswbox = create_aswindow_box( item.data.string );
			item.ok_to_free = 1;
		}else if( aswbox != NULL )
		{
			switch( options->term->id )
			{
				case WINDOWBOX_Area_ID   	  	  :
					aswbox->area = item.data.geometry;
		            set_flags( aswbox->set_flags, ASA_AreaSet);
			    break ;
                case WINDOWBOX_MinWidth_ID        :
                    aswbox->min_width = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_MinWidthSet);
                    break ;
                case WINDOWBOX_MinHeight_ID      :
                    aswbox->min_height = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_MinHeightSet);
                    break ;
                case WINDOWBOX_MaxWidth_ID        :
                    aswbox->max_width = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_MaxWidthSet);
                    break ;
                case WINDOWBOX_MaxHeight_ID      :
                    aswbox->max_height = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_MaxHeightSet);
                    break ;
                case WINDOWBOX_FirstTry_ID        :
					if( options->sub != NULL && options->sub->term->id >= FEEL_PLACEMENT_START_ID ) 
					{	
                    	aswbox->main_strategy = options->sub->term->id - FEEL_PLACEMENT_START_ID ;
                    	set_flags( aswbox->set_flags, ASA_MainStrategySet);
					}
                    break ;
                case WINDOWBOX_ThenTry_ID         :
					if( options->sub != NULL && options->sub->term->id >= FEEL_PLACEMENT_START_ID ) 
					{	
                    	aswbox->backup_strategy = options->sub->term->id - FEEL_PLACEMENT_START_ID ;
                    	set_flags( aswbox->set_flags, ASA_BackupStrategySet);
					}
                    break ;
                case WINDOWBOX_Desk_ID            :
                    aswbox->desk = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_DesktopSet);
                    break ;
                case WINDOWBOX_MinLayer_ID        :
                    aswbox->min_layer = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_MinLayerSet);
                    break ;
                case WINDOWBOX_MaxLayer_ID        :
                    aswbox->max_layer = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_MaxLayerSet);
                    break ;
                case WINDOWBOX_XSpacing_ID        :
                    aswbox->x_spacing = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_XSpacingSet);
                    break ;
                case WINDOWBOX_YSpacing_ID        :
                    aswbox->y_spacing = item.data.integer ;
                    set_flags( aswbox->set_flags, ASA_YSpacingSet);
                    break ;
                default:
					item.ok_to_free = 1;
                    show_warning( "Unexpected WindowBox definition keyword \"%s\" . Ignoring.", options->term->keyword );
			}
		}
    }
    LOCAL_DEBUG_OUT("done parsing WindowBox: [%s]", aswbox->name);
    ReadConfigItem (&item, NULL);
	return aswbox;
}

void
windowbox_parse (char *tline, FILE * fd, char **unused1, int *unused2)
{
    FilePtrAndData fpd ;
    ConfigDef    *ConfigReader ;
    FreeStorageElem *Storage = NULL, *more_stuff = NULL;
    ASWindowBox **aswbox_list = &(ASDefaultScr->Feel.window_boxes);
	unsigned int *count = &(ASDefaultScr->Feel.window_boxes_num);
    ASWindowBox *new_box ;
	ConfigData cd ;

    fpd.fp = fd ;
    fpd.data = safemalloc( 12+1+strlen(tline)+1+1 ) ;
    sprintf( fpd.data, "WindowBox %s\n", tline );
    LOCAL_DEBUG_OUT( "fd(%p)->tline(\"%s\")->fpd.data(\"%s\")", fd, tline, fpd.data );
	cd.fileptranddata = &fpd ;
    ConfigReader = InitConfigReader ((char*)get_application_name(), &WindowBoxSyntax, CDT_FilePtrAndData, cd, NULL);
    free( fpd.data );

    if (!ConfigReader)
        return ;

	PrintConfigReader (ConfigReader);
	ParseConfig (ConfigReader, &Storage);

	/* getting rid of all the crap first */
    StorageCleanUp (&Storage, &more_stuff, CF_DISABLED_OPTION);
    DestroyFreeStorage (&more_stuff);

    if( (new_box = ProcessWindowBoxOptions (Storage)) != NULL )
    {
        int i = *count ;
        ++(*count) ;
        *aswbox_list = realloc( *aswbox_list, sizeof(ASWindowBox)*(i+1));
        (*aswbox_list)[i] = *new_box ;
        free( new_box );
    }

	DestroyConfig (ConfigReader);
	DestroyFreeStorage (&Storage);
}


/**************************************************************************************
 * FeelConfig management code.
 *************************************************************************************/
FeelConfig *
CreateFeelConfig ()
{
  FeelConfig *config = (FeelConfig *) safecalloc (1, sizeof (FeelConfig));
  config->more_stuff = NULL;

  config->feel = create_asfeel();

  return config;
}

void
DestroyFeelConfig (FeelConfig * config)
{
    register int i ;

    if( config->feel )
        destroy_asfeel( config->feel, False );
    if( config->menu_locations )
    {
        for( i = 0 ; i < config->menu_locs_num ; i++ )
            if( config->menu_locations[i] )
                free( config->menu_locations[i] );
        free( config->menu_locations );
    }

    DestroyFreeStorage (&(config->more_stuff));
    free (config);
}

/**********************************************************************
 *  Feel Parsing code :
 **********************************************************************/
unsigned long
BindingSpecialFunc (ConfigDef * config)
{
    /* since we have have subconfig of Functions that has \n as line terminator
     * we are going to get entire line again at config->cursor
     * so lets skip 3 tokens of <key/mouse button> <context> <modifyers> ,
     * since those are not parts of following function */
	return TrailingFuncSpecial( config, 3 );
}

void
ParseKeyBinding( ConfigItem *item, FreeStorageElem *func_elem, struct FuncKey **keyboard )
{
    KeySym        keysym;
	KeyCode       keycode;
    int           context, mods ;
	int 		  min = 0, max = 0 ;

    if( item == NULL )
		return ;
    item->ok_to_free = 1;

    if( item == NULL || func_elem == NULL || keyboard == NULL )
        return ;
    if( func_elem->term == NULL || func_elem->term->type != TT_FUNCTION )
        return ;

	/*
	 * Don't let a 0 keycode go through, since that means AnyKey to the
	 * XGrabKey call in GrabKeys().
	 */
	if( (keysym = XStringToKeysym (item->data.binding.sym)) == NoSymbol ||
		(keycode = XKeysymToKeycode (dpy, keysym)) == 0)
		return ;

	XDisplayKeycodes (dpy, &min, &max);
	for (keycode = min; keycode <= max; keycode++)
		if (XKeycodeToKeysym (dpy, keycode, 0) == keysym)
			break;

	if (keycode > max)
		return ;

    context = item->data.binding.context ;
    mods = item->data.binding.mods ;

    if( !ReadConfigItem( item, func_elem ) )
        return ;

    if( item->data.function )
    { /* gotta add it to the keyboard hash */
		FuncKey *tmp ;

		tmp = (FuncKey *) safecalloc (1, sizeof (FuncKey));

        tmp->next = *keyboard;
		*keyboard = tmp ;

		tmp->name = mystrdup(item->data.binding.sym);
		tmp->keycode = keycode;
		tmp->cont = context;
		tmp->mods = mods;
		tmp->fdata = item->data.function;
		item->data.function = NULL ;
    }
    item->ok_to_free = (item->data.function != NULL);
}


FreeStorageElem **
Keyboard2FreeStorage( SyntaxDef *syntax, FreeStorageElem **tail, struct FuncKey *ic, int id )
{
    FreeStorageElem **d_tail;

    if ( syntax == NULL || tail == NULL )
        return tail ;

    while( ic != NULL )
    {
        d_tail = tail ;
        tail = Binding2FreeStorage (syntax, tail, ic->name, ic->cont, ic->mods, id);
        if( tail != d_tail && *d_tail )   /* saving our function as a sub-elem */
            Func2FreeStorage( pFuncSyntax, &((*d_tail)->sub), ic->fdata );
		ic = ic->next ;
    }
    return tail;
}

FreeStorageElem **
Mouse2FreeStorage( SyntaxDef *syntax, FreeStorageElem **tail, struct MouseButton *ic, int id )
{
    FreeStorageElem **d_tail;

    if ( syntax == NULL || tail == NULL )
        return tail ;

    while( ic != NULL )
    {
		static char buffer[32] ;
        d_tail = tail ;
		sprintf( &(buffer[0]), "%d", ic->Button );
        tail = Binding2FreeStorage (syntax, tail, buffer, ic->Context, ic->Modifier, id);
        if( tail != d_tail && *d_tail )   /* saving our function as a sub-elem */
            Func2FreeStorage( pFuncSyntax, &((*d_tail)->sub), ic->fdata );
		ic = ic->NextButton ;
    }
    return tail;
}

/*********************************************************************************/
/**************************** Actual parsing code ********************************/
/*********************************************************************************/
FeelConfig *
ParseFeelOptions (const char *filename, char *myname)
{
    ConfigData cd ; 
	ConfigDef *ConfigReader;
    FeelConfig *config = CreateFeelConfig ();
	FreeStorageElem *Storage = NULL, *pCurr;
    ConfigItem item;

	cd.filename = filename ; 
	ConfigReader = InitConfigReader (myname, &FeelSyntax, CDT_Filename, cd, BindingSpecialFunc);
	if (!ConfigReader)
  		return config;

    item.memory = NULL;
	PrintConfigReader (ConfigReader);
	ParseConfig (ConfigReader, &Storage);

	/* getting rid of all the crap first */
    StorageCleanUp (&Storage, &(config->more_stuff), CF_DISABLED_OPTION);

	for (pCurr = Storage; pCurr; pCurr = pCurr->next)
  	{
  	    if (pCurr->term == NULL)
			continue;
        if (ReadFlagItem (NULL, &(config->feel->flags), pCurr, FeelFlagsXref))
            continue;
        if (!ReadConfigItem (&item, pCurr))
			continue;
        switch (pCurr->term->id)
		{
            case FEEL_ClickTime_ID          :
                config->feel->ClickTime = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_ClickTime);
                break ;
            case FEEL_OpaqueMove_ID         :
                config->feel->OpaqueMove = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_OpaqueMove);
                break ;
            case FEEL_OpaqueResize_ID       :
                config->feel->OpaqueResize = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_OpaqueResize);
                break ;
            case FEEL_AutoRaise_ID          :
                config->feel->AutoRaiseDelay = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_AutoRaise);
                break ;
            case FEEL_AutoReverse_ID        :
                config->feel->AutoReverse = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_AutoReverse);
                break ;
            case FEEL_ShadeAnimationSteps_ID :
                config->feel->ShadeAnimationSteps = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_ShadeAnimationSteps);
                break ;

            case FEEL_XorValue_ID           :
                config->feel->XorValue = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_XorValue);
                break ;
            case FEEL_Xzap_ID               :
                config->feel->Xzap = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_Xzap);
                break ;
            case FEEL_Yzap_ID               :
                config->feel->Yzap = item.data.integer;
                set_flags (config->feel->set_val_flags, FEEL_Yzap);
                break ;

            case FEEL_Cursor_ID             :                   /* TT_INTEGER */
				/* TODO: backport from as-devel : */
                /*if ( item.index  > 0 && item.index < MAX_CURSORS)
                    config->feel->standard_cursors[item.index] = item.data.integer ;
				 */
                break ;
            case FEEL_CustomCursor_ID       :                   /* TT_BUTTON  */
				/* TODO: backport from as-devel : */
				/*
					if ( item.index  > 0 && item.index < MAX_CURSORS)
                	{
                    	if( config->feel->custom_cursors[item.index] )
                        	destroy_ascursor( &(config->feel->custom_cursors[item.index]));
                    	config->feel->custom_cursors[item.index] = item.data.cursor ;
                	}
				 */
                break ;

            case FEEL_ClickToRaise_ID       :                   /* TT_BITLIST */
                config->feel->RaiseButtons = item.data.integer ;
                set_flags (config->feel->set_val_flags, FEEL_ClickToRaise);
                set_flags (config->feel->set_flags, ClickToRaise);
				set_flags (config->feel->flags, ClickToRaise);
                break ;

            case FEEL_EdgeScroll_ID         :                   /* TT_INTARRAY*/
				item.ok_to_free = 1;
                if( item.data.int_array.size > 0 )
                {
                    config->feel->EdgeScrollX = item.data.int_array.array[0];
                    if( item.data.int_array.size > 1 )
                        config->feel->EdgeScrollY = item.data.int_array.array[1];
                    set_flags (config->feel->set_val_flags, FEEL_EdgeScroll );
                }
                break ;
            case FEEL_EdgeResistance_ID     :                   /* TT_INTARRAY*/
				item.ok_to_free = 1;
                if( item.data.int_array.size > 0 )
                {
                    config->feel->EdgeResistanceScroll = item.data.int_array.array[0];
                    if( item.data.int_array.size > 1 )
                        config->feel->EdgeResistanceMove = item.data.int_array.array[1];
                    set_flags (config->feel->set_val_flags, FEEL_EdgeResistance );
                }
                break ;

            case FEEL_Popup_ID              :
				/* TODO: backport from as-devel : */
                /* FreeStorage2MenuData( pCurr, &item, config->feel->Popups ); */
                break ;
            case FEEL_Function_ID           :
                FreeStorage2ComplexFunction( pCurr, &item, config->feel->ComplexFunctions );
                break ;
            case FEEL_Mouse_ID              :
                if( item.data.binding.sym )
                    if( isdigit( (int)item.data.binding.sym[0] ) && pCurr->sub )
                    {
                        int button_num = item.data.binding.sym[0] - '0' ;
                        if( button_num >= 0 && button_num <= MAX_MOUSE_BUTTONS &&
                            pCurr->sub->term->type == TT_FUNCTION )
                        {
                            ConfigItem func_item ;
                            func_item.memory = NULL ;
                            if( ReadConfigItem( &func_item, pCurr->sub ) )
							{
								MouseButton *tmp = safecalloc( 1, sizeof(MouseButton) );
								tmp->Button = button_num ;
								tmp->Modifier = item.data.binding.mods ;
								tmp->Context = item.data.binding.context ;
								tmp->fdata = func_item.data.function ;
								func_item.data.function = NULL ;
								tmp->NextButton = config->feel->MouseButtonRoot ;
								config->feel->MouseButtonRoot = tmp ;
							}
                            if( func_item.data.function )
                            {
                                func_item.ok_to_free = 1;
                                ReadConfigItem( &func_item, NULL );
                            }
                        }
                    }
                item.ok_to_free = 1;
                break ;
            case FEEL_Key_ID                :
                ParseKeyBinding( &item, pCurr->sub, &(config->feel->FuncKeyRoot) );
                break ;
          default:
				item.ok_to_free = 1;
		}
    }
	ReadConfigItem (&item, NULL);

	DestroyConfig (ConfigReader);
	DestroyFreeStorage (&Storage);
	return config;
}

/**********************************************************************
 *  Menu merging/parsing code :
 **********************************************************************/

void
load_feel_menu( ASFeel *feel, char *location )
{
    char *fullfilename ;
    if( location && feel )
    {
        fullfilename = put_file_home( location );
#if 0
        if ( CheckDir (fullfilename) == 0 )
            dir2menu_data( fullfilename, &(feel->menus_list));
        else if ( CheckFile (fullfilename) == 0 )
            file2menu_data( fullfilename, &(feel->menus_list));
        else
            show_error("unable to locate menu at location [%s].", fullfilename);
#endif
        free( fullfilename );
    }
}

void
LoadFeelMenus (FeelConfig *config)
{
    if( config )
    {
        if( config->feel )
        {
/* TODO: add config options to be able to specify menu locations : */
            if( config->menu_locations )
            {
                register int i ;

                for( i = 0 ; i < config->menu_locs_num ; i++ )
                    if( config->menu_locations[i] )
                        load_feel_menu( config->feel, config->menu_locations[i] );
            }
/*		    if( config->feel->menus_list )
				print_ashash( config->feel->menus_list, string_print );
*/
			set_flags( config->feel->set_flags, FEEL_ExternalMenus );
  		}
    }
}

/**********************************************************************
 *  Feel Writing code :
 **********************************************************************/
#if 0
/* returns:
 *            0 on success
 *              1 if data is empty
 *              2 if ConfigWriter cannot be initialized
 *
 */
int
WriteFeelOptions (const char *filename, char *myname,
		  FeelConfig * config, unsigned long flags)
{
	ConfigDef *ConfigWriter = NULL;
    FreeStorageElem *Storage = NULL, **tail = &Storage;
    int i ;

	if (config == NULL)
  		return 1;
	if ((ConfigWriter = InitConfigWriter (myname, &FeelSyntax, CDT_Filename,
			  (void *) filename)) == NULL)
	    return 2;

    CopyFreeStorage (&Storage, config->more_stuff);

	/* building free storage here */
    /* flags : */
    tail = Flags2FreeStorage (&FeelSyntax, tail, FeelFlagsXref, config->feel->flags, config->feel->flags);

    /* integer parameters : */
    if (get_flags (config->feel->set_flags, FEEL_ClickTime))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->ClickTime, FEEL_ClickTime_ID);
    if (get_flags (config->feel->set_flags, FEEL_OpaqueMove))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->OpaqueMove, FEEL_OpaqueMove_ID);
    if (get_flags (config->feel->set_flags, FEEL_OpaqueResize))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->OpaqueResize, FEEL_OpaqueResize_ID);
    if (get_flags (config->feel->set_flags, FEEL_AutoRaise))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->AutoRaiseDelay, FEEL_AutoRaise_ID);
    if (get_flags (config->feel->set_flags, FEEL_AutoReverse))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->AutoReverse, FEEL_AutoReverse_ID);
    if (get_flags (config->feel->set_flags, FEEL_DeskAnimationType))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->DeskAnimationType, FEEL_DeskAnimationType_ID);
    if (get_flags (config->feel->set_flags, FEEL_DeskAnimationSteps))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->DeskAnimationSteps, FEEL_DeskAnimationSteps_ID);
    if (get_flags (config->feel->set_flags, FEEL_ShadeAnimationSteps))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->ShadeAnimationSteps, FEEL_ShadeAnimationSteps_ID);

    if (get_flags (config->feel->set_flags, FEEL_XorValue))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->XorValue, FEEL_XorValue_ID);
    if (get_flags (config->feel->set_flags, FEEL_Xzap))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->Xzap, FEEL_Xzap_ID);
    if (get_flags (config->feel->set_flags, FEEL_Yzap))
        tail = Integer2FreeStorage (&FeelSyntax, tail, NULL, config->feel->Yzap, FEEL_Yzap_ID);

    for( i = 0 ; i < MAX_CURSORS ; i++ )
    {
        if (config->feel->standard_cursors[i] != 0)
            tail = Integer2FreeStorage (&FeelSyntax, tail, &i, config->feel->standard_cursors[i], FEEL_Cursor_ID);
        if (config->feel->custom_cursors[i] != NULL)
            tail = ASCursor2FreeStorage (&FeelSyntax, tail, i, config->feel->custom_cursors[i], FEEL_CustomCursor_ID);
    }

    if (get_flags (config->feel->set_flags, FEEL_ClickToRaise))
        Bitlist2FreeStorage( &FeelSyntax, tail, config->feel->RaiseButtons, FEEL_ClickToRaise_ID);

    if (get_flags (config->feel->set_flags, FEEL_EdgeScroll))
        Integer2FreeStorage( &FeelSyntax, tail, &(config->feel->EdgeScrollX), config->feel->EdgeScrollY, FEEL_EdgeScroll_ID);
    if (get_flags (config->feel->set_flags, FEEL_EdgeResistance))
        Integer2FreeStorage( &FeelSyntax, tail, &(config->feel->EdgeResistanceScroll), config->feel->EdgeResistanceMove, FEEL_EdgeResistance_ID);

    /* complex functions : */
    if( config->feel->funcs_list )
        if( config->feel->funcs_list->items_num )
        {
            ComplexFunction **list ;

            list = safecalloc( config->feel->funcs_list->items_num, sizeof(ComplexFunction*));
            if( (i = sort_hash_items( config->feel->funcs_list, NULL, (void**)list, 0 )) > 0 )
                while ( --i >= 0 )
                    tail = ComplexFunction2FreeStorage( &FeelSyntax, tail, list[i]);
            free( list );
        }
    /* menus */
    /* menus writing require additional work due to the nature of the problem -
     * menus can come from file or directory or from feel file itself.
     * plus there has to be some mechanismus in place in order to preserve
     * .include configuration when menu is saved as directory.
     *          Sasha.
     */
    /* this is preliminary code that only saves menus into feel file itself  - make sure
     * you remove all the other menus from the feel->menus_list prior to calling this */
    if( config->feel->menus_list )
        if( config->feel->menus_list->items_num )
        {
            MenuData **list ;
            register int i ;
            list = safecalloc( config->feel->menus_list->items_num, sizeof(MenuData*));
            if( (i = sort_hash_items( config->feel->menus_list, NULL, (void**)list, 0 )) > 0 )
                while ( --i >= 0 )
                    tail = MenuData2FreeStorage( &FeelSyntax, tail, list[i]);
            free( list );
        }
    /* mouse bindings : */
    for( i = 0 ; i < MAX_MOUSE_BUTTONS+1 ; i++ )
    {
        char btn_id[2] ;
        btn_id[0] = '0'+i ;
        btn_id[1] = '\0' ;
        tail = Contexts2FreeStorage( &FeelSyntax, tail, btn_id, &(config->feel->mouse[i]), FEEL_Mouse_ID );
    }

    /* keyboard bindings : */
    if( config->feel->keyboard )
        if( config->feel->keyboard->items_num > 0 )
        {
            ASHashableValue  *keys;
            ASInputContexts **contexts;
            unsigned int items_num = config->feel->keyboard->items_num ;

            keys = safecalloc( items_num, sizeof(ASHashableValue) );
            contexts = safecalloc( items_num, sizeof(ASInputContexts*) );
            items_num = sort_hash_items( config->feel->keyboard, keys, (void**)contexts, 0 );
            for( i = 0 ; i < items_num ; i++ )
            {
                KeySym keysym = (KeySym)keys[i];
                tail = Contexts2FreeStorage( &FeelSyntax, tail, XKeysymToString(keysym), contexts[i], FEEL_Key_ID );
            }
        }

    /* writing config into the file */
	WriteConfig (ConfigWriter, Storage, CDT_Filename, (void **) &filename, flags);
	DestroyFreeStorage (&Storage);
    DestroyConfig (ConfigWriter);

	if (Storage)
  	{
    	fprintf (stderr,
	  		     "\n%s:Config Writing warning: Not all Free Storage discarded! Trying again...",
	      		 myname);
    	DestroyFreeStorage (&Storage);
  		fprintf (stderr, (Storage != NULL) ? " failed." : " success.");
    }
	return 0;
}

#endif
/********************************************************************************/
/*   AutoExec config :                                                          */
/********************************************************************************/

AutoExecConfig *
CreateAutoExecConfig ()
{
  AutoExecConfig *config = (AutoExecConfig *) safecalloc (1, sizeof (AutoExecConfig));
  return config;
}

void
DestroyAutoExecConfig (AutoExecConfig * config)
{
    if( config->init )
        really_destroy_complex_func( config->init );
    if( config->restart )
        really_destroy_complex_func( config->restart );

    DestroyFreeStorage (&(config->more_stuff));
    free (config);
}

AutoExecConfig *
ParseAutoExecOptions (const char *filename, char *myname)
{
    ConfigData cd ;
	ConfigDef *ConfigReader;
    AutoExecConfig *config = CreateAutoExecConfig ();
	FreeStorageElem *Storage = NULL, *pCurr;
    ConfigItem item;

	cd.filename = filename ;
	ConfigReader = InitConfigReader (myname, &AutoExecSyntax, CDT_Filename, cd, NULL);
	if (!ConfigReader)
  		return config;

    item.memory = NULL;
	PrintConfigReader (ConfigReader);
	ParseConfig (ConfigReader, &Storage);

	/* getting rid of all the crap first */
    StorageCleanUp (&Storage, &(config->more_stuff), CF_DISABLED_OPTION);

    for (pCurr = Storage; pCurr; pCurr = pCurr->next)
  	{
  	    if (pCurr->term == NULL)
			continue;
        if (!ReadConfigItem (&item, pCurr))
			continue;

        switch (pCurr->term->id)
		{
            case FEEL_Function_ID           :
                if( mystrncasecmp( item.data.string, "InitFunction", 12 ) == 0 )
                {
                    if( config->init )
                        really_destroy_complex_func( config->init );
                     config->init = FreeStorage2ComplexFunction( pCurr, &item, NULL );
                }else if( mystrncasecmp( item.data.string, "RestartFunction", 15 ) == 0 )
                {
                    if( config->restart )
                        really_destroy_complex_func( config->restart );
                    config->restart = FreeStorage2ComplexFunction( pCurr, &item, NULL );
                }
                break ;
          default:
				item.ok_to_free = 1;
		}
    }
	ReadConfigItem (&item, NULL);

	DestroyConfig (ConfigReader);
	DestroyFreeStorage (&Storage);
	return config;

}

int
WriteAutoExecOptions (const char *filename, char *myname,  AutoExecConfig * config, unsigned long flags)
{
    ConfigDef *ConfigWriter = NULL;
    FreeStorageElem *Storage = NULL, **tail = &Storage;
	ConfigData cd ;

	if (config == NULL)
  		return 1;
	cd.filename = filename ; 
    if ((ConfigWriter = InitConfigWriter (myname, &AutoExecSyntax, CDT_Filename, cd)) == NULL)
	    return 2;

    CopyFreeStorage (&Storage, config->more_stuff);

    if( config->init )
        tail = ComplexFunction2FreeStorage( &AutoExecSyntax, tail, config->init );
    if(config->restart)
        tail = ComplexFunction2FreeStorage( &AutoExecSyntax, tail, config->restart );

    /* writing config into the file */
	cd.filename = filename ; 
	WriteConfig (ConfigWriter, Storage, CDT_Filename, &cd, flags);
	DestroyFreeStorage (&Storage);
    DestroyConfig (ConfigWriter);

	if (Storage)
  	{
    	fprintf (stderr,
	  		     "\n%s:Config Writing warning: Not all Free Storage discarded! Trying again...",
	      		 myname);
    	DestroyFreeStorage (&Storage);
  		fprintf (stderr, (Storage != NULL) ? " failed." : " success.");
    }
	return 0;
}

/********************************************************************************/
/*   Theme file  :                                                              */
/********************************************************************************/
void
DestroyThemeConfig (ThemeConfig * config)
{
    if( config->install )
        really_destroy_complex_func( config->install );
    if( config->apply )
        really_destroy_complex_func( config->apply );

    DestroyFreeStorage (&(config->more_stuff));
    free (config);
}

ThemeConfig *
ParseThemeFile (const char *filename, char *myname)
{
	ThemeConfig *config ;
	ConfigData cd ;
    ConfigDef *ConfigReader;
    FreeStorageElem *Storage = NULL, *pCurr;
    ConfigItem item;

	cd.filename = filename ;
	ConfigReader = InitConfigReader (myname, &ThemeSyntax, CDT_Filename, cd, NULL);
	LOCAL_DEBUG_OUT( "ConfigReader is %p", ConfigReader );
    if (!ConfigReader)
        return NULL;

	config = safecalloc( 1, sizeof(ThemeConfig ) );

	PrintConfigReader (ConfigReader);
	ParseConfig (ConfigReader, &Storage);

	/* getting rid of all the crap first */
    StorageCleanUp (&Storage, &(config->more_stuff), CF_DISABLED_OPTION);

	for (pCurr = Storage; pCurr; pCurr = pCurr->next)
  	{
  	    if (pCurr->term == NULL)
			continue;
        if (!ReadConfigItem (&item, pCurr))
			continue;

        switch (pCurr->term->id)
		{
            case FEEL_Function_ID           :
                if( mystrncasecmp( item.data.string, THEME_INSTALL_FUNC_NAME, sizeof(THEME_INSTALL_FUNC_NAME))== 0 )
                {
                    if( config->install )
                        really_destroy_complex_func( config->install );
                     config->install = FreeStorage2ComplexFunction( pCurr, &item, NULL );
                }else if( mystrncasecmp( item.data.string, THEME_APPLY_FUNC_NAME, sizeof(THEME_APPLY_FUNC_NAME) )== 0 )
                {
                    if( config->apply )
                        really_destroy_complex_func( config->apply );
                    config->apply = FreeStorage2ComplexFunction( pCurr, &item, NULL );
                }
                break ;
          default:
				item.ok_to_free = 1;
		}
    }
	ReadConfigItem (&item, NULL);

	DestroyConfig (ConfigReader);
	DestroyFreeStorage (&Storage);

	return config;
}


