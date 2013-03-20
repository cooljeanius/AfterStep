/*
 * Copyright (c) 2005 Sasha Vasko <sasha at aftercode.net>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define LOCAL_DEBUG

#include "../../../configure.h"
#include "../../../libAfterStep/asapp.h"
#include "../../../libAfterImage/afterimage.h"
#include "../../../libAfterStep/screen.h"
#include "../../../libAfterStep/colorscheme.h"
#include "../../../libAfterStep/module.h"
#include "../../../libASGTK/asgtk.h"

#include <unistd.h>		   

#include "interface.h"

void
gtk_image_browser_destroy(GtkWidget *widget, gpointer user_data)
{
	if(WallpaperState.filechooser != NULL)
		WallpaperState.filechooser = NULL ; 
	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.list_browse_button), TRUE );
}

void
on_list_del_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( entry ) 
	{	
		if( asgtk_yes_no_question1( WallpaperState.main_window, "Do you really want to delete private background file \"%s\" ???", entry->name ) )
		{
			if( id->mini_extension ) 
			{	
				char *mini_filename, *mini_fullfilename ;
				asgtk_image_dir_make_mini_names( id, entry->name, &mini_filename, &mini_fullfilename ); 
				
				if( CheckFile( mini_fullfilename ) == 0 ) 
				{
					if( asgtk_yes_no_question1( WallpaperState.main_window, "It appears that there is a minipixmap for deleted background with the name \"%s\". Would you like to delete it as well ?", mini_filename ) )
					{
						unlink( mini_fullfilename );
					}	 				   
				}	
				free( mini_fullfilename );
				free( mini_filename );
			}			
			unlink( entry->fullfilename );
			asgtk_info2( WallpaperState.main_window, "Background image \"%s\" deleted.", entry->name, NULL );	  
			asgtk_image_dir_refresh( id );	 
		}	 
		unref_asimage_list_entry( entry );
	}
}

void
on_list_apply_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( entry ) 
	{	
		SendTextCommand ( F_CHANGE_BACKGROUND, NULL, entry->fullfilename, 0);
		unref_asimage_list_entry( entry );
	}
}
typedef struct ASGtkMakeXMLDlg
{
	GtkWidget *dlg ; 
	ASImageListEntry *entry ;	  
	GtkWidget *scale_check_box ;
	GtkWidget *color_check_box ;
	GtkWidget *border_check_box ;

	GtkWidget *tint_radio ; 
	GtkWidget *hsv_radio ; 

	GtkWidget *border_width ; 
	GtkWidget *solid_check_box ; 
	GtkWidget *single_color_check_box ; 
	GtkWidget *outline_check_box ; 

	GtkWidget *back_name ;
	
	char *fullfilename ;
	char *mini_fullfilename ;

}ASGtkMakeXMLDlg;

void
make_xml_dlg_destroy(ASGtkMakeXMLDlg *mx)
{
	if( mx )
	{ 
		gtk_widget_destroy( mx->dlg );
		if( mx->entry ) 
		{	
			unref_asimage_list_entry( mx->entry );
			mx->entry = NULL ; 
		}
		if( mx->fullfilename ) 
			free( mx->fullfilename );
		if( mx->mini_fullfilename ) 
			free( mx->mini_fullfilename );
		memset( mx, 0x00, sizeof(ASGtkMakeXMLDlg));
		free( mx );
	}
}


Bool 
make_xml_from_image( ASGtkMakeXMLDlg* mx, ASGtkImageDir *id )
{
	FILE *fp ; 
	const char *name = gtk_entry_get_text( GTK_ENTRY(mx->back_name) );
	int border_width = 0;
	Bool tint = False ;
	Bool hsv = False ;
	Bool scale = False ; 
	int pad = 0, i ;
	int size_offset = 0 ;

	if( (fp = open_xml_file_in_dir( id, name, False )) == NULL ) 
		return False;

	scale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->scale_check_box));
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->color_check_box)) )
	{
		if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->tint_radio)) )	
			tint = True ; 
		else
			hsv = True ;
	}	 
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->border_check_box)) )
		border_width = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(mx->border_width) );
	if( border_width > 0 ) 
	{
		int solid = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->solid_check_box))?1:0 ; 	  
		int outline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->outline_check_box))?1:0 ; 
		Bool single_color = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->single_color_check_box)) ;
		
		fprintf( fp, single_color?"<bevel colors=\"Base Base\"":"<bevel colors=\"BaseLight BaseDark\"");
		fprintf( fp, " width=\"$xroot.width\" height=\"$xroot.height\"");
		fprintf( fp, " border=\"%d %d %d %d\" solid=\"%d\" outline=\"%d\">\n", 
		  	 	 border_width, border_width, border_width, border_width, solid, outline );
		if( outline ) 
			size_offset = border_width*2 ;
		++pad ;
	}
	if( scale )
	{
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );		
		if( size_offset > 0 )
			fprintf( fp, "<scale width=\"$xroot.width-%d\" height=\"$xroot.height-%d\">\n", size_offset, size_offset );
		else
			fprintf( fp, "<scale width=\"$xroot.width\" height=\"$xroot.height\">\n" );
		++pad;
	}	
	if( tint || hsv ) 
	{	
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );
		if( tint )
			fprintf( fp, "<tile tint=\"Base\"" );
		else
			fprintf( fp, "<hsv hue_offset=\"$ascs.Base.hue\" value_offset=\"1\"" );
		
		if( !scale && border_width > 0 )
		{	
			if( size_offset > 0 )  
				fprintf( fp, " width=\"$xroot.width-%d\" height=\"$xroot.height-%d\"", size_offset, size_offset );
			else
				fprintf( fp, " width=\"$xroot.width\" height=\"$xroot.height\"" );
		}		
		fprintf( fp, ">\n" );
		++pad;
	}
	for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );		   
	fprintf( fp, "<img src=\"%s\"/>\n", mx->entry->name );
	--pad ;	 
	if( tint || hsv ) 
	{	
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );
		fprintf( fp, tint?"</tile>\n":"</hsv>\n");
		--pad;		
	}
	if( scale )
	{
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );		
		fprintf( fp, "</scale>\n");
		--pad;
	}	

	if( border_width > 0 )
		fprintf( fp, "</bevel>\n");
	
	fclose(fp);
	return True;
}	 

Bool 
make_minixml_from_image( ASGtkMakeXMLDlg* mx, ASGtkImageDir *id )
{
	FILE *fp ; 
	const char *name = gtk_entry_get_text( GTK_ENTRY(mx->back_name) );
	Bool tint = False ;
	Bool hsv = False ;
	Bool scale = False ; 
	int pad = 0, i ;
	
	if( (fp = open_xml_file_in_dir( id, name, True )) == NULL ) 
		return False;

	scale = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->scale_check_box));
	if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->color_check_box)) )
	{
		if( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mx->tint_radio)) )	
			tint = True ; 
		else
			hsv = True ;
	}	 
	if( tint || hsv ) 
	{	
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );
		if( tint )
			fprintf( fp, "<tile tint=\"Base\"" );
		else
			fprintf( fp, "<hsv hue_offset=\"$ascs.Base.hue\" value_offset=\"1\"" );
		
		if( !scale ) 
			fprintf( fp, " width=\"$minipixmap.width\" height=\"$minipixmap.height\"" );
		fprintf( fp, ">\n" );
		++pad;
	}
	if( scale )
	{
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );		
		fprintf( fp, "<scale width=\"$minipixmap.width\" height=\"$minipixmap.height\">\n" );
		++pad;
	}	
	for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );		   
	fprintf( fp, "<img src=\"%s\"/>\n", mx->entry->name );
	--pad ;	 
	if( scale )
	{
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );		
		fprintf( fp, "</scale>\n");
		--pad;
	}	
	if( tint || hsv ) 
	{	
		for( i = 0 ; i < pad ; ++i ) fprintf( fp, "    " );
		fprintf( fp, tint?"</tile>\n":"</hsv>\n");
		--pad;		
	}

	fclose(fp);
	return True;
}	 





void 
set_make_xml_widgets_sensitive( GtkWidget *button, gpointer user_data ) 
{
	ASGtkMakeXMLDlg *mx	= (ASGtkMakeXMLDlg *)user_data ;
	Bool active = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(button)); 	 

	if( button == mx->color_check_box ) 
	{
		gtk_widget_set_sensitive( mx->tint_radio, active );
		gtk_widget_set_sensitive( mx->hsv_radio, active );
	}else if( button == mx->border_check_box ) 
	{
		gtk_widget_set_sensitive( mx->border_width, active );
		gtk_widget_set_sensitive( mx->solid_check_box, active );
		gtk_widget_set_sensitive( mx->single_color_check_box, active );
		gtk_widget_set_sensitive( mx->outline_check_box, active );
	}
}

void
on_make_xml_clicked(GtkButton *clicked_button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASGtkMakeXMLDlg *mx = safecalloc( 1, sizeof( ASGtkMakeXMLDlg ) );
	GtkWidget *frame, *box, *box2 ;
	Bool files_added = False; 
	int response ;
	const char *name ;
		
	mx->entry = asgtk_image_dir_get_selection( id );
	if( mx->entry == NULL ) 
	{
		free( mx ); 	  
		return;
	}
	mx->dlg = gtk_dialog_new_with_buttons( "Making new XML based on selected image", 
											GTK_WINDOW(WallpaperState.main_window),
										   	GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, 
											GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
											GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
											NULL
										  );
//	g_signal_connect_swapped (  GTK_OBJECT (mx->dlg), "response",              
//								G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (mx->dlg));
    gtk_container_set_border_width (GTK_CONTAINER (mx->dlg), 5);
    //gtk_widget_set_size_request (mx->dlg, 400, 300);

	mx->scale_check_box = gtk_check_button_new_with_label( "Scale image to screen size" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(mx->scale_check_box), TRUE );
	colorize_gtk_widget( mx->scale_check_box, get_colorschemed_style_normal() );
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(mx->dlg)->vbox), mx->scale_check_box, FALSE, FALSE, 0);

	mx->color_check_box = gtk_check_button_new_with_label( "Adjust image color based on selected Color Scheme." );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(mx->color_check_box), TRUE );
	//colorize_gtk_widget( mx->color_check_box, get_colorschemed_style_normal() );
    //gtk_box_pack_start (GTK_BOX (GTK_DIALOG(mx->dlg)->vbox), mx->color_check_box, FALSE, FALSE, 0);
	
	g_signal_connect ((gpointer) mx->color_check_box, "clicked", G_CALLBACK (set_make_xml_widgets_sensitive), mx);

		   
	frame = gtk_frame_new(NULL);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(mx->dlg)->vbox), frame, FALSE, FALSE, 5);
	gtk_frame_set_label_widget( GTK_FRAME(frame), mx->color_check_box );

	box = gtk_vbox_new( TRUE, 5 );
	gtk_container_add (GTK_CONTAINER (frame), box);
	gtk_container_set_border_width (GTK_CONTAINER (box), 5);

	mx->tint_radio = gtk_radio_button_new_with_label( NULL, "Use Tinting (suitable for mostly grayscale images)" );
	gtk_box_pack_start (GTK_BOX (box), mx->tint_radio, FALSE, FALSE, 0);
	mx->hsv_radio = gtk_radio_button_new_with_label_from_widget( GTK_RADIO_BUTTON(mx->tint_radio), "Use Hue rotation (suitable for colorfull images)" );
	gtk_box_pack_start (GTK_BOX (box), mx->hsv_radio, FALSE, FALSE, 0);
	colorize_gtk_widget( frame, get_colorschemed_style_normal() );
	gtk_widget_show_all (box);
	gtk_widget_show (box);
	   
	mx->border_check_box = gtk_check_button_new_with_label( "Draw Border around the image" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(mx->border_check_box), TRUE );
	colorize_gtk_widget( mx->border_check_box, get_colorschemed_style_normal() );

	g_signal_connect ((gpointer) mx->border_check_box, "clicked", G_CALLBACK (set_make_xml_widgets_sensitive), mx);    
	
	frame = gtk_frame_new(NULL);
    gtk_box_pack_start (GTK_BOX (GTK_DIALOG(mx->dlg)->vbox), frame, FALSE, FALSE, 5);
	gtk_frame_set_label_widget( GTK_FRAME(frame), mx->border_check_box );

	box = gtk_vbox_new( TRUE, 5 );
	gtk_container_set_border_width (GTK_CONTAINER (box), 5);
	gtk_container_add (GTK_CONTAINER (frame), box);

	mx->solid_check_box = gtk_check_button_new_with_label( "Draw solid bevel" );
	gtk_box_pack_start (GTK_BOX (box), mx->solid_check_box, FALSE, FALSE, 0);

	box2 = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start (GTK_BOX (box), box2, TRUE, TRUE, 0);

	mx->single_color_check_box = gtk_check_button_new_with_label( "Use single color" );
	gtk_box_pack_start (GTK_BOX (box2), mx->single_color_check_box, FALSE, FALSE, 0); 
	mx->outline_check_box  = gtk_check_button_new_with_label( "Outline image" );
	gtk_box_pack_start (GTK_BOX (box2), mx->outline_check_box, FALSE, FALSE, 0); ; 
	
	gtk_widget_show_all (box2);
	
	box2 = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_start (GTK_BOX (box), box2, TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (box2), gtk_label_new("Border width : "), FALSE, FALSE, 0);
	mx->border_width = gtk_spin_button_new_with_range( 1.0, Scr.MyDisplayWidth/2, 1.0 ); 
	gtk_box_pack_start (GTK_BOX (box2), mx->border_width, FALSE, FALSE, 0);
	gtk_widget_show_all (box2);
	
	colorize_gtk_widget( frame, get_colorschemed_style_normal() );
	gtk_widget_show_all (box);
	gtk_widget_show (box);
	
	box2 = gtk_hbox_new( FALSE, 5 );
	gtk_container_set_border_width (GTK_CONTAINER (box2), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG(mx->dlg)->vbox), box2, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box2), gtk_label_new("New Background name : "), FALSE, FALSE, 0);
	mx->back_name = gtk_entry_new(); 
	gtk_box_pack_start (GTK_BOX (box2), mx->back_name, TRUE, TRUE, 0);
	gtk_widget_show_all (box2);

	gtk_widget_show_all (mx->dlg);

	do
	{	
		response = gtk_dialog_run( GTK_DIALOG(mx->dlg) );
		if( response == GTK_RESPONSE_ACCEPT ) 
		{	
			name = gtk_entry_get_text( GTK_ENTRY(mx->back_name) );
			if( name == NULL || strlen(name) == 0 ) 
				asgtk_warning2( WallpaperState.main_window, "Empty name specified for a new background.", NULL, NULL ); 	   				   		   			   			
			else
				break;
		}
	}while( response == GTK_RESPONSE_ACCEPT ); 
	if( response == GTK_RESPONSE_ACCEPT ) 
	{
		if( make_xml_from_image( mx, id ) ) 
		{
			files_added = True ;	  
			make_minixml_from_image( mx, id ); 
		}
	}	 
	if( files_added ) 
		asgtk_info2( WallpaperState.main_window, "New background \"%s\" file created.", name, NULL );	  
	
	make_xml_dlg_destroy( mx );
	if( files_added ) 
		asgtk_image_dir_refresh( id );
}

void gtk_xml_editor_destroy( GtkWidget *widget, gpointer user_data ) 
{
	WallpaperState.xml_editor = NULL ;
	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.edit_xml_button), TRUE );
}	 

void on_backgrounds_dir_changed( ASGtkXMLView *xv, gpointer user_data, Bool new_file)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	if( new_file ) 
		asgtk_image_dir_refresh( id );
	else
	{
		ASImageListEntry *le = asgtk_image_dir_get_selection( id );
		asgtk_image_view_set_entry ( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), le);
		unref_asimage_list_entry( le );
	}	 
}	 

void
on_edit_xml_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( WallpaperState.xml_editor == NULL ) 
	{	
		WallpaperState.xml_editor = asgtk_xml_editor_new();
		g_signal_connect (G_OBJECT (WallpaperState.xml_editor), "destroy", G_CALLBACK (gtk_xml_editor_destroy), NULL);
		asgtk_xml_editor_file_change_handler( ASGTK_XML_EDITOR(WallpaperState.xml_editor), 
											  on_backgrounds_dir_changed, 
											  (gpointer)WallpaperState.backs_list );
	}

	gtk_widget_show( WallpaperState.xml_editor );
	asgtk_xml_editor_set_entry( ASGTK_XML_EDITOR(WallpaperState.xml_editor), entry );
	unref_asimage_list_entry( entry );

	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.edit_xml_button), FALSE );
}

void
on_make_mini_clicked(GtkButton *clicked_button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	char *mini_fullfilename = NULL ;

	if( asgtk_image_dir_make_mini_names( id, entry->name, NULL, &mini_fullfilename ) )
	{	
		if( make_mini_for_image_entry(id, entry, mini_fullfilename) )
		{	
			gtk_button_set_label(GTK_BUTTON(WallpaperState.make_mini_button), "Update mini");
			asgtk_info2( WallpaperState.main_window, "Minipixmap successfully updated for background \"%s\".", entry->name, NULL );	  
		}else
			asgtk_warning2( WallpaperState.main_window, "Failed to updated minipixmap for background \"%s\".", entry->name, NULL );	  


		free( mini_fullfilename );
	}
	unref_asimage_list_entry( entry );
}

void
on_list_add_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	ASImageListEntry *entry = asgtk_image_dir_get_selection( id );
	if( entry ) 
	{	
		ASGtkImageDir *backs_list = ASGTK_IMAGE_DIR(WallpaperState.backs_list);
		char *new_filename = make_file_name( backs_list->fulldirname, entry->name );
		if( CheckFile( new_filename ) == 0 ) 
		{
			if( !asgtk_yes_no_question1( WallpaperState.main_window, "Private background with the name \"%s\" already exists. Would you like to replace it ???", entry->name ) )
			{
				free( new_filename );
				return;
			}	 				   
		}	
		copy_file (entry->fullfilename, new_filename);
		free( new_filename );
		if( backs_list->mini_extension != NULL  && entry->preview != NULL ) 
		{
			char *mini_filename, *mini_fullfilename ;
			Bool do_mini = True ;
			asgtk_image_dir_make_mini_names( backs_list, entry->name, &mini_filename, &mini_fullfilename );
			if( CheckFile( mini_fullfilename ) == 0 ) 
			{
				if( !asgtk_yes_no_question1( WallpaperState.main_window, "Overwrite minipixmap \"%s\" with the new one ?", mini_filename ) )
					do_mini = False ;
			}	

			if( do_mini ) 
				make_mini_for_image_entry( id, entry, mini_fullfilename);

			free( mini_fullfilename );
			free( mini_filename );
		}	 
		asgtk_info2( WallpaperState.main_window, "New background \"%s\" added.", entry->name, NULL );	  

		unref_asimage_list_entry( entry );
		asgtk_image_dir_refresh( backs_list );	 
	}
}

void
on_update_as_menu_clicked(GtkButton *button, gpointer user_data)
{
	SendTextCommand ( F_QUICKRESTART, NULL,  "startmenu", 0);
}

void
on_browse_clicked(GtkButton *button, gpointer user_data)
{
	if( WallpaperState.filechooser == NULL ) 
	{	
		GtkWidget *add_button, *apply_button ; 
		WallpaperState.filechooser = asgtk_image_browser_new();
	
/*		close_button = asgtk_image_browser_add_main_button (ASGTK_IMAGE_BROWSER(WallpaperState.filechooser), "gtk-close", G_CALLBACK(on_filechooser_close_clicked), NULL); */
		add_button = asgtk_image_browser_add_selection_button (ASGTK_IMAGE_BROWSER(WallpaperState.filechooser), GTK_STOCK_ADD, G_CALLBACK(on_list_add_clicked)); 
		apply_button = asgtk_image_browser_add_selection_button (ASGTK_IMAGE_BROWSER(WallpaperState.filechooser), GTK_STOCK_APPLY, G_CALLBACK(on_list_apply_clicked));

		g_signal_connect (G_OBJECT (WallpaperState.filechooser), "destroy", G_CALLBACK (gtk_image_browser_destroy), NULL);  		   
		/*gtk_widget_set_size_request (GTK_WIDGET(close_button), 150, -1);*/
	}
	
	gtk_widget_show (GTK_WIDGET(WallpaperState.filechooser));
	gtk_widget_set_sensitive( GTK_WIDGET(WallpaperState.list_browse_button), FALSE );
}

