/*
 * Copyright (C) 2005 Fabian Yamaguchi <fabiany at gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "../../configure.h"
#include "../../libAfterStep/asapp.h"
#include "../../libAfterStep/module.h"
#include "../../libAfterStep/ascommand.h"
#include "../../libAfterStep/operations.h"
#include "../../libAfterConf/afterconf.h"
#include "../../libASGTK/asgtk.h"
#include "../../libASGTK/asgtkapp.h"


#include <gdk/gdkx.h>

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

/* ugly gui-definitions */
  GtkWidget *window1;
  GtkWidget *frame1;
  GtkWidget *alignment1;
  GtkWidget *table1;
  GtkWidget *alldesks_radio;
  GSList *alldesks_radio_group = NULL;
  GtkWidget *thisdesk_radio;
  GtkWidget *thisscreen_radio;
  GtkWidget *frame2;
  GtkWidget *alignment2;
  GtkWidget *pattern_entry;
  GtkWidget *label2;
  GtkWidget *action_combo;
  GtkWidget *frame3;
  GtkWidget *alignment3;
  GtkWidget *notebook1;
  GtkWidget *label13;
  GtkWidget *table2;
  GtkWidget *label9;
  GtkWidget *label10;
  GtkWidget *entry2;
  GtkWidget *entry1;
  GtkWidget *hbox1;
  GtkWidget *label11;
  GtkWidget *entry3;
  GtkWidget *label3;
  GtkWidget *label1;
  GtkEntryCompletion *completion;
  GtkTreeModel *completion_model;
  GtkWidget *run_button;
  GtkWidget *hbox2;
  GtkWidget *table3;
  GtkWidget *label14;
  GtkWidget *label15;
  GtkWidget *entry4;
  GtkWidget *entry5;

/* /ugly gui-definitions */

typedef struct
{
	char *name;
	int param_page;
	Bool (*exec_wrapper)(void);
	void (*init_defaults)(void);
} action_t ;


typedef
enum{ THIS_SCREEN = 0, THIS_DESK, ALL_DESKS }
selected_area;


struct ASGWCommandState
{
	char *action;
	ASFlagType flags;
	char **win_list;
	selected_area area;
} GWCommandState;

/******** Prototypes *********************/
GtkWidget* create_window (void);
static void destroy( GtkWidget *widget,
                     gpointer   data );
Bool no_args_wrapper(void);
Bool move_wrapper(void);
Bool send_to_desk_wrapper(void);
Bool jump_wrapper(void);
Bool resize_wrapper(void);
Bool group_wrapper(void);

void default_defaults(void);
void jump_defaults(void);

/******** /Prototypes ********************/

action_t Actions[] = 
{
	{"center", 0, no_args_wrapper, default_defaults },
	{"center jump", 0, jump_wrapper, jump_defaults },
	{"deiconify", 0, no_args_wrapper, default_defaults},
	{"group", 0, group_wrapper, jump_defaults},
	{"iconify", 0, no_args_wrapper, default_defaults },
	{"jump", 0, jump_wrapper, jump_defaults},
	{"kill", 0, no_args_wrapper, default_defaults},
	{"move", 1, move_wrapper, default_defaults},
	{"raise", 0, no_args_wrapper, default_defaults},
	{"resize", 2, resize_wrapper, default_defaults},
	{"sendtodesk", 3, send_to_desk_wrapper, default_defaults},
        { NULL, 0, NULL, NULL}
};

static gboolean
match_func (GtkEntryCompletion *completion,
	    const gchar        *key,
	    GtkTreeIter        *iter,
	    gpointer            user_data)
{
	gchar *item = NULL;
	GtkTreeModel *model;
	regex_t my_reg;

	gboolean ret = FALSE;
	
	if( regcomp( &my_reg, key, REG_EXTENDED | REG_ICASE) != 0)
		return FALSE;

	model = gtk_entry_completion_get_model (completion);
	
	gtk_tree_model_get (model, iter, 0, &item, -1);
	
	if (item != NULL)
	{
		if( regexec( &my_reg, (char *) item, 0, NULL, 0) == 0)
			ret = TRUE;
		
		g_free (item);
	}
	
	regfree( &my_reg );
	
	return ret;
}

