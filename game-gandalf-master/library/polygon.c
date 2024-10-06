#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "polygon.h"

const vector_t GRAVITY = (vector_t){.x = 0.0, .y = -10.0};

struct polygon {
  list_t *vertex_list;
  vector_t velocity;
  vector_t centroid;
  double rotation_speed;
  rgb_color_t *color;
};

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *polygon = malloc(sizeof(polygon_t));
  assert(polygon);
  polygon->vertex_list = points;
  polygon->velocity = initial_velocity;
  polygon->rotation_speed = rotation_speed;
  polygon->color = color_init(red, green, blue);
  polygon->centroid = polygon_centroid(polygon);
  return polygon;
}

list_t *polygon_get_points(polygon_t *polygon) {
  assert(polygon);
  return polygon->vertex_list;
}

void polygon_move(polygon_t *polygon, double time_elapsed) {
  vector_t old_vel = polygon->velocity;
  vector_t translation = vec_multiply(time_elapsed, old_vel);
  polygon_translate(polygon, translation);

  double angle = polygon->rotation_speed * time_elapsed;
  vector_t centroid = polygon_centroid(polygon);
  polygon_rotate(polygon, angle, centroid);

  // v_f = v_i + at
  vector_t new_vel = vec_add(old_vel, vec_multiply(time_elapsed, GRAVITY));
  polygon_set_velocity(polygon, new_vel);
}

void polygon_set_velocity(polygon_t *polygon, vector_t vel) {
  polygon->velocity = vel;
}

void polygon_free(polygon_t *polygon) {
  list_free(polygon->vertex_list);
  color_free(polygon->color);
  free(polygon);
}

vector_t polygon_get_velocity(polygon_t *polygon) { return polygon->velocity; }

double polygon_area(polygon_t *polygon) {
  list_t *vertex_list = polygon->vertex_list;
  double sum = 0.0;
  size_t size = list_size(vertex_list);
  // Shoelace Theorem
  for (size_t i = 0; i < size; i++) {
    vector_t *vec_i = list_get(vertex_list, i);
    vector_t *vec_i_plus = list_get(vertex_list, (i + 1) % size);

    double x_i = vec_i->x;
    double y_i = vec_i->y;
    double x_i_plus = vec_i_plus->x;
    double y_i_plus = vec_i_plus->y;

    sum += (x_i_plus + x_i) * (y_i_plus - y_i);
  }
  return 0.5 * (fabs(sum));
}

vector_t polygon_centroid(polygon_t *polygon) {
  list_t *vertex_list = polygon->vertex_list;
  vector_t centroid = VEC_ZERO;
  double area = polygon_area(polygon);
  size_t size = list_size(vertex_list);

  for (size_t i = 0; i < size; i++) {
    vector_t *vec_i = list_get(vertex_list, i);
    vector_t *vec_i_plus = list_get(vertex_list, (i + 1) % size);

    double x_i = vec_i->x;
    double x_i_plus = vec_i_plus->x;
    double y_i = vec_i->y;
    double y_i_plus = vec_i_plus->y;

    // (x_i + x_{i+1})(x_i * y_{i+1} - x_{i+1} * y_i)
    centroid.x += (x_i + x_i_plus) * (x_i * y_i_plus - y_i * x_i_plus);
    // (y_i + y_{i+1})(x_i * y_{i+1} - x_{i+1} * y_i)
    centroid.y += (y_i + y_i_plus) * (x_i * y_i_plus - y_i * x_i_plus);
  }
  centroid.x /= (6 * area);
  centroid.y /= (6 * area);
  polygon->centroid = centroid;
  return centroid;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  list_t *vertex_list = polygon->vertex_list;
  size_t size = list_size(vertex_list);
  for (size_t i = 0; i < size; i++) {
    vector_t *vec_i = list_get(vertex_list, i);
    vec_i->x += translation.x;
    vec_i->y += translation.y;
  }
  polygon->centroid = polygon_centroid(polygon);
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  list_t *vertex_list = polygon->vertex_list;
  size_t size = list_size(vertex_list);
  for (size_t i = 0; i < size; i++) {
    vector_t v_rotate = VEC_ZERO;
    vector_t *vec_i = list_get(vertex_list, i);

    v_rotate.x += vec_i->x - point.x;
    v_rotate.y += vec_i->y - point.y;
    v_rotate = vec_rotate(v_rotate, angle);

    vec_i->x = v_rotate.x + point.x;
    vec_i->y = v_rotate.y + point.y;
  }
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  polygon->color = color;
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  polygon->centroid = centroid;
}

vector_t polygon_get_center(polygon_t *polygon) { return polygon->centroid; }

void polygon_set_rotation(polygon_t *polygon, double rot) {
  polygon->rotation_speed = rot;
}

double polygon_get_rotation(polygon_t *polygon) {
  return polygon->rotation_speed;
}