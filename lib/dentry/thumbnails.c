#include <gtk/gtk.h>

#define GNOME_DESKTOP_USE_UNSTABLE_API
#include "gnome-desktop-thumbnail.h"

static GnomeDesktopThumbnailFactory *
get_thumbnail_factory ()
{
    static GnomeDesktopThumbnailFactory *thumbnail_factory = NULL;

    if (thumbnail_factory == NULL) {
        thumbnail_factory = gnome_desktop_thumbnail_factory_new (GNOME_DESKTOP_THUMBNAIL_SIZE_NORMAL);
    }

    return thumbnail_factory;
}

gboolean
gfile_can_thumbnail (GFile* file)
{
    GnomeDesktopThumbnailFactory *factory;
    gboolean can_thumbnail;
    char* uri;
    GFileInfo* info;
    time_t mtime;
    const char* content_type;
    char* mime_type;

    uri = g_file_get_uri (file);

    info = g_file_query_info (file, "standard::content-type,time::modified",
                              G_FILE_QUERY_INFO_NONE,
                              NULL, NULL);
    content_type = g_file_info_get_content_type (info);
    mime_type = g_content_type_get_mime_type (content_type);

    mtime = g_file_info_get_attribute_uint64(info, "time::modified");
    g_object_unref (info);

    factory = get_thumbnail_factory ();
    //1' check if we can thumbnail
    can_thumbnail = gnome_desktop_thumbnail_factory_can_thumbnail (factory,
                                                         uri,
                                                         mime_type,
                                                         mtime);
    g_debug ("%s can thumbnail(mime: %s): %d", uri, mime_type, can_thumbnail);
    g_free (uri);
    g_free (mime_type);

    return can_thumbnail;
}
/*
 *      syncronously create thumbnails. shall we move to a threaded
 *      implementation?
 */
#define THUMBNAIL_CREATION_DELAY 3
char*
gfile_lookup_thumbnail (GFile* file)
{
    char* thumbnail_path = NULL;

    GnomeDesktopThumbnailFactory *factory;
    //gboolean can_thumbnail;
    char* uri;
    GFileInfo* info;
    time_t mtime;
    const char* content_type;
    char* mime_type;

    uri = g_file_get_uri (file);

    info = g_file_query_info (file, "standard::content-type,time::modified",
                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                              NULL, NULL);
    content_type = g_file_info_get_content_type (info);
    mime_type = g_content_type_get_mime_type (content_type);

    mtime = g_file_info_get_attribute_uint64(info, "time::modified");
    g_object_unref (info);

    factory = get_thumbnail_factory ();
#if 0
    //1' check if we can thumbnail
    can_thumbnail = gnome_desktop_thumbnail_factory_can_thumbnail (factory,
                                                         uri,
                                                         mime_type,
                                                         mtime);
    g_debug ("%s can thumbnail(mime: %s): %d", uri, mime_type, can_thumbnail);
    if (can_thumbnail == FALSE)
    {
	g_free (uri);
	g_free (mime_type);
	return NULL;
    }
#endif
    //2' lookup existing thumbnail
    thumbnail_path = gnome_desktop_thumbnail_factory_lookup (factory, uri, mtime);
    g_debug ("uri: %s\nthumbnail_path: %s\n", uri, thumbnail_path);

    //3' create thumbnail if not exist
    if (thumbnail_path == NULL)
    {
        time_t current_time = 0;
        time (&current_time);
        if (current_time - mtime < THUMBNAIL_CREATION_DELAY)
        {
            return NULL;
        }
        //try to create thumbnails
        GdkPixbuf *pixbuf;
        pixbuf = gnome_desktop_thumbnail_factory_generate_thumbnail (factory, uri, mime_type);

        if (pixbuf)
        {
            gnome_desktop_thumbnail_factory_save_thumbnail (factory, pixbuf, uri, mtime);
            g_object_unref (pixbuf);
        }
        else
        {
            gnome_desktop_thumbnail_factory_create_failed_thumbnail (factory, uri, mtime);
        }
        thumbnail_path = gnome_desktop_thumbnail_factory_lookup (factory, uri, mtime);
    }
    g_free (uri);
    g_free (mime_type);

    g_debug ("thumbnail_path: %s", thumbnail_path);

    return thumbnail_path;
}

