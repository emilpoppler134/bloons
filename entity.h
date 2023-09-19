#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "raylib.h"
#include "time_interval.h"

typedef struct entity_t
{
  // Entity
  Vector2 position;
  Vector2 direction;
  float speed;
  float rotation;
  int hp;
  int radius;
  time_interval_t interval;

  // Resource
  int texture_index;
  int damage;
  int cost;
} entity_t;

typedef struct dynamic_entity_array
{
  entity_t* data;
  int count;
  int capacity;
} dynamic_entity_array;



dynamic_entity_array init_entity_array() {
  dynamic_entity_array arr;
  arr.data = (entity_t*)malloc(sizeof(entity_t));
  arr.count = 0;
  arr.capacity = 1;
  return arr;
}

void push(dynamic_entity_array *arr, entity_t item) {
  if (arr->count == arr->capacity) {
    arr->capacity++;
    arr->data = (entity_t*)realloc(arr->data, arr->capacity * sizeof(entity_t));
  }
  arr->data[arr->count] = item;
  arr->count++;
}

void remove_at(dynamic_entity_array *arr, int index) {
  if (index < 0 || index >= arr->count) {
    return;
  }

  entity_t* new_arr = (entity_t*)malloc((arr->capacity - 1) * sizeof(entity_t));
  if (!new_arr) {
    return;
  }

  int new_arr_index = 0;

  for (int i = 0; i < arr->count; i++) {
    if (i != index) {
      new_arr[new_arr_index] = arr->data[i];
      new_arr_index++;
    }
  }

  free(arr->data);
  arr->data = new_arr;
  arr->count--;
  arr->capacity--;
}

// Resources
void deserialize_players(dynamic_entity_array *players)
{
  DIR *directory;
  struct dirent *entry;

  // Open the directory
  directory = opendir("./resources/players");

  if (directory == NULL) {
    perror("Error opening directory");
    exit(1);
  }

  // Read each entry in the directory
  while ((entry = readdir(directory)) != NULL) {
    if (entry->d_type == DT_REG) {  // Check if it's a regular file
      entity_t player;

      FILE *file = fopen(TextFormat("./resources/players/%s", entry->d_name), "rb");
      if (!file)
      {
        perror("Failed to open level file");
        exit(1);
      }

      if (fread(&player, sizeof(entity_t), 1, file) != 1)
      {
        perror("Failed to read level data");
        fclose(file);
        exit(1);
      }

      push(players, player);

      fclose(file);
    }
  }

  // Close the directory
  closedir(directory);
}

#endif