/* Creates a tree model containing the completions */
GtkTreeModel *
create_simple_completion_model (void)
{
	GtkListStore *store;
	GtkTreeIter iter;
	int i;
	
	store = gtk_list_store_new (1, G_TYPE_STRING);
	
	for( i = 0 ;
	     GWCommandState.win_list[i] != NULL; i++ )
	{
		gtk_list_store_append (store, &iter);
	  gtk_list_store_set (store, &iter, 0, GWCommandState.win_list[i], -1);
	}
	
	return GTK_TREE_MODEL (store);
}

GtkWidget*
create_window (void)
{
  

	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window1), "GWCommand");

	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_container_add (GTK_CONTAINER (window1), frame1);
	gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);
	gtk_frame_set_shadow_type (GTK_FRAME (frame1), GTK_SHADOW_NONE);

	alignment1 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment1);
	gtk_container_add (GTK_CONTAINER (frame1), alignment1);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment1), 0, 5, 5, 5);
	
	table1 = gtk_table_new (3, 3, FALSE);
	gtk_widget_show (table1);
	gtk_container_add (GTK_CONTAINER (alignment1), table1);
	
	alldesks_radio = gtk_radio_button_new_with_mnemonic (NULL, "on all Desktops");
	gtk_widget_show (alldesks_radio);
	gtk_table_attach (GTK_TABLE (table1), alldesks_radio, 2, 3, 2, 3,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (alldesks_radio), alldesks_radio_group);
	alldesks_radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (alldesks_radio));
	
	thisdesk_radio = gtk_radio_button_new_with_mnemonic (NULL, "on this Desktop");
	gtk_widget_show (thisdesk_radio);
	gtk_table_attach (GTK_TABLE (table1), thisdesk_radio, 1, 2, 2, 3,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (thisdesk_radio), alldesks_radio_group);
	alldesks_radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (thisdesk_radio));
	
	thisscreen_radio = gtk_radio_button_new_with_mnemonic (NULL, "on this screen");
	gtk_widget_show (thisscreen_radio);
	gtk_table_attach (GTK_TABLE (table1), thisscreen_radio, 0, 1, 2, 3,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (thisscreen_radio), alldesks_radio_group);
	alldesks_radio_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (thisscreen_radio));
	
	frame2 = gtk_frame_new (NULL);
	gtk_widget_show (frame2);
	gtk_table_attach (GTK_TABLE (table1), frame2, 0, 3, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 5, 5);
	gtk_container_set_border_width (GTK_CONTAINER (frame2), 5);
	gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);
	
	alignment2 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment2);
	gtk_container_add (GTK_CONTAINER (frame2), alignment2);
	
	hbox2 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox2);
	gtk_container_add (GTK_CONTAINER (alignment2), hbox2);
	
	pattern_entry = gtk_entry_new ();
	gtk_widget_show (pattern_entry);
	gtk_box_pack_start (GTK_BOX (hbox2), pattern_entry, TRUE, TRUE, 0);
	
	run_button = gtk_button_new_with_label("go!");
	gtk_widget_show (run_button);
	gtk_box_pack_start (GTK_BOX (hbox2), run_button, TRUE, TRUE, 0);

	completion = gtk_entry_completion_new();
	gtk_entry_set_completion (GTK_ENTRY (pattern_entry), completion);
	gtk_entry_completion_set_match_func (completion, match_func, NULL, NULL);
	g_object_unref (completion);
	
	/* Create a tree model and use it as the completion model */
	completion_model = create_simple_completion_model ();
	gtk_entry_completion_set_model (completion, completion_model);
	g_object_unref (completion_model);
	
	/* Use model column 0 as the text column */
	gtk_entry_completion_set_text_column (completion, 0);
	
	label2 = gtk_label_new ("<b>pattern</b>");
	gtk_widget_show (label2);
	gtk_frame_set_label_widget (GTK_FRAME (frame2), label2);
	gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
	gtk_misc_set_padding (GTK_MISC (label2), 5, 0);
	
	action_combo = gtk_combo_box_new_text ();
	gtk_widget_show (action_combo);
	gtk_table_attach (GTK_TABLE (table1), action_combo, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	
	
	frame3 = gtk_frame_new (NULL);
	gtk_widget_show (frame3);
	gtk_table_attach (GTK_TABLE (table1), frame3, 1, 3, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (GTK_FILL), 5, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame3), 5);
	gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);
	
	alignment3 = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment3);
	gtk_container_add (GTK_CONTAINER (frame3), alignment3);
	
	notebook1 = gtk_notebook_new ();
	gtk_widget_show (notebook1);
	gtk_container_add (GTK_CONTAINER (alignment3), notebook1);
	GTK_WIDGET_UNSET_FLAGS (notebook1, GTK_CAN_FOCUS);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook1), FALSE);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook1), FALSE);
	
	label13 = gtk_label_new ("No arguments");
	gtk_widget_show (label13);
	gtk_container_add (GTK_CONTAINER (notebook1), label13);
	
	
	table2 = gtk_table_new (2, 2, TRUE);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (notebook1), table2);
	
	label9 = gtk_label_new ("X:");
	gtk_widget_show (label9);
	gtk_table_attach (GTK_TABLE (table2), label9, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);
	
	label10 = gtk_label_new ("Y:");
	gtk_widget_show (label10);
	gtk_table_attach (GTK_TABLE (table2), label10, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);
	
	entry2 = gtk_entry_new ();
	gtk_widget_show (entry2);
	gtk_table_attach (GTK_TABLE (table2), entry2, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	
	entry1 = gtk_entry_new ();
	gtk_widget_show (entry1);
	gtk_table_attach (GTK_TABLE (table2), entry1, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	
	table3 = gtk_table_new (2, 2, TRUE);
	gtk_widget_show (table3);
	gtk_container_add (GTK_CONTAINER (notebook1), table3);
	
	label14 = gtk_label_new ("Width:");
	gtk_widget_show (label14);
	gtk_table_attach (GTK_TABLE (table3), label14, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);
	
	label15 = gtk_label_new ("Height:");
	gtk_widget_show (label15);
	gtk_table_attach (GTK_TABLE (table3), label15, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (label15), 0, 0.5);
	
	entry4 = gtk_entry_new ();
	gtk_widget_show (entry4);
	gtk_table_attach (GTK_TABLE (table3), entry4, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	
	entry5 = gtk_entry_new ();
	gtk_widget_show (entry5);
	gtk_table_attach (GTK_TABLE (table3), entry5, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_container_add (GTK_CONTAINER (notebook1), hbox1);
	
	label11 = gtk_label_new ("Desk: ");
	gtk_widget_show (label11);
	gtk_box_pack_start (GTK_BOX (hbox1), label11, FALSE, FALSE, 0);
	
	entry3 = gtk_entry_new ();
	gtk_widget_show (entry3);
	gtk_box_pack_start (GTK_BOX (hbox1), entry3, TRUE, TRUE, 0);
	
	label3 = gtk_label_new ("<b>arguments</b>");
	gtk_widget_show (label3);
	gtk_frame_set_label_widget (GTK_FRAME (frame3), label3);
	gtk_label_set_use_markup (GTK_LABEL (label3), TRUE);
	gtk_misc_set_padding (GTK_MISC (label3), 5, 0);
	
	label1 = gtk_label_new ("<b>Command to perform on a window:</b>");
	gtk_widget_show (label1);
	gtk_frame_set_label_widget (GTK_FRAME (frame1), label1);
	gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);
	gtk_misc_set_padding (GTK_MISC (label1), 5, 0);
	
	gtk_widget_grab_focus (pattern_entry);
	
	colorize_gtk_window( window1 ); 

	return window1;
}

action_t *get_action_by_name(const char *needle)
{
	int i;
	for( i = 0; Actions[i].name != NULL; i++)
		if(mystrcasecmp(Actions[i].name, needle) == 0)
			return &Actions[i];
	return NULL;
}

int get_index_by_action_name(const char *needle)
{
	int i;
	for( i = 0; Actions[i].name != NULL; i++)
		if(mystrcasecmp(Actions[i].name, needle) == 0)
			return i;
	return -1;
}

/* Callback-functions */
static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

void
on_action_combo_changed                (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	action_t *a;
	char *new_action = (char *) asgtk_combo_box_get_active_text( combobox );
       
	free( GWCommandState.action);
	GWCommandState.action = NULL;
	
	if( new_action != NULL )
	{	
		
		GWCommandState.action = strdup(new_action);
		a = get_action_by_name(GWCommandState.action);
		if( a == NULL ) return;
		
		gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook1), a->param_page );
		a->init_defaults();
	}
}

