#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-client.h>
#include <cairo/cairo.h>
#include <linux/input-event-codes.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

// Globals
static struct wl_display *display;
static struct wl_compositor *compositor;
static struct wl_shm *shm;
static struct zwlr_layer_shell_v1 *layer_shell;

static struct wl_surface *surface;
static struct zwlr_layer_surface_v1 *layer_surface;

static int configured = 0;
static uint32_t configure_serial = 0;
static int width = 200;
static int height = 200;

#define M_PI (3.14159265358979323846264338327950288)

// ----------------------------------------------
// Registry handlers
// ----------------------------------------------
static void registry_handler(void *data, struct wl_registry *registry,
                             uint32_t id, const char *interface, uint32_t version) {
  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    layer_shell = wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 1);
  }
}

static void registry_remover(void *data, struct wl_registry *registry, uint32_t id) {
  (void)data;
  (void)registry;
  (void)id;
}

static const struct wl_registry_listener registry_listener = {
  .global = registry_handler,
  .global_remove = registry_remover,
};

// ----------------------------------------------
// Layer surface handlers
// ----------------------------------------------
static void handle_layer_surface_configure(void *data,
                                           struct zwlr_layer_surface_v1 *surf,
                                           uint32_t serial,
                                           uint32_t new_width,
                                           uint32_t new_height) {
  configure_serial = serial;
  configured = 1;
  zwlr_layer_surface_v1_ack_configure(surf, serial);
}

static void handle_layer_surface_closed(void *data,
                                        struct zwlr_layer_surface_v1 *surf) {
  (void)data;
  (void)surf;
  wl_display_disconnect(display);
  exit(0);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
  .configure = handle_layer_surface_configure,
  .closed = handle_layer_surface_closed,
};

// ----------------------------------------------
// Simple SHM buffer helper
// ----------------------------------------------
#include <sys/mman.h>
#include <fcntl.h>

static int create_shm_file(size_t size) {
  char name[] = "/leaderd-shm-XXXXXX";
  int fd = mkstemp(name);
  if (fd < 0)
    return -1;
  unlink(name);
  if (ftruncate(fd, size) < 0) {
    close(fd);
    return -1;
  }
  return fd;
}

static struct wl_buffer *create_buffer(int width, int height, cairo_t **out_cr, cairo_surface_t **out_surface) {
  int stride = width * 4;
  int size = stride * height;

  int fd = create_shm_file(size);
  void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
  struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0,
                                                       width, height, stride,
                                                       WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  close(fd);

  *out_surface = cairo_image_surface_create_for_data(data, CAIRO_FORMAT_ARGB32,
                                                     width, height, stride);
  *out_cr = cairo_create(*out_surface);
  return buffer;
}

// ----------------------------------------------
// Draw rounded rectangle with Cairo
// ----------------------------------------------
static void draw(cairo_t *cr) {
  double r = 20.0; // corner radius
  cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 0.7);
  cairo_new_path(cr);
  cairo_move_to(cr, r, 0);
  cairo_arc(cr, width - r, r, r, -M_PI/2, 0);
  cairo_arc(cr, width - r, height - r, r, 0, M_PI/2);
  cairo_arc(cr, r, height - r, r, M_PI/2, M_PI);
  cairo_arc(cr, r, r, r, M_PI, 1.5*M_PI);
  cairo_close_path(cr);
  cairo_fill(cr);
}

// ----------------------------------------------
// Main
// ----------------------------------------------
int main(void) {
  display = wl_display_connect(NULL);
  if (!display) {
    fprintf(stderr, "Can't connect to Wayland display\n");
    return 1;
  }

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display);

  if (!compositor || !layer_shell || !shm) {
    fprintf(stderr, "Missing required Wayland globals\n");
    return 1;
  }

  surface = wl_compositor_create_surface(compositor);
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
    layer_shell, surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "leaderd");

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, NULL);
  zwlr_layer_surface_v1_set_size(layer_surface, width, height);
  zwlr_layer_surface_v1_set_anchor(layer_surface, 0);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
  wl_surface_commit(surface);

  // Wait for configure
  while (!configured) {
    wl_display_dispatch(display);
  }

  // Create buffer and draw
  cairo_t *cr;
  cairo_surface_t *c_surface;
  struct wl_buffer *buffer = create_buffer(width, height, &cr, &c_surface);
  draw(cr);

  wl_surface_attach(surface, buffer, 0, 0);
  wl_surface_damage_buffer(surface, 0, 0, width, height);
  wl_surface_commit(surface);

  // Main loop: wait for Esc key (or kill)
  printf("leaderd active — press Ctrl+C or close the layer.\n");
  while (wl_display_dispatch(display) != -1) {}

  wl_display_disconnect(display);
  return 0;
}

