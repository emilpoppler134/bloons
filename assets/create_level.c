#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#define ROWS 10
#define COLS 15
#define LEVEL_FILE_PATH "./resources/level.texture"

typedef struct TurningPoint
{
	Vector2 direction;
	Vector2 point;
} TurningPoint;

typedef struct Level
{
	int tiles[ROWS][COLS];
	TurningPoint turning_points[6];
} Level;

void SerializeLevel(const char *path, Level *level)
{
	FILE *file = fopen(path, "wb");
	if (!file)
	{
		perror("Failed to open level file");
		exit(1);
	}

	if (fwrite(level, sizeof(Level), 1, file) != 1)
	{
		perror("Failed to write level data");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

void InitializeLevel(Level *level)
{
	int tiles[ROWS][COLS] = {
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 7, 11, 16, 16, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 15, 0, 0, 15, 0, 0, 0, 7, 0, 0, 0, 0, 0},
    {0, 0, 15, 0, 0, 15, 0, 0, 0, 11, 16, 16, 16, 16, 16},
    {16, 16, 14, 0, 0, 15, 0, 0, 0, 15, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 13, 16, 16, 16, 14, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	};

	TurningPoint turning_points[6] = {
		{
      .direction = (Vector2){0, -1},
      .point = (Vector2){200, 440}
    },
    {
      .direction = (Vector2){1, 0},
      .point = (Vector2){200, 200}
    },
    {
      .direction = (Vector2){0, 1},
      .point = (Vector2){440, 200}
    },
    {
      .direction = (Vector2){1, 0},
      .point = (Vector2){440, 520}
    },
    {
      .direction = (Vector2){0, -1},
      .point = (Vector2){760, 520}
    },
    {
      .direction = (Vector2){1, 0},
      .point = (Vector2){760, 370}
    }
	};

	memcpy(level->tiles, tiles, sizeof(int) * ROWS * COLS);
	memcpy(level->turning_points, turning_points, sizeof(TurningPoint) * 6);
}

int main()
{
	Level level;
	InitializeLevel(&level);
	SerializeLevel(LEVEL_FILE_PATH, &level);

	return 0;
}