void
on_pattern_activate ( GtkEntry *entry,
		    gpointer data )
{
	action_t *act;
	clear_selection();
	
	act = get_action_by_name(GWCommandState.action);
	
	if( GWCommandState.area == THIS_SCREEN )
		select_windows_on_screen( False );
	else if ( GWCommandState.area == THIS_DESK )
		select_windows_on_desk( False );
	
	/* call the wrapper */
	if( act->exec_wrapper() )
		gtk_main_quit();
	else
	  asgtk_warning2( NULL, "Invalid pattern.", NULL, NULL);
}

void
on_thisscreen_radio_toggled (GtkToggleButton *togglebutton,
			     gpointer         user_data)
{
	GWCommandState.area = THIS_SCREEN;
}

void
on_thisdesk_radio_toggled (GtkToggleButton *togglebutton,
			     gpointer         user_data)
{
	GWCommandState.area = THIS_DESK;
}

void
on_alldesks_radio_toggled (GtkToggleButton *togglebutton,
			   gpointer         user_data)
{
	GWCommandState.area = ALL_DESKS;
}


/*********************************/

void fill_action_combo( void )
{
	int i;
	for( i = 0; Actions[i].name != NULL; i++)
		gtk_combo_box_append_text
			(GTK_COMBO_BOX (action_combo), Actions[i].name );
}

