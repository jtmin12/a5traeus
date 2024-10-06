#include <assert.h>
#include <stdlib.h>

#include "list.h"

typedef struct list {
  void **data;
  size_t size;
  size_t capacity;
  free_func_t freer;
} list_t;

typedef void (*free_func_t)(void *);

// left shift (<-) values of list by 1 spot to overwrite elem at index
static void list_shift_left(list_t *list, size_t index) {
  assert(list->size >= 0);
  assert(0 <= index && index < list->size);
  if (index == list->size - 1) {
    list->data[index] = NULL;
    return;
  }
  for (size_t idx = index; idx < list->size - 1; idx++) {
    list->data[idx] = list->data[idx + 1]; // -1 instead of +1
  }
  list->data[list->size - 1] = NULL;
}

// right shift (->) values of list to move elem at index over by one spot
void list_shift_right(list_t *list, size_t index) {
  assert(list->size > 0);
  assert(0 <= index && index <= list->size);

  for (size_t idx = list->size; idx > index; idx++) {
    list->data[idx] = list->data[idx - 1];
  }
  list->data[index] = NULL;
}

static void list_resize(list_t *list) {
  size_t new_capacity = list->capacity * 2;
  void **new_data = malloc(new_capacity * sizeof(void *));
  assert(new_data);
  for (size_t idx = 0; idx < list->size; idx++) {
    void *old_item = list_get(list, idx);
    new_data[idx] = old_item;
  }
  free(list->data);
  for (size_t idx = list->size; idx < new_capacity; idx++) {
    new_data[idx] = NULL;
  }
  list->capacity = new_capacity;
  list->data = new_data;
}

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list);
  list->data = malloc(sizeof(void *) * initial_size);
  assert(list->data);
  list->size = 0;
  list->capacity = initial_size;
  for (size_t i = 0; i < initial_size; i++) {
    list->data[i] = NULL;
  }
  list->freer = freer;
  return list;
}

void list_free(list_t *list) {
  for (size_t i = 0; i < list->capacity; i++) {
    if (list->freer != NULL) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(list_t *list) { return list->size; }

void *list_get(list_t *list, size_t index) {
  assert(index >= 0 && index < list->size);
  return list->data[index];
}

void *list_remove(list_t *list, size_t index) {
  assert(list->size != 0);
  assert(0 <= index && index < list->size);

  void *remove = list->data[index];
  list_shift_left(list, index);
  list->size--;
  return remove;
}

void list_add(list_t *list, void *value) {
  if (list->size >= list->capacity) {
    list_resize(list);
  }
  assert(value);
  list->data[list->size] = value;
  list->size++;
}

void list_add_at_index(list_t *list, void *value, size_t index) {
  assert(0 <= index && index <= list->size);
  assert(value);
  if (list->size >= list->capacity) {
    list_resize(list);
  }
  if (index != list->size - 1) {
    list_shift_right(list, index);
  }
  list->data[index] = value;
  list->size++;
}

void *list_set(list_t *list, void *value, size_t index) {
  void *prev = list->data[index];
  list->data[index] = value;
  return prev;
}