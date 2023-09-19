#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raylib.h"

#define ROWS 10
#define COLS 15
#define LEVEL_FILE_PATH "./resources/level.ep"

typedef enum direction_e {
  DIR_NONE,
  DIR_UP,
  DIR_RIGHT,
  DIR_DOWN,
  DOR_LEFT
} direction_e;

typedef struct Level
{
	int tiles[ROWS][COLS];
  direction_e directions[ROWS][COLS];
  int startingTileY;
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
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
		{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  7,  11, 16, 16, 12, 0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  0,  15, 0,  0,  15, 0,  0,  0,  7,  0,  0,  0,  0,  0},
    {0,  0,  15, 0,  0,  15, 0,  0,  0,  11, 16, 16, 16, 16, 16},
    {16, 16, 14, 0,  0,  15, 0,  0,  0,  15, 0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  13, 16, 16, 16, 14, 0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	};

  direction_e directions[ROWS][COLS] = {
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_RIGHT,  DIR_RIGHT, DIR_RIGHT, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_UP, DIR_NONE, DIR_NONE, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_UP, DIR_NONE, DIR_NONE, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_RIGHT,  DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT},
    {DIR_RIGHT, DIR_RIGHT, DIR_UP, DIR_NONE, DIR_NONE, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_UP, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_UP, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE}
  };

  level->startingTileY = 5;

	memcpy(level->tiles, tiles, sizeof(int) * ROWS * COLS);
	memcpy(level->directions, directions, sizeof(direction_e) * ROWS * COLS);
}

int main()
{
	Level level;
	InitializeLevel(&level);
	SerializeLevel(LEVEL_FILE_PATH, &level);

	return 0;
}