GtkWidget*	
make_back_name_hbox(GtkBox *owner, int owner_spacing)
{
	GtkWidget *hbox;
	GtkWidget *name_edit;
	
	hbox = gtk_hbox_new( FALSE, 5 );
	gtk_box_pack_end (owner, hbox, TRUE, TRUE, owner_spacing);

	gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new("New background name: "), FALSE, FALSE, 0);
	name_edit = gtk_entry_new();
	gtk_box_pack_end (GTK_BOX (hbox), name_edit, TRUE, TRUE, 0);
	gtk_widget_show_all(hbox);
	gtk_widget_show(hbox);
	return name_edit;
}

void
on_solid_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	GtkDialog *cs = GTK_DIALOG(asgtk_color_selection_new());
	GtkWidget *name_edit = make_back_name_hbox(GTK_BOX (cs->vbox), 10);
	int response  = 0;
	const char *name = NULL ;
	Bool files_added = False ;

	gtk_dialog_add_buttons( cs, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
							  	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
								NULL );

	gtk_window_set_title(GTK_WINDOW(cs), "Creating new solid color background ...   ");
	gtk_window_set_modal(GTK_WINDOW(cs), TRUE);
	do
	{	
		response = gtk_dialog_run( cs );
		if( response == GTK_RESPONSE_ACCEPT ) 
		{	
			name = gtk_entry_get_text( GTK_ENTRY(name_edit) );
			if( name == NULL || strlen(name) == 0 ) 
				asgtk_warning2( WallpaperState.main_window, "Empty name specified for a new background.", NULL, NULL ); 	   				   		   			   			
			else
				break;
		}
	}while( response == GTK_RESPONSE_ACCEPT ); 
	
	if( response == GTK_RESPONSE_ACCEPT && name != NULL ) 
	{
		char *color = asgtk_color_selection_get_color_str(ASGTK_COLOR_SELECTION(cs));
		if( color ) 
		{	
			char *buffer = safemalloc( 128 + strlen(color));
			sprintf( buffer, "<solid color=\"%s\" width=\"$xroot.width\" height=\"$xroot.height\"/>", color);
			if( make_xml_from_string( id, name, buffer, False ) )
			{
				files_added = True ;	  
				sprintf( buffer, "<solid color=\"%s\" width=\"$minipixmap.width\" height=\"$minipixmap.height\"/>", color);
				make_xml_from_string( id, name, buffer, True );
			}	 
			free( buffer );
			free( color );
		}		
	}
	gtk_widget_destroy( GTK_WIDGET(cs) );
	if( files_added ) 
	{
		asgtk_info2( WallpaperState.main_window, "New solid color background \"%s\" created.", name, NULL );	  
		asgtk_image_dir_refresh( id );
	}	
}

