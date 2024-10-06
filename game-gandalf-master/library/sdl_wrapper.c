#include "sdl_wrapper.h"
#include "asset_cache.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "a5traeμs";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

const SDL_Color BLACK = {0, 0, 0};
const double IMG_SCALE = 1;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/*
 * The mouse handler, or NULL if none has been configured.
 */
mouse_handler_t mouse_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width);
  assert(height);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);

  SDL_Init(SDL_INIT_AUDIO);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  TTF_Init();
  Mix_Init(MIX_INIT_OGG || MIX_INIT_WAVPACK);
  Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}

bool sdl_is_done(void *state) {
  int x = 0, y = 0;
  uint32_t bitmask = SDL_GetMouseState(&x, &y);
  if (bitmask > 0 && SDL_BUTTON(bitmask) == 1) {
    asset_cache_handle_buttons(state, (double)x, (double)y);
    return false;
  }

  const uint8_t *keys = SDL_GetKeyboardState(NULL);
  if (keys[SDL_QUIT] || key_handler == NULL) {
    return true;
  }
  if (keys[SDL_SCANCODE_UP]) {
    key_handler(UP_ARROW, KEY_PRESSED, 0, state);
  } else if (keys[SDL_SCANCODE_DOWN]) {
    key_handler(DOWN_ARROW, KEY_PRESSED, 0, state);
  }
  if (keys[SDL_SCANCODE_LEFT]) {
    key_handler(LEFT_ARROW, KEY_PRESSED, 0, state);
  } else if (keys[SDL_SCANCODE_RIGHT]) {
    key_handler(RIGHT_ARROW, KEY_PRESSED, 0, state);
  }
  if (keys[SDL_SCANCODE_W]) {
    key_handler('w', KEY_PRESSED, 0, state);
  } else if (keys[SDL_SCANCODE_S]) {
    key_handler('s', KEY_PRESSED, 0, state);
  }
  if (keys[SDL_SCANCODE_A]) {
    key_handler('a', KEY_PRESSED, 0, state);
  } else if (keys[SDL_SCANCODE_D]) {
    key_handler('d', KEY_PRESSED, 0, state);
  }

  if (keys[SDL_SCANCODE_M]) {
    key_handler('m', KEY_PRESSED, 0, state);
  }
  if (keys[SDL_SCANCODE_V]) {
    key_handler('v', KEY_PRESSED, 0, state);
  }
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(polygon_t *poly, rgb_color_t *color) {
  list_t *points = polygon_get_points(poly);
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color->r && color->r <= 1);
  assert(0 <= color->g && color->g <= 1);
  assert(0 <= color->b && color->b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points);
  assert(y_points);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color->r * 255,
                    color->g * 255, color->b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene, void *aux) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    polygon_t *poly = polygon_init(shape, (vector_t){0, 0}, 0, 0, 0, 0);
    sdl_draw_polygon(poly, body_get_color(body));
    list_free(shape);
  }
  if (aux != NULL) {
    body_t *body = aux;
    sdl_draw_polygon(body_get_polygon(body), body_get_color(body));
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

void sdl_on_click(mouse_handler_t handler) { mouse_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

SDL_Rect sdl_get_bounding_box(body_t *body) {
  vector_t maxes = (vector_t){-__DBL_MAX__, -__DBL_MAX__};
  vector_t mins = (vector_t){__DBL_MAX__, __DBL_MAX__};
  list_t *vertices = body_get_shape(body);
  size_t n = list_size(vertices);

  vector_t window_center = get_window_center();

  for (size_t i = 0; i < n; i++) {
    vector_t *vec = list_get(vertices, i);
    assert(vec);
    vector_t pixel = get_window_position(*vec, window_center);
    if (maxes.x < pixel.x) {
      maxes.x = pixel.x;
    }
    if (mins.x > pixel.x) {
      mins.x = pixel.x;
    }
    if (maxes.y < pixel.y) {
      maxes.y = pixel.y;
    }
    if (mins.y > pixel.y) {
      mins.y = pixel.y;
    }
  }

  list_free(vertices);
  double x = mins.x;
  double y = mins.y;
  double w = maxes.x - mins.x;
  double h = maxes.y - mins.y;

  SDL_Rect rect = (SDL_Rect){x, y, w, h};
  return rect;
}

SDL_Texture *sdl_load_text_texture(TTF_Font *font, const char *msg,
                                   rgb_color_t color) {
  SDL_Color sdl_color =
      (SDL_Color){(uint8_t)color.r, (uint8_t)color.g, (uint8_t)color.b};
  SDL_Surface *message = TTF_RenderText_Blended(font, msg, sdl_color);
  assert(message);
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, message);
  SDL_FreeSurface(message);
  return texture;
}

SDL_Texture *sdl_load_img_texture(const char *IMG_PATH) {
  SDL_Texture *texture = IMG_LoadTexture(renderer, IMG_PATH);
  return texture;
}

void sdl_render_texture(SDL_Texture *texture, SDL_Rect bounding_box) {
  SDL_RenderCopy(renderer, texture, NULL, &bounding_box);
}

void sdl_render_rotate_texture(SDL_Texture *texture, SDL_Rect bounding_box,
                               double angle) {
  SDL_RenderCopyEx(renderer, texture, NULL, &bounding_box, 180 * angle / M_PI,
                   NULL, SDL_FLIP_NONE);
}
