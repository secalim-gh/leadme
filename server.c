#include <stdlib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef USE_LAYER_SHELL
#include <gtk-layer-shell.h>
#endif

#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define SOCK_PATH "/tmp/leadme.sock"
#define LOCK_PATH "/tmp/leadme.lock"

#define PATH_LEN 256

int busy = 0;

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

  double x      = width/4,        /* parameters like cairo_rectangle */
  y							= height/4,
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

void widget(void) {
	char path[PATH_LEN];
	sprintf(path, "%s/.config/leadme/config", getenv("HOME"));
  load_config(path);
  gtk_init(NULL, NULL);

  GtkWindow *window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

  GtkWidget *menu = gtk_menu_new();
  GtkWidget *item_quit = gtk_menu_item_new_with_label("Quit");
  g_signal_connect(item_quit, "activate", G_CALLBACK(on_quit), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_quit);
  gtk_widget_show_all(menu);

  AppState *state = g_new0(AppState, 1);
  g_object_set_data_full(G_OBJECT(window), "state", state, g_free);

  gtk_window_set_default_size(window, 400, 400);
  gtk_window_set_resizable(window, FALSE);
  gtk_window_set_decorated(window, FALSE);

#ifdef USE_LAYER_SHELL
	gtk_layer_init_for_window(window);
	gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_OVERLAY);
	gtk_layer_set_keyboard_mode(window, GTK_LAYER_SHELL_KEYBOARD_MODE_EXCLUSIVE);

  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, FALSE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_BOTTOM, FALSE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, FALSE);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_RIGHT, FALSE);
#else 
	// X11-specific setup
  GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(window));
  GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
  if (visual != NULL && gdk_screen_is_composited(screen)) {
      gtk_widget_set_visual(GTK_WIDGET(window), visual);
  }
	gtk_window_set_keep_above(window, TRUE);
	gtk_window_set_position(window, GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_type_hint(window, GDK_WINDOW_TYPE_HINT_DIALOG);
#endif
  gtk_widget_set_app_paintable(GTK_WIDGET(window), TRUE);

  g_signal_connect(G_OBJECT(window), "draw", G_CALLBACK(on_draw), NULL);
  g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_widget_show_all(GTK_WIDGET(window));
  gtk_main();

}

void sigchld_handler(int sig) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
	busy = 0;
}

void server(void) {
	int lock_fd = open(LOCK_PATH, O_CREAT | O_RDWR, 0666);
	if (flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
		printf("Another instance is already running.\n");
		return;
	}
	int s, s2, len;
	struct sockaddr_un remote, local = {
		.sun_family = AF_UNIX,
	};
	char str[100];

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	strcpy(local.sun_path, SOCK_PATH);
	unlink(local.sun_path);
	len = strlen(local.sun_path) + sizeof(local.sun_family);
	if (bind(s, (struct sockaddr *)&local, len) < 0) {
		perror("bind");
		exit(1);
	}

	if (listen(s, 5) == -1) {
		perror("listen");
		exit(1);
	}


	struct sigaction sa = {.sa_handler = sigchld_handler, .sa_flags = SA_RESTART | SA_NOCLDSTOP};
  sigaction(SIGCHLD, &sa, NULL);

	char *home = getenv("HOME");
	char *old_path = getenv("PATH");

	if (home && old_path) {
		char new_path[1024];
		// Combine current PATH with your local bin and Applications folders
		snprintf(new_path, sizeof(new_path), "%s:%s/.local/bin:%s/Applications", 
					 old_path, home, home);
		setenv("PATH", new_path, 1);
	}
	
	for(;;) {
		socklen_t slen = sizeof(remote);
		if ((s2 = accept(s, (struct sockaddr *)&remote, &slen)) == -1) {
			if (errno == EINTR) continue;
			perror("accept");
			exit(1);
		}

		if (busy) {
			send(s2, "FAILURE", 7, 0);
		} else {
			send(s2, "SUCCESS", 7, 0);
			busy = 1;
			pid_t pid = fork();
			if (-1 == pid) {
				perror("Fork error");
				exit(EXIT_FAILURE);
			}
			if (0 == pid) {
				close(s);
				widget();
				exit(EXIT_SUCCESS);
			}
		}

		close(s2);
	}
	return;
}
