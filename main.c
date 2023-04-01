#include "show_image.h"

#define FILE_BYTE_ORDER EXIF_BYTE_ORDER_INTEL

static void activate(GtkApplication* app, gpointer user_data)
{
    builder = gtk_builder_new();

    gtk_builder_add_from_file(builder, "show_image.glade", NULL);

    mainwindow = GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(builder), "mainwindow"));
    image1 = GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(builder), "image1"));
    viewport1 = GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(builder), "viewport1"));
    list_properties = GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(builder), "list_properties"));
    storeImgProps = GTK_LIST_STORE(gtk_builder_get_object (GTK_BUILDER(builder), "storeImgProps"));
    
    gtk_builder_connect_signals(builder, NULL);

    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(mainwindow));

    gtk_widget_show(mainwindow);

}

void on_itmSair_activate(GtkMenuItem *menu, gpointer user_data)
{
    g_application_quit(G_APPLICATION(app));
}

void on_itmAbrirImagem_activate(GtkMenuItem *menu, gpointer user_data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    gint res, viewport_width, viewport_height, pixbuf_width, pixbuf_height, scale_x, scale_y;
    FILE *fp;
    int file_size = 0, ch, i = 0;

    dialog = gtk_file_chooser_dialog_new("Abrir Imagem", GTK_WINDOW(mainwindow),  GTK_FILE_CHOOSER_ACTION_OPEN, "_Abrir", GTK_RESPONSE_OK, "_Cancelar",
     GTK_RESPONSE_CANCEL, NULL);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.JPG");
    gtk_file_filter_add_pattern(filter, "*.JPEG");
    gtk_file_filter_set_name(filter, "Todos Arquivos de Imagem");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_set_name(filter, "Imagens jpg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_set_name(filter, "Imagens jpeg");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.png");
    gtk_file_filter_set_name(filter, "Imagens png");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.*");
    gtk_file_filter_set_name(filter, "Todos Arquivos");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    gtk_file_chooser_set_current_folder(chooser, current_folder);
        
    res = gtk_dialog_run (GTK_DIALOG (dialog));

    if (res == GTK_RESPONSE_OK)
    {
        int image_width, image_height;
        GtkWidget *image;
        GdkPixbuf *pixbuf, *pixbuf1, *pixbuf2;
        gint width, height;
        char title[100] = "", **data;
        
        filename = gtk_file_chooser_get_filename (chooser);
        current_folder = gtk_file_chooser_get_current_folder(chooser);
        gtk_file_chooser_set_current_folder(chooser, current_folder);
        
        main_pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
        image_width = gdk_pixbuf_get_width(main_pixbuf);
        image_height = gdk_pixbuf_get_height(main_pixbuf);

        sprintf(title, "Image Browser - %s", g_path_get_basename(filename));
        rotation = GDK_PIXBUF_ROTATE_NONE;
        
        read_exif();

        pixbuf1 = gdk_pixbuf_new_from_file_at_scale(filename, 1024, 768, TRUE, NULL);
        pixbuf = gdk_pixbuf_rotate_simple(pixbuf1, rotation);
        gtk_image_set_from_pixbuf(GTK_IMAGE(image1), pixbuf);

        gtk_window_set_title(GTK_WINDOW(mainwindow), title);

    }

    gtk_widget_destroy (dialog);

}

/* Remove spaces on the right of the string */
static void trim_spaces(char *buf)
{
    char *s = buf-1;
    for (; *buf; ++buf) {
        if (*buf != ' ')
            s = buf;
    }
    *++s = 0; /* nul terminate the string on the first of the final spaces */
}