/* exec_wrappers */

Bool
no_args_wrapper(void)
{
	if ( ! select_windows_by_pattern
	     (gtk_entry_get_text(GTK_ENTRY(pattern_entry)), False, False) )
		return False;
	
	ascom_do(GWCommandState.action, NULL);
	return True;
}

Bool
move_wrapper(void)
{
	move_params p;
	p.x = atoi (gtk_entry_get_text(GTK_ENTRY(entry1)));
	p.y = atoi (gtk_entry_get_text(GTK_ENTRY(entry2)));
	
	if ( ! select_windows_by_pattern
	     (gtk_entry_get_text(GTK_ENTRY(pattern_entry)), False, False) )
		return False;
	
	ascom_do(GWCommandState.action, &p);
	return True;
}

Bool
resize_wrapper(void)
{
	resize_params p;
	p.width = atoi (gtk_entry_get_text(GTK_ENTRY(entry4)));
	p.height = atoi (gtk_entry_get_text(GTK_ENTRY(entry5)));
	
	if ( ! select_windows_by_pattern
	     (gtk_entry_get_text(GTK_ENTRY(pattern_entry)), False, False) )
		return False;
	
	ascom_do(GWCommandState.action, &p);
	return True;
}

Bool
send_to_desk_wrapper(void)
{
	send_to_desk_params p;
	p.desk = atoi (gtk_entry_get_text(GTK_ENTRY(entry3)));
	
	if ( ! select_windows_by_pattern
	     (gtk_entry_get_text(GTK_ENTRY(pattern_entry)), False, False) )
		return False;
		
	ascom_do(GWCommandState.action, &p);

	return True;
}

Bool
jump_wrapper(void)
{
	if ( ! select_windows_by_pattern
	     (gtk_entry_get_text(GTK_ENTRY(pattern_entry)), True, False) )
		return False;
	
	ascom_do(GWCommandState.action, NULL);
	return True;
}

