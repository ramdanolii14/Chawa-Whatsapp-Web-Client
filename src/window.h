#pragma once
#include <gtk/gtk.h>

#define CHAWA_TYPE_WINDOW (chawa_window_get_type())
G_DECLARE_FINAL_TYPE(ChawaWindow, chawa_window, CHAWA, WINDOW, GtkApplicationWindow)

GtkWidget *chawa_window_new(GtkApplication *app);