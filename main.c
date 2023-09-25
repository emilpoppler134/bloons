#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#include "raylib.h"
#include "raymath.h"

#include "tiles.h"
#include "entity.h"
#include "interval.h"

#define TILE_SIZE 80
#define COLS 15
#define ROWS 10

// Structs
//--------------------------------------------------------------------------------------
typedef enum state_e
{
  STATE_BUY,
  STATE_PLACE,
  STATE_REMOVE,
  STATE_NUM
} state_e;

typedef struct level_t
{ 
  tile_type_e tiles[ROWS][COLS];
  direction_e path[ROWS][COLS];
  Vector2 enemy_starting_tile;
  Vector2 enemy_starting_direction;
} level_t;

typedef struct game_t
{
  int bank;
  int hp;
  int enemy_type;
  int killed_enemy_count;
  int enemy_spawn_speed;
  dynamic_entity_array players;
  dynamic_entity_array enemies;
  dynamic_entity_array bullets;
} game_t;


// Methods
//--------------------------------------------------------------------------------------
void deserialize_level(level_t *level)
{
  FILE *file = fopen("./resources/levels/level_1", "rb");
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

bool is_position_empty(dynamic_entity_array *players, int tile_x, int tile_y)
{
  for (int i = 0; i < players->count; i++)
  {
    entity_t *player = &players->data[i];

    // Create the bounds
    Rectangle player_bounds = {player->position.x, player->position.y, TILE_SIZE, TILE_SIZE};
    Rectangle mouse_bounds = {tile_x * TILE_SIZE, tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

    // Check if the players's bounding box overlaps with the mouse's bounding box
    if (CheckCollisionRecs(player_bounds, mouse_bounds))
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
  const int screen_width = 1200;
  const int screen_height = 800;

  InitWindow(screen_width, screen_height, "Bloons TD");

  state_e state = STATE_BUY;

  level_t level;
  deserialize_level(&level); // deserialize level

  // Setting the game settings
  game_t game;
  game.bank = 100;
  game.hp = 100;
  game.killed_enemy_count = 0;
  game.enemy_spawn_speed = 3;

  // Init the arrays for all entities
  game.players = init_entity_array();
  game.enemies = init_entity_array();
  game.bullets = init_entity_array();

  // Loading the tileset
  Texture2D tileset = LoadTexture("./resources/tileset.png");
  tileset.width = TILE_SIZE * 10;
  tileset.height = TILE_SIZE * 10;

  // Loading the players texture
  Texture2D entity_texture = LoadTexture("./resources/entities.png");
  entity_texture.width = TILE_SIZE * 15;
  entity_texture.height = TILE_SIZE * 15;

  dynamic_entity_array player_types = init_entity_array();
  deserialize_entities(&player_types, "players");

  dynamic_entity_array enemy_types = init_entity_array();;
  deserialize_entities(&enemy_types, "enemies");

  entity_t placeing_player = init_entity();

  time_interval_t enemy_spawn_interval = init_time_interval(game.enemy_spawn_speed);
  int spawned_enemies = 0;
  //--------------------------------------------------------------------------------------


  while (!WindowShouldClose())
  {
    // Input
    //----------------------------------------------------------------------------------
    if (IsMouseButtonPressed(0))
    {
      int mouse_x = GetMouseX();
      int mouse_y = GetMouseY();

      // Calculate the tile coordinates based on mouse position
      int tile_x = mouse_x / TILE_SIZE;
      int tile_y = mouse_y / TILE_SIZE;

      // Check if the calculated tile coordinates are within the valid range
      if (tile_x >= 0 && tile_x < COLS && tile_y >= 0 && tile_y < ROWS)
      {
        switch (state)
        {
          case STATE_BUY:
          {
            for (int i = 0; i < player_types.count; i++)
            {
              entity_t player = player_types.data[i];
              Rectangle button_bounds = {160 + i * TILE_SIZE, screen_height - TILE_SIZE, TILE_SIZE, TILE_SIZE};

              if (CheckCollisionPointRec(GetMousePosition(), button_bounds))
              {
                if (game.bank >= player.cost)
                {
                  placeing_player = player;
                  game.bank -= player.cost;
                  state = STATE_PLACE;
                }
              }
            }
          } break;

          case STATE_PLACE:
          {
            if (is_position_empty(&game.players, tile_x, tile_y))
            {
              if (can_place_on_tile[level.tiles[tile_y][tile_x]])
              {
                // Create a new player
                entity_t player = init_entity();
                player.position = (Vector2){tile_x * TILE_SIZE, tile_y * TILE_SIZE};
                player.type = placeing_player.type;
                player.radius = placeing_player.radius;
                player.interval = placeing_player.interval;
                push(&game.players, player);

                state = STATE_BUY;
              }
            }
          } break;

          case STATE_REMOVE:
          {
            // Remove
          } break;

          default:
          break;
        }
      }
    }
    //----------------------------------------------------------------------------------


    // Update
    //----------------------------------------------------------------------------------

    // Enemy spawn loop
    if (check_time_interval(&enemy_spawn_interval))
    {
      // Create an enemy
      entity_t enemy = init_entity();
      enemy.position = (Vector2){level.enemy_starting_tile.x * TILE_SIZE, level.enemy_starting_tile.y * TILE_SIZE};
      enemy.direction = level.enemy_starting_direction;
      enemy.speed = enemy_types.data[game.enemy_type].speed;
      enemy.type = enemy_types.data[game.enemy_type].type;
      push(&game.enemies, enemy);

      spawned_enemies++;
      
      if (spawned_enemies % 20 == 0)
      {
        game.enemy_type++;
      }
    }

    // Player loop
    for (int i = 0; i < game.players.count; i++)
    {
      entity_t *player = &game.players.data[i];
      Vector2 player_center = {player->position.x + TILE_SIZE / 2, player->position.y + TILE_SIZE / 2};
      
      // Player shooting loop
      if (check_time_interval(&player->interval))
      {
        // Create an array with all of the enemies that is in the players radius
        dynamic_entity_array enemies_in_radius = init_entity_array();

        for (int j = 0; j < game.enemies.count; j++)
        {
          entity_t enemy = game.enemies.data[j];
          Rectangle enemy_bounds = {enemy.position.x, enemy.position.y, TILE_SIZE, TILE_SIZE};

          // If enemy is inside player radius
          if (CheckCollisionCircleRec(player_center, player->radius, enemy_bounds))
          {
            push(&enemies_in_radius, enemy); // Push to the dynamic array
          }
        }
        
        float lowest_enemy_type = FLT_MAX;
        Vector2 lowest_enemy_direction = {0, 0};

        for (int j = 0; j < enemies_in_radius.count; j++)
        {
          entity_t *enemy = &enemies_in_radius.data[j];
          Vector2 enemy_center = {enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2};

          // Calculate the distance between the player and the enemy
          float distance_to_enemy = Vector2Distance(player_center, enemy_center);

          // Check if this enemy is lower than the previously lowest enemy
          if (enemy->type < lowest_enemy_type)
          {
            lowest_enemy_type = enemy->type;
             // Calculate the direction vector from the player to the enemy
            lowest_enemy_direction = Vector2Normalize(Vector2Subtract(enemy_center, player_center));
          }
        }

        if (enemies_in_radius.count > 0) {
          float rotation_decimal = (float)(atan2(lowest_enemy_direction.y, lowest_enemy_direction.x) / (2 * PI));
          float rotation = rotation_decimal * 360;
          player->rotation = rotation + 90;

          // Create a bullet and set the direction and speed
          entity_t bullet = init_entity();
          bullet.position = (Vector2){player->position.x, player->position.y};
          bullet.direction = lowest_enemy_direction;
          bullet.speed = 2000.0f;
          bullet.rotation = rotation;
          bullet.type = player->type;
          push(&game.bullets, bullet);
        }
      }
    }

    // Enemy loop
    for (int i = 0; i < game.enemies.count; i++)
    {
      entity_t *enemy = &game.enemies.data[i];

      // Calculate the distance to move based on the elapsed time
      float delta_time = GetFrameTime();
      float distance_to_move = enemy->speed * delta_time;
      
      // Update enamy position based on direction and speed
      enemy->position.x += enemy->direction.x * distance_to_move;
      enemy->position.y += enemy->direction.y * distance_to_move;

      int tile_x = (int)enemy->position.x / TILE_SIZE;
      int tile_y = (int)enemy->position.y / TILE_SIZE;

      if ((int)enemy->position.y % TILE_SIZE == 0) {
        direction_e direction = level.path[tile_y][tile_x];
        enemy->direction = directions[direction];
      }

      // Check if the enemy is out of bounds and remove it
      if (enemy->position.x >= screen_width)
      {
        game.hp -= 10;
        remove_at(&game.enemies, i);
        break;
      }
    }

    // Bullet loop
    for (int i = 0; i < game.bullets.count; i++)
    {
      entity_t *bullet = &game.bullets.data[i];

      float delta_time = GetFrameTime();
      float distance_to_move = bullet->speed * delta_time;
      
      // Update bullet position based on direction and speed
      bullet->position.x += bullet->direction.x * distance_to_move;
      bullet->position.y += bullet->direction.y * distance_to_move;

      // Iterate through enemies to check for hits
      for (int j = 0; j < game.enemies.count; j++)
      {
        entity_t *enemy = &game.enemies.data[j];

        // Calculate the bounding boxes of the bullet and enemy
        Rectangle bullet_bounds = {bullet->position.x, bullet->position.y, TILE_SIZE, TILE_SIZE};
        Rectangle enemy_bounds = {enemy->position.x, enemy->position.y, TILE_SIZE, TILE_SIZE};

        // Check if the bullet's bounding box overlaps with the enemy's bounding box
        if (CheckCollisionRecs(bullet_bounds, enemy_bounds))
        {
          Vector2 enemy_center = {enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2};
          Vector2 bullet_center = {bullet->position.x + TILE_SIZE / 2, bullet->position.y + TILE_SIZE / 2};

          if (CheckCollisionCircles(enemy_center, TILE_SIZE / 4, bullet_center, TILE_SIZE / 4))
          {
            // Lower enamy hp
            enemy->type--;
            enemy->speed = enemy_types.data[enemy->type].speed;

            // If enemy gets killed
            if (enemy->type < 0)
            {
              game.killed_enemy_count += 1; // Add one to the killed enemy counter
              game.bank += 25; // Add to bank
              remove_at(&game.enemies, j); // Remove the enemy
            }

            // Remove the bullet
            remove_at(&game.bullets, i);
          }
        }
      }

      // Check if the bullet is out of bounds and remove it
      if (bullet->position.x >= screen_width ||
        bullet->position.x <= 0 - TILE_SIZE ||
        bullet->position.y >= screen_height ||
        bullet->position.y <= 0)
      {
        remove_at(&game.bullets, i);
      }
    }
    //----------------------------------------------------------------------------------


    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
      ClearBackground(RAYWHITE);
      
      // Draw the tilemap
      for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
          int tile = level.tiles[y][x];

          Rectangle dest_rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
          Rectangle source_rect = {(tile % (tileset.width / TILE_SIZE)) * TILE_SIZE, (tile / (tileset.width / TILE_SIZE)) * TILE_SIZE, TILE_SIZE, TILE_SIZE };

          DrawTexturePro(tileset, source_rect, dest_rect, Vector2Zero(), 0.0f, WHITE);
        }
      }

      DrawFPS(10, 10);

      // Draw the players
      for (int i = 0; i < game.players.count; i++)
      {
        entity_t *player = &game.players.data[i];

        DrawTexturePro(entity_texture,
          (Rectangle){80, player->type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
          (Rectangle){player->position.x + TILE_SIZE / 2, player->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
          player->rotation,
          WHITE);

        DrawCircle(player->position.x + TILE_SIZE / 2,
          player->position.y + TILE_SIZE / 2,
          player->radius,
          (Color){255, 255, 255, 10});
      }
      
      // Draw the enemies
      for (int i = 0; i < game.enemies.count; i++)
      {
        entity_t *enemy = &game.enemies.data[i];
        
        DrawTexturePro(entity_texture,
          (Rectangle){TILE_SIZE * 5, enemy->type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
          (Rectangle){enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
          0,
          WHITE);
      }

      // Draw the bullets
      for (int i = 0; i < game.bullets.count; i++)
      {
        entity_t *bullet = &game.bullets.data[i];

        DrawTexturePro(entity_texture,
          (Rectangle){TILE_SIZE * 10, bullet->type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
          (Rectangle){bullet->position.x + TILE_SIZE / 2, bullet->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
          bullet->rotation,
          WHITE);
      }

      if (state == STATE_BUY)
      {
        DrawText(TextFormat("Kills: %d", game.killed_enemy_count), screen_width - 120, 10, 28, WHITE);

        DrawRectangle(0, screen_height - TILE_SIZE, screen_width, TILE_SIZE, (Color){0, 0, 0, 100});
        DrawText(TextFormat("Bank: %d", game.bank), 15, screen_height - 65, 22, WHITE);
        DrawText(TextFormat("Hp: %d", game.hp), 15, screen_height - 35, 22, WHITE);

        for (int i = 0; i < player_types.count; i++)
        {
          entity_t *player = &player_types.data[i];

          DrawTexturePro(entity_texture,
            (Rectangle){0, i * TILE_SIZE, TILE_SIZE, TILE_SIZE},
            (Rectangle){160 + i * TILE_SIZE + TILE_SIZE / 2, screen_height - TILE_SIZE / 2, TILE_SIZE - 16, TILE_SIZE - 16},
            (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
            0,
            game.bank >= player->cost ? WHITE : (Color){255, 255, 255, 100});

          const char* cost_text = TextFormat("%d", player->cost);
          DrawText(cost_text, (160 + i * TILE_SIZE + (TILE_SIZE / 2) - (MeasureText(cost_text, 20) / 2)), (screen_height - 20), 20, WHITE);
        }
      }

      if (state == STATE_PLACE)
      {
        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // Calculate the tile coordinates based on mouse position
        int tile_x = mouse_x / TILE_SIZE;
        int tile_y = mouse_y / TILE_SIZE;

        // Check if the calculated tile coordinates are within the valid range
        if (tile_x >= 0 && tile_x < COLS && tile_y >= 0 && tile_y < ROWS)
        {
          bool can_place = is_position_empty(&game.players, tile_x, tile_y) && can_place_on_tile[level.tiles[tile_y][tile_x]];

          DrawTexturePro(entity_texture,
            (Rectangle){80, placeing_player.type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
            (Rectangle){tile_x * TILE_SIZE + TILE_SIZE / 2, tile_y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
            (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
            0,
            can_place ? (Color){255, 255, 255, 200} : (Color){255, 255, 255, 75});

          DrawCircle(tile_x * TILE_SIZE + TILE_SIZE / 2,
            tile_y * TILE_SIZE + TILE_SIZE / 2,
            placeing_player.radius,
            (Color){255, 255, 255, 25});
        }
      }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  UnloadTexture(tileset);

  free(game.players.data);
  free(game.enemies.data);
  free(game.bullets.data);

  CloseWindow();
  return 0;
}