void
on_gradient_clicked(GtkButton *button, gpointer user_data)
{
	ASGtkImageDir *id = ASGTK_IMAGE_DIR(user_data);
	GtkDialog *ge = GTK_DIALOG(asgtk_gradient_new());
	GtkWidget *name_edit = make_back_name_hbox(GTK_BOX (ge->vbox), 10);
	int response  = 0;
	const char *name = NULL ;
	Bool files_added = False ;

	gtk_dialog_add_buttons( ge, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
							  	GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, 
								NULL );

	gtk_window_set_title(GTK_WINDOW(ge), "Creating new gradient background ...   ");
	gtk_window_set_modal(GTK_WINDOW(ge), TRUE);
	do
	{	
		response = gtk_dialog_run( ge );
		if( response == GTK_RESPONSE_ACCEPT ) 
		{	
			name = gtk_entry_get_text( GTK_ENTRY(name_edit) );
			if( name == NULL || strlen(name) == 0 ) 
				asgtk_warning2( WallpaperState.main_window, "Empty name specified for a new background.", NULL, NULL ); 	   				   		   			   			
			else
				break;
		}
	}while( response == GTK_RESPONSE_ACCEPT ); 
	
	if( response == GTK_RESPONSE_ACCEPT && name != NULL ) 
	{
		char *mini_xml = NULL ; 
		char *xml = asgtk_gradient_get_xml( ASGTK_GRADIENT(ge), &mini_xml );
		if( xml ) 
		{	
			if( make_xml_from_string( id, name, xml, False ) )
			{
				files_added = True ;	  
				if( mini_xml )
					make_xml_from_string( id, name, mini_xml, True );
			}	 
			free( xml );
			if( mini_xml ) 
				free( mini_xml );
		}	
	}
	gtk_widget_destroy( GTK_WIDGET(ge) );
	if( files_added ) 
	{
		asgtk_info2( WallpaperState.main_window, "New gradient background \"%s\" created.", name, NULL );	  
		asgtk_image_dir_refresh( id );
	}
}


