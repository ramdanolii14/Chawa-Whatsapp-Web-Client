#pragma once
#include <gtk/gtk.h>

void chawa_notify_init    (void);
void chawa_notify_send    (const char *title, const char *body,
                           const char *icon_data, GtkWindow *win);
void chawa_notify_cleanup (void);