#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "list.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  switch (entry->type) {
  case ASSET_IMAGE: {
    SDL_DestroyTexture(entry->obj);
    break;
  }
  case ASSET_FONT: {
    TTF_CloseFont(entry->obj);
    break;
  }
  case ASSET_BUTTON: {
    free(entry->obj);
    break;
  }
  case ASSET_MUSIC: {
    Mix_FreeMusic(entry->obj);
    break;
  }
  case ASSET_SFX: {
    Mix_FreeChunk(entry->obj);
    break;
  }
  }
  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

entry_t *cache_contains_path(const char *filepath) {
  if (!filepath) {
    return NULL;
  }
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->filepath != NULL && strcmp(entry->filepath, filepath) == 0) {
      return entry;
    }
  }
  return NULL;
}

entry_t *entry_init(asset_type_t type, const char *filepath, void *obj) {
  entry_t *entry = malloc(sizeof(entry_t));
  assert(entry);
  entry->type = type;
  entry->filepath = filepath;
  entry->obj = obj;
  return entry;
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  entry_t *entry = cache_contains_path(filepath);
  if (entry) {
    assert(entry->type == ty);
    return entry->obj;
  }
  switch (ty) {
  case ASSET_IMAGE: {
    SDL_Texture *image = sdl_load_img_texture(filepath);
    entry_t *entry = entry_init(ty, filepath, image);
    list_add(ASSET_CACHE, entry);
    return image;
  }
  case ASSET_FONT: {
    TTF_Font *text = TTF_OpenFont(filepath, FONT_SIZE);
    entry_t *entry = entry_init(ty, filepath, text);
    list_add(ASSET_CACHE, entry);
    return text;
  }
  case ASSET_MUSIC: {
    Mix_Music *music = Mix_LoadMUS(filepath);
    entry_t *entry = entry_init(ty, filepath, music);
    list_add(ASSET_CACHE, entry);
    return music;
  }
  case ASSET_SFX: {
    Mix_Music *sfx = Mix_LoadWAV(filepath);
    entry_t *entry = entry_init(ty, filepath, sfx);
    list_add(ASSET_CACHE, entry);
    return sfx;
  }
  default:
    return NULL;
  }
}

void asset_cache_register_button(asset_t *button) {
  assert(asset_get_type(button) == ASSET_BUTTON);

  entry_t *entry = malloc(sizeof(entry_t));
  assert(entry);

  entry->filepath = NULL;
  entry->obj = button;
  entry->type = ASSET_BUTTON;
  list_add(ASSET_CACHE, entry);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {
  for (size_t i = 0; i < list_size(ASSET_CACHE); i++) {
    entry_t *entry = list_get(ASSET_CACHE, i);
    if (entry->type == ASSET_BUTTON) {
      asset_on_button_click((asset_t *)entry->obj, state, x, y);
    }
  }
}
