#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>

#include "raylib.h"
#include "raymath.h"

#include "tiles.h"

#define TILE_SIZE 80
#define COLS 15
#define ROWS 10

typedef Vector2 v2f;


// Structs for the entity, state and the dynamic array
//--------------------------------------------------------------------------------------
typedef struct level_t {
  tile_type_e tiles[ROWS][COLS];
  direction_e course[ROWS][COLS];
  int startingTileY;
} level_t;

typedef struct TimeInterval {
  double lastTime;
  double interval;
} TimeInterval;

typedef struct Entity
{
  v2f position;
  v2f direction;
  float speed;
  float rotation;
  int hp;
  TimeInterval interval;
} Entity;

typedef struct dynamicEntityArray
{
  Entity* data;
  int count;
  int capacity;
} dynamicEntityArray;

typedef struct State
{
  int economy;
  int wave;
  int killedEnemyCount;
  float shootingSpeed;
  float enemySpawnSpeed;
  dynamicEntityArray players;
  dynamicEntityArray enemies;
  dynamicEntityArray bullets;
} State;


// Methods for initializing the dynamic array, pushing, removeing at, checking if player position is empty
//--------------------------------------------------------------------------------------
void DeserializeLevel(level_t *level)
{
  FILE *file = fopen("./resources/level.ep", "rb");
  if (!file)
  {
    perror("Failed to open level file");
    exit(1);
  }

  if (fread(level, sizeof(level_t), 1, file) != 1)
  {
    perror("Failed to read level data");
    fclose(file);
    exit(1);
  }

  fclose(file);
}

dynamicEntityArray initEntityArray() {
  dynamicEntityArray arr;
  arr.data = (Entity*)malloc(sizeof(Entity));
  arr.count = 0;
  arr.capacity = 1;
  return arr;
}

void push(dynamicEntityArray *arr, Entity item) {
  if (arr->count == arr->capacity) {
    arr->capacity++;
    arr->data = (Entity*)realloc(arr->data, arr->capacity * sizeof(Entity));
  }
  arr->data[arr->count] = item;
  arr->count++;
}

