#include "forces.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const double MIN_DIST = 5;

struct fcreator_storer {
  force_creator_t creator;
  void *aux;
  list_t *bodies;
};

typedef struct body_aux {
  double force_const;
  list_t *bodies;
} body_aux_t;

typedef struct collision_aux {
  double force_const;
  list_t *bodies;
  collision_handler_t handler;
  bool collided;
  void *aux; // aux (if allocated in memory) should be free'd by the caller
} collision_aux_t;

body_aux_t *body_aux_init(double force_const, list_t *bodies) {
  body_aux_t *aux = malloc(sizeof(body_aux_t));
  assert(aux);

  aux->bodies = bodies;
  aux->force_const = force_const;

  return aux;
}

collision_aux_t *collision_aux_init(double force_const, list_t *bodies,
                                    collision_handler_t handler, bool collided,
                                    void *aux) {
  collision_aux_t *collision_aux = malloc(sizeof(collision_aux_t));
  assert(collision_aux);

  collision_aux->force_const = force_const;
  collision_aux->bodies = bodies;
  collision_aux->handler = handler;
  collision_aux->collided = collided;
  collision_aux->aux = aux;
  return collision_aux;
}

void body_aux_free(void *aux) {
  list_free(((body_aux_t *)aux)->bodies);
  free(aux);
}

fcreator_storer_t *fcreator_storer_init(force_creator_t creator, void *aux,
                                        list_t *bodies) {
  assert(aux);

  fcreator_storer_t *storer = malloc(sizeof(fcreator_storer_t));
  assert(storer);

  storer->creator = creator;
  storer->aux = aux;
  storer->bodies = bodies;
  return storer;
}

force_creator_t fcreator_storer_get_creator(fcreator_storer_t *storer) {
  return storer->creator;
}

void *fcreator_storer_get_aux(fcreator_storer_t *storer) { return storer->aux; }

list_t *fcreator_storer_get_bodies(fcreator_storer_t *storer) {
  return storer->bodies;
}

void fcreator_storer_free(fcreator_storer_t *storer) {
  if (storer == NULL) {
    return;
  }
  if (storer->aux != NULL) {
    body_aux_free(storer->aux);
  }
  if (storer->bodies != NULL) {
    list_free(storer->bodies);
  }
  free(storer);
}

/**
 * The force creator for gravitational forces between objects. Calculates
 * the magnitude of the force components and adds the force to each
 * associated body.
 *
 * @param info auxiliary information about the force and associated bodies
 */
static void newtonian_gravity(void *info) {
  body_aux_t *aux = (body_aux_t *)info;
  vector_t displacement =
      vec_subtract(body_get_centroid(list_get(aux->bodies, 0)),
                   body_get_centroid(list_get(aux->bodies, 1)));

  double distance = vec_get_length(displacement);
  vector_t unit_disp = vec_multiply(1 / distance, displacement);

  if (distance > MIN_DIST) {
    vector_t grav_force = vec_multiply(
        aux->force_const * body_get_mass(list_get(aux->bodies, 0)) *
            body_get_mass(list_get(aux->bodies, 1)) /
            vec_dot(displacement, displacement),
        unit_disp);
    body_add_force(list_get(aux->bodies, 1), grav_force);
    body_add_force(list_get(aux->bodies, 0), vec_multiply(-1, grav_force));
  }
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  list_t *bodies = list_init(2, NULL);
  list_t *aux_bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, body2);
  body_aux_t *aux = body_aux_init(G, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)newtonian_gravity, aux,
                                 bodies);
}

/**
 * The force creator for spring forces between objects. Calculates
 * the magnitude of the force components and adds the force to each
 * associated body.
 *
 * @param info auxiliary information about the force and associated bodies
 */
static void spring_force(void *info) {
  body_aux_t *aux = info;

  double k = aux->force_const;
  body_t *body1 = list_get(aux->bodies, 0);
  body_t *body2 = list_get(aux->bodies, 1);

  vector_t center_1 = body_get_centroid(body1);
  vector_t center_2 = body_get_centroid(body2);
  vector_t distance = vec_subtract(center_1, center_2);

  vector_t spring_force = {-k * distance.x, -k * distance.y};
  body_add_force(body1, spring_force);
  body_add_force(body2, vec_negate(spring_force));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  list_t *aux_bodies = list_init(2, NULL);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, body2);
  body_aux_t *aux = body_aux_init(k, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)spring_force, aux,
                                 bodies);
}

/**
 * The force creator for drag forces on an object. Calculates
 * the magnitude of the force components and adds the force to the
 * associated body.
 *
 * @param info auxiliary information about the force and associated body
 */
