#include <ctype.h>
#include <gtk/gtk.h>
#include <gtk-layer-shell.h>
#include <stdlib.h>
#include <math.h>
#include <libappindicator3-0.1/libappindicator/app-indicator.h>

#include "parser.h"

typedef struct {
  gboolean shaking;
  int shake_offset;
  int shake_count;
} AppState;

static gboolean shake_timeout(gpointer data) {
  GtkWidget *widget = GTK_WIDGET(data);
  AppState *state = g_object_get_data(G_OBJECT(widget), "state");

  if (state->shake_count > 0) {
    // Alternate offset: +10, -10, +8, -8, +6, -6, etc.
    state->shake_offset = (state->shake_count % 2 == 0) ? 2 : -2;
    state->shake_offset = state->shake_offset * (state->shake_count / 2 + 1) / 3;
    state->shake_count--;
    gtk_widget_queue_draw(widget);
    return TRUE; // Continue
  } else {
    state->shaking = FALSE;
    state->shake_offset = 0;
    gtk_widget_queue_draw(widget);
    return FALSE; // Stop
  }
}

static void start_shake(GtkWidget *widget) {
  AppState *state = g_object_get_data(G_OBJECT(widget), "state");
  if (!state->shaking) {
    state->shaking = TRUE;
    state->shake_count = 6; // Number of shakes
    g_timeout_add(50, shake_timeout, widget); // 50ms intervals
  }
}

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
  AppState *state = g_object_get_data(G_OBJECT(widget), "state");
  int width = gtk_widget_get_allocated_width(widget);
  int height = gtk_widget_get_allocated_height(widget);

  double x         = width/4,        /* parameters like cairo_rectangle */
  y         = height/4,
  aspect        = 0.5,/* aspect ratio */
  corner_radius = height / 20.0;   /* and corner curvature radius */

  double radius = corner_radius / aspect;
  double degrees = M_PI / 180.0;
  width /= 2; height /= 2;
  cairo_new_sub_path (cr);
  cairo_arc (cr, state->shake_offset + x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
  cairo_arc (cr, state->shake_offset + x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
  cairo_arc (cr, state->shake_offset + x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
  cairo_arc (cr, state->shake_offset + x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path (cr);

  cairo_set_source_rgba (cr, 0x23/255.f, 0x1F/255.f, 0x29/255.f, 0.9);
  cairo_fill_preserve (cr);
  cairo_set_source_rgba (cr, 0x5D/255.f, 0x28/255.f, 0x42/255.f, 0.98);
  cairo_set_line_width (cr, 10.0);
  cairo_stroke (cr);
  return FALSE;
}

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
  if (event->keyval == GDK_KEY_Escape || event->keyval == GDK_KEY_q) {
    gtk_main_quit();
    return TRUE;
  }

  if (!isprint(event->keyval)) return TRUE;

  if (exec(event->keyval)) {
    gtk_main_quit();
  } else {
    start_shake(widget);
  }
  return TRUE;
}

static void on_quit(GtkMenuItem *item, gpointer user_data) {
  gtk_main_quit();
}

int main(int argc, char *argv[]) {
  load_config("/home/secalim/.config/leadme/config");
  gtk_init(&argc, &argv);

  GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

  AppIndicator *indicator = app_indicator_new(
    "my-app-id",
    "/home/caralis/.local/share/leadme/leadme_flat2.png",
    APP_INDICATOR_CATEGORY_APPLICATION_STATUS
  );
  // Create menu
  GtkWidget *menu = gtk_menu_new();
  GtkWidget *item_quit = gtk_menu_item_new_with_label("Quit");
  g_signal_connect(item_quit, "activate", G_CALLBACK(on_quit), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_quit);
  gtk_widget_show_all(menu);

  app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_menu(indicator, GTK_MENU(menu));

  // Create and attach state
  AppState *state = g_new0(AppState, 1);
  g_object_set_data_full(G_OBJECT(window), "state", state, g_free);

  // Setup layer shell
  gtk_layer_init_for_window(window);
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_OVERLAY);
  gtk_layer_set_keyboard_mode(window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

  // Set size
  gtk_window_set_default_size(window, 400, 400);
  gtk_window_set_resizable(window, FALSE);
  gtk_window_set_decorated(window, FALSE);

  // Center the window
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, FALSE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);

  gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);

  // Connect signals
  g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(on_draw), NULL);
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(GTK_WIDGET(window));
  gtk_main();

  return 0;
}

