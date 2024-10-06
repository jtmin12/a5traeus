#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "asset.h"
#include "forces.h"
#include "scene.h"

const size_t INITIAL_NUM_BOD = 100;
const size_t INITIAL_NUM_FCREATOR = 10;

struct scene {
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creators;
};

scene_t *scene_init() {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene);

  scene->bodies = list_init(INITIAL_NUM_BOD, (free_func_t)body_free);
  scene->force_creators =
      list_init(INITIAL_NUM_FCREATOR, (free_func_t)fcreator_storer_free);
  scene->num_bodies = 0;
  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_creators);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return scene->num_bodies; }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(0 <= index && index < list_size(scene->bodies));
  return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  assert(body);
  scene->num_bodies++;
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(0 <= index && index < list_size(scene->bodies));
  body_t *body = scene_get_body(scene, index);
  body_remove(body);
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, NULL));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  fcreator_storer_t *fstore = fcreator_storer_init(forcer, aux, bodies);
  list_add(scene->force_creators, fstore);
}

list_t *scene_tick(scene_t *scene, list_t *assets, double dt) {
  list_t *destroyed_asters = list_init(5, NULL);
  for (size_t i = 0; i < list_size(scene->force_creators); i++) {
    fcreator_storer_t *storer = list_get(scene->force_creators, i);
    list_t *creator_bodies = fcreator_storer_get_bodies(storer);
    force_creator_t creator = fcreator_storer_get_creator(storer);
    void *aux = fcreator_storer_get_aux(storer);
    (*creator)(aux);
  }

  for (ssize_t i = 0; i < (ssize_t)(scene->num_bodies); i++) {
    body_t *body = scene_get_body(scene, i);
    if (body_is_removed(body)) {
      for (ssize_t j = 0; j < (ssize_t)(list_size(scene->force_creators));
           j++) {
        fcreator_storer_t *fstorer = list_get(scene->force_creators, j);
        list_t *creator_bodies = fcreator_storer_get_bodies(fstorer);
        for (size_t k = 0; k < list_size(creator_bodies); k++) {
          body_t *kbody = list_get(creator_bodies, k);
          if (kbody == body) {
            list_remove(scene->force_creators, j);
            fcreator_storer_free(fstorer);
            j--;
            break;
          }
        }
      }
      for (ssize_t k = 0; k < (ssize_t)(list_size(assets)); k++) {
        if (asset_get_body(list_get(assets, k)) == body) {
          list_remove(assets, k);
          break;
        }
      }
      if (strcmp(body_get_info(body), "Asteroid") == 0) {
        vector_t centroid = body_get_centroid(body);
        list_add(destroyed_asters, &centroid);
      }
      list_remove(scene->bodies, i);
      body_free(body);
      scene->num_bodies--;
      i--;
    } else {
      body_tick(body, dt);
    }
  }
  return destroyed_asters;
}