static void
backs_list_sel_handler(ASGtkImageDir *id, gpointer user_data)
{
	ASGtkImageView *iv = ASGTK_IMAGE_VIEW(user_data);
	ASImageListEntry *le;
	g_return_if_fail (ASGTK_IS_IMAGE_DIR (id));
	
	le = asgtk_image_dir_get_selection( id );
	asgtk_image_view_set_entry ( iv, le);

	if( le != NULL ) 
	{	
		if( WallpaperState.xml_editor && le->type == ASIT_XMLScript )
			asgtk_xml_editor_set_entry( ASGTK_XML_EDITOR( WallpaperState.xml_editor), le );
	
		if( iv ) 
		{	
			char *mini_fullname = NULL ; 
			Bool has_mini = False ; 
			if( le->type == ASIT_XMLScript ) 
			{	
				gtk_widget_show(WallpaperState.edit_xml_button);
				gtk_widget_hide(WallpaperState.make_xml_button);
			}else
			{
				gtk_widget_hide(WallpaperState.edit_xml_button);
				gtk_widget_show(WallpaperState.make_xml_button);
			}		   
			if( asgtk_image_dir_make_mini_names( id, le->name, NULL, &mini_fullname ) )
			{
				has_mini = ( CheckFile( mini_fullname ) == 0 );
				free( mini_fullname ); 				   
			}	 
			if( has_mini ) 
			{	
				gtk_button_set_label(GTK_BUTTON(WallpaperState.make_mini_button), "Update mini");
			}else
			{
				gtk_button_set_label(GTK_BUTTON(WallpaperState.make_mini_button), "Make mini");
			}		   
		}
		unref_asimage_list_entry( le );
	}
}


