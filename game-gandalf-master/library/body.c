#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include <math.h>

const double TWO_PI = 2 * M_PI;

struct body {
  polygon_t *poly;

  double mass;

  vector_t force;
  vector_t impulse;
  bool removed;
  double direction_angle;

  void *info;
  free_func_t info_freer;
};

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL, 0);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer,
                            double direction_angle) {
  body_t *body = malloc(sizeof(body_t));
  assert(body);

  body->poly = polygon_init(shape, VEC_ZERO, 0.0, color.r, color.g, color.b);
  body->mass = mass;
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
  body->removed = false;
  body->info = info;
  body->info_freer = info_freer;
  body->direction_angle = direction_angle;
  return body;
}

void body_free(body_t *body) {
  if (body == NULL) {
    return;
  }
  if (body->poly != NULL) {
    polygon_free(body->poly);
  }
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  free(body);
}

/**
 * Returns the simplified angle
 * @param angle to be simplified
 * @return angle bounded between 0 and 2pi
 */
double simplify_angle(double angle) {
  ssize_t n = (ssize_t)(angle / TWO_PI);
  angle = angle - n * TWO_PI;
  return angle;
}

/**
 * Rotates the direction of a body about an angle
 *
 * @param body body to rotate
 * @param angle in radians to rotate body by
 */
void body_rotate_direction(body_t *body, double angle) {
  double direction = body->direction_angle;
  body->direction_angle = simplify_angle(direction + angle);
}

list_t *body_get_shape(body_t *body) {
  polygon_t *polygon = body->poly;
  list_t *points = polygon_get_points(polygon);
  list_t *shape = list_init(list_size(points), free);
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *vec = list_get(points, i);
    assert(vec);
    vector_t *new_vec = malloc(sizeof(vector_t));
    assert(new_vec);
    new_vec->x = vec->x;
    new_vec->y = vec->y;
    list_add(shape, new_vec);
  }
  return shape;
}

vector_t body_get_centroid(body_t *body) {
  return polygon_get_center(body->poly);
}

vector_t body_get_velocity(body_t *body) {
  polygon_t *poly = body->poly;
  return polygon_get_velocity(poly);
}

rgb_color_t *body_get_color(body_t *body) {
  return polygon_get_color(body->poly);
}

double body_get_rotation_speed(body_t *body) {
  return polygon_get_rotation(body->poly);
}

void body_set_rotation_speed(body_t *body, double rotation_speed) {
  polygon_set_rotation(body->poly, rotation_speed);
}

double body_get_mass(body_t *body) { return body->mass; }

polygon_t *body_get_polygon(body_t *body) { return body->poly; }

void *body_get_info(body_t *body) { return body->info; }

void body_set_color(body_t *body, rgb_color_t *col) {
  polygon_set_color(body->poly, col);
}

void body_set_centroid(body_t *body, vector_t x) {
  vector_t old_centroid = polygon_get_center(body->poly);
  vector_t diff = vec_subtract(x, old_centroid);
  polygon_translate(body->poly, diff);
  polygon_set_center(body->poly, x);
}

void body_set_velocity(body_t *body, vector_t v) {
  polygon_set_velocity(body->poly, v);
}

void body_set_rotation(body_t *body, double angle) {
  body_rotate_direction(body, angle - body->direction_angle);
  polygon_rotate(body->poly, angle, body_get_centroid(body));
}

void body_tick(body_t *body, double dt) {
  // impulse = m * dv
  vector_t impulse_dv = vec_multiply(1.0 / body->mass, body->impulse);
  // force = m * (dv / dt)
  vector_t force_dv = vec_multiply(dt / body->mass, body->force);

  body->impulse = VEC_ZERO;
  body->force = VEC_ZERO;

  vector_t dv = vec_add(impulse_dv, force_dv);

  vector_t old_vel = body_get_velocity(body);
  vector_t new_vel = vec_add(old_vel, dv);
  body_set_velocity(body, new_vel);

  double ang_vel = body_get_rotation_speed(body);
  if (ang_vel != 0) {
    double ang_dis = ang_vel * dt;
    body_rotate_direction(body, ang_dis);
  }

  vector_t ave_vel = vec_multiply(0.5, vec_add(old_vel, new_vel));
  body_set_centroid(
      body, vec_add(body_get_centroid(body), vec_multiply(dt, ave_vel)));
}

void body_add_force(body_t *body, vector_t force) {
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_remove(body_t *body) { body->removed = true; }

bool body_is_removed(body_t *body) { return body->removed; }

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}

double body_get_direction_angle(body_t *body) { return body->direction_angle; }

void body_is_valid(body_t *body) {
  assert(body);
  polygon_t *poly = body->poly;
  assert(poly);
  list_t *points = polygon_get_points(poly);
  assert(points);
  for (size_t i = 0; i < list_size(points); i++) {
    vector_t *vec = list_get(points, i);
    assert(vec);
  }
}
