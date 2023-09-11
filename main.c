#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <float.h>

#include "raylib.h"
#include "raymath.h"

typedef Vector2 v2f;


// Structs for the entity, state and the dynamic array
//--------------------------------------------------------------------------------------
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

typedef struct TurningPoint
{
  v2f direction;
  v2f point;
} TurningPoint;

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

bool isPositionEmpty(dynamicEntityArray players, float width, float height, float mouseX, float mouseY)
{
  for (int i = 0; i < players.count; i++)
  {
    Entity *player = &players.data[i];

    // Create the bounds
    Rectangle playerBounds = { player->position.x - width / 2, player->position.y - height / 2, width, height };
    Rectangle mouseBounds = { mouseX - width / 2, mouseY - height / 2, width, height };

    // Check if the bullet's bounding box overlaps with the enemy's bounding box
    if (CheckCollisionRecs(playerBounds, mouseBounds))
    {
      return false;
    }
  }

  return true;
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


// Main
//--------------------------------------------------------------------------------------
int main()
{
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1200;
  const int screenHeight = 800;

  InitWindow(screenWidth, screenHeight, "My window");

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
  Texture2D background = LoadTexture("resources/BACKGROUND.png");
  background.width = screenWidth;
  background.height = screenHeight;

  Texture2D bulletImage = LoadTexture("resources/BULLET.png");
  bulletImage.width = 77;
  bulletImage.height = 50;

  Texture2D enemyImage = LoadTexture("resources/ENEMY.png");
  enemyImage.width = 100;
  enemyImage.height = 100;

  Texture2D playerImage = LoadTexture("resources/PLAYER.png");
  playerImage.width = 100;
  playerImage.height = 100;

  clock_t startTime = GetTime();

  int spawnedEnemies = 0;

  TimeInterval enemySpawnInterval = InitTimeInterval(state.enemySpawnSpeed);

  TurningPoint turning_points[6] = {
    {
      .direction = (v2f){0, -1},
      .point = (v2f){200, 440}
    },
    {
      .direction = (v2f){1, 0},
      .point = (v2f){200, 200}
    },
    {
      .direction = (v2f){0, 1},
      .point = (v2f){440, 200}
    },
    {
      .direction = (v2f){1, 0},
      .point = (v2f){440, 520}
    },
    {
      .direction = (v2f){0, -1},
      .point = (v2f){760, 520}
    },
    {
      .direction = (v2f){1, 0},
      .point = (v2f){760, 370}
    }
  };

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

      if (state.economy >= 100) {
        if (isPositionEmpty(state.players, playerImage.width, playerImage.height, mouseX, mouseY))
        {
          // Take 100 from the users bank for a player
          state.economy -= 100;

          // Create a new player
          Entity player;
          player.position = (v2f){mouseX, mouseY};
          player.hp = 100;
          player.interval = InitTimeInterval(state.shootingSpeed);

          // Push to the dynamic array
          push(&state.players, player);
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
      if (spawnedEnemies < 20)
      {
        // Createing a enemy
        Entity enemy;
        enemy.position = (v2f){0 - enemyImage.width, 440 - enemyImage.height / 2};
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
      v2f playerCenter = {player->position.x + playerImage.width / 2, player->position.y + playerImage.height / 2};

      if (CheckTimeInterval(&player->interval))
      {
        if (state.enemies.count != 0)
        {
          // Find the closest enemy and get direction from player to that enemy
          float closestEnemyDistance = FLT_MAX;
          v2f closestEnemyDirection = {0, 0};
          // For the lowest enemy
          float lowestEnemyHp = FLT_MAX;
          v2f lowestEnemyDirection = {0, 0};

          // Loop through the enemies
          for (int j = 0; j < state.enemies.count; j++)
          {
            Entity enemy = state.enemies.data[j];
            v2f enemyCenter = {enemy.position.x + enemyImage.width / 2, enemy.position.y + enemyImage.height / 2};
            // Calculate the distance between the player and the enemy
            float distance = Vector2Distance(playerCenter, enemyCenter);
            
            // Check if this enemy is lower than the previously lowest enemy
            if (enemy.hp < lowestEnemyHp)
            {
              lowestEnemyHp = enemy.hp;
              lowestEnemyDirection = Vector2Normalize(Vector2Subtract(enemyCenter, playerCenter)); // Calculate the direction vector from the player to the enemy
            }
            
            // Check if this enemy is closer to the player than the previously closest enemy
            if (distance < closestEnemyDistance)
            {
              closestEnemyDistance = distance;
              closestEnemyDirection = Vector2Normalize(Vector2Subtract(enemyCenter, playerCenter)); // Calculate the direction vector from the player to the enemy
            }
          }

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

      for (int j = 0; j < sizeof(turning_points); j++)
      {
        TurningPoint turningPoint = turning_points[j];

        if (floor(enemy->position.x + enemyImage.width / 2) == turningPoint.point.x && floor(enemy->position.y + enemyImage.height / 2) == turningPoint.point.y)
        {
          enemy->direction = turningPoint.direction;
        }
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
        Rectangle bulletBounds = { bullet->position.x, bullet->position.y, bulletImage.width, bulletImage.height };
        Rectangle enemyBounds = { enemy->position.x, enemy->position.y, enemyImage.width, enemyImage.height };

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
      DrawTexture(background, 0, 0, WHITE);

      DrawFPS(20, screenHeight - 20);

      DrawText(TextFormat("Bank: %d", state.economy), 10, 10, 28, WHITE);
      DrawText(TextFormat("Killed enemies: %d", state.killedEnemyCount), screenWidth - 240, 10, 28, WHITE);
      DrawText(TextFormat("Wave: %d", state.wave), screenWidth - 125, 50, 28, WHITE);

      for (int i = 0; i < state.players.count; i++)
      {
        Entity *player = &state.players.data[i];

        DrawTexturePro(playerImage,
          (Rectangle){0, 0, playerImage.width, playerImage.height},
          (Rectangle){player->position.x, player->position.y, playerImage.width, playerImage.height},
          (v2f){playerImage.width / 2, playerImage.height / 2},
          player->rotation,
          WHITE);

        // tmp rectangle around the players
        Rectangle playerBounds = { player->position.x - 100 / 2, player->position.y - 100 / 2, 100, 100 };
        DrawRectangleLinesEx(playerBounds, 1, BLACK);
      }
      for (int i = 0; i < state.enemies.count; i++)
      {
        Entity *enemy = &state.enemies.data[i];

        const char* hp_text = TextFormat("%d", enemy->hp);
        DrawText(hp_text, (enemy->position.x + (enemyImage.width / 2) - (MeasureText(hp_text, 20) / 2)), (enemy->position.y - 20), 20, WHITE);
        
        DrawTexturePro(enemyImage,
          (Rectangle){0, 0, enemyImage.width, enemyImage.height},
          (Rectangle){enemy->position.x + enemyImage.width / 2, enemy->position.y + enemyImage.height / 2, enemyImage.width, enemyImage.height},
          (v2f){enemyImage.width / 2, enemyImage.height / 2},
          0,
          WHITE);

        // tmp rectangle around the enemys
        Rectangle playerBounds = { enemy->position.x, enemy->position.y, 100, 100 };
        DrawRectangleLinesEx(playerBounds, 1, BLACK);
      }
      for (int i = 0; i < state.bullets.count; i++)
      {
        Entity *bullet = &state.bullets.data[i];

        DrawTexturePro(bulletImage,
          (Rectangle){0, 0, bulletImage.width, bulletImage.height},
          (Rectangle){bullet->position.x, bullet->position.y, bulletImage.width, bulletImage.height},
          (v2f){bulletImage.width / 2, bulletImage.height / 2},
          bullet->rotation,
          WHITE);
      }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  UnloadTexture(background);
  UnloadTexture(bulletImage);
  UnloadTexture(enemyImage);
  UnloadTexture(playerImage);

  free(state.players.data);
  free(state.enemies.data);
  free(state.bullets.data);

  CloseWindow();
  return 0;
}