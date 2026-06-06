#include "notify.h"
#include <libnotify/notify.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct {
    GtkWindow          *window;
    NotifyNotification *notif;
} Ctx;

static void
on_action(NotifyNotification *n G_GNUC_UNUSED,
          char               *a G_GNUC_UNUSED,
          gpointer            ud)
{
    Ctx *ctx = ud;
    if (ctx->window)
        gtk_window_present(ctx->window);
}

static void
on_closed(NotifyNotification *n, gpointer ud)
{
    /* fired oleh daemon setelah notif hilang (timeout / klik / dismiss) */
    g_free(ud);
    g_object_unref(n);   /* sekarang aman di-free */
}

/* "data:<mime>;base64,<data>" → GdkPixbuf, NULL jika gagal */
static GdkPixbuf *
pixbuf_from_data_url(const char *url)
{
    if (!url || !g_str_has_prefix(url, "data:")) return NULL;

    const char *comma = strchr(url, ',');
    if (!comma) return NULL;

    gsize len = 0;
    guchar *raw = g_base64_decode(comma + 1, &len);
    if (!raw || !len) { g_free(raw); return NULL; }

    GInputStream *s = g_memory_input_stream_new_from_data(
                            raw, (gssize)len, g_free);
    GdkPixbuf *pb = gdk_pixbuf_new_from_stream(s, NULL, NULL);
    g_object_unref(s);
    return pb;
}

void chawa_notify_init    (void) { notify_init("Chawa"); }
void chawa_notify_cleanup (void) { notify_uninit(); }

void
chawa_notify_send(const char *title, const char *body,
                  const char *icon_data, GtkWindow *win)
{
    if (!title || !*title) return;

    NotifyNotification *n = notify_notification_new(
        title,
        (body && *body) ? body : NULL,
        "chawa");

    notify_notification_set_urgency(n, NOTIFY_URGENCY_NORMAL);
    notify_notification_set_timeout(n, NOTIFY_EXPIRES_DEFAULT);

    /* ── Foto profil ── */
    GdkPixbuf *pb = pixbuf_from_data_url(icon_data);
    if (pb) {
        /* Crop lingkaran 48×48 */
        GdkPixbuf *sc = gdk_pixbuf_scale_simple(
                            pb, 48, 48, GDK_INTERP_BILINEAR);
        g_object_unref(pb);
        if (sc) {
            notify_notification_set_image_from_pixbuf(n, sc);
            g_object_unref(sc);
        }
    }

    /* ── Action: klik → fokus window ── */
    Ctx *ctx = g_new(Ctx, 1);
    ctx->window = win;
    ctx->notif  = n;

    notify_notification_add_action(
        n, "default", "Buka", on_action, ctx, NULL);

    /* closed SELALU fire oleh daemon → ini tempat cleanup-nya */
    g_signal_connect(n, "closed", G_CALLBACK(on_closed), ctx);

    /* Jangan unref n di sini — on_closed yang akan lakukan */
    notify_notification_show(n, NULL);
}