/* Show the tag name and contents if the tag exists */
static void show_tag(ExifData *d, ExifIfd ifd, ExifTag tag)
{
    GtkTreeIter iter;
    char indent[100];
    
    /* See if this tag exists */
    ExifEntry *entry = exif_content_get_entry(d->ifd[ifd],tag);

    if (tag == EXIF_TAG_ORIENTATION)
    {
        ExifByteOrder order = exif_data_get_byte_order(d);
        ExifShort val = exif_get_short(entry->data, exif_data_get_byte_order(d));
        
        rotation = GDK_PIXBUF_ROTATE_NONE;
        
        switch (val)
        {
            case 1: //Horizontal (normal) No rotation.
                rotation = GDK_PIXBUF_ROTATE_NONE;
                break;
            case 2: //Mirror horizontal Rotate by 180 degrees.
                rotation = GDK_PIXBUF_ROTATE_UPSIDEDOWN;
                break;
            case 3: //Rotate 180
                rotation = GDK_PIXBUF_ROTATE_UPSIDEDOWN;
                break;
            case 4: //Mirror vertical
                rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
                break;
            case 5: //Mirror horizontal and rotate 270 CW
                rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;
                break;
            case 6: //Rotate 90 CW
                rotation = GDK_PIXBUF_ROTATE_CLOCKWISE;//GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;;
                break;
            case 7: //Mirror horizontal and rotate 90 CW
                rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;
                break;
            case 8: //Rotate 270 CW
                rotation = GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE;//GDK_PIXBUF_ROTATE_CLOCKWISE;
                break;
            
            default:
                rotation = GDK_PIXBUF_ROTATE_NONE;
                break;
        }
    }
    
    if (entry) {
        char buf[1024];
        
        /* Get the contents of the tag in human-readable form */
        exif_entry_get_value(entry, buf, sizeof(buf));
        
        /* Don't bother printing it if it's entirely blank */
        trim_spaces(buf);
        
        if (*buf)
        {
            gtk_list_store_append(GTK_LIST_STORE(storeImgProps), &iter);
            gtk_list_store_set(GTK_LIST_STORE(storeImgProps), &iter, 0, exif_tag_get_name_in_ifd(tag,ifd), 1, buf, -1);
        }
    }
}

void read_exif()
{
    ExifData *ed;
    ExifEntry *entry;
    
    gtk_list_store_clear(GTK_LIST_STORE(storeImgProps));

    ed = exif_data_new_from_file(filename);
    if (!ed) {
        printf("File not readable or no EXIF data in file\n");
        return;
    }

    /* Show all the tags that might contain information about the photographer */
    /* Grupo 0 */
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_ORIENTATION);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_FNUMBER);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_ARTIST);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_XP_AUTHOR);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_COPYRIGHT);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_IMAGE_LENGTH);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_IMAGE_WIDTH);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_IMAGE_DESCRIPTION);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_BITS_PER_SAMPLE);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_COMPRESSION);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_PHOTOMETRIC_INTERPRETATION);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_SAMPLES_PER_PIXEL);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_X_RESOLUTION);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_Y_RESOLUTION);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_RESOLUTION_UNIT);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_SOFTWARE);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKE);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_MAKER_NOTE);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_MODEL);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_DATE_TIME);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_YCBCR_COEFFICIENTS);
    show_tag(ed, EXIF_IFD_0, EXIF_TAG_YCBCR_SUB_SAMPLING);

    /* Grupo EXIF */
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_APERTURE_VALUE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FOCAL_LENGTH_IN_35MM_FILM);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_DIGITAL_ZOOM_RATIO);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_TIME);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_PROGRAM);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_ISO_SPEED_RATINGS);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_DIGITIZED);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FNUMBER);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_MAX_APERTURE_VALUE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_METERING_MODE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_LIGHT_SOURCE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FLASH);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FLASH_PIX_VERSION);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FLASH_ENERGY);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_SCENE_TYPE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_SCENE_CAPTURE_TYPE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXPOSURE_MODE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_WHITE_BALANCE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_EXIF_VERSION);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_USER_COMMENT);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_FILE_SOURCE);
    show_tag(ed, EXIF_IFD_EXIF, EXIF_TAG_CUSTOM_RENDERED);
    
    exif_data_unref(ed);
}

int main(int argc, char **argv)
{
    app = gtk_application_new("org.gtk.show_message", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
    
    g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);
}

/*
https://docs.gtk.org/gdk-pixbuf/method.Pixbuf.save.html
https://docs.gtk.org/gtk3/iface.TreeModel.html
https://github.com/GNOME/gdk-pixbuf/blob/master/gdk-pixbuf/io-jpeg.c
https://www.media.mit.edu/pia/Research/deepview/exif.html
https://docs.gtk.org/gdk-pixbuf/class.Pixbuf.html#image-data
*/