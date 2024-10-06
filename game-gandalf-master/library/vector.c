#include "vector.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t VEC_ZERO = (vector_t){.x = 0.0, .y = 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t sum = v1;
  sum.x += v2.x;
  sum.y += v2.y;
  return sum;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  return vec_add(v1, vec_negate(v2));
}

vector_t vec_negate(vector_t v) { return vec_multiply(-1.0, v); }

vector_t vec_multiply(double scalar, vector_t v) {
  v.x *= scalar;
  v.y *= scalar;
  return v;
}

double vec_dot(vector_t v1, vector_t v2) { return v1.x * v2.x + v1.y * v2.y; }

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t rotated = VEC_ZERO;
  rotated.x += v.x * cos(angle) - v.y * sin(angle);
  rotated.y += v.x * sin(angle) + v.y * cos(angle);
  return rotated;
}

double vec_get_length(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }

vector_t create_unit_vector(double angle) {
  vector_t vec = (vector_t){cos(angle), sin(angle)};
  return vec;
}

vector_t create_vector(double scalar, double angle) {
  return vec_multiply(scalar, create_unit_vector(angle));
}