void
create_main_window (void)
{
    GtkWidget *main_vbox;

  	WallpaperState.main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  	gtk_window_set_title (GTK_WINDOW (WallpaperState.main_window), "AfterStep Wallpaper Manager");

 	colorize_gtk_window( WallpaperState.main_window ); 	
		
	main_vbox = gtk_vbox_new (FALSE, 0);
  	gtk_widget_show (main_vbox);
	gtk_container_add (GTK_CONTAINER (WallpaperState.main_window), main_vbox);

  	WallpaperState.list_hbox = gtk_hbox_new (FALSE, 0);
  	gtk_widget_show (WallpaperState.list_hbox);
  	gtk_box_pack_start (GTK_BOX (main_vbox), WallpaperState.list_hbox, TRUE, TRUE, 5);

}

GtkWidget*
create_list_button( GtkWidget *buttons_hbox, const char *stock, GCallback func )
{
	GtkWidget *btn = gtk_button_new_from_stock (stock);
  	gtk_widget_show (btn);
  	gtk_box_pack_start (GTK_BOX (buttons_hbox), btn, FALSE, FALSE, 0);
  	g_signal_connect ((gpointer) btn, "clicked", G_CALLBACK (func), NULL);
	colorize_gtk_widget( GTK_WIDGET(btn), get_colorschemed_style_button());
	return btn;	
}	   

