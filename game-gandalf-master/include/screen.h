#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "list.h"
#include "scene.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct screen screen_t;

screen_t *screen_init(scene_t *scene, list_t *body_assets);

void screen_free(screen_t *screen);

scene_t *screen_get_scene(screen_t *screen);

list_t *screen_get_body_assets(screen_t *screen);

#endif // #ifndef __SCREEN_H__