#include "window.h"
#include "notify.h"
#include <webkit/webkit.h>
#include <string.h>

#define WHATSAPP_URL "https://web.whatsapp.com"

#define CHAWA_UA \
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 " \
    "(KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36"

#define JS_NOTIFY_HOOK \
    "(function(){"                                                          \
    "  var _N=window.Notification;"                                         \
    "  function ChawaNotif(t,o){"                                           \
    "    o=o||{};"                                                          \
    "    try{"                                                              \
    "      window.webkit.messageHandlers.chawaNotify.postMessage("          \
    "        String(t)+'\\x01'+String(o.body||'')"                         \
    "        +'\\x01'+String(o.icon||''));"                                 \
    "    }catch(e){}"                                                       \
    "    if(typeof _N==='function')try{return new _N(t,o);}catch(e){}"     \
    "  }"                                                                   \
    "  ChawaNotif.permission='granted';"                                    \
    "  ChawaNotif.requestPermission=function(){"                            \
    "    return Promise.resolve('granted');"                                \
    "  };"                                                                  \
    "  if(_N)ChawaNotif.maxActions=_N.maxActions||2;"                       \
    "  window.Notification=ChawaNotif;"                                     \
    "})();"

struct _ChawaWindow {
    GtkApplicationWindow parent_instance;
    WebKitWebView       *webview;
};

G_DEFINE_TYPE(ChawaWindow, chawa_window, GTK_TYPE_APPLICATION_WINDOW)

/* ───────────────────────────── signal handlers ───────────────────────────── */

static void
on_notify_received(WebKitUserContentManager *mgr G_GNUC_UNUSED,
                   JSCValue                 *val,
                   gpointer                  ud)
{
    char *msg = jsc_value_to_string(val);
    if (!msg) return;

    /* Format: title\x01body\x01icon_data_url */
    char *title = msg;
    char *body  = "";
    char *icon  = "";

    char *p1 = strchr(msg, '\x01');
    if (p1) {
        *p1 = '\0';
        body = p1 + 1;
        char *p2 = strchr(body, '\x01');
        if (p2) {
            *p2 = '\0';
            icon = p2 + 1;
        }
    }

    /* ud = ChawaWindow* */
    chawa_notify_send(title, body, icon, GTK_WINDOW(ud));
    g_free(msg);
}

/* Belt-and-suspenders: inject ulang hook setiap halaman committed */
static void
on_load_changed(WebKitWebView  *wv,
                WebKitLoadEvent ev,
                gpointer        ud G_GNUC_UNUSED)
{
    if (ev == WEBKIT_LOAD_COMMITTED)
        webkit_web_view_evaluate_javascript(wv, JS_NOTIFY_HOOK, -1,
            NULL, NULL, NULL, NULL, NULL);
}

static gboolean
on_policy(WebKitWebView            *wv  G_GNUC_UNUSED,
          WebKitPolicyDecision     *dec,
          WebKitPolicyDecisionType  type,
          gpointer                  ud  G_GNUC_UNUSED)
{
    if (type != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION)
        return FALSE;

    WebKitNavigationAction *act =
        webkit_navigation_policy_decision_get_navigation_action(
            WEBKIT_NAVIGATION_POLICY_DECISION(dec));

    /* Hanya intercept navigasi yang dipicu klik link oleh user.
     * Navigasi internal WebKit (reload, redirect, form submit, dsb.)
     * dibiarkan berjalan normal agar tidak membuka browser secara
     * tiba-tiba saat app pertama kali load. */
    WebKitNavigationType nav_type = webkit_navigation_action_get_navigation_type(act);
    if (nav_type != WEBKIT_NAVIGATION_TYPE_LINK_CLICKED)
        return FALSE;

    const char *uri =
        webkit_uri_request_get_uri(webkit_navigation_action_get_request(act));

    if (!uri || !*uri) return FALSE;

    /* URL WhatsApp Web & skema internal → izinkan di dalam app */
    if (g_str_has_prefix(uri, "https://web.whatsapp.com")  ||
        g_str_has_prefix(uri, "https://www.whatsapp.com")  ||
        g_str_has_prefix(uri, "blob:")                     ||
        g_str_has_prefix(uri, "data:")                     ||
        g_str_has_prefix(uri, "about:"))
        return FALSE;

    /* Link eksternal yang diklik user → buka di browser sistem */
    GError *err = NULL;
    g_app_info_launch_default_for_uri(uri, NULL, &err);
    if (err) g_error_free(err);
    webkit_policy_decision_ignore(dec);
    return TRUE;
}

static gboolean
on_permission(WebKitWebView           *wv  G_GNUC_UNUSED,
              WebKitPermissionRequest  *req,
              gpointer                  ud  G_GNUC_UNUSED)
{
    /* Auto-allow: notifikasi, mikrofon, kamera */
    webkit_permission_request_allow(req);
    return TRUE;
}

static void
on_process_crash(WebKitWebView                    *wv,
                 WebKitWebProcessTerminationReason reason G_GNUC_UNUSED,
                 gpointer                          ud     G_GNUC_UNUSED)
{
    /* Reload otomatis jika web process crash */
    webkit_web_view_reload(wv);
}

/* ─────────────────────────── GObject boilerplate ────────────────────────── */

static void
chawa_window_class_init(ChawaWindowClass *klass G_GNUC_UNUSED) {}

static void
chawa_window_init(ChawaWindow *self)
{
    gtk_window_set_title(GTK_WINDOW(self), "WhatsApp");
    gtk_window_set_default_size(GTK_WINDOW(self), 1100, 750);

    /* ── User Content Manager: jembatan JS ↔ native ── */
    WebKitUserContentManager *ucm = webkit_user_content_manager_new();

    webkit_user_content_manager_register_script_message_handler(
        ucm, "chawaNotify", NULL);

    /* self sebagai user data agar on_notify_received bisa present window */
    g_signal_connect(ucm, "script-message-received::chawaNotify",
                     G_CALLBACK(on_notify_received), self);

    /* Inject hook SEBELUM script halaman berjalan */
    WebKitUserScript *hook = webkit_user_script_new(
        JS_NOTIFY_HOOK,
        WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
        NULL, NULL);
    webkit_user_content_manager_add_script(ucm, hook);
    webkit_user_script_unref(hook);

    /* ── WebView ── */
    self->webview = WEBKIT_WEB_VIEW(
        g_object_new(WEBKIT_TYPE_WEB_VIEW,
            "user-content-manager", ucm,
            NULL));
    g_object_unref(ucm);

    /* ── Settings / optimasi ── */
    WebKitSettings *s = webkit_web_view_get_settings(self->webview);

    /* GPU acceleration – VA-API / Mesa AMD */
    webkit_settings_set_hardware_acceleration_policy(s,
        WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS);

    /* Fungsionalitas */
    webkit_settings_set_user_agent(s, CHAWA_UA);
    webkit_settings_set_enable_webgl(s, TRUE);
    webkit_settings_set_enable_media_stream(s, TRUE);
    webkit_settings_set_enable_javascript(s, TRUE);

    /* Keamanan / minimalis */
    webkit_settings_set_javascript_can_open_windows_automatically(s, FALSE);
    webkit_settings_set_enable_developer_extras(s, FALSE);
    webkit_settings_set_enable_write_console_messages_to_stdout(s, FALSE);
    webkit_settings_set_allow_modal_dialogs(s, FALSE);

    /* ── Signals ── */
    g_signal_connect(self->webview, "load-changed",
                     G_CALLBACK(on_load_changed),  NULL);
    g_signal_connect(self->webview, "decide-policy",
                     G_CALLBACK(on_policy),         NULL);
    g_signal_connect(self->webview, "permission-request",
                     G_CALLBACK(on_permission),     NULL);
    g_signal_connect(self->webview, "web-process-terminated",
                     G_CALLBACK(on_process_crash),  NULL);

    gtk_window_set_child(GTK_WINDOW(self), GTK_WIDGET(self->webview));
    webkit_web_view_load_uri(self->webview, WHATSAPP_URL);
}

GtkWidget *
chawa_window_new(GtkApplication *app)
{
    return g_object_new(CHAWA_TYPE_WINDOW, "application", app, NULL);
}