void
create_backs_list()
{
	GtkWidget *vbox, *hbox ;
  	
	vbox = gtk_vbox_new (FALSE, 3);
  	gtk_widget_show (vbox);
  	gtk_box_pack_start (GTK_BOX (WallpaperState.list_hbox), vbox, FALSE, FALSE, 5);

  	WallpaperState.backs_list = asgtk_image_dir_new();
	gtk_box_pack_start (GTK_BOX (vbox), WallpaperState.backs_list, TRUE, TRUE, 0);
	gtk_widget_set_size_request (WallpaperState.backs_list, 200, INITIAL_PREVIEW_HEIGHT);
	gtk_widget_show (WallpaperState.backs_list);

	/* creating the list widget itself */
	asgtk_image_dir_set_title(ASGTK_IMAGE_DIR(WallpaperState.backs_list),"Images in your private backgrounds folder:");
	asgtk_image_dir_set_mini (ASGTK_IMAGE_DIR(WallpaperState.backs_list), ".mini" );
		
	colorize_gtk_widget( WallpaperState.backs_list, get_colorschemed_style_button());
	gtk_widget_set_style( WallpaperState.backs_list, get_colorschemed_style_normal());
	
	/* adding list manipulation buttons : */

	WallpaperState.list_update_as_button = asgtk_add_button_to_box( NULL, GTK_STOCK_REFRESH, "Update AfterStep Menu", G_CALLBACK(on_update_as_menu_clicked), NULL );
  	gtk_box_pack_end (GTK_BOX (vbox), WallpaperState.list_update_as_button, FALSE, FALSE, 0);
	WallpaperState.list_browse_button = asgtk_add_button_to_box( NULL, GTK_STOCK_ADD, "Browse for more", G_CALLBACK(on_browse_clicked), NULL );
  	gtk_box_pack_end (GTK_BOX (vbox), WallpaperState.list_browse_button, FALSE, FALSE, 0);

	hbox = gtk_hbutton_box_new( );
	gtk_widget_show (hbox);
	gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

	WallpaperState.new_solid_button = asgtk_add_button_to_box( GTK_BOX (hbox), GTK_STOCK_ADD, "New solid color", G_CALLBACK(on_solid_clicked), WallpaperState.backs_list );
	WallpaperState.new_gradient_button = asgtk_add_button_to_box( GTK_BOX (hbox), GTK_STOCK_ADD, "New gradient", G_CALLBACK(on_gradient_clicked), WallpaperState.backs_list );
}