Bool
group_wrapper(void)
{
	char com[1024];
	/* Launch WinTabs */
	snprintf(com, 1024 - 1 , "WinTabs --pattern \"posix:%s\" &",
		 gtk_entry_get_text(GTK_ENTRY(pattern_entry)));
	
	system( com );
	return True;
}

/* init_defaults */

void default_defaults(void)
{
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(thisscreen_radio), TRUE);
	GWCommandState.area = THIS_SCREEN;
}

void jump_defaults(void)
{
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(alldesks_radio), TRUE);
	GWCommandState.area = ALL_DESKS;
}

/****************************/

int main(int argc, char **argv)
{
	GdkDisplay *gdk_display ;
	GtkWidget *my_window;
	int i, index;
	static char *deleted_arg = "_deleted_arg_" ;

	memset( &GWCommandState, 0x00, sizeof(GWCommandState));

  	InitMyApp (CLASS_ASCP, argc, argv, NULL, NULL, 0 );
	for( i = 1 ; i < argc ; ++i ){ 
		if( argv[i] == NULL ) 
			argv[i] = deleted_arg ;
		/* It's a flag */
		else if(argv[i][0] == '-')
		{
			
		}else
		{
			/* It's the command! */
			
			GWCommandState.action =  argv[i];
		}
	}

	LinkAfterStepConfig();
  	InitSession();
	
	gtk_init( &argc, &argv);
	gdk_display = gdk_display_get_default();
	
	LoadColorScheme();
	ConnectXDisplay (gdk_x11_display_get_xdisplay(gdk_display), NULL, False);
	ReloadASEnvironment( NULL, NULL, NULL, False, True );
 
   
	ConnectAfterStep (WINDOW_CONFIG_MASK |
                      WINDOW_NAME_MASK |
                      M_END_WINDOWLIST|
				      M_NEW_DESKVIEWPORT, 0);
	
	ascom_init();
	ascom_update_winlist();

	GWCommandState.win_list = ascom_get_win_names();
	
	my_window = create_window();
	
	/* Fill interface with data */
	fill_action_combo();
	
	if(GWCommandState.action == NULL)
		index = 0;
	else
		index = get_index_by_action_name(GWCommandState.action);
	
	if( index == -1 )
		index = 0;
	
	/* Up to this point, GWCommand.action either points to NULL
	   or so an adress of an argument on the stack: no free. */
	GWCommandState.action = strdup ( Actions[index].name );
	
	/* connect signals */
	g_signal_connect (G_OBJECT (my_window), "destroy",
			  G_CALLBACK (destroy), NULL);
	
	g_signal_connect ((gpointer) action_combo, "changed",
			  G_CALLBACK (on_action_combo_changed),
			  NULL);

	g_signal_connect ((gpointer) pattern_entry, "activate",
			 G_CALLBACK (on_pattern_activate), NULL);

	g_signal_connect( (gpointer) run_button, "clicked",
			  G_CALLBACK (on_pattern_activate), NULL);

	g_signal_connect ((gpointer) thisscreen_radio, "toggled",
			  G_CALLBACK (on_thisscreen_radio_toggled),
			  NULL);

	g_signal_connect ((gpointer) thisdesk_radio, "toggled",
			  G_CALLBACK (on_thisdesk_radio_toggled),
			  NULL);
	
	g_signal_connect ((gpointer) alldesks_radio, "toggled",
			  G_CALLBACK (on_alldesks_radio_toggled),
			  NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(action_combo), index);
	gtk_notebook_set_current_page (GTK_NOTEBOOK(notebook1), Actions[index].param_page);
	
	
	if( Actions[index].init_defaults)
		Actions[index].init_defaults();
	

	gtk_widget_show(my_window);
	gtk_main();
	
	/* Deinit */
	ascom_wait();
	ascom_deinit();
	
	/* free memory */
	i = 0;
	while( GWCommandState.win_list[i] != NULL)
		free( GWCommandState.win_list[i++] );
	free( GWCommandState.win_list );
	free(GWCommandState.action );

	return 0;
}
