#include <gtk/gtk.h>
#include "window.h"
#include "notify.h"

static void
on_activate(GApplication *app, gpointer ud G_GNUC_UNUSED)
{
    /* Single-instance: jika window sudah ada, fokuskan saja */
    GtkWindow *win = gtk_application_get_active_window(GTK_APPLICATION(app));
    if (win) {
        gtk_window_present(win);
        return;
    }
    gtk_window_present(GTK_WINDOW(chawa_window_new(GTK_APPLICATION(app))));
}

int main(int argc, char *argv[])
{
    /*
     * VA-API / Mesa AMD (radeonsi)
     * g_setenv dengan overwrite=FALSE berarti tidak menimpa
     * jika user sudah set via environment sendiri.
     */
    g_setenv("GST_VAAPI_ALL_DRIVERS",       "1",         FALSE);
    g_setenv("LIBVA_DRIVER_NAME",           "radeonsi",  FALSE);
    g_setenv("MESA_LOADER_DRIVER_OVERRIDE", "radeonsi",  FALSE);
    /* Aktifkan hardware video decode di WebKit */
    g_setenv("WEBKIT_DISABLE_COMPOSITING_MODE", "0",     FALSE);

    chawa_notify_init();

    GtkApplication *app = gtk_application_new(
        "id.my.ramdanolii.chawa",
        G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int ret = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    chawa_notify_cleanup();
    return ret;
}