void 
create_list_preview()
{
	int preview_width ; 

	WallpaperState.list_preview = asgtk_image_view_new();
	preview_width = (INITIAL_PREVIEW_HEIGHT *Scr.MyDisplayWidth)/Scr.MyDisplayHeight ;
	gtk_widget_set_size_request (WallpaperState.list_preview, preview_width, INITIAL_PREVIEW_HEIGHT);
	asgtk_image_view_set_aspect (ASGTK_IMAGE_VIEW(WallpaperState.list_preview), Scr.MyDisplayWidth, Scr.MyDisplayHeight );
	asgtk_image_view_set_resize (ASGTK_IMAGE_VIEW(WallpaperState.list_preview), 
								 ASGTK_IMAGE_VIEW_SCALE_TO_VIEW|
								 ASGTK_IMAGE_VIEW_TILE_TO_ASPECT, ASGTK_IMAGE_VIEW_RESIZE_ALL );
  	gtk_box_pack_end (GTK_BOX (WallpaperState.list_hbox), WallpaperState.list_preview, TRUE, TRUE, 0);
	gtk_widget_show (WallpaperState.list_preview);

}

void
reload_private_backs_list()
{	
	char *private_back_dir = PutHome("~/.afterstep/backgrounds");
	asgtk_image_dir_set_path(ASGTK_IMAGE_DIR(WallpaperState.backs_list), private_back_dir);
	free( private_back_dir );
}

