#ifndef SHOW_IMAGE_H
#define SHOW_IMAGE_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <libexif/exif-data.h>
#include <libexif/exif-utils.h>

GtkApplication *app;
GtkBuilder *builder; 
GtkWidget *mainwindow, *image1, *viewport1, *box, *list_properties;
GError **error;
GtkTreeModel *storeImgProps;
GdkPixbuf *main_pixbuf;

ExifData *ed;
ExifEntry *entry;

char *current_folder = "./", *filename;
int rotation, image_width, image_height;

void read_exif();

#endif