void remove_at(dynamicEntityArray *arr, int index) {
  if (index < 0 || index >= arr->count) {
    return;
  }

  Entity* new_arr = (Entity*)malloc((arr->capacity - 1) * sizeof(Entity));
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

// Initialize a time interval with a specified interval
TimeInterval InitTimeInterval(double seconds) {
  TimeInterval interval;
  interval.lastTime = GetTime();
  interval.interval = seconds;
  return interval;
}

// Check if the time interval has elapsed and reset the timer if it has
bool CheckTimeInterval(TimeInterval *timer) {
  double currentTime = GetTime();
  if (currentTime - timer->lastTime >= timer->interval) {
    timer->lastTime = currentTime;
    return true;
  }
  return false;
}

bool isPositionEmpty(dynamicEntityArray *players, int tileX, int tileY)
{
  for (int i = 0; i < players->count; i++)
  {
    Entity *player = &players->data[i];

    // Create the bounds
    Rectangle playerBounds = { player->position.x - TILE_SIZE / 2, player->position.y - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE };
    Rectangle mouseBounds = { tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE };

    // Check if the bullet's bounding box overlaps with the enemy's bounding box
    if (CheckCollisionRecs(playerBounds, mouseBounds))
    {
      return false;
    }
  }

  return true;
}

// Main
//--------------------------------------------------------------------------------------
int main()
{
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1200;
  const int screenHeight = 800;

  InitWindow(screenWidth, screenHeight, "My window");
  
  level_t level;
  DeserializeLevel(&level);
  
  // Creating a state for the game
  State state;
  state.economy = 100;
  state.wave = 1;
  state.killedEnemyCount = 0;
  state.shootingSpeed = 1;
  state.enemySpawnSpeed = 4;
  state.players = initEntityArray();
  state.enemies = initEntityArray();
  state.bullets = initEntityArray();  

  // Loading all of the resources
  Texture2D tileset = LoadTexture("resources/tileset.png");
  tileset.width = 880;
  tileset.height = 880;

  TimeInterval enemySpawnInterval = InitTimeInterval(state.enemySpawnSpeed);
  
  int spawnedEnemies = 0;
  //--------------------------------------------------------------------------------------

  // Game Loop
  //----------------------------------------------------------------------------------
  while (!WindowShouldClose())
  {
    // Input
    //----------------------------------------------------------------------------------
    if (IsMouseButtonPressed(0)) // If user left click, create a player there
    {
      int mouseX = GetMouseX();
      int mouseY = GetMouseY();

      // Calculate the tile coordinates based on mouse position
      int tileX = mouseX / TILE_SIZE;
      int tileY = mouseY / TILE_SIZE;

      // Check if the calculated tile coordinates are within the valid range
      if (tileX >= 0 && tileX < COLS && tileY >= 0 && tileY < ROWS)
      {
        if (isPositionEmpty(&state.players, tileX, tileY))
        {
          if (can_place_on_tile[level.tiles[tileY][tileX]])
          {
            if (state.economy >= 100)
            {
              // Take 100 from the user's bank for a player
              state.economy -= 100;

              // Create a new player
              Entity player;
              player.position = (v2f){tileX * TILE_SIZE + TILE_SIZE / 2, tileY * TILE_SIZE + TILE_SIZE / 2};
              player.hp = 100;
              player.interval = InitTimeInterval(state.shootingSpeed);

              // Push to the dynamic array
              push(&state.players, player);
            }
          }
        }
      }
    }
    //----------------------------------------------------------------------------------

    // Update
    //----------------------------------------------------------------------------------
    if (state.killedEnemyCount == 20)
    {
      state.wave += 1;
      break;
    }

    // Enemy spawn loop
    if (CheckTimeInterval(&enemySpawnInterval)) {
      if (spawnedEnemies < 1)
      {
        // Createing a enemy
        Entity enemy;
        enemy.position = (v2f){0 - TILE_SIZE, level.startingTileY * TILE_SIZE};
        enemy.direction = (v2f){1, 0};
        enemy.speed = 100.0f;
        enemy.hp = 100;
        push(&state.enemies, enemy);

        spawnedEnemies += 1;
      }
    }

    // Player loop
    for (int i = 0; i < state.players.count; i++)
    {
      Entity *player = &state.players.data[i];
      v2f playerCenter = {player->position.x + TILE_SIZE / 2, player->position.y + TILE_SIZE / 2};

      if (CheckTimeInterval(&player->interval))
      {
        if (state.enemies.count != 0)
        {
          dynamicEntityArray enemies_in_radius = initEntityArray();

          for (int j = 0; j < state.enemies.count; j++)
          {
            Entity enemy = state.enemies.data[j];
            Rectangle enemyBounds = { enemy.position.x, enemy.position.y, TILE_SIZE, TILE_SIZE };

            // If enemy is inside player radius
            if (CheckCollisionCircleRec(playerCenter, 300, enemyBounds))
            {
              push(&enemies_in_radius, enemy);
            }
          }
          
          // Find the closest enemy and get direction from player to that enemy
          float closestEnemyDistance = FLT_MAX;
          v2f closestEnemyDirection = {0, 0};
          // For the lowest enemy
          float lowestEnemyHp = FLT_MAX;
          v2f lowestEnemyDirection = {0, 0};

          // Loop through the enemies
          for (int j = 0; j < enemies_in_radius.count; j++)
          {
            Entity *enemy = &enemies_in_radius.data[j];
            v2f enemyCenter = {enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2};

            // Calculate the distance between the player and the enemy
            float distance = Vector2Distance(playerCenter, enemyCenter);

            // Check if this enemy is lower than the previously lowest enemy
            if (enemy->hp < lowestEnemyHp)
            {
              lowestEnemyHp = enemy->hp;
              lowestEnemyDirection = Vector2Normalize(Vector2Subtract(enemyCenter, playerCenter)); // Calculate the direction vector from the player to the enemy
            }

            // Check if this enemy is closer to the player than the previously closest enemy
            if (distance < closestEnemyDistance)
            {
              closestEnemyDistance = distance;
              closestEnemyDirection = Vector2Normalize(Vector2Subtract(enemyCenter, playerCenter)); // Calculate the direction vector from the player to the enemy
            }

          }

          if (enemies_in_radius.count > 0) {
            float rotationDecimal = (float)(atan2(lowestEnemyDirection.y, lowestEnemyDirection.x) / (2 * PI));
            float rotation = rotationDecimal * 360;
            state.players.data[i].rotation = rotation + 90;

            // Set the direction and speed for the bullet
            Entity bullet;
            bullet.position = (v2f){player->position.x, player->position.y};
            bullet.direction = lowestEnemyDirection;
            bullet.speed = 5000.0f;
            bullet.rotation = rotation;

            // Push to the dynamic array
            push(&state.bullets, bullet);
          }
        }
      }
    }

    // Enemy loop
    for (int i = 0; i < state.enemies.count; i++)
    {
      Entity *enemy = &state.enemies.data[i];

      // Calculate the distance to move based on the elapsed time
      float deltaTime = GetFrameTime();
      float distanceToMove = enemy->speed * deltaTime;
      
      // Update enamy position based on direction and speed
      enemy->position.x += enemy->direction.x * distanceToMove;
      enemy->position.y += enemy->direction.y * distanceToMove;

      int tileX = (int)enemy->position.x / TILE_SIZE;
      int tileY = (int)enemy->position.y / TILE_SIZE;

      if ((int)enemy->position.y % TILE_SIZE == 0) {
        direction_e direction = level.course[tileY][tileX];
        enemy->direction = directions[direction];
      }

      // Check if the enemy is out of bounds and remove it
      if (enemy->position.x >= screenWidth)
      {
        printf("You lost\n");
        remove_at(&state.enemies, i);
        break;
      }
    }

    // Bullet loop
    for (int i = 0; i < state.bullets.count; i++)
    {
      Entity *bullet = &state.bullets.data[i];

      float deltaTime = GetFrameTime();
      float distanceToMove = bullet->speed * deltaTime;
      
      // Update bullet position based on direction and speed
      bullet->position.x += bullet->direction.x * distanceToMove;
      bullet->position.y += bullet->direction.y * distanceToMove;

      // Iterate through enemies to check for hits
      for (int j = 0; j < state.enemies.count; j++)
      {
        Entity *enemy = &state.enemies.data[j];

        // Calculate the bounding boxes of the bullet and enemy
        Rectangle bulletBounds = { bullet->position.x, bullet->position.y, TILE_SIZE, TILE_SIZE };
        Rectangle enemyBounds = { enemy->position.x, enemy->position.y, TILE_SIZE, TILE_SIZE };

        // Check if the bullet's bounding box overlaps with the enemy's bounding box
        if (CheckCollisionRecs(bulletBounds, enemyBounds))
        {
          // Lower enamy hp
          enemy->hp -= 20;

          // If enemy gets killed
          if (enemy->hp <= 0)
          {
            // Remove the enemy
            remove_at(&state.enemies, j);

            // Add one to the killed enemy counter
            state.killedEnemyCount += 1;

            // Add to bank
            state.economy += 25;
          }
          // Remove the bullet
          remove_at(&state.bullets, i);
        }
      }

      // Check if the bullet is out of bounds and remove it
      if (bullet->position.x >= screenWidth ||
        bullet->position.x <= 0 ||
        bullet->position.y >= screenHeight ||
        bullet->position.y <= 0)
      {
        remove_at(&state.bullets, i);
      }
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

      ClearBackground(RAYWHITE);

      // Draw the tilemap
      // Draw the tilemap
      for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
          int tile = level.tiles[y][x];

          Rectangle destRect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
          Rectangle sourceRect = {(tile % (tileset.width / TILE_SIZE)) * TILE_SIZE, (tile / (tileset.width / TILE_SIZE)) * TILE_SIZE, TILE_SIZE, TILE_SIZE };

          DrawTexturePro(tileset, sourceRect, destRect, Vector2Zero(), 0.0f, WHITE);
        }
      }


      DrawFPS(20, screenHeight - 20);

      DrawText(TextFormat("Bank: %d", state.economy), 10, 10, 28, WHITE);
      DrawText(TextFormat("Killed enemies: %d", state.killedEnemyCount), screenWidth - 240, 10, 28, WHITE);
      DrawText(TextFormat("Wave: %d", state.wave), screenWidth - 125, 50, 28, WHITE);

      for (int i = 0; i < state.players.count; i++)
      {
        Entity *player = &state.players.data[i];

        DrawTexturePro(tileset,
          (Rectangle){640, 0, TILE_SIZE, TILE_SIZE},
          (Rectangle){player->position.x, player->position.y, TILE_SIZE, TILE_SIZE},
          (v2f){TILE_SIZE / 2, TILE_SIZE / 2},
          player->rotation,
          WHITE);

        DrawCircleLines(player->position.x,
          player->position.y,
          300,
          BLACK);

        // tmp rectangle around the players
        Rectangle playerBounds = { player->position.x - TILE_SIZE / 2, player->position.y - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE };
        DrawRectangleLinesEx(playerBounds, 1, BLACK);
      }
      for (int i = 0; i < state.enemies.count; i++)
      {
        Entity *enemy = &state.enemies.data[i];

        const char* hp_text = TextFormat("%d", enemy->hp);
        DrawText(hp_text, (enemy->position.x + (TILE_SIZE / 2) - (MeasureText(hp_text, 20) / 2)), (enemy->position.y - 20), 20, WHITE);
        
        DrawTexturePro(tileset,
          (Rectangle){720, 0, TILE_SIZE, TILE_SIZE},
          (Rectangle){enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (v2f){TILE_SIZE / 2, TILE_SIZE / 2},
          0,
          WHITE);

        // tmp rectangle around the enemys
        Rectangle playerBounds = { enemy->position.x, enemy->position.y, TILE_SIZE, TILE_SIZE };
        DrawRectangleLinesEx(playerBounds, 1, BLACK);
      }
      for (int i = 0; i < state.bullets.count; i++)
      {
        Entity *bullet = &state.bullets.data[i];

        DrawTexturePro(tileset,
          (Rectangle){800, 0, TILE_SIZE, TILE_SIZE},
          (Rectangle){bullet->position.x, bullet->position.y, TILE_SIZE, TILE_SIZE},
          (v2f){TILE_SIZE / 2, TILE_SIZE / 2},
          bullet->rotation,
          WHITE);
      }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  UnloadTexture(tileset);

  free(state.players.data);
  free(state.enemies.data);
  free(state.bullets.data);

  CloseWindow();
  return 0;
}