void
on_destroy(GtkWidget *widget, gpointer user_data)
{
	if( WallpaperState.xml_editor ) 
		gtk_widget_destroy( GTK_WIDGET(WallpaperState.xml_editor) );
		
	gtk_main_quit();
}

void 
init_ASWallpaper()
{
	memset( &WallpaperState, 0x00, sizeof(ASWallpaperState));
	
	create_main_window(); 
	create_backs_list();
	create_list_preview();
	
	WallpaperState.sel_apply_button = asgtk_add_button_to_box( NULL, GTK_STOCK_APPLY, NULL, G_CALLBACK(on_list_apply_clicked), WallpaperState.backs_list );
	WallpaperState.make_xml_button = asgtk_add_button_to_box( NULL, GTK_STOCK_PROPERTIES, "Make XML", G_CALLBACK(on_make_xml_clicked), WallpaperState.backs_list );
	WallpaperState.edit_xml_button = asgtk_add_button_to_box( NULL, GTK_STOCK_PROPERTIES, "Edit XML", G_CALLBACK(on_edit_xml_clicked), WallpaperState.backs_list );
	WallpaperState.make_mini_button = asgtk_add_button_to_box( NULL, GTK_STOCK_PROPERTIES, "Make mini", G_CALLBACK(on_make_mini_clicked), WallpaperState.backs_list );
	
	WallpaperState.sel_del_button = asgtk_add_button_to_box( NULL, GTK_STOCK_DELETE, NULL, G_CALLBACK(on_list_del_clicked), WallpaperState.backs_list );

	gtk_widget_hide(WallpaperState.edit_xml_button);

	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.sel_apply_button, 0 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.make_mini_button, 5 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.make_xml_button, 5 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.edit_xml_button, 5 );
	asgtk_image_view_add_tool( ASGTK_IMAGE_VIEW(WallpaperState.list_preview), WallpaperState.sel_del_button, 5 );
	
	asgtk_image_dir_set_sel_handler( ASGTK_IMAGE_DIR(WallpaperState.backs_list), backs_list_sel_handler, WallpaperState.list_preview);

	reload_private_backs_list();

	g_signal_connect (G_OBJECT (WallpaperState.main_window), "destroy", G_CALLBACK (on_destroy), NULL);
  	gtk_widget_show (WallpaperState.main_window);
}	 