static void drag_force(void *info) {
  body_aux_t *aux = (body_aux_t *)info;
  vector_t cons_force = vec_multiply(
      -1 * aux->force_const, body_get_velocity(list_get(aux->bodies, 0)));

  body_add_force(list_get(aux->bodies, 0), cons_force);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  list_t *bodies = list_init(1, NULL);
  list_t *aux_bodies = list_init(1, NULL);
  list_add(bodies, body);
  list_add(aux_bodies, body);
  body_aux_t *aux = body_aux_init(gamma, aux_bodies);
  scene_add_bodies_force_creator(scene, (force_creator_t)drag_force, aux,
                                 bodies);
}

/**
 * The force creator for collisions. Checks if the bodies in the collision aux
 * are colliding, and if they do, runs the collision handler on the bodies.
 *
 * @param info auxiliary information about the force and associated body
 */
static void collision_force_creator(void *collision_aux) {
  collision_aux_t *col_aux = collision_aux;

  list_t *bodies = col_aux->bodies;
  body_t *body1 = list_get(bodies, 0);
  body_t *body2 = list_get(bodies, 1);

  // Check for collision; if bodies collide, call collision_handler
  bool prev_collision = col_aux->collided;

  collision_info_t info = find_collision(body1, body2);
  // avoids registering impulse multiple times while bodies are still colliding
  if (info.collided && !prev_collision) {
    collision_handler_t handler = col_aux->handler;

    handler(body1, body2, info.axis, col_aux->aux, col_aux->force_const);
    col_aux->collided = true;
  } else if (!info.collided && prev_collision) {
    col_aux->collided = false;
  }
}

// // NEW FUNCTION TO RETURN WHETHER COLLIDED BASED ON COLLISION AUX!
// bool collided_with_obstacle(void *collision_aux) {
//   collision_aux_t *col_aux = collision_aux;

//   list_t *bodies = col_aux->bodies;
//   body_t *body1 = list_get(bodies, 0);
//   body_t *body2 = list_get(bodies, 1);
//   if (body_get_mass(body1) == INFINITY || body_get_mass(body2) == INFINITY) {
//     return col_aux->collided;
//   }
//   return false;
// }

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      double force_const) {
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);

  list_t *aux_bodies = list_init(2, NULL);
  list_add(aux_bodies, body1);
  list_add(aux_bodies, body2);

  collision_aux_t *collision_aux =
      collision_aux_init(force_const, aux_bodies, handler, false, aux);

  scene_add_bodies_force_creator(scene, collision_force_creator, collision_aux,
                                 bodies);
}

// /**
//  * The collision handler for one-sided destructive collisions.
//  */
// static void one_sided_destructive_collision_handler(body_t *body1,
//                                                     body_t *body2,
//                                                     vector_t axis, void *aux,
//                                                     double force_const) {
//   body_remove(body2);
// }

// void create_one_sided_destructive_collision(scene_t *scene, body_t *body1,
//                                             body_t *body2) {
//   create_collision(scene, body1, body2,
//   one_sided_destructive_collision_handler,
//                    NULL, 0);
// }

/**
 * The collision handler for destructive collisions.
 */
static void destructive_collision(body_t *body1, body_t *body2, vector_t axis,
                                  void *aux, double force_const) {
  body_remove(body1);
  body_remove(body2);
}

void one_sided_destructive_collision_handler(body_t *body1, body_t *body2,
                                             vector_t axis, void *aux,
                                             double force_const) {
  body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  create_collision(scene, body1, body2, destructive_collision, NULL, 0);
}

void create_one_sided_destructive_collision(scene_t *scene, body_t *body1,
                                            body_t *body2) {
  create_collision(scene, body1, body2, one_sided_destructive_collision_handler,
                   NULL, 1);
}

void physics_collision_handler(body_t *body1, body_t *body2, vector_t axis,
                               void *aux, double force_const) {
  double m1 = body_get_mass(body1);
  vector_t velocity1 = body_get_velocity(body1);
  double u1 = vec_dot(velocity1, axis);
  double m2 = body_get_mass(body2);
  vector_t velocity2 = body_get_velocity(body2);
  double u2 = vec_dot(velocity2, axis);
  double impulse = (m1 * m2) / (m1 + m2) * (1 + force_const) * (u2 - u1);

  if (m1 == INFINITY) {
    impulse = m2 * (1 + force_const) * (u2 - u1);
  } else if (m2 == INFINITY) {
    impulse = m1 * (1 + force_const) * (u2 - u1);
  }
  vector_t impulse_vec = vec_multiply(impulse, axis);

  body_add_impulse(body1, impulse_vec);
  body_add_impulse(body2, vec_multiply(-1, impulse_vec));
}

void create_physics_collision(scene_t *scene, body_t *body1, body_t *body2,
                              double elasticity) {
  create_collision(scene, body1, body2, physics_collision_handler, NULL,
                   elasticity);
}
