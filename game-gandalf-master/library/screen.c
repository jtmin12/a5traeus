#include "screen.h"

typedef struct screen {
  scene_t *scene;
  list_t *body_assets;
} screen_t;

screen_t *screen_init(scene_t *scene, list_t *body_assets) {
  screen_t *screen = malloc(sizeof(screen_t));
  assert(screen);
  screen->body_assets = body_assets;
  screen->scene = scene;
  return screen;
}

void screen_free(screen_t *screen) {
  list_free(screen->body_assets);
  scene_free(screen->scene);
  free(screen);
}

scene_t *screen_get_scene(screen_t *screen) { return screen->scene; }

list_t *screen_get_body_assets(screen_t *screen) { return screen->body_assets; }