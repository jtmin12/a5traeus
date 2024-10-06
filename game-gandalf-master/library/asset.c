#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *system;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

typedef struct music_asset {
  asset_t base;
  Mix_Music *music;
} music_asset_t;

typedef struct sfx_asset {
  asset_t base;
  Mix_Chunk *sfx;
} sfx_asset_t;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  case ASSET_MUSIC: {
    new = malloc(sizeof(music_asset_t));
    break;
  }
  case ASSET_SFX: {
    new = malloc(sizeof(sfx_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  SDL_Texture *img = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  asset_t *asset = asset_init(ASSET_IMAGE, bounding_box);
  ((image_asset_t *)asset)->texture = img;
  ((image_asset_t *)asset)->body = NULL;
  return asset;
}

asset_t *asset_make_image_with_body(const char *filepath, body_t *body) {
  SDL_Texture *img = asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);
  SDL_Rect rect = sdl_get_bounding_box(body);
  asset_t *asset = asset_init(ASSET_IMAGE, rect);
  ((image_asset_t *)asset)->texture = img;
  ((image_asset_t *)asset)->body = body;
  return asset;
}

// void asset_change_filepath(asset_t *asset, char* path) {
//   asset->
// }

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *system, const char *text,
                         rgb_color_t color) {
  TTF_Font *font = asset_cache_obj_get_or_create(ASSET_FONT, filepath);
  asset_t *asset = asset_init(ASSET_FONT, bounding_box);
  ((text_asset_t *)asset)->font = font;
  ((text_asset_t *)asset)->system = system;
  ((text_asset_t *)asset)->text = text;
  ((text_asset_t *)asset)->color = color;
  return asset;
}

asset_t *asset_make_music(const char *filepath) {
  Mix_Music *mus = asset_cache_obj_get_or_create(ASSET_MUSIC, filepath);
  SDL_Rect rect = {0, 0, 0, 0};
  asset_t *asset = asset_init(ASSET_MUSIC, rect);
  ((music_asset_t *)asset)->music = mus;
  return asset;
}

void asset_play_music(asset_t *music, int volume) {
  int vol = (MIX_MAX_VOLUME * volume) / 100;
  if (Mix_PlayingMusic() == 0) {
    Mix_Volume(1, vol);
    Mix_PlayMusic(((music_asset_t *)music)->music, -1);
  }
}

asset_t *asset_make_sfx(const char *filepath) {
  Mix_Chunk *sfx = asset_cache_obj_get_or_create(ASSET_SFX, filepath);
  SDL_Rect rect = {0, 0, 0, 0};
  asset_t *asset = asset_init(ASSET_SFX, rect);
  ((sfx_asset_t *)asset)->sfx = sfx;
  return asset;
}

void asset_play_sfx(asset_t *sfx, int volume) {
  int vol = (MIX_MAX_VOLUME * volume) / 100;
  Mix_Volume(-1, vol);
  Mix_PlayChannel(-1, ((sfx_asset_t *)sfx)->sfx, 0);
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  asset_t *asset = asset_init(ASSET_BUTTON, bounding_box);

  if (image_asset != NULL && ((image_asset_t *)image_asset)->texture != NULL) {
    assert(image_asset->type == ASSET_IMAGE);
    ((button_asset_t *)asset)->image_asset = (image_asset_t *)image_asset;
  } else {
    ((button_asset_t *)asset)->image_asset = NULL;
  }

  if (text_asset != NULL && ((text_asset_t *)text_asset)->font != NULL) {
    assert(text_asset->type == ASSET_FONT);
    ((button_asset_t *)asset)->text_asset = (text_asset_t *)text_asset;
  } else {
    ((button_asset_t *)asset)->text_asset = NULL;
  }

  ((button_asset_t *)asset)->handler = handler;
  ((button_asset_t *)asset)->is_rendered = false;

  asset_cache_register_button(asset);
  return asset;
}

// checks whether the cursor with the given x and y coordinates are in the
// bounds of the rectangle
bool is_mouse_in_box(double x, double y, SDL_Rect bounding_box) {
  double x_left_bound = bounding_box.x;
  double x_right_bound = bounding_box.x + bounding_box.w;
  double y_low_bound = bounding_box.y;
  double y_up_bound = bounding_box.y + bounding_box.h;

  return x_left_bound <= x && x <= x_right_bound && y_low_bound <= y &&
         y <= y_up_bound;
}

void asset_on_button_click(asset_t *button, state_t *state, double x,
                           double y) {
  if (!(((button_asset_t *)button)->is_rendered)) {
    return;
  }
  if (is_mouse_in_box(x, y, button->bounding_box) &&
      ((button_asset_t *)button)->is_rendered) {
    (*(((button_asset_t *)button)->handler))(state);
  }
  ((button_asset_t *)button)->is_rendered = false;
}

void asset_render(asset_t *asset) {
  switch (asset->type) {
  case ASSET_IMAGE: {
    SDL_Texture *texture = ((image_asset_t *)asset)->texture;
    body_t *body = ((image_asset_t *)asset)->body;
    if (body && !body_is_removed(body)) {
      SDL_Rect box = sdl_get_bounding_box(body);
      double body_rot = body_get_direction_angle(body);
      sdl_render_rotate_texture(texture, box, body_rot);
    } else {
      SDL_Rect box = asset->bounding_box;
      sdl_render_texture(texture, box);
    }
    break;
  }
  case ASSET_FONT: {
    SDL_Rect box = asset->bounding_box;
    TTF_Font *font = ((text_asset_t *)asset)->font;
    const char *msg = ((text_asset_t *)asset)->text;
    rgb_color_t color = ((text_asset_t *)asset)->color;
    SDL_Texture *texture = sdl_load_text_texture(font, msg, color);
    sdl_render_texture(texture, box);
    break;
  }
  case ASSET_BUTTON: {
    if (((button_asset_t *)asset)->image_asset != NULL) {
      asset_t *img = &(((button_asset_t *)asset)->image_asset->base);
      asset_render(img);
    }
    if (((button_asset_t *)asset)->text_asset != NULL) {
      asset_t *txt = &(((button_asset_t *)asset)->text_asset->base);
      asset_render(txt);
    }
    ((button_asset_t *)asset)->is_rendered = true;
    break;
  }
  default:
    return;
  }
}

body_t *asset_get_body(asset_t *asset) {
  if (asset->type == ASSET_IMAGE) {
    return ((image_asset_t *)asset)->body;
  }
  return NULL;
}

char *asset_get_system(asset_t *asset) {
  if (asset->type == ASSET_FONT) {
    return ((text_asset_t *)asset)->system;
  }
  return NULL;
}

char *asset_get_text(asset_t *asset) {
  if (asset->type == ASSET_FONT) {
    return ((text_asset_t *)asset)->text;
  }
  return NULL;
}

SDL_Rect asset_get_bounding_box(asset_t *asset) { return asset->bounding_box; }

void asset_destroy(asset_t *asset) { free(asset); }
